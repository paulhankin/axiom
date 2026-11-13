// Structural checks of a Windows executable written by the linker
// (cmd/axiom/src/plink.ax) for --target=x86_64-windows or aarch64-windows,
// and of its program database (cmd/axiom/src/pdb.ax). It checks what the
// loader and the system's unwinder rely on, without running the program:
//
//   the headers (PE32+, the machine, a console program with ASLR), the
//   sections and data directories; the imports (each DLL's lookup and
//   address tables, every entry a function named by a hint/name entry);
//   the base relocations (each one an address within the image); the TLS
//   directory; the exception directory (sorted, each entry within .text).
//
// For ARM64 also the code, with the functions the PDB lists (their offsets
// and sizes): every function's unwind record decodes (the format of the
// Windows ARM64 exception-handling specification), describes the function's
// length, and its codes match its prologue's instructions and each
// epilogue's, one code per instruction; every function that saves its frame
// record or calls has a record, and every return of such a function ends a
// described epilogue; every bl lands on a function's start or on an import
// thunk, every b within its function or on a function's start (a sibling
// call) or a thunk; every thunk loads an import address table slot.
// Returns { errs, ... } (errs empty when the file is sound).
import fs from 'node:fs';
import { checkPDB } from './pdb.mjs';

export function checkPE(d, opts = {}) {
  const errs = [];
  const E = (m) => { if (errs.length < 50) errs.push(m); };
  const u16 = (o) => d.readUInt16LE(o), u32 = (o) => d.readUInt32LE(o), u64 = (o) => Number(d.readBigUInt64LE(o));
  if (d.length < 0x40 || u16(0) !== 0x5a4d) return { errs: ['no MZ header'] };
  const pe = u32(0x3c);
  if (u32(pe) !== 0x4550) return { errs: ['no PE signature'] };
  const machine = u16(pe + 4), arm64 = machine === 0xaa64;
  if (machine !== 0x8664 && !arm64) return { errs: [`machine ${machine.toString(16)}`] };
  if (opts.arm64 !== undefined && opts.arm64 !== arm64) return { errs: [`machine ${machine.toString(16)}, not the target's`] };
  const nsec = u16(pe + 6), opt = pe + 24;
  if (u16(pe + 20) !== 240 || u16(opt) !== 0x20b) return { errs: ['not PE32+'] };
  if ((u16(pe + 22) & 0x22) !== 0x22) E('not an executable image that is large-address aware');
  const subsystem = u16(opt + 68);
  if (subsystem !== (opts.subsystem || 3)) E(`subsystem ${subsystem}`);
  if ((u16(opt + 70) & 0x60) !== 0x60) E('no (high-entropy) ASLR');
  const base = u64(opt + 24), sizeImage = u32(opt + 56), entry = u32(opt + 16);
  const salign = u32(opt + 32), falign = u32(opt + 36);
  const secs = [];
  for (let k = 0; k < nsec; k++) {
    const s = opt + 240 + 40 * k;
    const name = d.toString('latin1', s, s + 8).replace(/\0+$/, '');
    secs.push({ name, vsize: u32(s + 8), rva: u32(s + 12), rsize: u32(s + 16), raw: u32(s + 20), flags: u32(s + 36) });
  }
  const names = secs.map((s) => s.name).join(' ');
  if (names !== '.text .rdata .idata .data .reloc' && names !== '.text .rdata .idata .data .reloc .rsrc') E(`sections ${names}`);
  let prev = 0;
  for (const s of secs) {
    if (s.rva % salign || s.raw % falign || s.rva < prev) E(`section ${s.name} is misplaced`);
    if (s.raw + s.rsize > d.length) E(`section ${s.name} lies past the file's end`);
    prev = s.rva + s.vsize;
  }
  if (prev > sizeImage) E('SizeOfImage is too small');
  const secOf = (rva) => secs.find((s) => rva >= s.rva && rva < s.rva + Math.max(s.vsize, s.rsize));
  // file offset of an RVA in a section's file data, or -1
  const off = (rva, n = 1) => {
    const s = secOf(rva);
    return s && rva + n <= s.rva + Math.min(s.vsize, s.rsize) ? s.raw + rva - s.rva : -1;
  };
  const cstr = (rva) => { const o = off(rva); return o < 0 ? null : d.toString('latin1', o, d.indexOf(0, o)); };
  const dir = (k) => ({ rva: u32(opt + 112 + 8 * k), size: u32(opt + 116 + 8 * k) });
  const text = secs[0];
  if (!(text.flags & 0x20000000) || secs.slice(1).some((s) => s.flags & 0x20000000)) E('only .text is executable');
  if (entry < text.rva || entry >= text.rva + text.vsize) E('the entry point is outside .text');

  // imports
  const imports = [], iat = new Map();
  const id = dir(1);
  for (let k = 0; ; k++) {
    const o = off(id.rva + 20 * k, 20);
    if (o < 0) { E('the import directory is outside the file data'); break; }
    const [ilt, , , nameRva, iatRva] = [u32(o), u32(o + 4), u32(o + 8), u32(o + 12), u32(o + 16)];
    if (!ilt && !nameRva && !iatRva) break;
    const dll = cstr(nameRva);
    if (!dll || !/\.dll$/i.test(dll)) E(`import descriptor ${k} names ${dll}`);
    for (let j = 0; ; j++) {
      const lo = off(ilt + 8 * j, 8), ao = off(iatRva + 8 * j, 8);
      if (lo < 0 || ao < 0) { E(`${dll}: the lookup tables are outside the file data`); break; }
      const e = u64(lo);
      if (u64(ao) !== e) E(`${dll}: the address table differs from the lookup table`);
      if (e === 0) break;
      if (e >= 2 ** 63) { E(`${dll}: an import by ordinal`); continue; }
      const nm = cstr(e + 2);
      if (!nm) { E(`${dll}: a hint/name entry outside the file data`); continue; }
      imports.push(`${dll}:${nm}`);
      iat.set(iatRva + 8 * j, `${dll}:${nm}`);
    }
  }
  const iatDir = dir(12);
  for (const a of iat.keys()) if (a < iatDir.rva || a >= iatDir.rva + iatDir.size) E('an import address table slot outside the IAT directory');

  // base relocations: every absolute address within the image
  const rd = dir(5);
  let nrel = 0;
  for (let at = rd.rva; at < rd.rva + rd.size;) {
    const o = off(at, 8);
    if (o < 0) { E('base relocations outside the file data'); break; }
    const page = u32(o), size = u32(o + 4);
    if (size < 8 || size % 4) { E('a malformed base relocation block'); break; }
    for (let k = 8; k < size; k += 2) {
      const e = u16(o + k), ty = e >> 12;
      if (ty === 0) continue;
      if (ty !== 10) { E(`base relocation type ${ty}`); continue; }
      const vo = off(page + (e & 0xfff), 8);
      if (vo < 0) { E('a base relocation outside the file data'); continue; }
      const v = u64(vo);
      if (v < base || v >= base + sizeImage) E(`a relocated address 0x${v.toString(16)} outside the image`);
      nrel++;
    }
    at += size;
  }

  // TLS directory
  const td = dir(9);
  if (td.size) {
    const o = off(td.rva, 40);
    if (o < 0) E('the TLS directory is outside the file data');
    else for (const k of [0, 8, 16]) { const v = u64(o + k); if (v < base || v >= base + sizeImage) E('a TLS directory address outside the image'); }
  }

  // the exception directory: sorted entries within .text
  const xd = dir(3), es = arm64 ? 8 : 12;
  const pdata = [];
  if (xd.size % es) E('the exception directory is not a whole number of entries');
  for (let k = 0; k * es < xd.size; k++) {
    const o = off(xd.rva + k * es, es);
    if (o < 0) { E('the exception directory is outside the file data'); break; }
    pdata.push(arm64 ? { begin: u32(o), info: u32(o + 4) } : { begin: u32(o), end: u32(o + 4), info: u32(o + 8) });
  }
  for (let k = 0; k < pdata.length; k++) {
    const p = pdata[k];
    if (k > 0 && p.begin <= pdata[k - 1].begin) E(`the function table is not sorted (0x${p.begin.toString(16)} after 0x${pdata[k - 1].begin.toString(16)})`);
    if (p.begin < text.rva || p.begin >= text.rva + text.vsize) E(`a function table entry at 0x${p.begin.toString(16)} outside .text`);
  }
  const r = { errs, machine, imports, relocs: nrel, functions: pdata.length };
  if (arm64) checkArm64Code(d, { E, off, text, pdata, iat, entry, funcs: opts.pdb ? pdbFunctions(opts.pdb, E) : null, r });
  else checkX64Unwind(d, { E, off, secOf, text, pdata, funcs: opts.pdb ? pdbFunctions(opts.pdb, E) : null, r });
  return r;
}

