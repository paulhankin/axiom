// Structural checks of a Mach-O executable written by the
// linker for --target=aarch64-macos or x86_64-macos (cmd/axiom/src/mlink.ax).
// Nothing here can run the program, so this checks what dyld and the kernel
// check: load commands, segment and section layout, the rebase and bind
// opcode streams, the symbol table, the entry point and the ad-hoc code
// signature; and the code's links: every stub jumps through a __got slot
// that dyld binds, and every call that the frame table records (each one's
// return address) goes to a function's start (a symbol) or a stub (on arm64
// every bl in __text, too).
// Returns a list of problems (empty when the file is sound) and a summary.
import crypto from 'node:crypto';

export function checkMachO(d, opts = {}) {
  const errs = [];
  const E = (m) => errs.push(m);
  const u32 = (o) => d.readUInt32LE(o), u64 = (o) => Number(d.readBigUInt64LE(o));
  const str = (o, n) => { const s = d.subarray(o, o + n); const z = s.indexOf(0); return s.subarray(0, z < 0 ? n : z).toString(); };
  const cstr = (o) => d.subarray(o, d.indexOf(0, o)).toString();
  const x64 = u32(4) === 0x01000007;
  if (u32(0) !== 0xfeedfacf || u32(12) !== 2 || !((u32(4) === 0x0100000c && u32(8) === 0) || (x64 && u32(8) === 3))) {
    return { errs: ['not an arm64 or x86-64 Mach-O executable'] };
  }
  if (opts.x86_64 !== undefined && opts.x86_64 !== x64) return { errs: [`a Mach-O executable for ${x64 ? 'x86-64' : 'arm64'}, not the target's`] };
  const ncmds = u32(16), sizeofcmds = u32(20);
  const segs = [], sects = [], cmds = new Map();
  let off = 32;
  for (let i = 0; i < ncmds; i++) {
    const cmd = u32(off), size = u32(off + 4);
    if (size % 8) E(`load command ${cmd.toString(16)} size ${size} is not 8-aligned`);
    if (cmd === 0x19) {
      const name = str(off + 8, 16);
      const [vmaddr, vmsize, fileoff, filesize] = [u64(off + 24), u64(off + 32), u64(off + 40), u64(off + 48)];
      segs.push({ name, vmaddr, vmsize, fileoff, filesize });
      for (let k = 0; k < u32(off + 64); k++) {
        const so = off + 72 + 80 * k;
        const sn = str(so, 16), sg = str(so + 16, 16);
        const addr = u64(so + 32), ssize = u64(so + 40), soff = u32(so + 48), salign = u32(so + 52), zerofill = [1, 0xc, 0x12].includes(u32(so + 64) & 0xff);
        sects.push({ sn, addr, ssize, soff });
        if (sg !== name) E(`section ${sn} has segment name ${sg} in ${name}`);
        if (addr < vmaddr || addr + ssize > vmaddr + vmsize) E(`section ${sn} lies outside its segment`);
        if (!zerofill && ssize && (soff < fileoff || soff + ssize > fileoff + filesize || soff - fileoff !== addr - vmaddr)) E(`section ${sn} file offset disagrees with its address`);
        if (addr % 2 ** salign) E(`section ${sn} is misaligned`);
      }
      if (name !== '__PAGEZERO' && (vmaddr % 0x4000 || fileoff % 0x4000 || vmsize % 0x4000)) E(`segment ${name} is not 16K aligned`);
    }
    if (!cmds.has(cmd)) cmds.set(cmd, off);
    off += size;
  }
  if (off !== 32 + sizeofcmds) E('sizeofcmds is wrong');
  const order = segs.map((s) => s.name).join(' ');
  if (order !== '__PAGEZERO __TEXT __DATA __LINKEDIT') return { errs: [...errs, `segments are ${order}`] };
  const [, text, data, le] = segs;
  if (le.fileoff + le.filesize !== d.length) E('__LINKEDIT does not end the file');
  const lo = text.vmaddr, hi = le.vmaddr + le.vmsize;
  for (const c of [0x80000022, 0x2, 0xb, 0x1d, 0x80000028, 0xc, 0x1b, 0x32]) if (!cmds.has(c)) E(`load command ${c.toString(16)} is missing`);
  if (errs.length) return { errs };

  const uleb = (s) => { let r = 0, sh = 1, x; do { x = d[s.i++]; r += (x & 0x7f) * sh; sh *= 128; } while (x & 0x80); return r; };
  const sleb = (s) => { let r = 0, sh = 1, x; do { x = d[s.i++]; r += (x & 0x7f) * sh; sh *= 128; } while (x & 0x80); return x & 0x40 ? r - sh : r; };
  const di = cmds.get(0x80000022);
  const [ro, rs, bo, bs, , , , , eo, es] = Array.from({ length: 10 }, (_, k) => u32(di + 8 + 4 * k));
  const segOf = { 1: text, 2: data };
  // Rebases: every rebased pointer must point into the image.
  const rebased = new Set();
  for (let s = { i: ro }, seg = 0, so = 0; s.i < ro + rs;) {
    const b = d[s.i++], op = b & 0xf0, imm = b & 0xf;
    if (op === 0x00) break;
    else if (op === 0x10) continue;
    else if (op === 0x20) { seg = imm; so = uleb(s); }
    else if (op === 0x30) so += uleb(s);
    else if (op === 0x50 || op === 0x60) {
      for (let n = op === 0x50 ? imm : uleb(s); n > 0; n--, so += 8) {
        const g = segOf[seg];
        if (!g || so + 8 > g.filesize) { E(`rebase at ${seg}:${so} is outside file-backed data`); continue; }
        const v = u64(g.fileoff + so);
        if (v < lo || v >= hi) E(`rebased pointer ${v.toString(16)} points outside the image`);
        rebased.add(`${seg}:${so}`);
      }
    } else { E(`unexpected rebase opcode ${op.toString(16)}`); break; }
  }
  // Binds: each from a library a load command names (libSystem is 1), into
  // __DATA, never also rebased.
  let ndylibs = 0;
  for (let k = 0, o = 32; k < ncmds; k++) { if (u32(o) === 0xc) ndylibs++; o += u32(o + 4); }
  let nbinds = 0;
  const bound = new Set();
  for (let s = { i: bo }, name = '', seg = 0, so = 0; s.i < bo + bs;) {
    const b = d[s.i++], op = b & 0xf0, imm = b & 0xf;
    if (op === 0x00) continue;
    else if (op === 0x10) { if (imm < 1 || imm > ndylibs) E(`bind to library ${imm}, of ${ndylibs}`); }
    else if (op === 0x20) { const o = uleb(s); if (o < 1 || o > ndylibs) E(`bind to library ${o}, of ${ndylibs}`); }
    else if (op === 0x30) { if (imm !== 0xe) E(`bind to special library ${imm}`); } // BIND_SPECIAL_DYLIB_FLAT_LOOKUP (-2)
    else if (op === 0x40) { name = cstr(s.i); s.i += name.length + 1; }
    else if (op === 0x50) continue;
    else if (op === 0x60) sleb(s);
    else if (op === 0x70) { seg = imm; so = uleb(s); }
    else if (op === 0x90) {
      nbinds++;
      if (seg !== 2 || so + 8 > data.filesize) E(`bind of ${name} is outside __DATA`);
      if (!name.startsWith('_')) E(`bind name ${name} lacks the leading underscore`);
      if (rebased.has(`${seg}:${so}`)) E(`${name} is both rebased and bound`);
      bound.add(data.vmaddr + so);
      so += 8;
    } else { E(`unexpected bind opcode ${op.toString(16)}`); break; }
  }
  // Symbols: locals, then sorted external definitions, then sorted imports.
  const st = cmds.get(0x2), [symoff, nsyms, stroff, strsize] = [u32(st + 8), u32(st + 12), u32(st + 16), u32(st + 20)];
  const dy = cmds.get(0xb), [il, nl, ie, ne, iu, nu] = Array.from({ length: 6 }, (_, k) => u32(dy + 8 + 4 * k));
  if (il !== 0 || ie !== nl || iu !== nl + ne || iu + nu !== nsyms) E('LC_DYSYMTAB ranges do not cover the symbol table');
  const names = [], defined = new Map(), starts = new Set();
  for (let k = 0; k < nsyms; k++) {
    const o = symoff + 16 * k, ty = d[o + 4], desc = d.readUInt16LE(o + 6), val = u64(o + 8);
    const nm = cstr(stroff + u32(o));
    names.push(nm);
    if ((ty & 0x0e) === 0x0e) { defined.set(nm, val); starts.add(val); }
    if (k < nl && ty & 1) E(`local symbol ${nm} is external`);
    if (k >= nl && k < iu && ty !== 0x0f) E(`external symbol ${nm} has type ${ty}`);
    if (k >= iu && (ty !== 0x01 || (((desc >> 8) < 1 || (desc >> 8) > ndylibs) && (desc >> 8) !== 0xfe))) E(`import ${nm} is malformed`);
    if ((ty & 0x0e) === 0x0e && (val < lo || val >= hi)) E(`symbol ${nm} points outside the image`);
  }
  const sorted = (a) => a.every((x, k) => k === 0 || Buffer.compare(Buffer.from(a[k - 1]), Buffer.from(x)) < 0);
  if (!sorted(names.slice(ie, ie + ne)) || !sorted(names.slice(iu))) E('external symbols are not sorted');
  if (!names.slice(ie, ie + ne).includes('_main')) E('_main is not exported');
  // The linkedit blobs are in order, aligned, and end the file.
  const cs = cmds.get(0x1d), csoff = u32(cs + 8), cssize = u32(cs + 12);
  let end = le.fileoff;
  for (const [n, b, s] of [['rebase', ro, rs], ['bind', bo, bs], ['export', eo, es], ['symtab', symoff, nsyms * 16], ['strtab', stroff, strsize], ['signature', csoff, cssize]]) {
    if (!s) continue;
    if (b < end || b % 8) E(`${n} info is out of order or misaligned`);
    end = b + s;
  }
  if (end !== d.length || csoff % 16) E('the code signature does not end the file');
  const tsect = sects.find((s) => s.sn === '__text');
  const entry = u64(cmds.get(0x80000028) + 8);
  if (entry < tsect.soff || entry >= tsect.soff + tsect.ssize) E('the entry point is outside __text');
  // The ad-hoc signature: one CodeDirectory hashing every 4K page with SHA-256.
  const be = (o) => d.readUInt32BE(csoff + o);
  if (be(0) !== 0xfade0cc0 || be(8) !== 1) E('the signature superblob is malformed');
  const cd = be(16);
  const at = (o) => d.readUInt32BE(csoff + cd + o);
  const hoff = at(16), ncode = at(28), limit = at(32);
  if (at(0) !== 0xfade0c02 || at(8) !== 0x20400 || at(12) !== 0x20002 || d[csoff + cd + 36] !== 32 || d[csoff + cd + 37] !== 2 || d[csoff + cd + 39] !== 12) E('the CodeDirectory header is wrong');
  if (limit !== csoff || ncode !== Math.ceil(limit / 4096)) E('the CodeDirectory does not cover the file');
  if (Number(d.readBigUInt64BE(csoff + cd + 72)) !== text.filesize || Number(d.readBigUInt64BE(csoff + cd + 80)) !== 1) E('execSeg fields are wrong');
  let bad = 0;
  for (let k = 0; k < ncode; k++) {
    const want = crypto.createHash('sha256').update(d.subarray(k * 4096, Math.min((k + 1) * 4096, limit))).digest();
    if (!want.equals(d.subarray(csoff + cd + hoff + 32 * k, csoff + cd + hoff + 32 * (k + 1)))) bad++;
  }
  if (bad) E(`${bad} page hashes are wrong`);
  const links = checkLinks(d, { x64, E, tsect, text, bound, defined, starts });
  return { errs, imports: names.slice(iu), rebases: rebased.size, binds: nbinds, pages: ncode, ...links };
}

