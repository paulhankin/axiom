// std/internal/runtime/host.mjs - the Axiom host ABI (import module "axiom") for
// WebAssembly under node, plus a runner:
//
//   node std/internal/runtime/host.mjs program.wasm [export] [-- args...]
//
// calls `export` (default: main, else run). If it returns a number, that is
// the process exit code. A trap exits with status 134. Arguments after `--`
// are the program's (its argument 0 is program.wasm). Its files are node's,
// or with --root=dir node's under dir, which the program sees as `/`, or
// with --files=memory a file system in memory (memfs.mjs). A threaded program
// (one that imports a shared memory) runs all its threads on workers, its
// main thread too, under a Supervisor on node's main thread, which ends the
// process as soon as any of them asks (ax_exit; spec §12.8.5, "Ending the
// program").
//
// As a library: createHost(opts) returns
//   { imports, instantiate(bytes, extraImports), instantiateModule(module, extraImports),
//     setMemory, close }.
// instantiate() handles modules that import a shared memory (threaded programs).
// opts.exit is the embedder's end of the program (default process.exit):
// it must end every thread and not return. Threads that the program
// starts reach it only through a Supervisor (opts.supervisor, which
// thread_worker.mjs passes on to each): without one (a program run on the
// embedder's own thread, which is then busy), an exit on such a thread ends
// that thread alone.
import fs from 'fs';
import os from 'os';
import path from 'path';
import { MessageChannel, SHARE_ENV, Worker } from 'worker_threads';
import tty from 'tty';

const READ_CHUNK = 1 << 16;

// What ax_thread_exit throws to leave a thread's code (thread_worker.mjs).
export const THREAD_EXIT = Symbol('axiom thread exit');

export const TRAP_NAMES = [
  'trap', 'index out of bounds', 'integer overflow', 'division by zero', 'int_cast out of range',
  'memory access out of bounds', 'length precondition failed', 'invalid union tag or enum value', 'stack exhausted',
  'invalid function pointer', 'misaligned atomic access', 'expectation failed', 'memory error',
  'processor lacks a required extension',
];

export function trapCodeName(n) { return 'T' + String(n).padStart(4, '0'); }

// Describes a trap: the site recorded by the program (spec Appendix B), or
// the engine's own trap mapped to its code.
export function describeTrap(instance, memory, err) {
  const msg = String(err && err.message || err);
  let t = null;
  const info = instance && instance.exports.axiom_trap_info;
  if (info && memory) {
    const dv = new DataView(memory.buffer);
    const base = Number(info());
    const site = Number(dv.getBigUint64(base, true));
    if (site) {
      t = readSite(instance, memory, site);
      t.name = TRAP_NAMES[t.code] || 'trap';
      const kind = Number(dv.getBigUint64(base + 8, true));
      if (kind) t.values = formatValues(kind, dv, base, memory, instance);
      dv.setBigUint64(base, 0n, true);  // consumed
    }
  }
  if (!t && err instanceof WebAssembly.Exception) {
    // (C's longjmp, whose exception no function caught: C17 7.13.2.1p2)
    t = { code: 0, name: TRAP_NAMES[0], engineMessage: 'longjmp to a jmp_buf whose setjmp is no longer active' };
  }
  if (!t) {
    let code = 0;
    if (err instanceof RangeError || /stack/i.test(msg)) code = 8;
    else if (/out of bounds memory|memory access out of bounds/i.test(msg)) code = 5;
    else if (/table|signature|indirect|null function/i.test(msg)) code = 9;
    else if (/divide by zero|division by zero/i.test(msg)) code = 3;
    else if (/integer overflow|unrepresentable/i.test(msg)) code = 2;
    else if (/unaligned/i.test(msg)) code = 10;
    t = { code, name: TRAP_NAMES[code], engineMessage: msg };
  }
  t.codeName = trapCodeName(t.code);
  t.stack = readStack(instance, memory);
  return t;
}

// Site records (see codegen: site_record_ex):
//   {u32 code, file_id, start, end, line, col; u32 file, text, fn, callee; ptr strs}
// the strings NUL-terminated in the unit's string table strs, at those offsets
function reader(instance, memory) {
  const wide = !!(instance && instance.exports.axiom_wasm64);
  const P = wide ? 8 : 4;
  const dv = new DataView(memory.buffer);
  const u = new Uint8Array(memory.buffer);
  const ptr = (a) => (wide ? Number(dv.getBigUint64(a, true)) : dv.getUint32(a, true));
  const cstr = (p) => {
    if (!p || p >= u.length) return '';
    let e = p;
    while (e < u.length && u[e]) e++;
    return Buffer.from(u.subarray(p, e)).toString('utf8');
  };
  return { P, dv, ptr, cstr };
}

export function readSite(instance, memory, addr) {
  const { P, dv, ptr, cstr } = reader(instance, memory);
  return {
    code: dv.getUint32(addr, true), fileId: dv.getUint32(addr + 4, true),
    start: dv.getUint32(addr + 8, true), end: dv.getUint32(addr + 12, true),
    line: dv.getUint32(addr + 16, true), col: dv.getUint32(addr + 20, true),
    ...((strs) => ({
      file: cstr(strs + dv.getUint32(addr + 24, true)), text: cstr(strs + dv.getUint32(addr + 28, true)),
      fn: cstr(strs + dv.getUint32(addr + 32, true)), callee: cstr(strs + dv.getUint32(addr + 36, true)),
    }))(ptr(addr + 40)),
  };
}

export const SS_CAP = 256, ET_CAP = 32;

// The calling thread's shadow stack (spec §16.4): {depth, calls (innermost
// first), errors (oldest first)}, or null if the program has none.
export function readStack(instance, memory) {
  const f = instance && instance.exports.axiom_shadow_stack;
  if (!f || !memory) return null;
  const base = Number(f());
  if (!base) return null;
  const { P, dv, ptr } = reader(instance, memory);
  const depth = dv.getUint32(base, true), nerr = dv.getUint32(base + 4, true);
  const calls = [], errors = [];
  for (let i = depth - 1; i >= 0 && i >= depth - SS_CAP; i--) {
    const a = ptr(base + 8 + (i & (SS_CAP - 1)) * P);
    if (a) calls.push(readSite(instance, memory, a));
  }
  for (let i = Math.max(0, nerr - ET_CAP); i < nerr; i++) {
    const a = ptr(base + 8 + SS_CAP * P + (i & (ET_CAP - 1)) * P);
    if (a) errors.push(readSite(instance, memory, a));
  }
  return { depth, calls, errors };
}

// Frames, innermost first: {fn, file, line, col, fileId, start, end} (the
// location is missing when it is unknown), plus {repeat: n} and
// {omitted: n} markers. `top` is the trap site, if the program trapped.
export function stackFrames(top, st, max = 40) {
  const frames = [];
  if (top && top.file) frames.push({ fn: top.fn, file: top.file, line: top.line, col: top.col, fileId: top.fileId, start: top.start, end: top.end });
  else if (st && st.calls.length) frames.push({ fn: st.calls[0].callee || '<indirect call>' });
  if (!st) return frames;
  let shown = 0;
  for (let i = 0; i < st.calls.length; i++) {
    const c = st.calls[i];
    let j = i;
    while (j + 1 < st.calls.length && st.calls[j + 1].start === c.start && st.calls[j + 1].fileId === c.fileId && st.calls[j + 1].fn === c.fn) j++;
    if (shown >= max) { frames.push({ omitted: st.depth - i }); return frames; }
    frames.push({ fn: c.fn, file: c.file, line: c.line, col: c.col, fileId: c.fileId, start: c.start, end: c.end });
    shown++;
    if (j > i) { frames.push({ repeat: j - i }); i = j; }
  }
  if (st.depth > SS_CAP) frames.push({ omitted: st.depth - SS_CAP });
  return frames;
}

export function formatFrames(frames, indent = '  ') {
  return frames.map((f) => {
    if (f.repeat) return `${indent}... the frame above repeats ${f.repeat} more time${f.repeat === 1 ? '' : 's'}`;
    if (f.omitted) return `${indent}... ${f.omitted} more frame${f.omitted === 1 ? '' : 's'}`;
    return `${indent}at ${f.fn || '?'}${f.file ? ` (${f.file}:${f.line}:${f.col})` : ''}`;
  }).join('\n');
}

export function formatErrorTrace(st, indent = '  ') {
  if (!st || !st.errors.length) return '';
  return `${indent}error return trace (oldest first):\n` +
    st.errors.map((e) => `${indent}  ${e.file}:${e.line}:${e.col} in ${e.fn}: ${e.text}`).join('\n');
}

