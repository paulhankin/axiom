// Structural checks of a Mach-O executable written by the
// linker for --target=aarch64-macos or x86_64-macos (cmd/axiom/src/mlink.ax).
// Nothing here can run the program, so this checks what dyld and the kernel
// check: load commands, segment and section layout, the rebase and bind
// opcode streams, the symbol table, the entry point and the ad-hoc code
// signature; and the code's links: every stub jumps through a __got slot
// that dyld binds, and every call that the frame table records (each one's
// return address) goes to a function's start (a symbol) or a stub (on arm64
// every bl in __text, too); the unwind tables (checkUnwind); and the
// thread-local variables, which are the platform's (checkTLV).
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
        sects.push({ sn, addr, ssize, soff, flags: u32(so + 64) });
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
  const bound = new Set(), bindNames = new Map();
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
      bindNames.set(data.vmaddr + so, name);
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
  const unwind = checkUnwind(d, { x64, E, sects, text, tsect, starts, stubs: links.stubAddrs });
  const tlv = checkTLV(d, { x64, E, sects, data, tsect, rebased, bindNames, flags: u32(24), platform: opts.platformTLS });
  delete links.stubAddrs;
  return { errs, imports: names.slice(iu), rebases: rebased.size, binds: nbinds, pages: ncode, ...links, unwind, tlv };
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
  return { stubs: stubs.size, calls, stubAddrs: stubs };
}

