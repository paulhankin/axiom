// cmd/axiom/internal/tests/dwarf.mjs - a small reader of the debug
// information the compiler writes with -g (cmd/axiom/src/dwarf.ax), and the
// checks the suite makes of it without any debugger or system tool:
//   - every compilation unit is DWARF 5, its functions lie inside it;
//   - every function has a subprogram entry (name, range), and a line table
//     sequence that covers all of its code, at increasing addresses;
//   - every function has an FDE over the same range (native targets), whose
//     rules are those of the frames the code generators make (CFA = sp or
//     the frame pointer, by the right amounts);
//   - each call's return address in the frame tables (axiom_rets, in an
//     object) is in code whose CFA rule is the frame pointer's (or, for a
//     call made without a frame, the stack pointer's);
//   - lexical blocks lie inside their parents' code (ranges: low and high
//     pc, or a list of .debug_rnglists); variables' locations are
//     expressions of the forms the compiler writes, and a location list's
//     ranges lie inside the variable's block;
//   - an inline instance (DW_TAG_inlined_subroutine) lies inside its
//     parent's code, names its abstract instance (a subprogram of its unit
//     with DW_AT_inline, without code), and its variables name that one's
//     (of the same tag) and are located as above.
// Files: ELF executables and relocatable objects (x86-64, AArch64), a
// Mach-O debug bundle's DWARF file, WebAssembly modules (the DWARF custom
// sections, code-section offsets). Formats as in the DWARF 5 standard.

// -----------------------------------------------------------------------------
// Containers
// -----------------------------------------------------------------------------
const u16 = (d, o) => d.readUInt16LE(o), u32 = (d, o) => d.readUInt32LE(o), u64 = (d, o) => Number(d.readBigUInt64LE(o));
const cstr = (d, o) => d.subarray(o, d.indexOf(0, o)).toString();

// { kind, machine, sections: Map name -> Buffer, funcs: [{ name, addr, size }], rets: [{ ret, flags }] (objects) }
export function loadImage(d) {
  if (d.subarray(0, 4).toString('latin1') === '\x7fELF') return loadElf(d);
  if (u32(d, 0) === 0xfeedfacf) return loadMachO(d);
  if (d.subarray(0, 4).toString('latin1') === '\0asm') return loadWasm(d);
  throw new Error('not an ELF, Mach-O or WebAssembly file');
}

function loadElf(d) {
  const type = u16(d, 16), machine = u16(d, 18) === 62 ? 'x64' : 'a64';
  const shoff = u64(d, 40), shnum = u16(d, 60), shstrndx = u16(d, 62);
  const sh = (i) => shoff + 64 * i;
  const strOff = u64(d, sh(shstrndx) + 24);
  const secs = [];
  for (let i = 0; i < shnum; i++) {
    const h = sh(i);
    secs.push({ name: cstr(d, strOff + u32(d, h)), type: u32(d, h + 4), flags: u64(d, h + 8), addr: u64(d, h + 16),
                off: u64(d, h + 24), size: u64(d, h + 32), link: u32(d, h + 40), info: u32(d, h + 44),
                align: u64(d, h + 48) });
  }
  // an object's sections of code one after the other (from address 0, in
  // their order: a function in a section of its own, a COMDAT group's, a
  // cold one's), its other sections at 0
  if (type === 1) {
    let at = 0;
    for (const s of secs) {
      s.base = 0;
      if ((s.flags & 6) === 6 && s.type === 1) {
        const al = Math.max(s.align, 1);
        at = Math.ceil(at / al) * al;
        s.base = at;
        at += s.size;
      }
    }
  }
  const data = (s) => Buffer.from(d.subarray(s.off, s.off + (s.type === 8 ? 0 : s.size)));
  const sections = new Map();
  for (const s of secs) if (s.name.startsWith('.debug_')) sections.set(s.name, data(s));
  // symbols
  const syms = [];
  const st = secs.find((s) => s.type === 2);
  if (st) {
    const so = secs[st.link].off;
    for (let k = 0; k < st.size / 24; k++) {
      const e = st.off + 24 * k;
      syms.push({ name: cstr(d, so + u32(d, e)), info: d[e + 4], shndx: u16(d, e + 6), value: u64(d, e + 8), size: u64(d, e + 16) });
    }
  }
  const text = secs.findIndex((s) => s.name === '.text');
  // (an object's: its symbols at their sections' places)
  const code = (i) => i === text || (type === 1 && i > 0 && i < secs.length && (secs[i].flags & 6) === 6);
  const symBase = (sy) => (type === 1 && sy.shndx > 0 && sy.shndx < secs.length ? secs[sy.shndx].base : 0);
  const funcs = syms.filter((s) => (s.info & 15) === 2 && code(s.shndx) && s.size > 0)
    .map((s) => ({ name: s.name, addr: s.value + symBase(s), size: s.size }));
  const img = { kind: type === 1 ? 'elf-rel' : 'elf-exec', machine, sections, funcs, rets: [] };
  if (type !== 1) return img;
  // an object: its sections of code one after the other (above), the debug
  // sections' relocations applied
  const symVal = (i) => syms[i].value + symBase(syms[i]);
  for (const r of secs.filter((s) => s.type === 4)) {
    const target = secs[r.info];
    const buf = sections.get(target.name);
    const isRets = target.name === 'axiom_rets';  // (one per section of code: all together)
    if (!buf && !isRets) continue;
    const rel = [];
    for (let k = 0; k < r.size / 24; k++) {
      const e = r.off + 24 * k;
      rel.push({ off: u64(d, e), ty: Number(d.readBigUInt64LE(e + 8) & 0xffffffffn), sym: Number(d.readBigUInt64LE(e + 8) >> 32n),
                 add: Number(d.readBigInt64LE(e + 16)) });
    }
    if (isRets) {
      const by = new Map(rel.map((x) => [x.off, x]));
      for (let o = 0; o < target.size; o += 8) {
        const a = by.get(o), c = by.get(o + 4);
        if (!a || !c) continue;
        img.rets.push({ ret: symVal(a.sym) + a.add, flags: (symVal(c.sym) + c.add) & 3 });
      }
      continue;
    }
    for (const x of rel) {
      const v = symVal(x.sym) + x.add;
      if (x.ty === 1 || x.ty === 257 || x.ty === 17 || x.ty === 1028) buf.writeBigUInt64LE(BigInt(v), x.off);
      else if (x.ty === 10 || x.ty === 258) buf.writeUInt32LE(v >>> 0, x.off);
      else throw new Error(`relocation type ${x.ty} in ${target.name}`);
    }
  }
  return img;
}

function loadMachO(d) {
  const ncmds = u32(d, 16);
  const sections = new Map();
  let off = 32;
  for (let i = 0; i < ncmds; i++) {
    const cmd = u32(d, off), size = u32(d, off + 4);
    if (cmd === 0x19) {
      for (let k = 0; k < u32(d, off + 64); k++) {
        const so = off + 72 + 80 * k;
        const sn = cstr(d.subarray(so, so + 17), 0), seg = cstr(d.subarray(so + 16, so + 33), 0);
        if (seg === '__DWARF') sections.set('.' + sn.slice(2), Buffer.from(d.subarray(u32(d, so + 48), u32(d, so + 48) + u64(d, so + 40))));
      }
    }
    off += size;
  }
  return { kind: 'macho', machine: u32(d, 4) === 0x01000007 ? 'x64' : 'a64', sections, funcs: null, rets: [] };
}