// A full crash report: message, location and stack.
export function formatTrap(t, where = '') {
  let s = `axiom: ${trapMessage(t)}${where}${t.file ? ` at ${t.file}:${t.line}:${t.col}` : ''}`;
  const frames = stackFrames(t, t.stack);
  if (frames.length) s += '\nstack trace (innermost first):\n' + formatFrames(frames);
  return s;
}

function formatValues(kind, dv, base, memory, instance) {
  const a = dv.getBigUint64(base + 16, true), b = dv.getBigUint64(base + 24, true);
  const alen = Number(dv.getBigUint64(base + 32, true)), blen = Number(dv.getBigUint64(base + 40, true));
  const f64 = (x) => { const d = new DataView(new ArrayBuffer(8)); d.setBigUint64(0, x, true); return String(d.getFloat64(0, true)); };
  const bytes = (p, n) => JSON.stringify(Buffer.from(new Uint8Array(memory.buffer, Number(p), Math.min(n, 200))).toString('utf8'));
  const errName = (x) => {
    const f = instance.exports.axiom_error_name;
    if (!f) return `error #${x}`;
    const p = f(Number(x)) >>> 0;
    const u = new Uint8Array(memory.buffer);
    let e = p;
    while (u[e]) e++;
    return 'error.' + Buffer.from(u.subarray(p, e)).toString('utf8');
  };
  switch (kind) {
    case 1: return [String(BigInt.asIntN(64, a)), String(BigInt.asIntN(64, b))];
    case 2: return [String(a), String(b)];
    case 3: return [f64(a), f64(b)];
    case 4: return [a ? 'true' : 'false', b ? 'true' : 'false'];
    case 5: return [bytes(a, alen), bytes(b, blen)];
    case 6: return [errName(a), errName(b)];
    default: return ['0x' + a.toString(16), '0x' + b.toString(16)];
  }
}

// Frames in the AXT protocol of `axiom test` (see test_main.c: encode_stack).
export function encodeFrames(frames, kindOfFirst) {
  return frames.map((f, i) => {
    if (f.repeat) return `R\x1f${f.repeat}\x1f0\x1f0\x1f`;
    if (f.omitted) return `O\x1f${f.omitted}\x1f0\x1f0\x1f`;
    const k = i === 0 ? kindOfFirst : 'C';
    return `${f.file ? k : 'U'}\x1f${f.fileId ?? 0}\x1f${f.start ?? 0}\x1f${f.end ?? 0}\x1f${f.fn || ''}`;
  }).map((x) => x + '\x1e').join('');
}

// A test's result when it trapped (t from describeTrap), given what it
// expected: -1 no trap, -2 any trap, n >= 0 trap Tn.
export function trapResult(t, expect) {
  if (expect === -2 || expect === t.code) return { status: 'pass' };
  const r = { status: 'fail', code: t.codeName, message: trapMessage(t), fileId: t.fileId, start: t.start, end: t.end,
    stack: encodeFrames(stackFrames(t, t.stack), 'T') };
  if (expect >= 0) r.message = `expected trap T${String(expect).padStart(4, '0')}, got ${r.message}`;
  return r;
}

export function trapMessage(t) {
  let m = `trap ${t.codeName} (${t.name})`;
  if (t.values) m = `expected equal values, left: ${t.values[0]}, right: ${t.values[1]}`;
  else if (t.code === 11) m = 'expectation failed';
  if (t.text) m += `: ${t.text}`;
  if (t.engineMessage && t.code === 0) m += ` [${t.engineMessage}]`;
  return m;
}
const THREAD_STACK = 1 << 20;

// Limits of an imported memory, read from the module binary (the JS API does
// not expose them): pages, whether it is shared and whether it is a memory64.
function memoryImportLimits(bytes) {
  let i = 8;
  const u32 = () => {
    // (a Number: a memory64's limits may pass 32 bits)
    let r = 0, scale = 1, b;
    do { b = bytes[i++]; r += (b & 0x7f) * scale; scale *= 128; } while (b & 0x80);
    return r;
  };
  const skipName = () => { const n = u32(); i += n; };
  while (i < bytes.length) {
    const id = bytes[i++];
    const size = u32();
    const end = i + size;
    if (id === 2) {
      let count = u32();
      while (count--) {
        skipName(); skipName();
        const kind = bytes[i++];
        if (kind === 0) u32();
        else if (kind === 1) { i++; const f = u32(); u32(); if (f & 1) u32(); }
        else if (kind === 2) {
          const flags = u32();
          const initial = u32();
          const maximum = flags & 1 ? u32() : undefined;
          return { initial, maximum, shared: (flags & 2) !== 0, memory64: (flags & 4) !== 0 };
        } else if (kind === 3) { i += 2; }
        else { i++; u32(); }
      }
      return null;
    }
    i = end;
  }
  return null;
}

// Blocking bridge to std/internal/runtime/net_worker.mjs: the wasm thread posts a request
// and sleeps in Atomics.wait until the worker has completed it. The main
// thread starts the worker; other threads talk to the same worker through a
// MessagePort, so all threads share socket handles.
class NetClient {
  constructor(port) {
    this.ctrl = new Int32Array(new SharedArrayBuffer(8));
    this.data = new Uint8Array(new SharedArrayBuffer(READ_CHUNK));
    if (port) {
      this.port = port;
      this.worker = null;
    } else {
      this.worker = new Worker(new URL('./net_worker.mjs', import.meta.url));
      this.worker.unref();
      this.port = this.worker;
    }
  }
  call(op, args, payload) {
    Atomics.store(this.ctrl, 0, 0);
    this.port.postMessage({ op, args, payload, ctrl: this.ctrl.buffer, data: this.data.buffer });
    // Wait until this request is done, not merely until a wakeup: the worker
    // completes a request with a store and then a notify, and when this
    // thread sees the store before it sleeps, that request's notify is still
    // to come and may wake the wait for the next request, which would then
    // return the previous result (a read returning a write's count, with
    // stale data in the buffer).
    while (Atomics.load(this.ctrl, 0) === 0) Atomics.wait(this.ctrl, 0, 0);
    return Atomics.load(this.ctrl, 1);
  }
  // A new port to the same worker, for another thread.
  newPort() {
    const ch = new MessageChannel();
    this.port.postMessage({ op: 'port', port: ch.port2 }, [ch.port2]);
    return ch.port1;
  }
  close() {
    if (this.worker) this.worker.terminate();
    else this.port.close();
  }
}