// The x64 function table and unwind information (the x64 exception-handling
// specification: RUNTIME_FUNCTION, UNWIND_INFO, UNWIND_CODE), decoded as the
// system's unwinder decodes them: each entry's range within .text, after the
// one before it; its UNWIND_INFO 4-aligned in read-only data, version 1 or
// 2, no handler (the compiler writes none), the
// prologue within the function, its codes whole (CountOfCodes slots, each
// operation its size), each a defined operation with its operand in range,
// in descending code offsets within the prologue, UWOP_SET_FPREG exactly
// when a frame register is named (and a frame offset only then). Then the
// prologue's instructions (cmd/axiom/src/x64emit.ax's prologue, and the
// linker's entry: push, mov rbp rsp, sub rsp, the stack probe's loop, xmm
// stores) are matched to the codes, in order: every instruction within
// SizeOfProlog that pushes, moves rsp, sets the frame register or saves a
// register has the code its end offset names, with its operand, and no
// code is left over; an instruction of another kind there is an error. A
// function with sibling calls has more entries (x64emit.ax's emit_unwind):
// each jump's, right after an entry of its function, without codes and
// holding just the jump, and the code after it, chained (UNW_FLAG_CHAININFO,
// no codes) to its function's first entry, with its frame register. A
// function the PDB lists that starts by pushing rbp has an entry, and in a
// function with an entry each epilogue (lea rsp / leave, then pops, then
// ret or a jmp out of the function) pops rbp last.
function checkX64Unwind(d, { E, off, secOf, text, pdata, funcs, r }) {
  const to = off(text.rva, text.vsize);
  if (to < 0) { E('.text is not all in the file'); return; }
  const byte = (rva) => d[to + rva - text.rva];
  const i32 = (rva) => d.readInt32LE(to + rva - text.rva);
  let ncodes = 0, nepi = 0;
  const OPS = { 0: 'push', 1: 'alloc_large', 2: 'alloc_small', 3: 'set_fpreg', 4: 'save_nonvol', 5: 'save_nonvol_far',
                6: 'epilog', 8: 'save_xmm128', 9: 'save_xmm128_far', 10: 'push_machframe' };
  const ranges = [];
  for (let k = 0; k < pdata.length; k++) {
    const p = pdata[k], at = `function table entry 0x${p.begin.toString(16)}`;
    if (p.end <= p.begin || p.end > text.rva + text.vsize) { E(`${at}: its end 0x${p.end.toString(16)} is not after its start within .text`); continue; }
    if (k > 0 && p.begin < pdata[k - 1].end) E(`${at} overlaps the one before it`);
    if (p.info & 3) { E(`${at}: its unwind information's address is not 4-aligned (or is an indirect entry)`); continue; }
    const s = secOf(p.info);
    if (!s || (s.flags & 0x20000000) || (s.flags & 0x80000000)) { E(`${at}: its unwind information is not in read-only data`); continue; }
    const xo = off(p.info, 4);
    if (xo < 0) { E(`${at}: its unwind information is outside the file data`); continue; }
    const ver = d[xo] & 7, flags = d[xo] >> 3, plen = d[xo + 1], n = d[xo + 2], freg = d[xo + 3] & 15, foff = d[xo + 3] >> 4;
    if (ver !== 1 && ver !== 2) { E(`${at}: unwind information version ${ver}`); continue; }
    if (flags & ~4) E(`${at}: unwind flags ${flags} (the compiler writes no handler)`);
    if (flags & 4) {
      // a chained entry (x64emit.ax's emit_unwind: the code after a sibling
      // call's jump): no codes, the frame register of its function's first
      // entry, which it names and which comes before it
      const co = off(p.info + 4 + 2 * ((n + 1) & ~1), 12);
      if (n || co < 0) { E(`${at}: a chained entry with codes, or its first entry outside the file data`); continue; }
      const first = { begin: d.readUInt32LE(co), end: d.readUInt32LE(co + 4), info: d.readUInt32LE(co + 8) };
      const f = ranges.find((x) => x.begin === first.begin && x.end === first.end && x.info === first.info);
      if (!f || f.chained || first.end > p.begin) E(`${at}: chained to 0x${first.begin.toString(16)}, which is not an earlier first entry`);
      else if (f.freg !== freg) E(`${at}: chained with frame register ${freg}, its first entry's is ${f.freg}`);
      ranges.push({ ...p, framed: true, chained: true, freg });
      continue;
    }
    if (n === 0 && !freg && plen === 0 && ranges.length && ranges[ranges.length - 1].end === p.begin && ranges[ranges.length - 1].framed) {
      // a sibling call's jump after its function's epilogue (emit_unwind): jmp rel32 or rex.w jmp reg
      const b0 = byte(p.begin), len = p.end - p.begin;
      const ok = (b0 === 0xe9 && len === 5) || ((b0 === 0x48 || b0 === 0x49) && byte(p.begin + 1) === 0xff && ((byte(p.begin + 2) >> 3) & 7) === 4 && len === 3);
      if (!ok) E(`${at}: an entry without codes after a function's that is not one jump`);
      ranges.push({ ...p, framed: false, freg: 0, jump: true });
      continue;
    }
    if (plen > p.end - p.begin) E(`${at}: a prologue of ${plen} bytes in a function of ${p.end - p.begin}`);
    if (n && off(p.info + 4, 2 * n) < 0) { E(`${at}: its unwind codes are outside the file data`); continue; }
    if (!freg && foff) E(`${at}: a frame offset without a frame register`);
    const codes = [];
    let bad = false;
    for (let i = 0; i < n;) {
      const c = xo + 4 + 2 * i, o = d[c], op = d[c + 1] & 15, info = d[c + 1] >> 4;
      const x = { off: o, op: OPS[op], info };
      let size = 1;
      if (op === 1) {
        if (info > 1) { E(`${at}: UWOP_ALLOC_LARGE with operation info ${info}`); bad = true; break; }
        size = info ? 3 : 2;
        if (i + size <= n) x.size = info ? d.readUInt32LE(c + 2) : d.readUInt16LE(c + 2) * 8;
      } else if (op === 2) x.size = info * 8 + 8;
      else if (op === 3) { if (!freg) E(`${at}: UWOP_SET_FPREG without a frame register`); }
      else if (op === 4 || op === 8) { size = 2; if (i + size <= n) x.disp = d.readUInt16LE(c + 2) * (op === 8 ? 16 : 8); }
      else if (op === 5 || op === 9) { size = 3; if (i + size <= n) x.disp = d.readUInt32LE(c + 2); }
      else if (op === 6 && ver === 2) size = 2;
      else if (op === 10) { if (info > 1) E(`${at}: UWOP_PUSH_MACHFRAME with operation info ${info}`); }
      else if (op !== 0) { E(`${at}: unwind operation ${op}${op === 6 ? ' in a version 1 record' : ''}`); bad = true; break; }
      if (i + size > n) { E(`${at}: unwind code ${i} runs past CountOfCodes`); bad = true; break; }
      if (o > plen) E(`${at}: an unwind code at offset ${o}, past the prologue's ${plen} bytes`);
      if (codes.length && o > codes[codes.length - 1].off) E(`${at}: unwind codes not in descending offsets`);
      codes.push(x);
      i += size;
    }
    if (bad) continue;
    ncodes += codes.length;
    const nfp = codes.filter((c) => c.op === 'set_fpreg').length;
    if (freg && nfp !== 1) E(`${at}: frame register ${freg} with ${nfp} UWOP_SET_FPREG codes`);
    // the prologue, from the function's start: each instruction and its code
    const want = [...codes].reverse();
    let a = p.begin, used = 0;
    // (the codes the same offset carries: the dynamic-stack form's, all at set_fpreg's)
    const take = (op, ok) => {
      const c = want[used], end = a - p.begin;
      if (!c || c.off !== end || c.op !== op || !ok(c)) {
        E(`${at}+0x${end.toString(16)}: the prologue's ${op} has ${c ? `the code ${c.op} at offset ${c.off}` : 'no code'}`);
        return false;
      }
      used++;
      return true;
    };
    // (the dynamic-stack form (x64emit.ax's dyn_unwind_info) describes the
    // saves from rbp: its codes for the pushes and stores after mov rbp, rsp
    // are UWOP_SAVE_* at the offset of their instruction, its set_fpreg and
    // the lift of the frame offset at mov rbp, rsp's)
    // (a prologue written by hand, assembly's .seh_* directives, may set rbp
    // with lea rbp, [rsp + d] instead: its pushes and allocations are coded as such)
    let movrbp = false;
    for (let x = p.begin; x + 3 <= p.begin + plen; x++) if (byte(x) === 0x48 && byte(x + 1) === 0x89 && byte(x + 2) === 0xe5) movrbp = true;
    const dyn = freg === 5 && movrbp;
    // (a machine frame at the entry: core_signal.ax's trampoline, plink.ax)
    while (want[used] && want[used].op === 'push_machframe' && want[used].off === 0) used++;
    while (a - p.begin < plen) {
      const b0 = byte(a), b1 = byte(a + 1), b2 = byte(a + 2);
      let ok = true;
      const rex41 = b0 === 0x41 && b1 >= 0x50 && b1 <= 0x57;
      if ((b0 >= 0x50 && b0 <= 0x57) || rex41) {
        const reg = rex41 ? b1 - 0x50 + 8 : b0 - 0x50;
        a += rex41 ? 2 : 1;
        if (dyn && reg !== 5) ok = take('save_nonvol_far', (c) => c.info === reg);
        else ok = take('push', (c) => c.info === reg);
      } else if (b0 === 0x48 && b1 === 0x89 && b2 === 0xe5) {
        a += 3;
        if (dyn) {
          // (in prologue order) the lift of 16 * FrameOffset, then set_fpreg, both at this offset
          const fo = d[off(p.info, 4) + 3] >> 4;
          if (fo) ok = take(want[used] && want[used].op === 'alloc_large' ? 'alloc_large' : 'alloc_small', (c) => c.size === 16 * fo);
          if (ok) ok = take('set_fpreg', () => true);
        }
      } else if (b0 === 0x48 && b1 === 0x8d && b2 === 0x6c && byte(a + 3) === 0x24 && !dyn) {
        // lea rbp, [rsp + d8]: the frame register set (assembly's .seh_setframe)
        a += 5;
        ok = take('set_fpreg', () => true);
      } else if (b0 === 0x48 && b1 === 0x83 && b2 === 0xec) {
        const v = byte(a + 3);
        a += 4;
        if (!dyn) ok = take('alloc_small', (c) => c.size === v);
      } else if (b0 === 0x48 && b1 === 0x81 && b2 === 0xec) {
        const v = i32(a + 3) >>> 0;
        a += 7;
        if (!dyn) ok = take(want[used] && want[used].op === 'alloc_small' ? 'alloc_small' : 'alloc_large', (c) => c.size === v);
      } else if (b0 === 0x49 && b1 === 0xc7 && b2 === 0xc3 && i32(a + 3) === -4096) {
        // the stack probe's loop (x64emit.ax's prologue): mov r11, -4096; L: or qword [rsp+r11], 0;
        // sub r11, 4096; cmp r11, -n; jne L: rsp does not move
        a += 7;
        const shape = [0x4a, 0x83, 0x0c, 0x1c, 0x00, 0x49, 0x81, 0xeb, 0x00, 0x10, 0x00, 0x00, 0x49, 0x81, 0xfb];
        if (shape.some((x, j) => byte(a + j) !== x) || byte(a + 19) !== 0x75 || byte(a + 20) !== 0xeb) { E(`${at}: a malformed stack probe`); break; }
        a += 21;
      } else if ((b0 === 0x0f && b1 === 0x11) || (b0 === 0x44 && b1 === 0x0f && b2 === 0x11)) {
        // movups [rbp + disp32], xmm
        const q = a + (b0 === 0x44 ? 2 : 1) + 1, m = byte(q);
        if ((m & 0xc7) !== 0x85) { E(`${at}+0x${(a - p.begin).toString(16)}: an xmm store in the prologue that is not to [rbp + disp32]`); break; }
        a = q + 5;
        const reg = ((m >> 3) & 7) + (b0 === 0x44 ? 8 : 0);
        ok = take(want[used] && want[used].op === 'save_xmm128' ? 'save_xmm128' : 'save_xmm128_far', (c) => c.info === reg);
      } else {
        E(`${at}+0x${(a - p.begin).toString(16)}: an instruction in the prologue (0x${b0.toString(16)} 0x${b1.toString(16)} 0x${b2.toString(16)}) that is not one the compiler writes there`);
        break;
      }
      if (!ok) break;
    }
    if (a - p.begin > plen) E(`${at}: the prologue's last instruction ends at +0x${(a - p.begin).toString(16)}, past SizeOfProlog ${plen}`);
    if (used < want.length) E(`${at}: ${want.length - used} unwind codes describe no instruction of the prologue (it ends at +0x${(a - p.begin).toString(16)})`);
    ranges.push({ ...p, framed: want.length > 0, freg });
  }
  const entryAt = (rva) => {
    let lo = 0, hi = ranges.length - 1;
    while (lo <= hi) { const m = (lo + hi) >> 1; if (ranges[m].end <= rva) lo = m + 1; else if (ranges[m].begin > rva) hi = m - 1; else return ranges[m]; }
    return null;
  };
  // epilogues: the instructions are not decoded; a ret or a jmp out of the
  // function after pops and a lea rsp / leave before them is one
  for (const p of ranges) {
    if (!p.framed) continue;
    for (let a = p.begin + 1; a < p.end; a++) {
      const b = byte(a);
      const out = b === 0xe9 && a + 5 <= p.end && ((t) => t < p.begin || t >= p.end)(a + 5 + i32(a + 1));
      if (b !== 0xc3 && !out) continue;
      let q = a;
      while (q - 1 >= p.begin && byte(q - 1) >= 0x58 && byte(q - 1) <= 0x5f) {
        q--;
        if (q - 1 >= p.begin && byte(q - 1) === 0x41) q--;
      }
      const leave = byte(q - 1) === 0xc9 && q === a;
      const lea8 = byte(q - 4) === 0x48 && byte(q - 3) === 0x8d && byte(q - 2) === 0x65;
      const lea32 = byte(q - 7) === 0x48 && byte(q - 6) === 0x8d && byte(q - 5) === 0xa5;
      if (!(leave || lea8 || lea32)) continue;  // (an immediate's or displacement's byte, not an epilogue)
      nepi++;
      if (!leave && byte(a - 1) !== 0x5d) E(`function 0x${p.begin.toString(16)}+0x${(a - p.begin).toString(16)}: an epilogue that does not pop rbp last`);
    }
  }
  // a function the PDB lists that sets up a frame has an entry
  if (funcs) {
    let missing = 0;
    for (const f of funcs) {
      const a = text.rva + f.off;
      if (byte(a) === 0x55 && byte(a + 1) === 0x48 && byte(a + 2) === 0x89 && byte(a + 3) === 0xe5 && !entryAt(a) && missing++ < 5) E(`${f.name} pushes rbp but has no function table entry`);
    }
    r.funcs = funcs.length;
  }
  r.unwindCodes = ncodes;
  r.epilogues = nepi;
}