function loadWasm(d) {
  const uleb = (st) => { let r = 0, s = 0, b; do { b = d[st.p++]; r += (b & 0x7f) * 2 ** s; s += 7; } while (b & 0x80); return r; };
  const sections = new Map();
  const bodies = [];
  const st = { p: 8 };
  while (st.p < d.length) {
    const id = d[st.p++], size = uleb(st), end = st.p + size;
    if (id === 0) {
      const n = uleb(st);
      const name = d.subarray(st.p, st.p + n).toString();
      st.p += n;
      if (name.startsWith('.debug_')) sections.set(name, Buffer.from(d.subarray(st.p, end)));
    } else if (id === 10) {
      const start = st.p, count = uleb(st);
      for (let k = 0; k < count; k++) {
        const sz = uleb(st);
        bodies.push({ addr: st.p - start, size: sz });
        st.p += sz;
      }
    }
    st.p = end;
  }
  return { kind: 'wasm', machine: 'wasm', sections, funcs: null, bodies, rets: [] };
}

// -----------------------------------------------------------------------------
// DWARF
// -----------------------------------------------------------------------------
class Rd {
  constructor(d, p = 0) { this.d = d; this.p = p; }
  u8() { return this.d[this.p++]; }
  u16() { const x = u16(this.d, this.p); this.p += 2; return x; }
  u32() { const x = u32(this.d, this.p); this.p += 4; return x; }
  u64() { const x = u64(this.d, this.p); this.p += 8; return x; }
  addr(n) { return n === 4 ? this.u32() : this.u64(); }
  uleb() { let r = 0, s = 0, b; do { b = this.d[this.p++]; r += (b & 0x7f) * 2 ** s; s += 7; } while (b & 0x80); return r; }
  sleb() { let r = 0, s = 0, b; do { b = this.d[this.p++]; r += (b & 0x7f) * 2 ** s; s += 7; } while (b & 0x80); if (b & 0x40) r -= 2 ** s; return r; }
  str() { const s = cstr(this.d, this.p); this.p += Buffer.byteLength(s) + 1; return s; }
}

function abbrevs(d, off) {
  const r = new Rd(d, off), m = new Map();
  for (;;) {
    const code = r.uleb();
    if (!code) return m;
    const tag = r.uleb(), children = r.u8() === 1, attrs = [];
    for (;;) {
      const a = r.uleb(), f = r.uleb();
      if (!a && !f) break;
      attrs.push([a, f, f === 0x21 ? r.sleb() : undefined]);
    }
    m.set(code, { tag, children, attrs });
  }
}

// The value of an attribute of form f (strings resolved; references as
// offsets in .debug_info; blocks and expressions as Buffers).
function formValue(r, f, asz, S, unitOff, implicit) {
  switch (f) {
    case 0x01: return r.addr(asz);
    case 0x0b: return r.u8();
    case 0x05: return r.u16();
    case 0x06: return r.u32();
    case 0x07: return r.u64();
    case 0x0c: return r.u8() !== 0;
    case 0x0d: return r.sleb();
    case 0x0f: return r.uleb();
    case 0x0e: return cstr(S.str, r.u32());
    case 0x1f: return cstr(S.lstr, r.u32());
    case 0x08: return r.str();
    case 0x17: return r.u32();
    case 0x11: return unitOff + r.u8();
    case 0x12: return unitOff + r.u16();
    case 0x13: return unitOff + r.u32();
    case 0x18: case 0x09: { const n = r.uleb(); const b = r.d.subarray(r.p, r.p + n); r.p += n; return b; }
    case 0x0a: { const n = r.u8(); const b = r.d.subarray(r.p, r.p + n); r.p += n; return b; }
    case 0x1e: { const b = r.d.subarray(r.p, r.p + 16); r.p += 16; return b; }  // DW_FORM_data16
    case 0x19: return true;
    case 0x21: return implicit;
    default: throw new Error(`form 0x${f.toString(16)}`);
  }
}

// The units of .debug_info: each { off, version, asz, die } where a DIE is
// { off, tag, attrs: Map attr -> value, children: [DIE] }.
export function readInfo(S) {
  const units = [];
  const d = S.info;
  let off = 0;
  while (off < d.length) {
    const r = new Rd(d, off);
    const len = r.u32(), end = off + 4 + len, version = r.u16();
    if (version !== 5) throw new Error(`a unit of version ${version}`);
    const ut = r.u8(), asz = r.u8(), ab = abbrevs(S.abbrev, r.u32());
    const stack = [], dies = new Map();
    let top = null;
    while (r.p < end) {
      const at = r.p, code = r.uleb();
      if (!code) { stack.pop(); continue; }
      const a = ab.get(code);
      if (!a) throw new Error(`abbreviation ${code} undefined at 0x${at.toString(16)}`);
      const die = { off: at, tag: a.tag, attrs: new Map(), children: [] };
      for (const [an, f, imp] of a.attrs) die.attrs.set(an, formValue(r, f, asz, S, off, imp));
      dies.set(at, die);
      if (stack.length) stack[stack.length - 1].children.push(die);
      else top = die;
      if (a.children) stack.push(die);
    }
    units.push({ off, version, ut, asz, die: top, dies });
    off = end;
  }
  return units;
}