// The local time zone as the bytes of a TZif file (RFC 8536, version 2), for
// ax_tz_local: built from what the JavaScript engine knows of the zone (its
// Date offsets and Intl's abbreviations), so that it needs no time zone
// database (browsers, node on Windows). The transitions from 1900 to 2100
// are found by looking at every day's offset and bisecting to the second
// (two changes within a day are missed); the footer (a POSIX TZ rule, for
// later times) is the last year's rule, or its fixed offset. Whether a kind
// is daylight saving time is a judgement (isDst), as the engine does not
// say.
export function localTzif() {
  // (Date's offset is in whole minutes, enough to find the changes; a
  // kind's exact one, with the seconds of a local mean time, is Intl's
  // local clock less the UTC one)
  const off = (t) => Math.round(-new Date(t * 1000).getTimezoneOffset() * 60);
  const clock = new Intl.DateTimeFormat('en-US', { year: 'numeric', month: 'numeric', day: 'numeric', hour: 'numeric',
    minute: 'numeric', second: 'numeric', hourCycle: 'h23', era: 'short' });
  const exactOff = (t) => {
    const f = {};
    for (const x of clock.formatToParts(new Date(t * 1000))) f[x.type] = x.value;
    const y = f.era === 'BC' || f.era === 'B' ? 1 - Number(f.year) : Number(f.year);
    const d = new Date(0);
    d.setUTCFullYear(y, Number(f.month) - 1, Number(f.day));
    d.setUTCHours(Number(f.hour), Number(f.minute), Number(f.second), 0);
    const o = Math.round(d.getTime() / 1000 - t);
    return Math.abs(o - off(t)) < 60 ? o : off(t);
  };
  const fmt = new Intl.DateTimeFormat('en-US', { timeZoneName: 'short' });
  const abbr = (t) => {
    const part = fmt.formatToParts(new Date(t * 1000)).find((x) => x.type === 'timeZoneName');
    let a = part ? part.value : '';
    // ("GMT+5:30", where the zone has no abbreviation in English: "+0530")
    const m = /^(?:GMT|UTC)([+-])(\d{1,2})(?::(\d\d))?$/.exec(a);
    if (m) a = m[1] + m[2].padStart(2, '0') + (m[3] && m[3] !== '00' ? m[3] : '');
    a = a.replace(/[^A-Za-z0-9+-]/g, '');
    return a.length >= 3 ? a : (a === '' ? 'UTC' : a.padEnd(3, '-'));
  };
  const yearStart = (y) => Date.UTC(y, 0, 1) / 1000;
  // daylight saving time: an offset above one the zone has in the year
  // before and in the year after (a change of the standard offset has no
  // lower one on one side)
  const isDst = (t) => {
    const o = off(t);
    let before = false, after = false;
    for (let d = 15; d <= 366 && !(before && after); d += 15) {
      before ||= off(t - d * 86400) < o;
      after ||= off(t + d * 86400) < o;
    }
    return before && after;
  };
  const kinds = [], index = new Map();
  const kindOf = (t) => {
    const o = exactOff(t), k = { o, dst: isDst(t), a: abbr(t) };
    const key = `${k.o}/${k.dst}/${k.a}`;
    if (!index.has(key)) { index.set(key, kinds.length); kinds.push(k); }
    return index.get(key);
  };
  const first = yearStart(1900), end = yearStart(2101);
  kindOf(first);  // type 0: the time before the first transition
  const trans = [], types = [];
  let prevT = first, prev = off(first), prevKind = 0;
  for (let t = first + 86400; t <= end; t += 86400) {
    const o = off(t);
    if (o === prev) {
      prevT = t;
      continue;
    }
    // the first second of the new offset, in (prevT, t]
    let lo = prevT, hi = t;
    while (hi - lo > 1) {
      const mid = lo + Math.floor((hi - lo) / 2);
      if (off(mid) === prev) lo = mid; else hi = mid;
    }
    const k = kindOf(hi);
    if (k !== prevKind) { trans.push(hi); types.push(k); }
    prevT = t; prev = o; prevKind = k;
  }
  // the footer: POSIX TZ offsets are hours west, as [-]hh[:mm[:ss]]
  const posixOff = (o) => {
    const w = -o, a = Math.abs(w);
    const h = Math.floor(a / 3600), m = Math.floor((a % 3600) / 60), sec = a % 60;
    return (w < 0 ? '-' : '') + h + (m || sec ? ':' + String(m).padStart(2, '0') : '') + (sec ? ':' + String(sec).padStart(2, '0') : '');
  };
  const name = (a) => (/^[A-Za-z]+$/.test(a) ? a : `<${a}>`);
  // a transition as a POSIX date rule Mm.w.d/time (local time before it)
  const when = (t, before) => {
    const l = new Date((t + before) * 1000);
    const y = l.getUTCFullYear(), mo = l.getUTCMonth(), dom = l.getUTCDate();
    const dim = new Date(Date.UTC(y, mo + 1, 0)).getUTCDate();
    const w = dom + 7 > dim ? 5 : Math.ceil(dom / 7);
    const secs = l.getUTCHours() * 3600 + l.getUTCMinutes() * 60 + l.getUTCSeconds();
    return `M${mo + 1}.${w}.${l.getUTCDay()}` + (secs === 7200 ? '' : '/' + posixOff(-secs).replace(/^-/, ''));
  };
  let footer = '';
  const last = trans.map((t, i) => [t, types[i]]).filter(([t]) => t >= yearStart(2100));
  const lastKind = kinds[types.length ? types[types.length - 1] : 0];
  if (last.length === 0) footer = name(lastKind.a) + posixOff(lastKind.o);
  else if (last.length === 2 && kinds[last[0][1]].dst !== kinds[last[1][1]].dst) {
    const [on, back] = kinds[last[0][1]].dst ? [last[0], last[1]] : [last[1], last[0]];
    const ds = kinds[on[1]], st = kinds[back[1]];
    footer = name(st.a) + posixOff(st.o) + name(ds.a) + (ds.o === st.o + 3600 ? '' : posixOff(ds.o)) +
      ',' + when(on[0], st.o) + ',' + when(back[0], ds.o);
  }
  // the file: a version 1 header and block of no transitions, then version 2's
  const chars = [], at = new Map();
  for (const k of kinds) if (!at.has(k.a)) { at.set(k.a, chars.length); chars.push(...Buffer.from(k.a + '\0')); }
  const head = (timecnt, typecnt, charcnt) => {
    const b = Buffer.alloc(44);
    b.write('TZif2', 0, 'latin1');
    [0, 0, 0, timecnt, typecnt, charcnt].forEach((n, i) => b.writeUInt32BE(n, 20 + 4 * i));
    return b;
  };
  const v1 = Buffer.from([0, 0, 0, 0, 0, 0, 0]);  // one type (UTC, abbreviation ""), one char
  const body = Buffer.alloc(trans.length * 9 + kinds.length * 6);
  let p = 0;
  for (const t of trans) { body.writeBigInt64BE(BigInt(t), p); p += 8; }
  for (const k of types) body[p++] = k;
  for (const k of kinds) { body.writeInt32BE(k.o, p); body[p + 4] = k.dst ? 1 : 0; body[p + 5] = at.get(k.a); p += 6; }
  return Buffer.concat([head(0, 1, 1), v1, head(trans.length, kinds.length, chars.length), body, Buffer.from(chars),
    Buffer.from(`\n${footer}\n`, 'latin1')]);
}

// The end of a supervised program, from any thread: the first thread to ask
// records its status and wakes the supervisor, which ends the process; the
// asking thread waits for that (ctl: Supervisor.ctl).
export function endProgram(ctl, code) {
  if (Atomics.compareExchange(ctl, 3, 0, 1) === 0) {
    Atomics.store(ctl, 1, code | 0);
    Atomics.store(ctl, 0, 1);
    Atomics.notify(ctl, 0);
  }
  for (;;) Atomics.wait(ctl, 2, 0);
}

// The thread that supervises a threaded program (on node's main thread):
// node lets only the main thread end the process (a worker's process.exit
// ends that worker), and only it change directory or put a terminal in raw
// mode; the program's threads all run on workers and ask it (endProgram,
// SuperClient). ctl: [0] 1 once the program has asked to end, [1] the
// status, [2] a word nobody changes, [3] 1 once a thread has claimed the end.
export class Supervisor {
  constructor(ops = {}) {
    this.ctl = new Int32Array(new SharedArrayBuffer(16));
    this.ops = ops;  // more requests: name -> async (args) => result
  }
  // A port of a thread's SuperClient.
  attach(port) {
    port.on('message', (m) => this.handle(m));
  }
  async handle(m) {
    if (m.op === 'port') {
      this.attach(m.port);
      return;
    }
    const reply = new Int32Array(m.reply);
    let r = -1;
    try {
      if (m.op === 'chdir') {
        process.chdir(m.args.path);
        r = 0;
      } else if (m.op === 'term_raw') {
        r = -1;
        if (process.stdin.isTTY && process.stdin.setRawMode) {
          process.stdin.setRawMode(m.args.enable);
          r = 0;
        }
      } else if (m.op === 'term_size') {
        const st = m.args.fd === 1 ? process.stdout : m.args.fd === 2 ? process.stderr : null;
        if (st && st.isTTY && st.getWindowSize) {
          [reply[2], reply[3]] = st.getWindowSize();
          r = 0;
        }
      } else if (this.ops[m.op]) {
        r = await this.ops[m.op](m.args);
      }
    } catch (e) {
      r = fsErr(e);
    }
    Atomics.store(reply, 1, r);
    Atomics.store(reply, 0, 1);
    Atomics.notify(reply, 0);
  }
  // Resolves with the status once a thread has asked to end the program.
  ended() {
    const w = Atomics.waitAsync(this.ctl, 0, 0);
    return (w.async ? w.value : Promise.resolve()).then(() => Atomics.load(this.ctl, 1));
  }
}

// A worker's side of the Supervisor: requests it serves at once (the
// worker waits), on a port of its own.
class SuperClient {
  constructor(sup) {
    this.ctl = sup.ctl;
    this.port = sup.port;
    this.reply = new Int32Array(new SharedArrayBuffer(16));
  }
  call(op, args) {
    Atomics.store(this.reply, 0, 0);
    this.port.postMessage({ op, args, reply: this.reply.buffer });
    while (Atomics.load(this.reply, 0) === 0) Atomics.wait(this.reply, 0, 0);
    return Atomics.load(this.reply, 1);
  }
  // A new port to the supervisor, for another thread.
  newPort() {
    const ch = new MessageChannel();
    this.port.postMessage({ op: 'port', port: ch.port2 }, [ch.port2]);
    return ch.port1;
  }
}