// The functions a program database lists: [{ off (in section 1), size, name }]
// from its modules' procedure records (pdb.mjs reads them).
export function pdbFunctions(p, E = () => {}) {
  const r = checkPDB(p);
  for (const e of r.errs) E(`the PDB: ${e}`);
  return r.procs.filter((f) => f.sec === 1).map(({ off, size, name }) => ({ off, size, name })).sort((a, b) => a.off - b.off);
}

// The unwind codes of an ARM64 .xdata record (cmd/axiom/src/a64unwind.ax):
// [{ op, n (bytes), reg, off, size }] from byte i to the first end.
function decodeCodes(c, i0, E) {
  const ops = [];
  for (let i = i0; i < c.length;) {
    const b = c[i], b1 = c[i + 1];
    let x;
    if (b < 0x20) x = { op: 'alloc', size: b * 16, n: 1 };
    else if (b >= 0x80 && b < 0xc0) x = { op: 'save_fplr_x', off: ((b & 0x3f) + 1) * 8, n: 1 };
    else if (b >= 0xc0 && b < 0xc8) x = { op: 'alloc', size: (((b & 7) << 8) | b1) * 16, n: 2 };
    else if (b >= 0xc8 && b < 0xcc) x = { op: 'save_regp', reg: 19 + (((b & 3) << 2) | (b1 >> 6)), off: (b1 & 0x3f) * 8, n: 2 };
    else if (b >= 0xd0 && b < 0xd4) x = { op: 'save_reg', reg: 19 + (((b & 3) << 2) | (b1 >> 6)), off: (b1 & 0x3f) * 8, n: 2 };
    else if (b === 0xdc || b === 0xdd) x = { op: 'save_freg', reg: 8 + (((b & 1) << 2) | (b1 >> 6)), off: (b1 & 0x3f) * 8, n: 2 };
    else if (b === 0xe0) x = { op: 'alloc', size: ((c[i + 1] << 16) | (c[i + 2] << 8) | c[i + 3]) * 16, n: 4 };
    else if (b === 0xe1) x = { op: 'set_fp', n: 1 };
    else if (b === 0xe2) x = { op: 'add_fp', off: b1 * 8, n: 2 };
    else if (b === 0xe3) x = { op: 'nop', n: 1 };
    else if (b === 0xe4) return ops;
    else { E(`unwind code 0x${b.toString(16)} (not one the compiler writes)`); return ops; }
    ops.push(x);
    i += x.n;
  }
  E('unwind codes without an end');
  return ops;
}