// The sequences of .debug_line's programs: [{ rows: [{ addr, file, line, end, pe }], files: [names], sums:
// [MD5 checksums, or null] }]
export function readLines(S) {
  const out = [];
  const d = S.line;
  let off = 0;
  while (off < d.length) {
    const r = new Rd(d, off);
    const len = r.u32(), end = off + 4 + len, version = r.u16();
    if (version !== 5) throw new Error(`a line table of version ${version}`);
    const asz = r.u8(); r.u8();
    const hlen = r.u32(), prog = r.p + hlen;
    const minInst = r.u8(); r.u8(); const defStmt = r.u8();
    const lineBase = (r.u8() << 24) >> 24, lineRange = r.u8(), opBase = r.u8();
    const lens = [0]; for (let k = 1; k < opBase; k++) lens.push(r.u8());
    const entries = () => {
      const nf = r.u8(), fmt = [];
      for (let k = 0; k < nf; k++) fmt.push([r.uleb(), r.uleb()]);
      const n = r.uleb(), list = [];
      for (let k = 0; k < n; k++) {
        const e = {};
        for (const [ct, f] of fmt) e[ct] = formValue(r, f, asz, S, 0);
        list.push(e);
      }
      return list;
    };
    const dirs = entries(), fents = entries(), files = fents.map((e) => e[1]);
    // (the files' MD5 checksums, DW_LNCT_MD5, when given)
    const sums = fents.map((e) => e[5] || null);
    r.p = prog;
    let st = null;
    const reset = () => ({ addr: 0, file: 1, line: 1, pe: false });
    let s = reset(), seq = { rows: [], files, sums };
    const emit = (endSeq) => { seq.rows.push({ addr: s.addr, file: s.file, line: s.line, end: endSeq, pe: s.pe }); s.pe = false; };
    while (r.p < end) {
      const op = r.u8();
      if (op >= opBase) {
        const adj = op - opBase;
        s.addr += Math.floor(adj / lineRange) * minInst;
        s.line += lineBase + (adj % lineRange);
        emit(false);
      } else if (op === 0) {
        const n = r.uleb(), at = r.p, sub = r.u8();
        if (sub === 1) { emit(true); out.push(seq); seq = { rows: [], files, sums }; s = reset(); }
        else if (sub === 2) s.addr = r.addr(asz);
        r.p = at + n;
      } else if (op === 1) emit(false);
      else if (op === 2) s.addr += r.uleb() * minInst;
      else if (op === 3) s.line += r.sleb();
      else if (op === 4) s.file = r.uleb();
      else if (op === 10) s.pe = true;
      else if (op === 5) r.uleb();
      else if (op === 6 || op === 7 || op === 11) { /* stmt, basic block, epilogue begin */ }
      else if (op === 8) s.addr += Math.floor((255 - opBase) / lineRange) * minInst;
      else if (op === 9) s.addr += r.u16();
      else if (op === 12) r.uleb();
      else for (let k = 0; k < lens[op]; k++) r.uleb();
    }
    void st; void dirs; void defStmt;
    off = end;
  }
  return out;
}

// The FDEs of .debug_frame, each with its rows: [{ low, high, rows: [{ loc, cfa: [reg, off], regs: Map reg -> rule }] }]
// (a rule: ['off', n] saved at CFA + n, ['same'], ['undef']).
export function readFrames(S) {
  const d = S.frame, cies = new Map(), fdes = [];
  let off = 0;
  while (off < d.length) {
    const r = new Rd(d, off);
    const len = r.u32(), end = off + 4 + len, id = r.u32();
    if (id === 0xffffffff) {
      const version = r.u8(); const aug = r.str(); const asz = r.u8(); r.u8();
      const caf = r.uleb(), daf = r.sleb(), ra = r.uleb();
      cies.set(off, { version, aug, asz, caf, daf, ra, init: d.subarray(r.p, end) });
    } else {
      const cie = cies.get(id);
      if (!cie) throw new Error(`an FDE at 0x${off.toString(16)} refers to no CIE`);
      const low = r.addr(cie.asz), size = r.addr(cie.asz);
      const fde = { low, high: low + size, rows: [] };
      const state = { cfa: [0, 0], regs: new Map() };
      run(cie, cie.init, state, null, 0);
      const initial = { cfa: [...state.cfa], regs: new Map(state.regs) };
      fde.rows.push({ loc: low, cfa: [...state.cfa], regs: new Map(state.regs) });
      run(cie, d.subarray(r.p, end), state, fde, low, initial);
      fdes.push(fde);
    }
    off = end;
  }
  return fdes;
}

function run(cie, prog, state, fde, low, initial) {
  const r = new Rd(prog);
  let loc = low;
  const row = () => {
    const last = fde.rows[fde.rows.length - 1];
    const x = { loc, cfa: [...state.cfa], regs: new Map(state.regs) };
    if (last.loc === loc) fde.rows[fde.rows.length - 1] = x; else fde.rows.push(x);
  };
  const adv = (n) => { loc += n * cie.caf; };
  while (r.p < prog.length) {
    const op = r.u8(), hi = op & 0xc0, lo = op & 0x3f;
    if (hi === 0x40) { adv(lo); }
    else if (hi === 0x80) state.regs.set(lo, ['off', r.uleb() * cie.daf]);
    else if (hi === 0xc0) state.regs.set(lo, initial && initial.regs.has(lo) ? initial.regs.get(lo) : ['same']);
    else if (op === 0) continue;
    else if (op === 2) adv(r.u8());
    else if (op === 3) adv(r.u16());
    else if (op === 4) adv(r.u32());
    else if (op === 5) { const g = r.uleb(); state.regs.set(g, ['off', r.uleb() * cie.daf]); }
    else if (op === 7) state.regs.set(r.uleb(), ['undef']);
    else if (op === 8) state.regs.set(r.uleb(), ['same']);
    else if (op === 0x0c) state.cfa = [r.uleb(), r.uleb()];
    else if (op === 0x0d) state.cfa = [r.uleb(), state.cfa[1]];
    else if (op === 0x0e) state.cfa = [state.cfa[0], r.uleb()];
    else if (op === 0x06) { const g = r.uleb(); state.regs.set(g, initial && initial.regs.has(g) ? initial.regs.get(g) : ['same']); }
    else if (op === 0x09) { const g = r.uleb(); state.regs.set(g, ['reg', r.uleb()]); }
    else if (op === 0x0a) { (state.stack ||= []).push({ cfa: [...state.cfa], regs: new Map(state.regs) }); }
    else if (op === 0x0b) { const t = state.stack.pop(); state.cfa = t.cfa; state.regs = t.regs; }
    else if (op === 0x10) { const g = r.uleb(), n = r.uleb(); state.regs.set(g, ['expr', Buffer.from(prog.subarray(r.p, r.p + n)).toString('hex')]); r.p += n; }
    else if (op === 0x11) { const g = r.uleb(); state.regs.set(g, ['off', r.sleb() * cie.daf]); }
    else if (op === 0x12) state.cfa = [r.uleb(), r.sleb() * cie.daf];
    else if (op === 0x13) state.cfa = [state.cfa[0], r.sleb() * cie.daf];
    else if (op === 0x2d) state.regs.set('ra_sign', ['negate']);
    else throw new Error(`call frame instruction 0x${op.toString(16)}`);
    if (fde && (hi === 0x40 || op === 2 || op === 3 || op === 4)) continue;
    if (fde) row();
  }
}

// The state of an FDE at address a.
export function frameAt(fde, a) {
  let s = fde.rows[0];
  for (const x of fde.rows) if (x.loc <= a) s = x;
  return s;
}

export function readDwarf(img) {
  const g = (n) => img.sections.get(n) || Buffer.alloc(0);
  const S = { info: g('.debug_info'), abbrev: g('.debug_abbrev'), str: g('.debug_str'), lstr: g('.debug_line_str'),
              line: g('.debug_line'), frame: g('.debug_frame'), aranges: g('.debug_aranges'),
              rnglists: g('.debug_rnglists'), loclists: g('.debug_loclists'), names: g('.debug_names') };
  return { S, units: readInfo(S), seqs: readLines(S), fdes: S.frame.length ? readFrames(S) : [] };
}

// The hash of .debug_names (DWARF 5, 6.1.1.4.5).
// (DWARF 5 6.1.1.4.5: of the name case-folded; ASCII capitals as small letters)
const djb = (s) => { let h = 5381; for (const c of Buffer.from(s, 'latin1')) h = (h * 33 + (c >= 65 && c <= 90 ? c + 32 : c)) >>> 0; return h; };