// The unwind tables (cmd/axiom/src/macunwind.ax): __unwind_info decoded as
// the system's unwinder reads it (<mach-o/compact_unwind_encoding.h>): the
// header, the first-level index (in order, its sentinel the end of the
// code), each second-level page (regular or compressed: in bounds, its
// entries in order and within its range) and each encoding (a mode of the
// architecture with well-formed fields; the DWARF mode's offset an FDE of
// __eh_frame that starts at the function and covers it); __eh_frame's
// entries (lengths, each FDE's CIE, augmentations); and every function (a
// symbol in __text before the stubs) has an entry with an encoding.
function checkUnwind(d, { x64, E, sects, text, tsect, starts, stubs }) {
  const ui = sects.find((s) => s.sn === '__unwind_info'), eh = sects.find((s) => s.sn === '__eh_frame');
  if (!ui) { E('there is no __unwind_info'); return {}; }
  const u32 = (o) => d.readUInt32LE(o), u16 = (o) => d.readUInt16LE(o);
  const base = text.vmaddr, uo = ui.soff, n = ui.ssize;
  const inUi = (o, len) => o >= 0 && o + len <= n;
  // __eh_frame: CIEs and FDEs (pc-relative 4-byte addresses: "zR" 0x1b)
  const fdes = new Map();
  if (eh && eh.ssize) {
    const cies = new Map();
    for (let at = 0; at < eh.ssize;) {
      const len = u32(eh.soff + at);
      if (len === 0 || len === 0xffffffff || at + 4 + len > eh.ssize) { E(`__eh_frame entry at ${at} has length ${len}`); break; }
      const id = u32(eh.soff + at + 4);
      if (id === 0) {
        const z = d.indexOf(0, eh.soff + at + 9), aug = d.subarray(eh.soff + at + 9, z).toString();
        if (d[eh.soff + at + 8] !== 1 || !/^z[PLR]*S?$/.test(aug)) { E(`__eh_frame CIE at ${at}: version ${d[eh.soff + at + 8]}, augmentation "${aug}"`); break; }
        // (code and data alignment, the return address register, the
        // augmentation data's length; then 'R': the FDEs' pointer encoding)
        const s = { i: z + 1 };
        const leb = () => { let x; do { x = d[s.i++]; } while (x & 0x80); };
        leb(); leb(); s.i++; leb();
        let enc = 0;
        for (const c of aug.slice(1)) {
          if (c === 'R') enc = d[s.i++];
          else if (c === 'L') s.i++;
          else if (c === 'P') { const pe = d[s.i++]; s.i += (pe & 0xf) === 3 || (pe & 0xf) === 0xb ? 4 : 8; }
        }
        if ((enc & 0x70) !== 0x10 || ![0, 3, 4, 0xb, 0xc].includes(enc & 0xf)) E(`__eh_frame CIE at ${at}: FDE pointer encoding 0x${enc.toString(16)}`);
        cies.set(at, enc);
      } else {
        const enc = cies.get(at + 4 - id);
        if (enc === undefined) E(`__eh_frame FDE at ${at} has no CIE at ${at + 4 - id}`);
        else if ((enc & 0xf) === 3 || (enc & 0xf) === 0xb) {
          fdes.set(at, { pc: eh.addr + at + 8 + d.readInt32LE(eh.soff + at + 8), range: u32(eh.soff + at + 12) });
        } else {
          fdes.set(at, { pc: eh.addr + at + 8 + Number(d.readBigInt64LE(eh.soff + at + 8)), range: Number(d.readBigUInt64LE(eh.soff + at + 16)) });
        }
      }
      at += 4 + len;
    }
  }
  if (u32(uo) !== 1) E(`__unwind_info version ${u32(uo)}`);
  const [ceo, cen, peo, pen, io, icount] = [4, 8, 12, 16, 20, 24].map((k) => u32(uo + k));
  if (!inUi(ceo, 4 * cen) || !inUi(peo, 4 * pen) || !inUi(io, 12 * icount) || icount < 2) { E('__unwind_info header is out of bounds'); return {}; }
  const common = Array.from({ length: cen }, (_, k) => u32(uo + ceo + 4 * k));
  const ents = [];
  const tlo = tsect.addr - base, thi = tsect.addr + tsect.ssize - base;
  let prev = -1;
  for (let k = 0; k < icount; k++) {
    const e = uo + io + 12 * k, fo = u32(e), po = u32(e + 4);
    if (fo <= prev) E(`__unwind_info index entry ${k} is out of order`);
    prev = fo;
    if (fo < tlo || fo > thi) E(`__unwind_info index entry ${k} is outside __text`);
    if (k === icount - 1) { if (po !== 0) E('the index sentinel has a page'); break; }
    const next = u32(e + 12);
    if (!inUi(po, 8)) { E(`second-level page ${k} is out of bounds`); continue; }
    const kind = u32(uo + po);
    if (kind === 2) {
      const eo = u16(uo + po + 4), cnt = u16(uo + po + 6);
      if (!inUi(po + eo, 8 * cnt) || eo < 8) { E(`regular page ${k} is out of bounds`); continue; }
      for (let j = 0; j < cnt; j++) ents.push({ off: u32(uo + po + eo + 8 * j), enc: u32(uo + po + eo + 8 * j + 4), page: k, first: fo, next });
    } else if (kind === 3) {
      const eo = u16(uo + po + 4), cnt = u16(uo + po + 6), xo = u16(uo + po + 8), xn = u16(uo + po + 10);
      if (!inUi(po + eo, 4 * cnt) || !inUi(po + xo, 4 * xn)) { E(`compressed page ${k} is out of bounds`); continue; }
      for (let j = 0; j < cnt; j++) {
        const w = u32(uo + po + eo + 4 * j), ix = w >>> 24;
        const enc = ix < cen ? common[ix] : ix - cen < xn ? u32(uo + po + xo + 4 * (ix - cen)) : undefined;
        if (enc === undefined) E(`compressed page ${k} entry ${j} has no encoding ${ix}`);
        ents.push({ off: fo + (w & 0xffffff), enc: enc ?? 0, page: k, first: fo, next });
      }
    } else E(`second-level page ${k} has kind ${kind}`);
  }
  const end = prev;
  let dwarf = 0, none = 0;
  for (let j = 0; j < ents.length; j++) {
    const e = ents[j];
    if (j > 0 && e.off <= ents[j - 1].off) E(`unwind entry ${j} (0x${e.off.toString(16)}) is out of order`);
    if (e.off < e.first || e.off >= e.next) E(`unwind entry 0x${e.off.toString(16)} is outside its page's range`);
    const mode = e.enc & 0x0f000000, x = e.enc & 0x00ffffff;
    if (e.enc === 0) { none++; continue; }
    const fend = j + 1 < ents.length ? ents[j + 1].off : end;
    if ((x64 && mode === 0x04000000) || (!x64 && mode === 0x03000000)) {
      dwarf++;
      const f = fdes.get(x);
      if (!f) E(`unwind entry 0x${e.off.toString(16)} names no FDE at ${x} of __eh_frame`);
      else if (f.pc !== base + e.off || f.pc + f.range > base + fend) E(`the FDE of the function at 0x${e.off.toString(16)} covers 0x${f.pc.toString(16)}+${f.range}`);
    } else if (x64 && mode === 0x01000000) {
      const regs = [0, 1, 2, 3, 4].map((k) => (e.enc >> (3 * k)) & 7), used = regs.filter((r) => r);
      if ((e.enc & 0x8000) || used.some((r) => r > 5) || new Set(used).size !== used.length || ((e.enc >> 16) & 0xff) < used.length) E(`the RBP_FRAME encoding 0x${e.enc.toString(16)} is malformed`);
    } else if (x64 && mode === 0x02000000) {
      if (((e.enc >> 16) & 0xff) === 0) E(`the STACK_IMMD encoding 0x${e.enc.toString(16)} has no return address`);
    } else if (!x64 && mode === 0x04000000) {
      if (x & ~0xf1f) E(`the FRAME encoding 0x${e.enc.toString(16)} is malformed`);
    } else if (!x64 && mode === 0x02000000) {
      if (x & 0xfff) E(`the FRAMELESS encoding 0x${e.enc.toString(16)} saves registers`);
    } else E(`unwind entry 0x${e.off.toString(16)} has encoding 0x${e.enc.toString(16)}`);
  }
  // every function has an entry: its own, or the one of the function it is
  // a label in (with an encoding)
  const stubLo = stubs.size ? Math.min(...stubs) : tsect.addr + tsect.ssize;
  let funcs = 0;
  for (const a of starts) {
    if (a < tsect.addr || a >= stubLo) continue;
    funcs++;
    let lo = 0, hi = ents.length;
    while (lo < hi) { const m = (lo + hi) >> 1; if (ents[m].off <= a - base) lo = m + 1; else hi = m; }
    // (an entry of encoding 0 says there is no information: an assembly
    // source's function without .cfi_ directives; macho.mjs's callers that
    // know there is none such check `none`)
    if (lo === 0 || (ents[lo - 1].enc === 0 && ents[lo - 1].off !== a - base)) E(`the function at 0x${a.toString(16)} has no unwind entry`);
  }
  if (end > stubLo - base) E('__unwind_info covers the stubs');
  return { entries: ents.length, dwarf, none, funcs, fdes: fdes.size };
}

