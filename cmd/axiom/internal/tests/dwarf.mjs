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
//     call made without a frame, the stack pointer's).
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
    secs.push({ name: cstr(d, strOff + u32(d, h)), type: u32(d, h + 4), addr: u64(d, h + 16), off: u64(d, h + 24),
                size: u64(d, h + 32), link: u32(d, h + 40), info: u32(d, h + 44) });
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
  const funcs = syms.filter((s) => (s.info & 15) === 2 && s.shndx === text && s.size > 0)
    .map((s) => ({ name: s.name, addr: s.value, size: s.size }));
  const img = { kind: type === 1 ? 'elf-rel' : 'elf-exec', machine, sections, funcs, rets: [] };
  if (type !== 1) return img;
  // an object: its sections at address 0, the debug sections' relocations applied
  const symVal = (i) => syms[i].value;
  for (const r of secs.filter((s) => s.type === 4)) {
    const target = secs[r.info];
    const buf = sections.get(target.name);
    const isRets = target.name === 'axiom_rets';
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
      if (x.ty === 1 || x.ty === 257) buf.writeBigUInt64LE(BigInt(v), x.off);
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
  return { kind: 'macho', machine: 'a64', sections, funcs: null, rets: [] };
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

// The sequences of .debug_line's programs: [{ rows: [{ addr, file, line, end, pe }], files: [names] }]
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
    const dirs = entries(), files = entries().map((e) => e[1]);
    r.p = prog;
    let st = null;
    const reset = () => ({ addr: 0, file: 1, line: 1, pe: false });
    let s = reset(), seq = { rows: [], files };
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
        if (sub === 1) { emit(true); out.push(seq); seq = { rows: [], files }; s = reset(); }
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
              line: g('.debug_line'), frame: g('.debug_frame'), aranges: g('.debug_aranges') };
  return { S, units: readInfo(S), seqs: readLines(S), fdes: S.frame.length ? readFrames(S) : [] };
}

// -----------------------------------------------------------------------------
// Checks
// -----------------------------------------------------------------------------
const AT_NAME = 0x03, AT_LOW = 0x11, AT_HIGH = 0x12, AT_DECL_LINE = 0x3b;
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
    const lo = cu.attrs.get(AT_LOW), hi = lo + cu.attrs.get(AT_HIGH);
    for (const c of cu.children) {
      if (c.tag !== TAG_SUBPROGRAM) continue;
      const name = c.attrs.get(AT_NAME), low = c.attrs.get(AT_LOW), high = low + c.attrs.get(AT_HIGH);
      if (!name) E(`a subprogram without a name at 0x${c.off.toString(16)}`);
      if (!(high > low)) E(`${name}: empty range`);
      if (low < lo || high > hi) E(`${name}: [0x${low.toString(16)}, 0x${high.toString(16)}) outside its unit`);
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
  const ranges = dw.units.map((un) => [un.die.attrs.get(AT_LOW), un.die.attrs.get(AT_LOW) + un.die.attrs.get(AT_HIGH)]);
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
  for (const sq of dw.seqs) {
    const rows = sq.rows;
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
  return { errs, nsubs: subs.length, nfuncs: nf, nseqs: dw.seqs.length, nfdes: dw.fdes.length, nrets: img.rets.length, dw };
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
      if (c.tag !== TAG_SUBPROGRAM) continue;
      const low = c.attrs.get(AT_LOW), high = low + c.attrs.get(AT_HIGH);
      if (a >= low && a < high) return c;
    }
  }
  return null;
}