// The name indexes of .debug_names, checked as read (errs): [{ cus, names:
// [{ name, entries: [{ tag, die (an offset in .debug_info) }] }] }]. Every
// name is in its hash's bucket with its hash.
export function readNames(S, errs) {
  const out = [], d = S.names;
  let off = 0;
  while (off < d.length) {
    const r = new Rd(d, off);
    const len = r.u32(), end = off + 4 + len, version = r.u16(); r.u16();
    if (version !== 5) { errs.push(`.debug_names version ${version}`); return out; }
    const ncu = r.u32(), ltu = r.u32(), ftu = r.u32(), nb = r.u32(), nn = r.u32(), absz = r.u32(), augsz = r.u32();
    r.p += augsz;
    const cus = []; for (let k = 0; k < ncu; k++) cus.push(r.u32());
    r.p += 4 * ltu + 8 * ftu;
    const buckets = []; for (let k = 0; k < nb; k++) buckets.push(r.u32());
    const hashes = []; for (let k = 0; k < (nb ? nn : 0); k++) hashes.push(r.u32());
    const strs = []; for (let k = 0; k < nn; k++) strs.push(r.u32());
    const eoffs = []; for (let k = 0; k < nn; k++) eoffs.push(r.u32());
    const abStart = r.p, abs = new Map();
    while (r.p < abStart + absz) {
      const code = r.uleb();
      if (!code) break;
      const tag = r.uleb(), at = [];
      for (;;) { const i = r.uleb(), f = r.uleb(); if (!i && !f) break; at.push([i, f]); }
      abs.set(code, { tag, at });
    }
    const pool = abStart + absz;
    const names = [];
    for (let k = 0; k < nn; k++) {
      const name = cstr(S.str, strs[k]);
      if (nb && hashes[k] !== djb(name)) errs.push(`.debug_names: ${name}'s hash`);
      if (nb) {
        const b = djb(name) % nb, first = buckets[b];
        if (!first || first - 1 > k || (k > 0 && first - 1 < k && djb(cstr(S.str, strs[k - 1])) % nb !== b)) errs.push(`.debug_names: ${name} not in its bucket ${b}`);
      }
      const e = new Rd(d, pool + eoffs[k]), entries = [];
      for (;;) {
        const code = e.uleb();
        if (!code) break;
        const a = abs.get(code);
        if (!a) { errs.push(`.debug_names: ${name}: abbreviation ${code} undefined`); break; }
        let die = -1, cu = 0;
        for (const [i, f] of a.at) {
          const v = f === 0x13 ? e.u32() : f === 0x0b ? e.u8() : f === 0x05 ? e.u16() : f === 0x0f ? e.uleb() : NaN;
          if (Number.isNaN(v)) { errs.push(`.debug_names: form 0x${f.toString(16)}`); return out; }
          if (i === 1) cu = v;   // DW_IDX_compile_unit
          if (i === 3) die = v;  // DW_IDX_die_offset (from its unit's start)
        }
        entries.push({ tag: a.tag, die: die < 0 ? -1 : cus[cu] + die });
      }
      names.push({ name, entries });
    }
    out.push({ cus, names });
    off = end;
  }
  return out;
}

// -----------------------------------------------------------------------------
// Checks
// -----------------------------------------------------------------------------
const AT_NAME = 0x03, AT_LOW = 0x11, AT_HIGH = 0x12, AT_DECL_LINE = 0x3b, AT_RANGES = 0x55, AT_INLINE = 0x20,
  AT_ORIGIN = 0x31, AT_CALL_FILE = 0x58, AT_CALL_LINE = 0x59;
const TAG_SUBPROGRAM = 0x2e;