// The thread-local variables (mlink.ax): __thread_vars, __thread_data and
// __thread_bss of their types, in that order, the template's two parts
// adjacent, MH_HAS_TLV_DESCRIPTORS in the header; each descriptor three
// words: its thunk bound to __tlv_bootstrap (dyld replaces it), a zero key,
// the variable's offset in the template (neither rebased); and with
// opts.platformTLS (a program of --macos-tls=platform) nothing of the
// runtime's scheme in the code (arm64: no read of TPIDRRO_EL0, the TSD
// slots). Returns the number of descriptors.
function checkTLV(d, { x64, E, sects, data, tsect, rebased, bindNames, flags, platform }) {
  const vars = sects.find((s) => s.sn === '__thread_vars');
  const tdata = sects.find((s) => s.sn === '__thread_data'), tbss = sects.find((s) => s.sn === '__thread_bss');
  if (!x64 && platform) {
    for (let o = tsect.soff; o + 4 <= tsect.soff + tsect.ssize; o += 4) {
      if (((d.readUInt32LE(o) & 0xffffffe0) >>> 0) === 0xd53bd060) { E(`mrs of TPIDRRO_EL0 at ${(tsect.addr + o - tsect.soff).toString(16)}`); break; }
    }
  }
  if (!vars) {
    if (tdata || tbss || (flags & 0x800000)) E('thread-local sections or MH_HAS_TLV_DESCRIPTORS without __thread_vars');
    return 0;
  }
  if (!(flags & 0x800000)) E('__thread_vars without MH_HAS_TLV_DESCRIPTORS');
  if ((vars.flags & 0xff) !== 0x13) E('__thread_vars is not S_THREAD_LOCAL_VARIABLES');
  if (!tdata || (tdata.flags & 0xff) !== 0x11) E('__thread_data is missing or not S_THREAD_LOCAL_REGULAR');
  if (!tbss || (tbss.flags & 0xff) !== 0x12) E('__thread_bss is missing or not S_THREAD_LOCAL_ZEROFILL');
  if (!tdata || !tbss) return 0;
  if (tbss.addr < tdata.addr + tdata.ssize || tbss.addr - (tdata.addr + tdata.ssize) >= 4096) E('the template\'s parts are not adjacent');
  const tsize = tbss.addr + tbss.ssize - tdata.addr;
  if (vars.ssize % 24) E('__thread_vars is not a whole number of descriptors');
  const n = vars.ssize / 24, segOff = (a) => a - data.vmaddr;
  for (let k = 0; k < n; k++) {
    const a = vars.addr + 24 * k, fo = vars.soff + 24 * k;
    if (bindNames.get(a) !== '__tlv_bootstrap') E(`descriptor ${k}'s thunk is not bound to __tlv_bootstrap`);
    if (Number(d.readBigUInt64LE(fo + 8)) !== 0) E(`descriptor ${k}'s key is not 0`);
    if (Number(d.readBigUInt64LE(fo + 16)) >= Math.max(tsize, 1)) E(`descriptor ${k}'s offset is outside the template`);
    for (const w of [0, 8, 16]) if (rebased.has(`2:${segOff(a + w)}`)) E(`descriptor ${k}'s word ${w / 8} is rebased`);
  }
  return n;
}