// node's error codes as the negated errno values the host functions return
function fsErr(e) {
  return ({ ENOENT: -2, EACCES: -13, EPERM: -13, EEXIST: -17, ENOTDIR: -20, EISDIR: -21, ENOTEMPTY: -39, ELOOP: -40 })[e && e.code] || -1;
}

// The in-memory file system of a program (--files=memory), with node's fs
// interface: its calls go to the one MemFS (memfs.mjs) on the net worker,
// which every thread of the program shares (see the net worker's memfs op).
const MEM_CODES = ['ENOENT', 'EACCES', 'EPERM', 'EEXIST', 'ENOTDIR', 'EISDIR', 'ENOTEMPTY', 'ELOOP', 'EBADF', 'EINVAL', 'EBUSY'];
class RemoteFS {
  constructor(net) {
    this.net = net;
    this.virtual = true;
    this.constants = { O_RDONLY: 0, O_WRONLY: 1, O_RDWR: 2, O_CREAT: 0o100, O_EXCL: 0o200, O_TRUNC: 0o1000, O_APPEND: 0o2000,
      O_DIRECTORY: 0o200000, O_NOFOLLOW: 0o400000 };
  }
  call(m, a, payload) {
    const r = this.net().call('memfs', { m, a }, payload);
    if (r < 0) throw Object.assign(new Error(MEM_CODES[-1 - r]), { code: MEM_CODES[-1 - r] });
    return r;
  }
  result(n) { return this.net().data.subarray(0, n); }
  json(m, a) { return JSON.parse(Buffer.from(this.result(this.call(m, a))).toString('utf8')); }
  stats(m, a, o) {
    const j = this.json(m, a), big = !!(o && o.bigint);
    const num = (x) => (big ? BigInt(x) : Number(x));
    return { size: num(j.size), mode: num(j.mode), ino: num(j.ino), dev: num(1), mtimeNs: BigInt(j.mtime),
      isFile: () => j.kind === 1, isDirectory: () => j.kind === 2, isSymbolicLink: () => j.kind === 3 };
  }
  openSync(p, flags = 'r', mode = 0o666) { return this.call('openSync', [p, flags, mode]); }
  closeSync(fd) { this.call('closeSync', [fd]); }
  readSync(fd, buf, off, len, at) {
    const n = this.call('readSync', [fd, null, Math.min(len, READ_CHUNK), at]);
    buf.set(this.result(n), off);
    return n;
  }
  writeSync(fd, buf, off = 0, len = buf.length - off, at = null) {
    return this.call('writeSync', [fd, at], buf.slice(off, off + len));
  }
  fstatSync(fd, o) { return this.stats('fstatSync', [fd], o); }
  statSync(p, o) { return this.stats('statSync', [p], o); }
  lstatSync(p, o) { return this.stats('lstatSync', [p], o); }
  readdirSync(p) {
    return this.json('readdirSync', [p]).map(([name, k]) => ({ name, isFile: () => k === 1, isDirectory: () => k === 2,
      isSymbolicLink: () => k === 3 }));
  }
  readlinkSync(p) { return Buffer.from(this.result(this.call('readlinkSync', [p]))); }
  mkdirSync(p, o = {}) { this.call('mkdirSync', [p, { mode: o.mode }]); }
  rmdirSync(p) { this.call('rmdirSync', [p]); }
  unlinkSync(p) { this.call('unlinkSync', [p]); }
  renameSync(a, b) { this.call('renameSync', [a, b]); }
  chmodSync(p, m) { this.call('chmodSync', [p, m]); }
  symlinkSync(t, p) { this.call('symlinkSync', [t, p]); }
  linkSync(a, b) { this.call('linkSync', [a, b]); }
  utimesSync(p, a, m) { this.call('utimesSync', [p, a, m]); }
  ftruncateSync(fd, n) { this.call('ftruncateSync', [fd, n]); }
  fsyncSync(fd) { this.call('fsyncSync', [fd]); }
  cwd() { return this.json('cwd', []); }
  chdir(p) { this.call('chdir', [p]); }
}

// Node's files under the directory `root` (--root=dir), which the program
// sees as `/`: every path is resolved against the program's working
// directory (node's, as a path under the root) and taken there; a path
// whose existing part leads outside the root through a symbolic link (or
// `..`, which stops at the root) fails with EACCES. The root's `tmp` is the
// program's /tmp (made if missing).
function rootFs(root) {
  const real = fs.realpathSync(root);
  try { fs.mkdirSync(path.join(real, 'tmp')); } catch { /* there */ }
  const inside = (r) => r === real || r.startsWith(real + path.sep);
  const cwd = () => {
    const c = process.cwd();
    return inside(c) ? '/' + path.relative(real, c).split(path.sep).join('/') : '/';
  };
  // The real path of p, checked: its deepest existing ancestor (or itself,
  // links followed but for a final one when follow is false) must be inside.
  const map = (p, follow = true) => {
    const v = path.posix.resolve(cwd(), String(p));
    const r = path.join(real, ...v.split('/').filter((x) => x));
    let probe = follow ? r : path.dirname(r);
    for (;;) {
      try {
        if (!inside(fs.realpathSync(probe))) throw Object.assign(new Error('outside the root'), { code: 'EACCES' });
        break;
      } catch (e) {
        if (e.code === 'EACCES') throw e;
        if (probe === real) break;
        probe = path.dirname(probe);
      }
    }
    return r;
  };
  const F = Object.create(fs);
  F.real = map;
  F.cwd = cwd;
  F.chdir = (p) => process.chdir(map(p));
  for (const m of ['openSync', 'statSync', 'mkdirSync', 'rmdirSync', 'readdirSync', 'chmodSync', 'utimesSync'])
    F[m] = (p, ...a) => fs[m](map(p), ...a);
  for (const m of ['lstatSync', 'unlinkSync', 'readlinkSync']) F[m] = (p, ...a) => fs[m](map(p, false), ...a);
  F.renameSync = (a, b) => fs.renameSync(map(a, false), map(b, false));
  F.linkSync = (a, b) => fs.linkSync(map(a, false), map(b, false));
  F.symlinkSync = (t, l) => fs.symlinkSync(t, map(l, false));  // (the target is checked when followed)
  return F;
}

// File positions shared by the threads: a table in shared memory for the
// first POS_FDS descriptors (stored as position + 2; 0 means untracked, 1
// means append), a plain map beyond (those are not shared).
const POS_FDS = 4096;
class SharedPositions {
  constructor(buf) {
    this.table = new Float64Array(buf);
    this.more = new Map();
  }
  has(fd) { return fd >= 0 && fd < POS_FDS ? this.table[fd] !== 0 : this.more.has(fd); }
  get(fd) { return fd >= 0 && fd < POS_FDS ? this.table[fd] - 2 : this.more.get(fd); }
  set(fd, v) {
    if (fd >= 0 && fd < POS_FDS) this.table[fd] = v + 2;
    else this.more.set(fd, v);
  }
  delete(fd) {
    if (fd >= 0 && fd < POS_FDS) this.table[fd] = 0;
    else this.more.delete(fd);
  }
}

// An address record (see std/internal/runtime/native.ax) as node's host and port.
function addrOf(r) {
  const port = (r[2] << 8) | r[3];
  if (r[0] === 4) return { host: `${r[4]}.${r[5]}.${r[6]}.${r[7]}`, port, family: 4 };
  if (r[0] !== 6) return null;
  const g = [];
  for (let i = 0; i < 8; i++) g.push(((r[4 + 2 * i] << 8) | r[5 + 2 * i]).toString(16));
  return { host: g.join(':'), port, family: 6 };
}

// The parameters (by position) of host functions that are i64 on every
// target. On wasm64 the other i64 parameters are pointers and sizes, which
// the host functions take as Numbers, like on wasm32 (std's declarations of
// the `axiom` imports).
const I64_PARAMS = { ax_ftruncate: [1], ax_mmap: [1], ax_seek: [1], ax_signal_take: [0], ax_sleep_ns: [0],
  ax_syscall: [0], ax_utimes: [2, 3] };

// The host functions whose result is an isize or usize (std's declarations
// of the `axiom` imports).
const SIZE_RESULTS = new Set(['ax_arg', 'ax_dir_read', 'ax_environ', 'ax_getcwd', 'ax_getenv', 'ax_mmap', 'ax_pipe_read', 'ax_tz_local',
  'ax_pipe_write', 'ax_read', 'ax_readdir', 'ax_readlink', 'ax_sock_read', 'ax_sock_write', 'ax_start_sp', 'ax_syscall',
  'ax_thread_create', 'ax_thread_stack_size', 'ax_udp_recv', 'ax_udp_send', 'ax_watch_read', 'ax_write']);