// The problems found in img's debug information (an empty list: none).
// opts.cfi: check the call frame information (native targets).
export function checkDebug(img, opts = {}) {
  const errs = [];
  const E = (m) => { if (errs.length < 40) errs.push(m); };
  let dw;
  try { dw = readDwarf(img); } catch (e) { return { errs: [`unreadable: ${e.message}`] }; }
  if (!dw.units.length) return { errs: ['no compilation units'] };
  const subs = [];
  for (const un of dw.units) {
    const cu = un.die;
    if (!cu || cu.tag !== 0x11) { E(`unit at 0x${un.off.toString(16)} is not a compile unit`); continue; }
    // (low and high pc, or DW_AT_ranges: code in several sections)
    const cur = dieRanges(dw, cu) || [];
    for (const c of cu.children) {
      if (c.tag !== TAG_SUBPROGRAM) continue;
      if (c.attrs.has(AT_INLINE)) {
        // (an abstract instance: no code; its variables without locations)
        if (!c.attrs.get(AT_NAME)) E(`an abstract instance without a name at 0x${c.off.toString(16)}`);
        if (c.attrs.has(AT_LOW) || c.attrs.has(AT_RANGES)) E(`${c.attrs.get(AT_NAME)}: an abstract instance with code`);
        for (const v of c.children) if (v.attrs.has(0x02) || !v.attrs.get(AT_NAME)) E(`${c.attrs.get(AT_NAME)}: an abstract variable at 0x${v.off.toString(16)} with a location or without a name`);
        continue;
      }
      const name = c.attrs.get(AT_NAME), low = c.attrs.get(AT_LOW), high = low + c.attrs.get(AT_HIGH);
      // (WebAssembly: a function the linker did not keep is at the tombstone
      // address, -1, as the tool conventions' DWARF has it)
      if (img.kind === 'wasm' && (low === 0xffffffff || low >= 2 ** 64 - 2 ** 16)) continue;
      if (!name) E(`a subprogram without a name at 0x${c.off.toString(16)}`);
      if (!(high > low)) E(`${name}: empty range`);
      if (!cur.some(([lo, hi]) => low >= lo && high <= hi)) E(`${name}: [0x${low.toString(16)}, 0x${high.toString(16)}) outside its unit`);
      if (!(c.attrs.get(AT_DECL_LINE) >= 0)) E(`${name}: no declaration line`);
      subs.push({ name, low, high, die: c, unit: un });
    }
  }
  subs.sort((a, b) => a.low - b.low);
  for (let k = 1; k < subs.length; k++) {
    if (subs[k].low < subs[k - 1].high) E(`${subs[k - 1].name} and ${subs[k].name} overlap`);
  }
  // every function of the image in a unit's range has its subprogram
  const byLow = new Map(subs.map((s) => [s.low, s]));
  const ranges = dw.units.flatMap((un) => dieRanges(dw, un.die) || []);
  const fns = img.funcs || (img.bodies || []).map((b) => ({ name: `(body at 0x${b.addr.toString(16)})`, addr: b.addr, size: b.size }));
  let nf = 0;
  for (const f of fns) {
    if (!ranges.some(([a, b]) => f.addr >= a && f.addr < b)) continue;
    nf++;
    const s = byLow.get(f.addr);
    if (!s) { E(`function ${f.name} at 0x${f.addr.toString(16)} has no subprogram`); continue; }
    if (s.high - s.low > f.size) E(`${f.name}: the subprogram is larger than the function`);
  }
  // line table: one sequence per subprogram, covering it, increasing
  const seqByLow = new Map();
  const tomb = (a) => img.kind === 'wasm' && (a === 0xffffffff || a >= 2 ** 64 - 2 ** 16);
  for (const sq of dw.seqs) {
    const rows = sq.rows;
    if (rows.length && tomb(rows[0].addr)) continue;  // (a function the linker did not keep: above)
    if (rows.length < 2) { E('a line sequence with no rows'); continue; }
    for (let k = 1; k < rows.length; k++) {
      if (rows[k].addr < rows[k - 1].addr || (!rows[k].end && rows[k].addr === rows[k - 1].addr)) {
        E(`line rows out of order at 0x${rows[k].addr.toString(16)}`);
        break;
      }
    }
    const none = rows.every((x) => x.line === 0);  // (code with no source: line 0 throughout)
    for (const x of rows) {
      if (!x.end && !(x.line >= 1) && !none) E(`line ${x.line} at 0x${x.addr.toString(16)}`);
      if (x.file < 0 || x.file >= sq.files.length) E(`file ${x.file} at 0x${x.addr.toString(16)}`);
    }
    seqByLow.set(rows[0].addr, sq);
  }
  for (const s of subs) {
    const sq = seqByLow.get(s.low);
    if (!sq) { E(`${s.name}: no line sequence starts at its address`); continue; }
    if (sq.rows[sq.rows.length - 1].addr !== s.high) E(`${s.name}: its line sequence ends at 0x${sq.rows[sq.rows.length - 1].addr.toString(16)}, not at its end`);
  }
  // call frame information
  if (opts.cfi) {
    const x64 = img.machine === 'x64';
    const SP = x64 ? 7 : 31, FP = x64 ? 6 : 29;
    const fdeByLow = new Map(dw.fdes.map((f) => [f.low, f]));
    for (const s of subs) {
      const f = fdeByLow.get(s.low);
      if (!f) { E(`${s.name}: no FDE`); continue; }
      if (f.high !== s.high) E(`${s.name}: its FDE's range differs`);
      for (const r of f.rows) {
        const [reg, off] = r.cfa;
        const ok = reg === FP ? off === 16 : reg === SP && (x64 ? off === 8 || off === 16 : off === 0 || off === 16);
        if (!ok) E(`${s.name}+0x${(r.loc - s.low).toString(16)}: CFA = r${reg} + ${off}`);
        if (r.loc < f.low || r.loc >= f.high) E(`${s.name}: a row outside its range`);
        if (reg === FP && !(r.regs.get(FP) && r.regs.get(FP)[0] === 'off' && r.regs.get(FP)[1] === -16)) E(`${s.name}+0x${(r.loc - s.low).toString(16)}: the CFA is the frame pointer's but the caller's is not saved`);
      }
    }
    // the frame tables' calls (an object's)
    for (const e of img.rets) {
      if (e.flags & 2) continue;  // (a marker)
      const f = dw.fdes.find((x) => e.ret > x.low && e.ret <= x.high);
      if (!f) { E(`the call returning to 0x${e.ret.toString(16)} is in no FDE`); continue; }
      const st = frameAt(f, e.ret - 1);
      const want = (e.flags & 1) ? SP : FP;
      if (st.cfa[0] !== want) E(`the call returning to 0x${e.ret.toString(16)}: CFA = r${st.cfa[0]} + ${st.cfa[1]}, not by r${want}`);
    }
  }
  // the name indexes: each entry a DIE of its name and tag; every
  // subprogram with a name indexed
  const indexed = new Set();
  for (const ix of readNames(dw.S, errs)) {
    for (const n of ix.names) {
      for (const e of n.entries) {
        const un = dw.units.find((x) => ix.cus.includes(x.off) && x.dies.has(e.die));
        const die = un && un.dies.get(e.die);
        // (an inline instance has its abstract instance's name)
        const org = die && die.attrs.has(AT_ORIGIN) ? un.dies.get(die.attrs.get(AT_ORIGIN)) : null;
        const dname = die && (die.attrs.get(AT_NAME) || (org && org.attrs.get(AT_NAME)));
        if (!die || die.tag !== e.tag || dname !== n.name) E(`.debug_names: ${n.name} (tag 0x${e.tag.toString(16)}) is not the DIE at 0x${e.die.toString(16)}`);
        indexed.add(e.die);
      }
    }
  }
  if (dw.S.names.length) {
    for (const un of dw.units) {
      for (const c of un.die ? un.die.children : []) {
        if (c.tag === TAG_SUBPROGRAM && c.attrs.get(AT_NAME) && !indexed.has(c.off)) E(`.debug_names: no entry for ${c.attrs.get(AT_NAME)}`);
      }
    }
  } else if (dw.units.length) E('no .debug_names');
  const sc = checkScopes(dw, img.machine === 'wasm' ? null : img.machine);
  for (const m of sc.errs) E(m);
  return { errs, nsubs: subs.length, nfuncs: nf, nseqs: dw.seqs.length, nfdes: dw.fdes.length, nrets: img.rets.length, dw,
           nblocks: sc.blocks, nvars: sc.vars, nlocated: sc.located, ninlined: sc.inlined };
}

// The ranges [[lo, hi)] of a DIE with low/high pc or DW_AT_ranges (DWARF 5,
// 2.17; .debug_rnglists entries DW_RLE_start_length, start_end, offset_pair
// with base_address), null if it has none.
export function dieRanges(dw, die) {
  const lo = die.attrs.get(AT_LOW);
  const off = die.attrs.get(AT_RANGES);
  if (off === undefined) return lo === undefined ? null : [[lo, lo + die.attrs.get(AT_HIGH)]];
  const r = new Rd(dw.S.rnglists, off), out = [];
  // (addresses of the units' size: 4 on wasm32; a low pc with ranges is
  // their base address)
  const asz = dw.units.length ? dw.units[0].asz : 8, A = () => r.addr(asz);
  let base = lo || 0;
  for (;;) {
    const k = r.u8();
    if (k === 0) return out;
    if (k === 7) { const a = A(); out.push([a, a + r.uleb()]); }
    else if (k === 6) { const a = A(); out.push([a, A()]); }
    else if (k === 5) base = A();
    else if (k === 4) { const a = r.uleb(); out.push([base + a, base + r.uleb()]); }
    else throw new Error(`range list entry ${k}`);
  }
}

// A location list (.debug_loclists at off): [{ lo, hi, expr }] (DW_LLE_start_length,
// start_end, offset_pair with base_address).
export function locList(dw, off) {
  const r = new Rd(dw.S.loclists, off), out = [];
  let base = 0;
  const ex = () => { const n = r.uleb(); const b = r.d.subarray(r.p, r.p + n); r.p += n; return b; };
  for (;;) {
    const k = r.u8();
    if (k === 0) return out;
    if (k === 8) { const a = r.u64(), n = r.uleb(); out.push({ lo: a, hi: a + n, expr: ex() }); }
    else if (k === 7) { const a = r.u64(), b = r.u64(); out.push({ lo: a, hi: b, expr: ex() }); }
    else if (k === 6) base = r.u64();
    else if (k === 4) { const a = r.uleb(), b = r.uleb(); out.push({ lo: base + a, hi: base + b, expr: ex() }); }
    else throw new Error(`location list entry ${k}`);
  }
}