// x16 after instruction w, if it is a movz, movn or movk of x16 (the
// prologue's large allocation: a64.ax's enc_movi), from x16 before it.
function movx16(w, x16) {
  const hw = BigInt((w >>> 21) & 3) * 16n, imm = BigInt((w >>> 5) & 0xffff) << hw, all = (1n << 64n) - 1n;
  const op = (w & 0xff80001f) >>> 0;
  let v = x16 === null ? null : BigInt(x16);
  if (op === 0xd2800010) v = imm;  // movz
  else if (op === 0x92800010) v = ~imm & all;  // movn
  else if (op === 0xf2800010 && v !== null) v = (v & ~(0xffffn << hw) & all) | imm;  // movk
  else return x16;
  return Number(v);
}

const SP = 31;
// Does instruction word w do what unwind code x undoes (prologue: the
// instruction the code describes; epilogue: the one that undoes it)? x16:
// the value movz/movk put in x16 before it (the prologue's large allocation).
function matches(w, x, epi, x16) {
  const imm12 = (w >> 10) & 0xfff, sh = (w >> 22) & 1, imm = imm12 << (sh ? 12 : 0);
  const rt = w & 31, rn = (w >> 5) & 31, rt2 = (w >> 10) & 31;
  const s7 = ((w >> 15) & 0x7f) - (((w >> 15) & 0x40) ? 128 : 0);
  switch (x.op) {
    case 'alloc':
      if (epi) return ((w & 0xff8003ff) >>> 0) === 0x910003ff && imm === x.size;  // add sp, sp, #n
      return (((w & 0xff8003ff) >>> 0) === 0xd10003ff && imm === x.size) || (w >>> 0 === 0xcb3063ff && x16 === x.size);
    case 'save_fplr_x':
      return x.off === 16 && (w >>> 0) === (epi ? 0xa8c17bfd : 0xa9bf7bfd);
    case 'set_fp':
      return (w >>> 0) === (epi ? 0x910003bf : 0x910003fd);
    case 'add_fp':
      if (epi) return ((w & 0xff8003ff) >>> 0) === 0xd10003bf && imm === x.off;  // sub sp, x29, #n
      return ((w & 0xff8003ff) >>> 0) === 0x910003fd && imm === x.off;  // add x29, sp, #n
    case 'save_regp':
      return ((w & 0xffc00000) >>> 0) === (epi ? 0xa9400000 : 0xa9000000) && rn === SP && rt === x.reg && rt2 === x.reg + 1 && s7 * 8 === x.off;
    case 'save_reg':
      return ((w & 0xffc00000) >>> 0) === (epi ? 0xf9400000 : 0xf9000000) && rn === SP && rt === x.reg && imm12 * 8 === x.off;
    case 'save_freg':
      // (a q register, whose low half is the d register; or d itself)
      return rn === SP && rt === x.reg && ((((w & 0xffc00000) >>> 0) === (epi ? 0x3dc00000 : 0x3d800000) && imm12 * 16 === x.off) ||
        (((w & 0xffc00000) >>> 0) === (epi ? 0xfd400000 : 0xfd000000) && imm12 * 8 === x.off));
    case 'nop': {
      // (anything but an add or sub of an immediate to sp or x29)
      const addsub = ((w >>> 23) & 0x3f) === 0x22;
      return !(addsub && ((w & 31) === 31 || (w & 31) === 29));
    }
    default:
      return false;
  }
}