// A memory for a module that imports one (lim from memoryImportLimits). A
// memory64 is asked for with the JS API's `index: 'i64'` (Numbers) or, where
// the engine has the later form, `address: 'i64'` (BigInts).
function newMemory(lim) {
  const maximum = lim.maximum ?? (lim.memory64 ? 262144 : 65536);
  if (!lim.memory64) return new WebAssembly.Memory({ initial: lim.initial, maximum, shared: lim.shared });
  try {
    return new WebAssembly.Memory({ initial: lim.initial, maximum, shared: lim.shared, index: 'i64' });
  } catch {
    return new WebAssembly.Memory({ initial: BigInt(lim.initial), maximum: BigInt(maximum), shared: lim.shared, address: 'i64' });
  }
}

export function createHost(opts = {}) {
  let memory = opts.memory || null;
  let module = null;
  let netClient = null;
  let nextThread = 1;
  const writeFd = opts.write || ((fd, bytes) => fs.writeSync(fd, bytes));
  const bytes = (p, n) => new Uint8Array(memory.buffer, p >>> 0, n >>> 0);
  const str = (p, n) => Buffer.from(memory.buffer, p >>> 0, n >>> 0).toString('utf8');
  const net = () => (netClient ||= new NetClient(opts.netPort));
  // State every thread of the program shares: its arguments, and the
  // positions of the files it opened (node has no lseek, so reads and
  // writes of them are positioned explicitly). Threads get it through
  // workerData.
  const shared = opts.shared || { args: opts.args || [], pos: new SharedArrayBuffer(POS_FDS * 8), files: opts.files || null,
    root: opts.root || null };
  const programArgs = shared.args;
  const pos = new SharedPositions(shared.pos);
  // The program's files (shared.files: one choice for every thread): node's
  // (fs), node's under a root directory (rootFs), or in memory (RemoteFS,
  // on the net worker, where every thread reaches the same files).
  const F = shared.files === 'memory' ? new RemoteFS(net) : shared.root ? rootFs(shared.root) : fs;
  // ax_stat's out[0..4]: kind, size, modification time, permission bits
  const statOut = (st, out) => {
    const kind = st.isFile() ? 1n : st.isDirectory() ? 2n : st.isSymbolicLink() ? 3n : 4n;
    const dv = new DataView(memory.buffer);
    const o = Number(out);
    dv.setBigInt64(o, kind, true);
    dv.setBigInt64(o + 8, st.size, true);
    dv.setBigInt64(o + 16, st.mtimeNs, true);
    dv.setBigInt64(o + 24, st.mode & 0o7777n, true);
  };
  // (a worker of a supervised program: its requests to the supervisor)
  const sup = opts.supervisor ? new SuperClient(opts.supervisor) : null;
  // Directory handles (see ax_dir_open): a descriptor of the directory,
  // opened without following links, pins it. Node has no openat, so names
  // under it are resolved through the descriptor where the system allows
  // (Linux: /proc/self/fd/N/name) and else by path, after checking that
  // the path is still the pinned directory (device and inode) -- a narrow
  // window, where a swapped-in link could still be followed.
  const dirs = new Map();
  let nextDir = 1;
  const O = F.constants;
  const DIR_FLAGS = O.O_RDONLY | (O.O_DIRECTORY || 0) | (O.O_NOFOLLOW || 0);
  const PROC_FD = F !== fs ? null : (() => { try { return fs.statSync('/proc/self/fd').isDirectory() ? '/proc/self/fd/' : null; } catch { return null; } })();
  // The path to use for the directory handle d (its descriptor's, or its
  // own after a check); throws ELOOP if the path is no longer it.
  const dirBase = (d) => {
    const dir = dirs.get(d);
    if (dir.via) return dir.via;
    if (dir.fd >= 0) {
      const st = F.lstatSync(dir.path);
      if (!st.isDirectory() || st.dev !== dir.dev || st.ino !== dir.ino) throw Object.assign(new Error('replaced'), { code: 'ELOOP' });
    }
    return dir.path;
  };
  const dirPath = (d, name) => (d < 0 ? name : dirBase(d) + '/' + name);
  // The path of d's parent, to open: by path, d's path less its last name
  // (never a link, see ax_dir_open) rather than d/.., since remove_all
  // removes the child it reopened its parent through, and d/../.. would
  // no longer resolve.
  const parentPath = (d) => {
    const base = dirBase(d);
    const slash = base.lastIndexOf('/');
    const last = base.slice(slash + 1);
    if (base === dirs.get(d).via || slash <= 0 || last === '' || last === '.' || last === '..') return base + '/..';
    return base.slice(0, slash);
  };
  // Seconds for fs.utimes, never later than the time ns: libuv takes the
  // nanoseconds back as (s - floor(s)) * 1e9, keeping only microseconds on
  // Linux but all of them on macOS, where rounding s up set a later time.
  const utimeSeconds = (ns) => {
    let s = Number(ns / 1000000000n) + Number(ns % 1000000000n) / 1e9;
    const back = (x) => BigInt(Math.floor(x)) * 1000000000n + BigInt(Math.trunc((x - Math.floor(x)) * 1e9));
    while (back(s) > ns) s -= Math.max(Math.abs(s) * Number.EPSILON, Number.MIN_VALUE);
    return s;
  };
  // Copies `buf` to p[0..cap] if it fits; returns its length.
  const give = (buf, p, cap) => {
    if (buf.length <= Number(cap)) bytes(Number(p), buf.length).set(buf);
    return buf.length;
  };
  const readdirPath = (dir, out, cap) => {
    try {
      const parts = [];
      for (const e of F.readdirSync(dir, { withFileTypes: true })) {
        const kind = e.isFile() ? 1 : e.isDirectory() ? 2 : e.isSymbolicLink() ? 3 : 4;
        parts.push(Buffer.from([kind]), Buffer.from(e.name, 'utf8'), Buffer.from([0]));
      }
      return give(Buffer.concat(parts), out, cap);
    } catch (e) {
      return fsErr(e);
    }
  };
  const axiom = {
    ax_write(fd, p, n) {
      try {
        if (!pos.has(fd)) return writeFd(fd, bytes(p, n).slice());
        const at = pos.get(fd);
        const w = F.writeSync(fd, bytes(p, n).slice(), 0, n >>> 0, at < 0 ? null : at);
        if (at >= 0) pos.set(fd, at + w);
        return w;
      } catch { return -1; }
    },
    ax_read(fd, p, n) {
      try {
        const buf = Buffer.alloc(Math.min(n >>> 0, READ_CHUNK));
        const at = pos.has(fd) ? pos.get(fd) : -1;
        const got = (at >= 0 || pos.has(fd) ? F : fs).readSync(fd, buf, 0, buf.length, at < 0 ? null : at);
        if (at >= 0) pos.set(fd, at + got);
        bytes(p, got).set(buf.subarray(0, got));
        return got;
      } catch (e) {
        return e.code === 'EOF' ? 0 : -1;
      }
    },
    ax_open(p, n, flags) {
      try {
        // (flags 4 and 5: w+ and a+; flag 8: the file must not exist)
        const excl = (flags & 8) && [1, 4].includes(flags & 7);
        const mode = excl ? ((flags & 7) === 1 ? 'wx' : 'wx+') : (['r', 'w', 'a', 'r+', 'w+', 'a+'][flags & 7] || 'r');
        // (flag 16: a file created can be read and written by its owner alone)
        const fd = F.openSync(str(p, n), mode, (flags & 16) ? 0o600 : 0o644);
        pos.set(fd, (flags & 7) === 2 ? -1 : 0);  // appends go to the end
        return fd;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_create(p, n, mode) {
      try {
        const fd = F.openSync(str(p, n), 'w', mode & 0o777);
        pos.set(fd, 0);
        return fd;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_close(fd) {
      pos.delete(fd);
      try { (F !== fs && fd > 2 ? F : fs).closeSync(fd); } catch { /* ignore */ }
    },
    ax_seek(fd, off, whence) {
      if (!pos.has(fd)) return -1n;
      try {
        let base = 0;
        if (whence === 1) base = Math.max(pos.get(fd), 0);
        else if (whence === 2) base = Number(F.fstatSync(fd).size);
        const at = base + Number(off);
        if (at < 0) return -1n;
        if (pos.get(fd) >= 0) pos.set(fd, at);
        return BigInt(at);
      } catch (e) {
        return BigInt(fsErr(e));
      }
    },
    ax_stat(p, n, follow, out) {
      try {
        const path = str(p, n);
        statOut(follow ? F.statSync(path, { bigint: true }) : F.lstatSync(path, { bigint: true }), out);
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_fstat(fd, out) {
      try { statOut((F !== fs && fd > 2 ? F : fs).fstatSync(fd, { bigint: true }), out); return 0; } catch (e) { return fsErr(e); }
    },
    ax_mkdir(p, n, mode) {
      try { F.mkdirSync(str(p, n), { mode: mode & 0o7777 }); return 0; } catch (e) { return fsErr(e); }
    },
    ax_remove(p, n, dir) {
      try {
        if (dir) F.rmdirSync(str(p, n));
        else F.unlinkSync(str(p, n));
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_rename(a, an, b, bn) {
      try { F.renameSync(str(a, an), str(b, bn)); return 0; } catch (e) { return fsErr(e); }
    },
    ax_readdir(p, n, out, cap) { return readdirPath(str(p, n), out, cap); },
    ax_dir_open(d, p, n) {
      try {
        if (d >= 0 && !dirs.has(d)) return -1;
        const name = str(p, n);
        const full = d >= 0 && name === '..' ? parentPath(d) : dirPath(d, name);
        const st = F.lstatSync(full);
        if (st.isSymbolicLink()) return -40;
        if (!st.isDirectory()) return -20;
        let fd = -1, dev = 0, ino = 0;
        try {
          fd = F.openSync(full, DIR_FLAGS);
          const fst = F.fstatSync(fd);
          if (!fst.isDirectory()) { F.closeSync(fd); return -20; }
          dev = fst.dev;
          ino = fst.ino;
        } catch (e) {
          if (fd >= 0) F.closeSync(fd);
          if (e.code === 'ELOOP' || e.code === 'ENOTDIR') return fsErr(e);
          fd = -1;  // directories cannot be opened here: by path, as before
        }
        const h = nextDir++;
        dirs.set(h, { path: full, fd, dev, ino,
                      via: fd >= 0 && PROC_FD && O.O_DIRECTORY ? PROC_FD + fd : null });
        return h;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_dir_read(d, out, cap) {
      if (!dirs.has(d)) return -1;
      try { return readdirPath(dirBase(d), out, cap); } catch (e) { return fsErr(e); }
    },
    ax_dir_remove(d, p, n, dir) {
      try {
        if (d >= 0 && !dirs.has(d)) return -1;
        const full = dirPath(d, str(p, n));
        if (dir) F.rmdirSync(full);
        else F.unlinkSync(full);
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_dir_close(d) {
      const dir = dirs.get(d);
      if (dir && dir.fd >= 0) try { F.closeSync(dir.fd); } catch {}
      dirs.delete(d);
    },
    // The identity (device, inode) of an open directory, to check one
    // reopened through a child's "..".
    ax_dir_stat(d, out) {
      const dir = dirs.get(d);
      if (!dir) return -1;
      try {
        const st = dir.fd >= 0 ? F.fstatSync(dir.fd, { bigint: true }) : F.lstatSync(dir.path, { bigint: true });
        const dv = new DataView(memory.buffer);
        dv.setBigInt64(Number(out), st.dev, true);
        dv.setBigInt64(Number(out) + 8, st.ino, true);
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_getcwd(p, cap) { return give(Buffer.from(F === fs ? process.cwd() : F.cwd(), 'utf8'), p, cap); },
    ax_argc: () => programArgs.length,
    ax_arg(i, p, cap) {
      if (i < 0 || i >= programArgs.length) return -1;
      return give(Buffer.from(programArgs[i], 'utf8'), p, cap);
    },
    ax_getenv(p, n, out, cap) {
      const v = process.env[str(p, n)];
      if (v === undefined) return -2;
      return give(Buffer.from(v, 'utf8'), out, cap);
    },
    // Ends the program, every thread (spec §12.8.5): through the supervisor
    // on a worker, else the embedder's opts.exit, which must not return.
    ax_exit(code) {
      if (sup) endProgram(sup.ctl, code);
      (opts.exit || process.exit)(code);
    },
    ax_tcp_connect: (p, n, port) => net().call('connect', { host: str(p, n), port }),
    ax_tcp_listen: (p, n, port) => net().call('listen', { host: str(p, n), port }),
    ax_tcp_accept: (h) => net().call('accept', { h }),
    ax_tcp_local_port: (h) => net().call('local_port', { h }),
    ax_sock_read(h, p, n) {
      const c = net();
      const got = c.call('read', { h, n: Math.min(n >>> 0, READ_CHUNK) });
      if (got > 0) bytes(p, got).set(c.data.subarray(0, got));
      return got;
    },
    ax_sock_write: (h, p, n) => net().call('write', { h }, bytes(p, n).slice()),
    ax_sock_close: (h) => { net().call('close', { h }); },
    ax_sock_timeout: (h, ms) => net().call('timeout', { h, ms: ms >>> 0 }),
    // Processes and the environment. The program's threads share the
    // environment (workers are started with SHARE_ENV) and the directory,
    // which only node's main thread may change: a worker asks the
    // supervisor. Signals are not supported: node delivers them only to the
    // main thread's event loop, which a running program does not return to.
    ax_getpid: () => process.pid,
    ax_fork: () => -38,  // ENOSYS: an instance cannot be copied
    ax_chdir(p, n) {
      try {
        if (F.virtual) { F.chdir(str(p, n)); return 0; }  // (the in-memory files' own directory)
        const real = F.real ? F.real(str(p, n)) : str(p, n);
        if (sup) return sup.call('chdir', { path: real });
        process.chdir(real);
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_setenv(k, kn, v, vn) {
      const name = str(k, kn), value = str(v, vn);
      if (!name || name.includes('=') || name.includes('\0') || value.includes('\0')) return -1;
      process.env[name] = value;
      return 0;
    },
    ax_unsetenv(k, kn) {
      const name = str(k, kn);
      if (!name || name.includes('=') || name.includes('\0')) return -1;
      delete process.env[name];
      return 0;
    },
    ax_environ(p, cap) {
      const b = Buffer.from(Object.entries(process.env).map(([k, v]) => `${k}=${v}\0`).join(''));
      if (b.length <= cap >>> 0) bytes(p, b.length).set(b);
      return b.length;
    },
    ax_proc_spawn(p, n, stdio) {
      // argv and the environment (0-terminated strings, each list ending in
      // an empty one), then the directory
      const b = Buffer.from(bytes(p, n));
      let i = 0;
      const list = () => {
        const xs = [];
        while (i < b.length && b[i] !== 0) {
          const j = b.indexOf(0, i);
          xs.push(b.toString('utf8', i, j));
          i = j + 1;
        }
        i++;
        return xs;
      };
      const argv = list();
      const envList = list();
      const cwd = b.toString('utf8', Math.min(i, b.length));
      const env = {};
      for (const e of envList) {
        const k = e.indexOf('=');
        if (k > 0) env[e.slice(0, k)] = e.slice(k + 1);
      }
      const view = new Int32Array(memory.buffer, stdio >>> 0, 3);
      if (process.platform === 'win32' && argv.length) {
        // cmd.exe runs a batch file, and would read the arguments as commands
        if (/\.(bat|cmd)[. ]*$/i.test(argv[0])) {
          view.fill(-1);
          return -22;
        }
        // a bare name is found on the child's PATH or not at all (node would
        // also look in the current directory)
        if (!/[\\/:]/.test(argv[0])) {
          const pv = Object.entries(env).find(([k]) => k.toUpperCase() === 'PATH');
          const found = (pv ? pv[1] : '').split(';').filter((d) => d)
            .flatMap((d) => [`${d}\\${argv[0]}`, `${d}\\${argv[0]}.exe`])
            .find((f) => { try { return fs.statSync(f).isFile(); } catch { return false; } });
          if (!found) {
            view.fill(-1);
            return -2;
          }
          argv[0] = found;
        }
      }
      const modes = Array.from(view);
      const c = net();
      const h = c.call('spawn', { argv, env, cwd, modes });
      const got = new Int32Array(c.data.buffer, 0, 3);
      for (let k = 0; k < 3; k++) view[k] = h >= 0 ? got[k] : -1;
      return h;
    },
    ax_proc_wait(h, block, status) {
      const c = net();
      const r = c.call('wait', { h, block });
      if (r > 0) new Int32Array(memory.buffer, status >>> 0, 1)[0] = new Int32Array(c.data.buffer, 0, 1)[0];
      return r;
    },
    ax_proc_kill: (h, sig) => net().call('kill', { h, sig }),
    ax_pipe_read(h, p, n) {
      const c = net();
      const got = c.call('read', { h, n: Math.min(n >>> 0, READ_CHUNK) });
      if (got > 0) bytes(p, got).set(c.data.subarray(0, got));
      return got;
    },
    ax_pipe_write: (h, p, n) => net().call('write', { h }, bytes(p, n).slice()),
    ax_pipe_close: (h) => { net().call('close', { h }); },
    ax_signal: () => -2,  // unsupported
    // Addresses, IPv6, UDP, Unix sockets and polling (on the net worker).
    ax_resolve(p, n, out, max) {
      const c = net();
      const k = c.call('resolve', { name: str(p, n), max: Math.min(max >>> 0, READ_CHUNK / 24) });
      if (k > 0) bytes(out, 24 * k).set(c.data.subarray(0, 24 * k));
      return k;
    },
    ax_tcp_connect_addr(r) {
      const a = addrOf(bytes(r, 24));
      return a ? net().call('connect_addr', a) : -4;
    },
    ax_tcp_listen_addr(r) {
      const a = addrOf(bytes(r, 24));
      return a ? net().call('listen_addr', a) : -4;
    },
    ax_sock_addr(h, peer, out) {
      const c = net();
      const r = c.call('sock_addr', { h, peer });
      if (r === 0) bytes(out, 24).set(c.data.subarray(0, 24));
      return r;
    },
    ax_udp_open(r) {
      const a = addrOf(bytes(r, 24));
      return a ? net().call('udp_open', a) : -4;
    },
    ax_udp_send(h, p, n, r) {
      const a = addrOf(bytes(r, 24));
      return a ? net().call('udp_send', { h, host: a.host, port: a.port }, bytes(p, n).slice()) : -4;
    },
    ax_udp_recv(h, p, n, from) {
      const c = net();
      const k = c.call('udp_recv', { h, n: Math.min(n >>> 0, READ_CHUNK - 24) });
      if (k >= 0) {
        bytes(from, 24).set(c.data.subarray(0, 24));
        bytes(p, k).set(c.data.subarray(24, 24 + k));
      }
      return k;
    },
    ax_unix_connect: (p, n) => net().call('unix_connect', { path: str(p, n) }),
    ax_unix_listen: (p, n) => net().call('listen_addr', { path: str(p, n) }),
    ax_poll(ep, n, timeout) {
      const view = new DataView(memory.buffer, ep >>> 0, 8 * n);
      const entries = [];
      for (let k = 0; k < n; k++) entries.push({ h: view.getInt32(8 * k, true), events: view.getUint16(8 * k + 4, true) });
      const c = net();
      const r = c.call('poll', { entries, timeout });
      if (r >= 0) {
        const rev = new Uint16Array(c.data.buffer, 0, n);
        for (let k = 0; k < n; k++) view.setUint16(8 * k + 6, rev[k], true);
      }
      return r;
    },
    // More of the file system. Node has no flock and cannot map files into
    // the program's memory: ax_flock reports -95 (not supported) and ax_mmap
    // 0, for std/os/fs to fall back on. Watching runs on the net worker.
    ax_chmod(p, n, mode) {
      try { F.chmodSync(str(p, n), mode & 0o7777); return 0; } catch (e) { return fsErr(e); }
    },
    ax_symlink(t, tn, l, ln) {
      try { F.symlinkSync(str(t, tn), str(l, ln)); return 0; } catch (e) { return fsErr(e); }
    },
    ax_link(a, an, b, bn) {
      try { F.linkSync(str(a, an), str(b, bn)); return 0; } catch (e) { return fsErr(e); }
    },
    // (none: argument 0 is the module's path)
    ax_self_path() { return -1; },
    ax_readlink(p, n, buf, cap) {
      try {
        const b = F.readlinkSync(str(p, n), { encoding: 'buffer' });
        if (b.length <= cap >>> 0) bytes(buf, b.length).set(b);
        return b.length;
      } catch (e) {
        return fsErr(e);
      }
    },
    ax_utimes(p, n, atime, mtime) {
      try { F.utimesSync(str(p, n), utimeSeconds(atime), utimeSeconds(mtime)); return 0; } catch (e) { return fsErr(e); }
    },
    ax_ftruncate(fd, size) {
      try { F.ftruncateSync(fd, Number(size)); return 0; } catch (e) { return fsErr(e); }
    },
    ax_fsync(fd) {
      try { F.fsyncSync(fd); return 0; } catch { return -1; }
    },
    ax_flock: () => -95,
    ax_mmap: () => 0,
    ax_munmap: () => -1,
    ax_msync: () => -1,
    ax_watch_open: () => net().call('watch_open', {}),
    ax_watch_add: (h, p, n) => net().call('watch_add', { h, path: str(p, n) }),
    ax_watch_remove: (h, wd) => net().call('watch_remove', { h, wd }),
    ax_watch_read(h, buf, cap, timeout) {
      const c = net();
      const got = c.call('watch_read', { h, n: Math.min(cap >>> 0, READ_CHUNK), timeout });
      if (got > 0) bytes(buf, got).set(c.data.subarray(0, got));
      return got;
    },
    ax_watch_close: (h) => { net().call('close', { h }); },
    // Terminals: the size of stdout's and stderr's, raw mode for stdin (node's
    // main thread's: the supervisor's for a worker).
    ax_isatty: (fd) => (!(F !== fs && fd > 2) && tty.isatty(fd) ? 1 : 0),
    ax_term_size(fd, out) {
      if (sup) {
        const r = sup.call('term_size', { fd });
        if (r === 0) new Uint32Array(memory.buffer, out >>> 0, 2).set([sup.reply[2], sup.reply[3]]);
        return r;
      }
      const s = fd === 1 ? process.stdout : fd === 2 ? process.stderr : null;
      if (!s || !s.isTTY || !s.getWindowSize) return -1;
      const [cols, rows] = s.getWindowSize();
      const o = new Uint32Array(memory.buffer, out >>> 0, 2);
      o[0] = cols;
      o[1] = rows;
      return 0;
    },
    ax_term_raw(fd, enable) {
      if (sup) return fd === 0 ? sup.call('term_raw', { enable: enable !== 0 }) : -1;
      if (fd !== 0 || !process.stdin.isTTY || !process.stdin.setRawMode) return -1;
      try { process.stdin.setRawMode(enable !== 0); return 0; } catch { return -1; }
    },
    ax_signal_take: () => -1n,
    ax_thread_spawn(entry, arg, stackTop, exited) {
      // The new thread shares this program's sockets.
      const netPort = net().newPort();
      // (and reaches the supervisor, if there is one, on a port of its own)
      const superPort = sup ? sup.newPort() : null;
      const supervisor = sup ? { ctl: sup.ctl, port: superPort } : null;
      const w = new Worker(new URL('./thread_worker.mjs', import.meta.url), {
        workerData: { module, memory, entry, arg, stackTop, exited, netPort, shared, supervisor, testTrap: opts.testTrap || null },
        transferList: sup ? [netPort, superPort] : [netPort],
        env: SHARE_ENV,
      });
      // A trap in any thread terminates the whole program (spec §12.8).
      w.on('error', (e) => {
        process.stderr.write(`axiom: trap in thread: ${e.message}\n`);
        process.exit(134);
      });
      w.unref();
      return nextThread++;
    },
    ax_thread_stack_size: () => THREAD_STACK,
    // Ends the calling thread (one ax_thread_spawn started): thread_worker.mjs
    // catches this, as the return of the thread's entry. Nothing on another.
    ax_thread_exit() {
      if (opts.thread) throw THREAD_EXIT;
    },
    ax_os: () => ({ linux: 1, darwin: 2, win32: 3 }[process.platform] || 0),
    // The local time zone, as a TZif file (localTzif: from the engine's own
    // knowledge of the zone, which follows $TZ, so no database is needed).
    ax_tz_local(p, cap) {
      try { return give(localTzif(), p, cap); } catch { return -2; }
    },
    // WebAssembly cannot ask the processor (std/os/cpu.ax): no extensions.
    ax_cpu_features: () => 0n,
    ax_cpu_count: () => (os.availableParallelism ? os.availableParallelism() : os.cpus().length),
    ax_sleep_ns(ns) {
      Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, Number(ns) / 1e6);
    },
    // Cryptographically secure random bytes (at most 64 KiB per call to
    // getRandomValues, which cannot fill shared memory directly).
    ax_random(p, n) {
      const out = bytes(p, n);
      for (let at = 0; at < out.length; at += 65536) {
        const chunk = new Uint8Array(Math.min(65536, out.length - at));
        globalThis.crypto.getRandomValues(chunk);
        out.set(chunk, at);
      }
      return 0;
    },
    // Nanoseconds on clock 0 (real time since 1970), 1 (monotonic), 2 (the
    // processor time of the process, in microseconds) or 3 (that of the
    // calling thread where node has it, process.threadCpuUsage; else the
    // process's).
    ax_clock_ns(clock) {
      if (clock === 1) return process.hrtime.bigint();
      if (clock === 3 && typeof process.threadCpuUsage === 'function') {
        const u = process.threadCpuUsage();
        return BigInt(u.user + u.system) * 1000n;
      }
      if (clock === 2 || clock === 3) { const u = process.cpuUsage(); return BigInt(u.user + u.system) * 1000n; }
      return BigInt(Math.round((performance.timeOrigin + performance.now()) * 1000)) * 1000n;  // microseconds
    },
  };
  const host = {
    axiom,  // the host functions themselves, for tests of the host
    imports: { axiom },
    setMemory(m) { memory = m; },
    memory() { return memory; },
    // a request to the supervisor (on a worker of a supervised program)
    superCall(op, args) { return sup.call(op, args); },
    close() { if (netClient) netClient.close(); },
    // Builds the import object for `mod`: the host ABI, the shared memory if
    // the module imports one, `extra`, and trapping stubs for anything else
    // (e.g. main-thread-only helpers reached from other threads).
    instantiateModule(mod, extra = {}) {
      module = mod;
      const imports = {};
      // (on wasm64, the functions whose result is an isize or usize return an
      // i64: a BigInt, where the others return a Number)
      const wide = WebAssembly.Module.exports(mod).some((e) => e.name === 'axiom_wasm64');
      for (const imp of WebAssembly.Module.imports(mod)) {
        imports[imp.module] ||= {};
        let v;
        if (imp.module === 'axiom' && axiom[imp.name] && wide) {
          // (pointer and size arguments as Numbers, size results as BigInts)
          const f = axiom[imp.name], keep = I64_PARAMS[imp.name] || [], size = SIZE_RESULTS.has(imp.name);
          v = (...a) => {
            for (let k = 0; k < a.length; k++) if (typeof a[k] === 'bigint' && !keep.includes(k)) a[k] = Number(a[k]);
            const r = f(...a);
            return size && typeof r === 'number' ? BigInt(r) : r;
          };
        } else if (imp.module === 'axiom' && axiom[imp.name]) v = axiom[imp.name];
        else if (imp.kind === 'memory') v = memory;
        else if (extra[imp.module] && extra[imp.module][imp.name] !== undefined) v = extra[imp.module][imp.name];
        else if (imp.kind === 'function') v = () => { throw new Error(`host import ${imp.module}.${imp.name} is not available`); };
        imports[imp.module][imp.name] = v;
      }
      const instance = new WebAssembly.Instance(mod, imports);
      if (!memory) memory = instance.exports.memory;
      return instance;
    },
    instantiate(bytes, extra = {}) {
      const lim = memoryImportLimits(bytes);
      if (lim && !memory) memory = newMemory(lim);
      return host.instantiateModule(new WebAssembly.Module(bytes), extra);
    },
  };
  return host;
}

// Sampling profiler (spec §16.6): a worker samples the main thread's shadow
// stack (at ss; wide: wasm64); the result is written in the raw format read
// by `axiom profile` (see std/internal/runtime/profile.c).
function startProfiler({ ss, wide }, memory, hz) {
  if (!(memory.buffer instanceof SharedArrayBuffer)) throw new Error('--profile needs a program built by axiom profile (shared memory)');
  const control = new SharedArrayBuffer(8);
  const instance = { exports: { axiom_wasm64: wide ? 1 : undefined } };  // (all readSite needs)
  const w = new Worker(new URL('./profile_worker.mjs', import.meta.url),
    { workerData: { memory, ss, wide, hz, control } });
  let ready;
  const started = new Promise((resolve) => { ready = resolve; });
  const result = new Promise((resolve, reject) => {
    w.on('message', (m) => (m === 'ready' ? ready() : resolve(m)));
    w.once('error', reject);
  });
  return {
    started,
    async stop(entry, outPath) {
      const ctl = new Int32Array(control);
      Atomics.store(ctl, 0, 1);
      Atomics.store(ctl, 1, 1);
      Atomics.notify(ctl, 1);
      const samples = await result;
      await w.terminate();
      let text = `# axiom-profile-raw 1\nhz ${hz}\nclock wall\n`;
      const cache = new Map();
      const site = (a) => {
        if (!cache.has(a)) {
          let s = null;
          try { s = readSite(instance, memory, a); } catch { s = null; }
          cache.set(a, s && s.code === 255 ? s : null);
        }
        return cache.get(a);
      };
      for (const [key, count] of samples) {
        const [depth, ...addrs] = key.split(',').filter((x) => x !== '').map(Number);
        const sites = addrs.map(site);
        if (sites.some((s) => !s)) continue;  // torn read
        const leaf = sites.length ? (sites[0].callee || '<indirect call>') : entry;
        text += `S\t${count}\t${depth}\t${leaf}` + sites.map((s) => `\t${s.fileId}\x1f${s.start}\x1f${s.end}\x1f${s.fn}`).join('') + '\n';
      }
      fs.writeFileSync(outPath, text);
    },
  };
}

async function main() {
  const args = process.argv.slice(2);
  const profile = (args.find((a) => a.startsWith('--profile=')) || '').slice(10);
  const hz = Number((args.find((a) => a.startsWith('--profile-hz=')) || '--profile-hz=999').slice(13));
  const dash = args.indexOf('--');
  const own = dash < 0 ? args : args.slice(0, dash);
  const [file, entry] = own.filter((a) => !a.startsWith('--'));
  if (!file) {
    console.error('usage: node std/internal/runtime/host.mjs [--profile=out.raw [--profile-hz=N]] [--root=dir | --files=memory] program.wasm [export] [-- args...]');
    process.exit(2);
  }
  const programArgs = [file, ...(dash < 0 ? [] : args.slice(dash + 1))];
  // (--files=memory: the program's files in memory; --root=dir: node's under dir)
  const files = own.includes('--files=memory') ? 'memory' : null;
  const root = (own.find((a) => a.startsWith('--root=')) || '').slice(7) || null;
  const bytes = fs.readFileSync(file);
  const lim = memoryImportLimits(bytes);
  if (lim && lim.shared) {
    await supervise(bytes, lim, entry, { args: programArgs, pos: new SharedArrayBuffer(POS_FDS * 8), files, root }, profile, hz);
    return;
  }
  if (profile) {
    console.error('axiom: --profile needs a program built by axiom profile (shared memory)');
    process.exit(2);
  }
  const host = createHost({ args: programArgs, files, root });
  const instance = host.instantiate(bytes);
  const name = entry || (instance.exports.main ? 'main' : 'run');
  const fn = instance.exports[name];
  if (typeof fn !== 'function') {
    console.error(`axiom: no export named '${entry || 'main'}'`);
    process.exit(2);
  }
  let code = 0;
  try {
    const r = fn();
    if (typeof r === 'number') code = r;
  } catch (e) {
    if (!(e instanceof WebAssembly.RuntimeError || e instanceof RangeError || e instanceof WebAssembly.Exception)) throw e;
    const t = describeTrap(instance, host.memory(), e);
    console.error(formatTrap(t));
    code = 134;
  }
  host.close();
  process.exit(code);
}

// A threaded program: its main thread on a worker (thread_worker.mjs, with
// workerData.main), this thread its supervisor, which ends the process with
// the status of the first thread to ask (ax_exit, or the main thread when
// its entry returns).
async function supervise(bytes, lim, entry, shared, profile, hz) {
  const module = new WebAssembly.Module(bytes);
  const memory = newMemory(lim);
  let profiler = null;
  const sup = new Supervisor({
    // the main thread, before it runs: starts the profiler on its shadow stack
    async profile_start(info) {
      profiler = startProfiler(info, memory, hz);
      await profiler.started;
      return 0;
    },
    // the main thread's entry returned (code), or was not there (-1)
    async main_done({ code, name }) {
      if (profiler) await profiler.stop(name, profile);
      process.exit(code);
    },
  });
  const ch = new MessageChannel();
  sup.attach(ch.port1);
  const w = new Worker(new URL('./thread_worker.mjs', import.meta.url), {
    workerData: { module, memory, shared, supervisor: { ctl: sup.ctl, port: ch.port2 },
                  main: { entry: entry || null, profile: !!profile } },
    transferList: [ch.port2],
    env: SHARE_ENV,
  });
  w.on('error', (e) => {
    process.stderr.write(`axiom: ${e && e.message}\n`);
    process.exit(134);
  });
  w.on('exit', (c) => process.exit(c === 0 ? 0 : 134));
  process.exit(await sup.ended());
}

if (process.argv[1] && new URL(import.meta.url).pathname === fs.realpathSync(process.argv[1])) main();