// Is expr a location expression of the forms the compiler writes: one of
// DW_OP_fbreg n [DW_OP_deref], DW_OP_addr a, DW_OP_const8u n
// DW_OP_form_tls_address; or (in optimized code) pieces (DW_OP_piece n,
// each after one of these or after nothing: not known) of DW_OP_regN,
// DW_OP_regx r, DW_OP_bregN n DW_OP_stack_value (a register plus n),
// DW_OP_bregx r n DW_OP_stack_value, DW_OP_fbreg n (a slot), DW_OP_fbreg n
// DW_OP_stack_value (a frame address), DW_OP_fbreg n DW_OP_deref[_size k]
// [DW_OP_consts c DW_OP_plus] DW_OP_stack_value, DW_OP_implicit_value n
// bytes, DW_OP_addr a DW_OP_stack_value (a datum's address); without the
// DW_OP_stack_value, the address of the variable (one held by reference: a
// variable length array). A piece with a location of its own is followed
// by DW_OP_piece.
export function knownLocation(expr) {
  const r = new Rd(expr);
  const op = r.u8();
  if (op === 0x91 && expr.length > 1) {
    const at = r.p;
    r.sleb();
    if (r.p === expr.length) return true;
    if (r.u8() === 0x06 && r.p === expr.length) return true;
    r.p = at;
  } else if (op === 0x03 && (expr.length === 9 || expr.length === 5)) return true;
  else if (op === 0x0e) { r.u64(); return r.u8() === 0x9b && r.p === expr.length; }
  r.p = 0;
  // one piece's location (or none); false if not of the forms
  const one = () => {
    const o = r.d[r.p];
    if (o === 0x93) return true;
    r.p++;
    if (o >= 0x50 && o <= 0x6f) return true;
    if (o === 0x90) { r.uleb(); return true; }
    // (a value: DW_OP_stack_value; else the variable is in memory there)
    const sv = () => { if (r.p < expr.length && r.d[r.p] === 0x9f) r.p++; return true; };
    if (o >= 0x70 && o <= 0x8f) { r.sleb(); return sv(); }
    if (o === 0x92) { r.uleb(); r.sleb(); return sv(); }
    if (o === 0x03) { r.p += 8; return sv(); }
    if (o === 0x9e) { const n = r.uleb(); r.p += n; return n > 0 && n <= 16 && r.p <= expr.length; }
    if (o === 0x91) {
      r.sleb();
      if (r.p === expr.length || r.d[r.p] === 0x93) return true;
      const k = r.u8();
      if (k === 0x9f) return true;
      if (k === 0x94) r.u8(); else if (k !== 0x06) return false;
      if (r.p === expr.length || r.d[r.p] === 0x93) return true;
      if (r.u8() !== 0x11) return false;
      r.sleb();
      if (r.u8() !== 0x22) return false;
      return sv();
    }
    return false;
  };
  let pieces = 0;
  while (r.p < expr.length) {
    if (!one()) return false;
    if (r.p === expr.length) return pieces === 0;
    if (r.u8() !== 0x93) return false;
    r.uleb();
    pieces++;
  }
  return true;
}

// The registers a variable may be in (DW_OP_regN): those the register
// allocators give values, never the stack, frame or link register, nor the
// scratch ones (x86-64: rax-rdi but rbp, rsp, r8-r15, xmm0-15; AArch64:
// x0-x15, x19-x27, v0-v31).
function allocatable(machine, r) {
  if (machine === 'x64') return (r >= 0 && r <= 5) || (r >= 8 && r <= 15) || (r >= 17 && r <= 32);
  return (r >= 0 && r <= 15) || (r >= 19 && r <= 27) || (r >= 64 && r <= 95);
}

function exprReg(expr) {
  if (expr[0] >= 0x50 && expr[0] <= 0x6f && expr.length === 1) return expr[0] - 0x50;
  if (expr[0] === 0x90) return new Rd(expr, 1).uleb();
  return -1;
}

// The registers an expression of knownLocation's forms reads (DW_OP_regN,
// regx, bregN, bregx), in each of its pieces.
export function exprRegs(expr) {
  const r = new Rd(expr), out = [];
  while (r.p < expr.length) {
    const o = r.u8();
    if (o >= 0x50 && o <= 0x6f) out.push(o - 0x50);
    else if (o === 0x90) out.push(r.uleb());
    else if (o >= 0x70 && o <= 0x8f) { out.push(o - 0x70); r.sleb(); }
    else if (o === 0x92) { out.push(r.uleb()); r.sleb(); }
    else if (o === 0x91 || o === 0x11) r.sleb();
    else if (o === 0x93) r.uleb();
    else if (o === 0x94) r.u8();
    else if (o === 0x9e) { const n = r.uleb(); r.p += n; }
    else if (o === 0x03) r.p += 8;
    else if (o === 0x0e) r.u64();
  }
  return out;
}