function checkArm64Code(d, { E, off, text, pdata, iat, entry, funcs, r }) {
  const to = off(text.rva, text.vsize);
  if (to < 0) { E('.text is not all in the file'); return; }
  const word = (rva) => d.readUInt32LE(to + rva - text.rva);
  const inText = (rva) => rva >= text.rva && rva + 4 <= text.rva + text.vsize;
  // the thunks: adrp x16; ldr x16, [x16, #lo]; br x16, each loading an IAT slot
  const thunks = new Set();
  for (let a = text.rva; a + 12 <= text.rva + text.vsize; a += 4) {
    const w0 = word(a), w1 = word(a + 4), w2 = word(a + 8);
    if (((w0 & 0x9f00001f) >>> 0) !== 0x90000010 || ((w1 & 0xffc003ff) >>> 0) !== 0xf9400210 || w2 >>> 0 !== 0xd61f0200) continue;
    const imm = ((w0 >> 29) & 3) | (((w0 >> 5) & 0x7ffff) << 2);
    const pages = imm >= 0x100000 ? imm - 0x200000 : imm;
    const slot = ((a & ~0xfff) + pages * 4096) + ((w1 >> 10) & 0xfff) * 8;
    if (!iat.has(slot)) E(`the thunk at 0x${a.toString(16)} loads 0x${slot.toString(16)}, no import address table slot`);
    thunks.add(a);
    a += 8;
  }
  r.thunks = thunks.size;
  if (thunks.size !== iat.size) E(`${thunks.size} thunks for ${iat.size} imports`);
  if (!funcs) return;
  const fstart = new Map(funcs.map((f) => [text.rva + f.off, f]));
  const pmap = new Map(pdata.map((p) => [p.begin, p]));
  r.funcs = funcs.length;
  for (let k = 1; k < funcs.length; k++) if (funcs[k].off < funcs[k - 1].off + funcs[k - 1].size) E(`functions ${funcs[k - 1].name} and ${funcs[k].name} overlap`);
  // (an entry no function of the PDB starts at: the linker's own entry
  // point, or a copy of a function that more than one object defines,
  // whose code is in the image but which the one kept supersedes: checked
  // by its record's length)
  const extra = [];
  for (const p of pdata) {
    if (fstart.has(p.begin)) continue;
    const xo = off(p.info, 4);
    if (xo < 0) { E(`a function table entry at 0x${p.begin.toString(16)} has its record outside the file data`); continue; }
    extra.push({ name: p.begin === entry ? 'axiom.entry' : `the copy at 0x${p.begin.toString(16)}`, at: p.begin,
                 size: (d.readUInt32LE(xo) & 0x3ffff) * 4, copy: p.begin !== entry });
  }
  r.copies = extra.filter((x) => x.copy).length;
  let nepi = 0, ncalls = 0;
  const all = [...funcs.map((f) => ({ ...f, at: text.rva + f.off })), ...extra];
  for (const f of all) {
    const a0 = f.at, end = a0 + f.size;
    if (!inText(a0) || end > text.rva + text.vsize) { E(`${f.name} is outside .text`); continue; }
    let calls = false, rets = [];
    for (let a = a0; a < end; a += 4) {
      const w = word(a);
      if (((w & 0xfc000000) >>> 0) === 0x94000000 || ((w & 0xfc000000) >>> 0) === 0x14000000) {
        const imm = w & 0x3ffffff, t = a + (imm >= 0x2000000 ? imm - 0x4000000 : imm) * 4;
        const bl = ((w & 0xfc000000) >>> 0) === 0x94000000;
        if (bl) { calls = true; ncalls++; }
        const ok = fstart.has(t) || thunks.has(t) || (!bl && t >= a0 && t < end);
        if (!ok) E(`${f.name}+0x${(a - a0).toString(16)}: a ${bl ? 'bl' : 'b'} to 0x${t.toString(16)}, no function's start${bl ? '' : ' and not within the function'}`);
      } else if (((w & 0xfffffc1f) >>> 0) === 0xd63f0000) calls = true;  // blr
      if (w >>> 0 === 0xd65f03c0) rets.push(a);
    }
    const framed = word(a0) >>> 0 === 0xa9bf7bfd || (word(a0) >>> 0 === 0xd10103ff && word(a0 + 4) >>> 0 === 0xa9bf7bfd);
    const p = pmap.get(a0);
    if (!p) {
      if (framed || calls) E(`${f.name} has a frame or calls, but no unwind information`);
      continue;
    }
    if (p.info & 3) { E(`${f.name}: packed unwind data (the compiler writes records)`); continue; }
    const xo = off(p.info, 8);
    if (xo < 0 || p.info % 4) { E(`${f.name}: its unwind record is outside the file data`); continue; }
    const h = d.readUInt32LE(xo);
    if (((h >> 18) & 3) !== 0 || (h >> 20) & 3) E(`${f.name}: the unwind record's version or flags`);
    if ((h & 0x3ffff) * 4 !== f.size) E(`${f.name}: the unwind record says ${(h & 0x3ffff) * 4} bytes, the function has ${f.size}`);
    let nep = (h >> 22) & 31, words = h >>> 27, at = xo + 4;
    if (nep === 0 && words === 0) { const x = d.readUInt32LE(at); nep = x & 0xffff; words = (x >> 16) & 0xff; at += 4; }
    const scopes = [];
    for (let k = 0; k < nep; k++) { const s = d.readUInt32LE(at + 4 * k); scopes.push({ off: (s & 0x3ffff) * 4, index: s >>> 22 }); }
    const codes = d.subarray(at + 4 * nep, at + 4 * nep + 4 * words);
    const pro = decodeCodes(codes, 0, E).reverse();
    // the prologue: one instruction per code, in order
    let x16 = null;
    for (let k = 0; k < pro.length; k++) {
      const w = word(a0 + 4 * k);
      x16 = movx16(w, x16);
      if (!matches(w, pro[k], false, x16)) { E(`${f.name}+0x${(4 * k).toString(16)}: the prologue's instruction ${(w >>> 0).toString(16)} is not its code's ${pro[k].op}`); break; }
    }
    if (!pro.length || pro.findIndex((x) => x.op === 'save_fplr_x') < 0) E(`${f.name}: the prologue saves no frame record`);
    // the epilogues: their codes from their index, in order, then a return or branch
    const ended = new Set();
    for (const s of scopes) {
      nepi++;
      const ops = decodeCodes(codes, s.index, E);
      let a = a0 + s.off;
      for (const x of ops) {
        if (a >= end || !matches(word(a), x, true, null)) { E(`${f.name}+0x${(a - a0).toString(16)}: the epilogue's instruction is not its code's ${x.op}`); break; }
        a += 4;
      }
      const w = word(a) >>> 0;
      if (w !== 0xd65f03c0 && ((w & 0xfc000000) >>> 0) !== 0x14000000 && ((w & 0xfffffc1f) >>> 0) !== 0xd61f0000) E(`${f.name}+0x${(a - a0).toString(16)}: an epilogue ends in no return or branch`);
      ended.add(a);
    }
    for (const a of rets) if (!ended.has(a)) E(`${f.name}+0x${(a - a0).toString(16)}: a return that ends no described epilogue`);
  }
  r.epilogues = nepi;
  r.calls = ncalls;
}