// The stubs at the end of __text (adrp x16, ldr x16, br x16 on arm64; jmp
// [rip + slot], int3, int3 on x86-64), each through a bound __got slot; the
// calls the frame table (__start_axiom_rets ... __stop_axiom_rets: entries
// of a return address and a chain, each relative to itself) records, each
// to a function's start or a stub.
function checkLinks(d, { x64, E, tsect, text, bound, defined, starts }) {
  const fileOf = (a) => a - text.vmaddr + text.fileoff;
  const tend = tsect.addr + tsect.ssize;
  const stubs = new Set();
  const size = x64 ? 8 : 12;
  for (let a = tend - size; a >= tsect.addr; a -= size) {
    const o = fileOf(a);
    let slot = -1;
    if (x64) {
      if (d[o] !== 0xff || d[o + 1] !== 0x25 || d[o + 6] !== 0xcc || d[o + 7] !== 0xcc) break;
      slot = a + 6 + d.readInt32LE(o + 2);
    } else {
      const w0 = d.readUInt32LE(o), w1 = d.readUInt32LE(o + 4), w2 = d.readUInt32LE(o + 8);
      if (((w0 & 0x9f00001f) >>> 0) !== 0x90000010 || ((w1 & 0xffc003ff) >>> 0) !== 0xf9400210 || w2 !== 0xd61f0200) break;
      const imm = ((w0 >> 29) & 3) | (((w0 >> 5) & 0x7ffff) << 2);
      slot = (a - (a % 4096)) + (imm >= 0x100000 ? imm - 0x200000 : imm) * 4096 + ((w1 >> 10) & 0xfff) * 8;
    }
    if (!bound.has(slot)) E(`the stub at 0x${a.toString(16)} jumps through 0x${slot.toString(16)}, which dyld does not bind`);
    stubs.add(a);
  }
  const ok = (t) => starts.has(t) || stubs.has(t);
  let calls = 0;
  const lo = defined.get('__start_axiom_rets'), hi = defined.get('__stop_axiom_rets');
  if (lo !== undefined && hi !== undefined) {
    for (let e = lo; e < hi; e += 8) {
      const eo = fileOf(e);
      const ra = e + d.readInt32LE(eo), chain = d.readInt32LE(eo + 4);
      if ((chain & 2) || ra < tsect.addr + 4 || ra > tend) continue;  // (a marker: no call)
      const ro = fileOf(ra);
      if (x64 && ra >= tsect.addr + 5 && d[ro - 5] === 0xe8) {
        const t = ra + d.readInt32LE(ro - 4);
        // (or an indirect call whose bytes before it happen to hold 0xe8:
        // call reg, with or without REX.B, or call [rip + disp32])
        const indirect = (d[ro - 2] === 0xff && (d[ro - 1] & 0xf8) === 0xd0) ||
          (d[ro - 3] === 0x41 && d[ro - 2] === 0xff && (d[ro - 1] & 0xf8) === 0xd0) || (d[ro - 6] === 0xff && d[ro - 5] === 0x15);
        if (ok(t) || !indirect) calls++;
        if (!ok(t) && !indirect) E(`the call before 0x${ra.toString(16)} goes to 0x${t.toString(16)}, no function's start`);
      }
    }
  }
  if (!x64) {
    for (let a = tsect.addr; a < tend; a += 4) {
      const w = d.readUInt32LE(fileOf(a));
      if (((w & 0xfc000000) >>> 0) !== 0x94000000) continue;
      calls++;
      const imm = w & 0x3ffffff, t = a + (imm >= 0x2000000 ? imm - 0x4000000 : imm) * 4;
      if (!ok(t)) E(`the bl at 0x${a.toString(16)} goes to 0x${t.toString(16)}, no function's start`);
    }
  }
  return { stubs: stubs.size, calls };
}