// The lexical blocks and variables of the subprograms: each block's ranges
// inside its parent's; each location known (knownLocation), a list's
// ranges inside its block, in order, apart, in allocatable registers
// (machine: 'x64', 'a64'). Returns { errs, blocks, vars, located }.
export function checkScopes(dw, machine) {
  const errs = [];
  let blocks = 0, vars = 0, located = 0, inlined = 0;
  const E = (m) => { if (errs.length < 40) errs.push(m); };
  const inside = (rs, outer) => rs.every(([a, b]) => outer.some(([c, d]) => a >= c && b <= d));
  const walk = (die, rs, fname, un, abs) => {
    for (const c of die.children) {
      if (c.tag === 0x0b || c.tag === 0x1d) {
        let r, a = abs;
        if (c.tag === 0x1d) {
          // (an inline instance: of an abstract instance of its unit)
          inlined++;
          a = un.dies.get(c.attrs.get(AT_ORIGIN));
          if (!a || a.tag !== TAG_SUBPROGRAM || !a.attrs.has(AT_INLINE)) { E(`${fname}: an inline instance at 0x${c.off.toString(16)} of no abstract instance`); continue; }
          if (c.attrs.has(AT_CALL_FILE) !== c.attrs.has(AT_CALL_LINE)) E(`${fname}: an inline instance's call has a file or a line only`);
        } else blocks++;
        try { r = dieRanges(dw, c); } catch (e) { E(`${fname}: ${e.message}`); continue; }
        const what = c.tag === 0x0b ? 'a lexical block' : `an inline instance of ${a.attrs.get(AT_NAME)}`;
        if (!r || !r.length) { E(`${fname}: ${what} without code at 0x${c.off.toString(16)}`); continue; }
        if (r.some(([a, b]) => !(b > a))) E(`${fname}: an empty range in ${what}`);
        if (!inside(r, rs)) E(`${fname}: ${what} at 0x${c.off.toString(16)} outside its parent's code`);
        walk(c, r, fname, un, a);
      } else if (c.tag === 0x34 || c.tag === 0x05) {
        vars++;
        if (c.attrs.has(AT_ORIGIN)) {
          // (a variable of an inline instance: its abstract instance's)
          const o = un.dies.get(c.attrs.get(AT_ORIGIN));
          if (!abs || !o || o.tag !== c.tag || !abs.children.includes(o)) E(`${fname}: a variable at 0x${c.off.toString(16)} not of its abstract instance`);
        }
        const loc = c.attrs.get(0x02);
        if (loc === undefined) continue;
        located++;
        const org = c.attrs.has(AT_ORIGIN) ? un.dies.get(c.attrs.get(AT_ORIGIN)) : null;
        const name = c.attrs.get(AT_NAME) || (org && org.attrs.get(AT_NAME)) || '(artificial)';
        if (typeof loc === 'number') {
          let l;
          try { l = locList(dw, loc); } catch (e) { E(`${fname}.${name}: ${e.message}`); continue; }
          if (!l.length) E(`${fname}.${name}: an empty location list`);
          for (let k = 1; k < l.length; k++) if (l[k].lo < l[k - 1].hi) E(`${fname}.${name}: location ranges out of order or overlapping`);
          for (const x of l) {
            for (const r of exprRegs(x.expr)) if (machine && !allocatable(machine, r)) E(`${fname}.${name}: in register ${r}, which holds no values`);
            if (!(x.hi > x.lo)) E(`${fname}.${name}: an empty location range`);
            if (!inside([[x.lo, x.hi]], rs)) E(`${fname}.${name}: a location at [0x${x.lo.toString(16)}, 0x${x.hi.toString(16)}) outside its block`);
            if (!knownLocation(x.expr)) E(`${fname}.${name}: location ${x.expr.toString('hex')}`);
          }
        } else if (!knownLocation(loc)) E(`${fname}.${name}: location ${loc.toString('hex')}`);
      }
    }
  };
  for (const un of dw.units) {
    for (const c of un.die.children) {
      if (c.tag !== TAG_SUBPROGRAM || c.attrs.has(AT_INLINE)) continue;
      // (WebAssembly, machine null: a function the linker did not keep is at -1)
      const low = c.attrs.get(AT_LOW);
      if (machine === null && (low === 0xffffffff || low >= 2 ** 64 - 2 ** 16)) continue;
      walk(c, dieRanges(dw, c), c.attrs.get(AT_NAME), un, null);
    }
  }
  return { errs, blocks, vars, located, inlined };
}

// The addresses of the rows of line `line` of a file whose name ends in `file`.
export function addrsOfLine(dw, file, line) {
  const out = [];
  for (const sq of dw.seqs) {
    for (const x of sq.rows) if (!x.end && x.line === line && String(sq.files[x.file]).endsWith(file)) out.push(x.addr);
  }
  return out;
}

// The subprogram containing address a.
export function subprogramAt(dw, a) {
  for (const un of dw.units) {
    for (const c of un.die.children) {
      if (c.tag !== TAG_SUBPROGRAM || c.attrs.has(AT_INLINE)) continue;
      const low = c.attrs.get(AT_LOW), high = low + c.attrs.get(AT_HIGH);
      if (a >= low && a < high) return c;
    }
  }
  return null;
}