// checkPE of the file f and its PDB (next to it, when there is one: then
// also the debug directory's CodeView record names it, with its GUID and age).
export function checkPEFile(f, opts = {}) {
  const pdbFile = /\.exe$/i.test(f) ? f.replace(/\.exe$/i, '.pdb') : `${f}.pdb`;  // (as plink.ax names it)
  const d = fs.readFileSync(f), pdb = fs.existsSync(pdbFile) ? fs.readFileSync(pdbFile) : null;
  const r = checkPE(d, { ...opts, pdb });
  if (pdb && !r.errs.length) {
    const pe = d.readUInt32LE(0x3c), opt = pe + 24, nsec = d.readUInt16LE(pe + 6);
    const fileOf = (rva) => {
      for (let k = 0; k < nsec; k++) {
        const s = opt + 240 + 40 * k, va = d.readUInt32LE(s + 12), raw = d.readUInt32LE(s + 20);
        if (rva >= va && rva < va + d.readUInt32LE(s + 8)) return raw + rva - va;
      }
      return -1;
    };
    const dir = fileOf(d.readUInt32LE(opt + 112 + 6 * 8));
    if (dir < 0 || d.readUInt32LE(dir + 12) !== 2) { r.errs.push('no CodeView debug directory'); return r; }
    const rsds = d.readUInt32LE(dir + 24);
    if (d.toString('latin1', rsds, rsds + 4) !== 'RSDS') { r.errs.push('no RSDS record'); return r; }
    const bs = pdb.readUInt32LE(32), dirBlock = pdb.readUInt32LE(pdb.readUInt32LE(52) * bs);
    const sd = dirBlock * bs, nstreams = pdb.readUInt32LE(sd);
    const info = pdb.readUInt32LE(sd + 4 + nstreams * 4 + 4 * Math.ceil(pdb.readUInt32LE(sd + 4) / bs)) * bs;
    if (pdb.readUInt32LE(info) !== 20000404) r.errs.push('the PDB has a bad info stream');
    else if (!pdb.subarray(info + 12, info + 28).equals(d.subarray(rsds + 4, rsds + 20))) r.errs.push('the PDB\'s GUID is not the executable\'s');
    else if (pdb.readUInt32LE(info + 8) !== d.readUInt32LE(rsds + 20)) r.errs.push('the PDB\'s age is not the executable\'s');
    r.pdb = true;
  }
  return r;
}