// -----------------------------------------------------------------------------
// The variables of debug/vars.c (run.mjs): what its entries must say.
// -----------------------------------------------------------------------------
// opts: O0 (every variable has a location), tls (the thread-local has one).
export function checkVars(dw, opts) {
  const errs = [];
  const E = (m) => errs.push(m);
  const dieAt = (un, off) => un.dies.get(off);
  const typeName = (un, die) => { const t = die && dieAt(un, die.attrs.get(0x49)); return t ? `${t.tag.toString(16)}:${t.attrs.get(AT_NAME) || ''}` : 'none'; };
  const kids = (die, tag, name) => die.children.filter((c) => c.tag === tag && (name === undefined || c.attrs.get(AT_NAME) === name));
  let un = null, sub = null;
  for (const u of dw.units) for (const c of u.die.children) if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE) && c.attrs.get(AT_NAME) === 'shadow') { un = u; sub = c; }
  if (!sub) return ['no subprogram shadow'];
  const TD = '16:', V = 0x34, P = 0x05, LB = 0x0b;
  const n = kids(sub, P, 'n')[0], x = kids(sub, V, 'x')[0], px = kids(sub, V, 'px')[0];
  if (!n || typeName(un, n) !== `${TD}count_t`) E(`shadow: the parameter n is not of type count_t (${typeName(un, n)})`);
  if (!x || typeName(un, x) !== `${TD}count_t`) E('shadow: x is not of type count_t');
  if (!px || typeName(un, px) !== `${TD}count_ptr`) E('shadow: px is not of type count_ptr');
  const blocks = kids(sub, LB);
  if (opts.O0) {
    if (blocks.length !== 2) E(`shadow: ${blocks.length} lexical blocks at its top, not 2 (the loop's, the block's)`);
    const loop = blocks.find((b) => kids(b, V, 'i').length);
    const inner = loop && kids(loop, LB)[0];
    if (!loop || !inner) E('shadow: no block of the loop with i, and its body\'s');
    else {
      const ix = kids(inner, V, 'x')[0];
      if (!ix || ix.attrs.get(AT_DECL_LINE) !== opts.markLine(opts.src, 'inner-x')) E('shadow: the inner x is not in the loop body\'s block');
      const w = kids(inner, V, 'w')[0], pr = kids(inner, V, 'pr')[0], calls = kids(inner, V, 'calls')[0];
      if (!w || typeName(un, w) !== `${TD}wide_t`) E('shadow: w is not of type wide_t');
      if (!kids(inner, 0x16, 'wide_t').length) E('shadow: the typedef wide_t is not in its block');
      if (!kids(inner, 0x13, 'pair').length) E('shadow: struct pair is not in its block');
      if (!kids(inner, 0x13, 'held').length) E('shadow: struct held (of struct holder\'s members) is not in its block');
      if (!pr || typeName(un, pr) !== '13:pair') E('shadow: pr is not a struct pair');
      if (!calls || !(calls.attrs.get(0x02) instanceof Uint8Array) || calls.attrs.get(0x02)[0] !== 0x03) E('shadow: the static calls is not in its block, at its address');
    }
    const other = blocks.find((b) => kids(b, V, 'x').length && !kids(b, V, 'i').length);
    if (!other) E('shadow: no block with the second inner x');
    const walk = (d) => { for (const c of d.children) { if ((c.tag === V || c.tag === P) && c.attrs.get(0x02) === undefined) E(`shadow: ${c.attrs.get(AT_NAME)} has no location at -O0`); walk(c); } };
    walk(sub);
    // the VLA: through its address, its bound an artificial variable's
    let vsub = null;
    for (const c of un.die.children) if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE) && c.attrs.get(AT_NAME) === 'vla') vsub = c;
    const v = vsub && kids(vsub, LB).concat([vsub]).flatMap((b) => kids(b, V, 'v'))[0];
    const loc = v && v.attrs.get(0x02);
    if (!v || !(loc instanceof Uint8Array) || loc[0] !== 0x91 || loc[loc.length - 1] !== 0x06) E('vla: v has no location through its address');
    else {
      const at = dieAt(un, v.attrs.get(0x49));
      const sr = at && at.tag === 0x01 && at.children.find((c) => c.tag === 0x21);
      const cnt = sr && dieAt(un, sr.attrs.get(0x37));
      if (!cnt || cnt.tag !== V || !cnt.attrs.get(0x34) || !cnt.attrs.get(0x02)) E('vla: the bound of v\'s type is not a variable with a location');
    }
  }
  if (!opts.O0) {
    // optimized: the parameters of once in their argument registers at its entry, s somewhere
    let osub = null;
    for (const c of un.die.children) if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE) && c.attrs.get(AT_NAME) === 'once') osub = c;
    if (!osub) E('no subprogram once');
    else {
      const regs = opts.machine === 'x64' ? [5, 4] : [0, 1];
      for (const [k, pn] of ['a', 'b'].entries()) {
        const p = kids(osub, P, pn)[0];
        const loc = p && p.attrs.get(0x02);
        const l = typeof loc === 'number' ? locList(dw, loc) : [];
        if (!l.length || l[0].lo !== osub.attrs.get(AT_LOW) || exprReg(l[0].expr) !== regs[k]) E(`once: the parameter ${pn} is not in its argument register at the entry`);
      }
      const sv = kids(osub, V, 's')[0];
      if (!sv || sv.attrs.get(0x02) === undefined) E('once: s has no location');
    }
    // a variable assigned again: locations of its values; a
    // structure in pieces; a pointer into a frame object (its address)
    let asub = null;
    for (const c of un.die.children) if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE) && c.attrs.get(AT_NAME) === 'again') asub = c;
    if (!asub) E('no subprogram again');
    else {
      const all = (d) => [d, ...d.children.flatMap((c) => (c.tag === LB ? all(c) : []))];
      const vof = (n) => all(asub).flatMap((d) => kids(d, V, n))[0];
      const ll = (v) => { const loc = v && v.attrs.get(0x02); return typeof loc === 'number' ? locList(dw, loc) : []; };
      const r = ll(vof('r'));
      if (r.length < 2) E('again: r has not more than one location range');
      // (taken apart on x86-64 Linux; elsewhere the optimizer may keep it whole)
      const q = vof('q');
      if (!q || q.attrs.get(0x02) === undefined) E('again: q has no location');
      else if (opts.machine === 'x64' && opts.tls && !ll(q).some((x) => x.expr.includes(0x93))) E('again: q is not in pieces');
      if (!ll(vof('pa')).some((x) => x.expr[0] === 0x91 && x.expr[x.expr.length - 1] === 0x9f)) E('again: pa is not a frame address');
    }
    // scaled, inlined into main: its inline instance there, at the call's
    // line, and its parameter v and variable m located in it
    const insts = [];
    const find = (d) => { for (const c of d.children) { if (c.tag === 0x1d) { const o = dieAt(un, c.attrs.get(AT_ORIGIN)); if (o && o.attrs.get(AT_NAME) === 'scaled') insts.push([c, o]); } find(c); } };
    for (const c of un.die.children) if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE)) find(c);
    if (!insts.length) E('scaled: no inline instance');
    else {
      const [ins, abs] = insts[0];
      if (ins.attrs.get(AT_CALL_LINE) !== opts.markLine(opts.src, 'scaled-call')) E(`scaled: the inline instance's call line ${ins.attrs.get(AT_CALL_LINE)}`);
      for (const [tag, nm] of [[P, 'v'], [V, 'm']]) {
        const a = kids(abs, tag, nm)[0];
        const c = a && ins.children.find((x) => x.attrs.get(AT_ORIGIN) === a.off);
        if (!c || c.attrs.get(0x02) === undefined) E(`scaled: ${nm} has no location in its inline instance`);
      }
    }
  }
  const tl = un.die.children.find((c) => c.tag === V && c.attrs.get(AT_NAME) === 'per_thread');
  if (opts.tls) {
    const loc = tl && tl.attrs.get(0x02);
    if (!loc || loc[0] !== 0x0e || loc[9] !== 0x9b) E('per_thread: no location in the TLS block');
  } else if (tl && tl.attrs.get(0x02) !== undefined) E('per_thread: a location where none can be given');
  return errs;
}

// The types and variables of debug/vars.ax (run.mjs): at -O0, main's
// variables with their types (a structure, an enumeration, a union's
// variant part, a slice's structure), the block of `inner`, the global
// COUNT; the units' language Axiom's (0x8a78).
export function checkAxVars(dw) {
  const errs = [];
  const E = (m) => errs.push(m);
  let un = null, sub = null;
  for (const u of dw.units) {
    for (const c of u.die.children) {
      if (c.tag === TAG_SUBPROGRAM && !c.attrs.has(AT_INLINE) && c.attrs.get(AT_NAME) === 'main' && c.children.some((x) => x.attrs.get(AT_NAME) === 'sh')) { un = u; sub = c; }
    }
  }
  if (!sub) return ['no subprogram main with variables'];
  if (un.die.attrs.get(0x13) !== 0x8a78) E(`the unit's language is ${un.die.attrs.get(0x13)}`);
  const ty = (die) => un.dies.get(die.attrs.get(0x49));
  const v = (name, d = sub) => d.children.find((c) => c.tag === 0x34 && c.attrs.get(AT_NAME) === name);
  const want = [['p', 0x13, 'Point'], ['c', 0x04, 'Color'], ['sh', 0x13, 'Shape'], ['sl', 0x13, '[]i32'], ['arr', 0x01, undefined]];
  for (const [n, tag, tn] of want) {
    const x = v(n);
    if (!x) { E(`main: no variable ${n}`); continue; }
    if (x.attrs.get(0x02) === undefined) E(`main: ${n} has no location at -O0`);
    const t = ty(x);
    if (!t || t.tag !== tag || (tn && t.attrs.get(AT_NAME) !== tn)) E(`main: ${n} is not of type ${tn || 'an array'}`);
  }
  const sh = v('sh') && ty(v('sh'));
  const vp = sh && sh.children.find((c) => c.tag === 0x33);
  if (!vp || vp.children.filter((c) => c.tag === 0x19).length !== 3) E('Shape: no variant part with its 3 variants');
  const color = v('c') && ty(v('c'));
  if (!color || !color.children.some((c) => c.attrs.get(AT_NAME) === 'Green' && c.attrs.get(0x1c) === 5)) E('Color: no Green = 5');
  const blk = sub.children.find((c) => c.tag === 0x0b && v('inner', c));
  if (!blk) E('main: inner is not in a lexical block');
  if (!dw.units.some((u) => u.die.children.some((c) => c.tag === 0x34 && String(c.attrs.get(AT_NAME)).endsWith('.COUNT') && c.attrs.get(0x02)))) E('no global COUNT');
  return errs;
}
