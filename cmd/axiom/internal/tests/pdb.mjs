// A structural reader of program databases (PDB), for the test runner
// (run.mjs: "windows" and "debug information"): checkPDB(buffer) reads the
// MSF container and every stream the compiler's PDB writer (pdb.ax) makes,
// and returns { errs, ... } with what does not hold:
//   - the container: the superblock, the block map and the stream
//     directory, every block of every stream inside the file and owned by
//     one stream only, those blocks marked used in the free block map;
//   - the info stream (its version, GUID and age, the /names stream), the
//     string table (its hash buckets: every string found by its hash);
//   - TPI and IPI: their headers, every record's length and 4-byte
//     alignment and known kind, every type and id index a record holds
//     resolvable (a primitive, or a record before it; ids in IPI, types in
//     TPI), each record's hash in the hash stream (recomputed here) and the
//     index offsets pointing at records;
//   - DBI: its header, the module records, each module's stream (its symbol
//     records: lengths, alignment, kinds, the scopes they open and close
//     (parent and end offsets agree with the nesting), every type index
//     resolvable, every code address inside its procedure; its line
//     tables: files in the module's checksums, names in /names), the
//     section contributions (sorted, of known modules), the section map;
//   - the symbol records of the publics and globals, and their hashes
//     (every record found by its name's bucket; procedure references to
//     procedures of a module).
// Formats as Microsoft publishes them (the MSF container, the PDB streams,
// cvinfo.h's records). With a path as the argument it prints what it found.
import fs from 'node:fs';

const BS = 4096;

// the hash of names (the string table's, the symbol tables')
export const hashV1 = (s) => {
  let r = 0;
  const n = s.length >> 2;
  for (let i = 0; i < n; i++) r = (r ^ s.readUInt32LE(i * 4)) >>> 0;
  let at = n * 4, rem = s.length & 3;
  if (rem >= 2) { r = (r ^ s.readUInt16LE(at)) >>> 0; at += 2; rem -= 2; }
  if (rem === 1) r = (r ^ s[at]) >>> 0;
  r = (r | 0x20202020) >>> 0;
  r = (r ^ (r >>> 11)) >>> 0;
  return (r ^ (r >>> 16)) >>> 0;
};

let crcTable;
const crc32 = (b) => {
  if (!crcTable) {
    crcTable = new Uint32Array(256);
    for (let i = 0; i < 256; i++) {
      let c = i;
      for (let k = 0; k < 8; k++) c = c & 1 ? (0xedb88320 ^ (c >>> 1)) >>> 0 : c >>> 1;
      crcTable[i] = c >>> 0;
    }
  }
  let c = 0;
  for (const x of b) c = (crcTable[(c ^ x) & 0xff] ^ (c >>> 8)) >>> 0;
  return c;
};

const cstr = (b, at) => {
  const e = b.indexOf(0, at);
  return b.subarray(at, e < 0 ? b.length : e);
};

// a numeric leaf at `at`: [its value, its size]
const numeric = (b, at) => {
  const v = b.readUInt16LE(at);
  if (v < 0x8000) return [v, 2];
  switch (v) {
    case 0x8000: return [b.readInt8(at + 2), 3];
    case 0x8001: return [b.readInt16LE(at + 2), 4];
    case 0x8002: return [b.readUInt16LE(at + 2), 4];
    case 0x8003: return [b.readInt32LE(at + 2), 6];
    case 0x8004: return [b.readUInt32LE(at + 2), 6];
    case 0x8009: return [Number(b.readBigInt64LE(at + 2)), 10];
    case 0x800a: return [Number(b.readBigUInt64LE(at + 2)), 10];
    default: return [NaN, 2];
  }
};

// The hash of a type or id record (from its length), as pdb.ax computes it.
export const recordHash = (r) => {
  const kind = r.readUInt16LE(2);
  if (kind >= 0x1504 && kind <= 0x1507) {
    const prop = r.readUInt16LE(6);
    const at = kind === 0x1507 ? 16 : kind === 0x1506 ? 12 + numeric(r, 12)[1] : 20 + numeric(r, 20)[1];
    const name = cstr(r, at);
    const fwd = prop & 0x80, scoped = prop & 0x100, unique = prop & 0x200;
    const s = name.toString('latin1');
    const anon = unique && (s === '<unnamed-tag>' || s === '__unnamed' || s.endsWith('::<unnamed-tag>') || s.endsWith('::__unnamed'));
    if (!fwd && !scoped && !anon) return hashV1(name);
    if (!fwd && unique && !anon) return hashV1(cstr(r, at + name.length + 1));
    return crc32(r);
  }
  if (kind === 0x1606 || kind === 0x1607) return hashV1(r.subarray(4, 8));
  return crc32(r);
};

// The index fields of a type record: [offset, 'T' or 'I'] pairs, or null for
// a kind not known. A field list's entries are walked.
const typeFields = (r, ipi, errs, where) => {
  const kind = r.readUInt16LE(2);
  const f = [];
  const T = (o) => f.push([o, 'T']), I = (o) => f.push([o, 'I']);
  switch (kind) {
    case 0x1001: case 0x1002: case 0x1205: T(4); break;  // LF_MODIFIER LF_POINTER LF_BITFIELD
    case 0x1008: T(4); T(12); break;                       // LF_PROCEDURE
    case 0x1201: for (let k = 0, n = r.readUInt32LE(4); k < n; k++) T(8 + 4 * k); break;  // LF_ARGLIST
    case 0x1503: T(4); T(8); break;                        // LF_ARRAY
    case 0x1505: T(8); T(12); T(16); break;                // LF_STRUCTURE
    case 0x1506: T(8); break;                              // LF_UNION
    case 0x1507: T(8); T(12); break;                       // LF_ENUM
    case 0x1601: I(4); T(8); break;                        // LF_FUNC_ID
    case 0x1605: I(4); break;                              // LF_STRING_ID
    case 0x1606: T(4); I(8); break;                        // LF_UDT_SRC_LINE
    case 0x1203: {                                         // LF_FIELDLIST
      let at = 4;
      while (at < r.length) {
        if (r[at] >= 0xf0) { at++; continue; }  // (LF_PAD)
        const k = r.readUInt16LE(at);
        if (k === 0x150d) {                       // LF_MEMBER
          T(at + 4);
          const [, n] = numeric(r, at + 8);
          at += 8 + n + cstr(r, at + 8 + n).length + 1;
        } else if (k === 0x1502) {                // LF_ENUMERATE
          const [, n] = numeric(r, at + 4);
          at += 4 + n + cstr(r, at + 4 + n).length + 1;
        } else if (k === 0x1404) {                // LF_INDEX
          T(at + 4);
          at += 8;
        } else {
          errs.push(`${where}: a field list entry of kind 0x${k.toString(16)}`);
          return f;
        }
      }
      break;
    }
    default: return null;
  }
  if (ipi !== (kind >= 0x1600)) errs.push(`${where}: a record of kind 0x${kind.toString(16)} in the ${ipi ? 'IPI' : 'TPI'}`);
  return f;
};

// The index fields of a symbol record (type indexes), or null for an unknown kind.
const symbolTypeFields = (kind) => ({
  0x0006: [], 0x1012: [], 0x1101: [], 0x113c: [], 0x1103: [],  // S_END S_FRAMEPROC S_OBJNAME S_COMPILE3 S_BLOCK32
  0x110e: [], 0x1125: [], 0x1127: [],                          // S_PUB32 S_PROCREF S_LPROCREF
  0x110f: [28], 0x1110: [28],                                  // S_LPROC32 S_GPROC32
  0x1107: [4], 0x1108: [4],                                    // S_CONSTANT S_UDT
  0x110c: [4], 0x110d: [4], 0x1112: [4], 0x1113: [4],          // S_LDATA32 S_GDATA32 S_LTHREAD32 S_GTHREAD32
  0x1111: [8], 0x113e: [4],                                    // S_REGREL32 S_LOCAL
  0x1141: [], 0x1142: [], 0x1143: [], 0x1145: [],              // S_DEFRANGE_REGISTER, _FRAMEPOINTER_REL, _SUBFIELD_REGISTER, _REGISTER_REL
  0x114d: [], 0x114e: [],                                      // S_INLINESITE (its inlinee: an id), S_INLINESITE_END
}[kind] ?? null);

// The code ranges of an inline site's binary annotations (cvinfo.h's
// BinaryAnnotationOpcode, numbers compressed as CVCompressData), from the
// procedure's start: { ranges: [[lo, hi)], files: [checksum entry offsets], bad: a problem or null }.
export function siteRanges(ann) {
  let p = 0, off = 0, start = -1;
  const ranges = [], files = [];
  const num = () => {
    const b = ann[p++];
    if ((b & 0x80) === 0) return b;
    if ((b & 0xc0) === 0x80) return ((b & 0x3f) << 8) | ann[p++];
    if ((b & 0xe0) === 0xc0) { const v = ((b & 0x1f) << 24) | (ann[p] << 16) | (ann[p + 1] << 8) | ann[p + 2]; p += 3; return v >>> 0; }
    return NaN;
  };
  while (p < ann.length) {
    if (ann[p] === 0 || ann[p] >= 0xf1) break;  // (the end, or the record's padding)
    const op = num();
    if (op === 3) { off += num(); if (start < 0) start = off; }
    else if (op === 4) { const n = num(); if (start < 0) start = off; ranges.push([start, off + n]); off += n; start = -1; }
    else if (op === 5) files.push(num());
    else if (op === 6 || op === 9 || op === 10 || op === 13 || op === 7 || op === 8) num();
    else if (op === 11) { off += num() & 0xf; if (start < 0) start = off; }
    else if (op === 12) { const n = num(); off += num(); if (start < 0) start = off; ranges.push([start, off + n]); off += n; start = -1; }
    else return { ranges, files, bad: `annotation ${op}` };
  }
  if (start >= 0) return { ranges, files, bad: 'a range without its length' };
  return { ranges, files, bad: null };
}

const kindName = (k) => '0x' + k.toString(16);

export function checkPDB(p, opts = {}) {
  const errs = [];
  // (procs: every procedure, { name, off, size, sec, module, locals: [{ name, kind }] })
  const out = { errs, streams: [], types: [], ids: [], modules: [], publics: [], globals: [], procs: [] };
  if (p.length < BS || p.toString('latin1', 0, 32) !== 'Microsoft C/C++ MSF 7.00\r\n\x1aDS\0\0\0') {
    errs.push('not an MSF file');
    return out;
  }
  const bs = p.readUInt32LE(32), fpm = p.readUInt32LE(36), nblocks = p.readUInt32LE(40);
  const dirSize = p.readUInt32LE(44), mapBlock = p.readUInt32LE(52);
  if (bs !== BS) errs.push(`block size ${bs}`);
  if (nblocks * bs !== p.length) errs.push(`${nblocks} blocks but ${p.length} bytes`);
  if (fpm !== 1 && fpm !== 2) errs.push(`free block map ${fpm}`);
  const owner = new Int32Array(nblocks).fill(-1);
  const own = (b, who) => {
    if (b >= nblocks || b === 0 || b % bs === 1 || b % bs === 2) { errs.push(`${who}: block ${b} out of range or reserved`); return false; }
    if (owner[b] !== -1) errs.push(`${who}: block ${b} is also stream ${owner[b]}'s`);
    owner[b] = who;
    return true;
  };
  const blk = (b) => p.subarray(b * bs, (b + 1) * bs);
  const ndir = Math.ceil(dirSize / bs);
  const dirBlocks = [];
  for (let k = 0; k < ndir; k++) {
    const b = p.readUInt32LE(mapBlock * bs + 4 * k);
    own(b, -2);
    dirBlocks.push(b);
  }
  own(mapBlock, -3);
  const dir = Buffer.concat(dirBlocks.map(blk)).subarray(0, dirSize);
  const ns = dir.readUInt32LE(0);
  let at = 4 + 4 * ns;
  for (let s = 0; s < ns; s++) {
    let size = dir.readUInt32LE(4 + 4 * s);
    if (size === 0xffffffff) size = 0;
    const n = Math.ceil(size / bs), bl = [];
    for (let k = 0; k < n; k++) {
      const b = dir.readUInt32LE(at + 4 * k);
      if (own(b, s)) bl.push(b);
    }
    at += 4 * n;
    out.streams.push(Buffer.concat(bl.map(blk)).subarray(0, size));
  }
  if (at !== dirSize) errs.push(`the directory has ${dirSize - at} bytes left over`);
  // the free block map: every block owned is marked used (a clear bit)
  for (let b = 0; b < nblocks; b++) {
    if (owner[b] === -1) continue;
    const map = fpm + bs * Math.floor(b / (bs * 8)), bit = b % (bs * 8);
    if (map < nblocks && (p[map * bs + (bit >> 3)] >> (bit & 7)) & 1) errs.push(`block ${b} is used but marked free`);
  }
  const S = out.streams;
  const need = (k, what) => { if (!S[k] || !S[k].length) { errs.push(`no ${what} stream (${k})`); return false; } return true; };
  if (!need(1, 'info') || !need(2, 'TPI') || !need(3, 'DBI') || !need(4, 'IPI')) return out;

  // the info stream
  const info = S[1];
  if (info.readUInt32LE(0) !== 20000404) errs.push(`info stream version ${info.readUInt32LE(0)}`);
  out.age = info.readUInt32LE(8);
  out.guid = info.subarray(12, 28);
  const nameLen = info.readUInt32LE(28);
  const nameBuf = info.subarray(32, 32 + nameLen);
  let ia = 32 + nameLen;
  const hsize = info.readUInt32LE(ia), hcap = info.readUInt32LE(ia + 4);
  const npresent = info.readUInt32LE(ia + 8);
  const present = [];
  for (let k = 0; k < npresent; k++) present.push(info.readUInt32LE(ia + 12 + 4 * k));
  ia += 12 + 4 * npresent;
  const ndeleted = info.readUInt32LE(ia);
  ia += 4 + 4 * ndeleted;
  const named = {};
  for (let k = 0; k < hcap; k++) {
    if (!((present[k >> 5] >>> (k & 31)) & 1)) continue;
    named[cstr(nameBuf, info.readUInt32LE(ia)).toString()] = info.readUInt32LE(ia + 4);
    ia += 8;
  }
  if (Object.keys(named).length !== hsize) errs.push('the named stream table\'s size');
  const namesAt = named['/names'];
  let strings = null;
  if (namesAt === undefined || !S[namesAt]) errs.push('no /names stream');
  else {
    const nm = S[namesAt];
    if (nm.readUInt32LE(0) !== 0xeffeeffe) errs.push('/names: bad signature');
    const len = nm.readUInt32LE(8);
    strings = nm.subarray(12, 12 + len);
    const nb = nm.readUInt32LE(12 + len);
    const buckets = [];
    for (let k = 0; k < nb; k++) buckets.push(nm.readUInt32LE(16 + len + 4 * k));
    for (const o of buckets) {
      if (o === 0) continue;
      if (o >= len) { errs.push(`/names: offset ${o} out of range`); continue; }
      // found from its hash by linear probing
      let h = hashV1(cstr(strings, o)) % nb, ok = false;
      for (let k = 0; k < nb && buckets[h] !== 0; k++, h = (h + 1) % nb) if (buckets[h] === o) { ok = true; break; }
      if (!ok) errs.push(`/names: "${cstr(strings, o)}" not found by its hash`);
    }
  }

  // TPI and IPI
  const typeStream = (k, ipi) => {
    const t = S[k], what = ipi ? 'IPI' : 'TPI';
    const recs = [];
    if (t.readUInt32LE(0) !== 20040203 || t.readUInt32LE(4) !== 56) { errs.push(`${what}: header`); return { recs, begin: 0x1000, end: 0x1000 }; }
    const begin = t.readUInt32LE(8), end = t.readUInt32LE(12), size = t.readUInt32LE(16);
    const hs = t.readUInt16LE(20), keySize = t.readUInt32LE(24), nb = t.readUInt32LE(28);
    const hvo = t.readUInt32LE(32), hvl = t.readUInt32LE(36), ioo = t.readUInt32LE(40), iol = t.readUInt32LE(44);
    if (begin !== 0x1000) errs.push(`${what}: indexes begin at ${begin}`);
    if (56 + size !== t.length) errs.push(`${what}: ${size} bytes of records in a stream of ${t.length}`);
    const offs = [];
    for (let a = 56; a < 56 + size;) {
      const len = t.readUInt16LE(a);
      if ((len + 2) % 4) errs.push(`${what} record 0x${(begin + recs.length).toString(16)}: length ${len} not aligned`);
      offs.push(a - 56);
      recs.push(t.subarray(a, a + len + 2));
      a += len + 2;
    }
    if (begin + recs.length !== end) errs.push(`${what}: ${recs.length} records, indexes [0x${begin.toString(16)}, 0x${end.toString(16)})`);
    if (keySize !== 4) errs.push(`${what}: hash key size ${keySize}`);
    const h = hs === 0xffff ? null : S[hs];
    if (!h && recs.length) errs.push(`${what}: no hash stream`);
    if (h) {
      if (hvl !== 4 * recs.length) errs.push(`${what}: ${hvl / 4} hash values for ${recs.length} records`);
      for (let j = 0; j < recs.length && 4 * j < hvl; j++) {
        const want = recordHash(recs[j]) % nb, got = h.readUInt32LE(hvo + 4 * j);
        if (want !== got) errs.push(`${what} record 0x${(begin + j).toString(16)}: hash ${got}, not ${want}`);
      }
      let prev = -1;
      for (let j = 0; j < iol; j += 8) {
        const ti = h.readUInt32LE(ioo + j), off = h.readUInt32LE(ioo + j + 4);
        if (ti <= prev || ti < begin || ti >= end || offs[ti - begin] !== off) errs.push(`${what}: index offset (0x${ti.toString(16)}, ${off}) wrong`);
        prev = ti;
      }
    }
    return { recs, begin, end };
  };
  const tpi = typeStream(2, false), ipi = typeStream(4, true);
  out.types = tpi.recs;
  out.ids = ipi.recs;
  const typeOk = (v, limit) => v < 0x1000 ? (v & 0xff) <= 0x7c && ((v >> 8) & 7) <= 7 : v < limit;
  for (const [st, ipiS] of [[tpi, false], [ipi, true]]) {
    st.recs.forEach((r, j) => {
      const ti = 0x1000 + j, where = `${ipiS ? 'IPI' : 'TPI'} record 0x${ti.toString(16)}`;
      const f = typeFields(r, ipiS, errs, where);
      if (!f) { errs.push(`${where}: unknown kind ${kindName(r.readUInt16LE(2))}`); return; }
      for (const [o, which] of f) {
        const v = r.readUInt32LE(o);
        // (in the same stream: a record before it; in the other: any)
        const limit = which === (ipiS ? 'I' : 'T') ? ti : (which === 'T' ? tpi.end : ipi.end);
        if (which === 'I' && v === 0) continue;
        if (!typeOk(v, limit)) errs.push(`${where}: index 0x${v.toString(16)} (at ${o}) does not resolve`);
      }
    });
  }

  // DBI
  const dbi = S[3];
  if (dbi.readInt32LE(0) !== -1 || dbi.readUInt32LE(4) !== 19990903) errs.push('DBI: header');
  const gsiStream = dbi.readUInt16LE(12), psiStream = dbi.readUInt16LE(16), symStream = dbi.readUInt16LE(20);
  // (the substreams' sizes; 44 is a stream index)
  const [modiSize, scSize, smSize, fiSize, tsmSize, dbgSize, ecSize] = [24, 28, 32, 36, 40, 48, 52].map((o) => dbi.readUInt32LE(o));
  out.machine = dbi.readUInt16LE(62);
  let da = 64;
  const modi = dbi.subarray(da, da + modiSize);
  da += modiSize;
  const sc = dbi.subarray(da, da + scSize);
  da += scSize + smSize + fiSize + tsmSize + ecSize + dbgSize;
  if (da !== dbi.length) errs.push(`DBI: ${dbi.length - da} bytes left over`);
  const mods = [];
  for (let m = 0; m < modi.length;) {
    const stream = modi.readUInt16LE(m + 34), symSize = modi.readUInt32LE(m + 36);
    const c11 = modi.readUInt32LE(m + 40), c13 = modi.readUInt32LE(m + 44);
    const name = cstr(modi, m + 64);
    const obj = cstr(modi, m + 64 + name.length + 1);
    mods.push({ name: name.toString(), stream, symSize, c11, c13 });
    m = (m + 64 + name.length + 1 + obj.length + 1 + 3) & ~3;
  }
  out.modules = mods;
  // section contributions
  if (sc.readUInt32LE(0) !== 0xeffe0000 + 19970605) errs.push('DBI: section contributions version');
  let prevSc = [0, -1];
  for (let o = 4; o + 28 <= sc.length; o += 28) {
    const sec = sc.readUInt16LE(o), off = sc.readInt32LE(o + 4), m = sc.readUInt16LE(o + 16);
    if (sec < prevSc[0] || (sec === prevSc[0] && off < prevSc[1])) errs.push(`DBI: section contribution ${sec}:${off.toString(16)} out of order`);
    if (m >= mods.length) errs.push(`DBI: section contribution of module ${m}`);
    prevSc = [sec, off];
  }
  // each module's symbols and lines
  const procs = new Map();  // module stream offset -> procedure record, per module
  let sites = 0, inlinees = 0;
  mods.forEach((md, mi) => {
    const s = S[md.stream];
    const where = `module ${mi} (${md.name})`;
    if (!s) { errs.push(`${where}: no stream ${md.stream}`); return; }
    if (4 + md.symSize - 4 + md.c11 + md.c13 > s.length) errs.push(`${where}: sizes beyond its stream`);
    if (s.readUInt32LE(0) !== 4) errs.push(`${where}: signature`);
    const stack = [];  // open scopes: [offset, kind, start, end, section, its end's offset, an inline site's ranges]
    const mprocs = new Map();
    const siteFiles = [];  // (the inline sites' files: checksum entries)
    let n = 0;
    for (let a = 4; a < md.symSize;) {
      const len = s.readUInt16LE(a), kind = s.readUInt16LE(a + 2), r = s.subarray(a, a + len + 2);
      const w = `${where} symbol at ${a} (${kindName(kind)})`;
      n++;
      if ((len + 2) % 4 || a + len + 2 > md.symSize) { errs.push(`${w}: length ${len}`); break; }
      const tf = symbolTypeFields(kind);
      if (!tf) errs.push(`${w}: unknown kind`);
      else for (const o of tf) { const v = r.readUInt32LE(o); if (!typeOk(v, tpi.end)) errs.push(`${w}: type 0x${v.toString(16)} does not resolve`); }
      const top = stack[stack.length - 1];
      if (kind === 0x114d) {
        // an inline site: in a procedure, its code (annotations) inside its parent's, of an id
        const parent = r.readUInt32LE(4), end = r.readUInt32LE(8), id = r.readUInt32LE(12);
        if (parent !== (top ? top[0] : 0)) errs.push(`${w}: parent ${parent}, not ${top ? top[0] : 0}`);
        if (!top) errs.push(`${w}: an inline site outside a procedure`);
        if (!(id >= 0x1000 && id < ipi.end)) errs.push(`${w}: inlinee 0x${id.toString(16)} is not an id`);
        if (end <= a || end >= md.symSize || s.readUInt16LE(end + 2) !== 0x114e) errs.push(`${w}: end ${end} is not an S_INLINESITE_END`);
        const proc = stack.find((x) => x[1] !== 0x1103 && x[1] !== 0x114d);
        const sr = siteRanges(r.subarray(16));
        if (sr.bad) errs.push(`${w}: ${sr.bad}`);
        if (!sr.ranges.length) errs.push(`${w}: no code`);
        const pr = top ? (top[1] === 0x114d ? top[6] : [[top[2] - (proc ? proc[2] : 0), top[3] - (proc ? proc[2] : 0)]]) : [];
        for (const [lo, hi] of sr.ranges) {
          if (!(hi > lo) || !pr.some(([c, d]) => lo >= c && hi <= d)) errs.push(`${w}: code [${lo.toString(16)}, ${hi.toString(16)}) outside its parent's`);
        }
        siteFiles.push(...sr.files.map((f) => [w, f]));
        stack.push([a, kind, 0, 0, 0, end, sr.ranges]);
        sites++;
      } else if (kind === 0x114e) {
        if (!top || top[1] !== 0x114d) errs.push(`${w}: S_INLINESITE_END outside an inline site`);
        else {
          if (top[5] !== a) errs.push(`${w}: closes the inline site at ${top[0]} whose end is ${top[5]}`);
          stack.pop();
        }
      } else if (kind === 0x110f || kind === 0x1110 || kind === 0x1103) {
        const parent = r.readUInt32LE(4), end = r.readUInt32LE(8);
        if (parent !== (top ? top[0] : 0)) errs.push(`${w}: parent ${parent}, not ${top ? top[0] : 0}`);
        const isProc = kind !== 0x1103;
        const size = r.readUInt32LE(isProc ? 16 : 12), off = r.readUInt32LE(isProc ? 32 : 16);
        const sec = r.readUInt16LE(isProc ? 36 : 20);
        if (isProc && top) errs.push(`${w}: a procedure inside a scope`);
        if (!isProc && !top) errs.push(`${w}: a block outside a procedure`);
        if (top && top[1] === 0x114d) {
          // (in an inline site: inside one of its pieces)
          const proc = stack.find((x) => x[1] !== 0x1103 && x[1] !== 0x114d);
          const lo = off - proc[2];
          if (!top[6].some(([c, d]) => lo >= c && lo + size <= d) || sec !== proc[4]) errs.push(`${w}: [${off.toString(16)}, +${size}) outside its inline site`);
        } else if (top && (off < top[2] || off + size > top[3] || sec !== top[4])) errs.push(`${w}: [${off.toString(16)}, +${size}) outside its parent`);
        if (end <= a || end >= md.symSize || s.readUInt16LE(end + 2) !== 6) errs.push(`${w}: end ${end} is not an S_END`);
        stack.push([a, kind, off, off + size, sec, end]);
        if (isProc) {
          mprocs.set(a, r);
          out.procs.push({ name: cstr(r, 39).toString('latin1'), off, size, sec, module: mi, locals: [] });
        }
      } else if (kind === 0x0006) {
        if (!top) errs.push(`${w}: S_END outside a scope`);
        else {
          if (top[5] !== a) errs.push(`${w}: closes the scope at ${top[0]} whose end is ${top[5]}`);
          stack.pop();
        }
      } else if (kind === 0x1141 || kind === 0x1142 || kind === 0x1143 || kind === 0x1145) {
        // a location's range: inside the procedure
        const ro = kind === 0x1145 || kind === 0x1143 ? 12 : 8;
        const off = r.readUInt32LE(ro), sec = r.readUInt16LE(ro + 4), cb = r.readUInt16LE(ro + 6);
        const proc = stack.find((x) => x[1] !== 0x1103 && x[1] !== 0x114d);
        if (!proc) errs.push(`${w}: a range outside a procedure`);
        else if (cb === 0 || off < proc[2] || off + cb > proc[3] || sec !== proc[4]) errs.push(`${w}: range [${off.toString(16)}, +${cb}) outside its procedure`);
      } else if (kind === 0x113e || kind === 0x1111 || kind === 0x1107) {
        if (!stack.length) errs.push(`${w}: a local outside a procedure`);
        else {
          const at = kind === 0x113e ? 10 : kind === 0x1111 ? 14 : 8 + numeric(r, 8)[1];
          out.procs[out.procs.length - 1].locals.push({ name: cstr(r, at).toString('latin1'), kind, flags: kind === 0x113e ? r.readUInt16LE(8) : 0 });
        }
      }
      a += len + 2;
    }
    if (stack.length) errs.push(`${where}: ${stack.length} scopes not closed`);
    procs.set(mi, mprocs);
    md.nsyms = n;
    // the line information
    let checksums = null;
    const c13 = s.subarray(md.symSize, md.symSize + md.c13);
    for (let a = 0; a + 8 <= c13.length;) {
      const kind = c13.readUInt32LE(a), len = c13.readUInt32LE(a + 4), sub = c13.subarray(a + 8, a + 8 + len);
      if (kind === 0xf4) {
        checksums = new Set();
        for (let o = 0; o < sub.length;) {
          checksums.add(o);
          const name = sub.readUInt32LE(o), cs = sub[o + 4];
          if (strings && (name >= strings.length)) errs.push(`${where}: a file name at ${name} beyond /names`);
          o = (o + 6 + cs + 3) & ~3;
        }
      } else if (kind === 0xf6) {
        // the functions inlined: an id, a file, a line each
        if (sub.readUInt32LE(0) !== 0) errs.push(`${where}: inlinee lines' signature`);
        for (let o = 4; o + 12 <= sub.length; o += 12) {
          const id = sub.readUInt32LE(o), file = sub.readUInt32LE(o + 4);
          if (!(id >= 0x1000 && id < ipi.end)) errs.push(`${where}: inlinee lines of 0x${id.toString(16)}, not an id`);
          if (!checksums || !checksums.has(file)) errs.push(`${where}: inlinee lines of file entry ${file}, not in its checksums`);
          inlinees++;
        }
      } else if (kind === 0xf2) {
        const off = sub.readUInt32LE(0), size = sub.readUInt32LE(8);
        for (let o = 12; o < sub.length;) {
          const file = sub.readUInt32LE(o), nl = sub.readUInt32LE(o + 4), bl = sub.readUInt32LE(o + 8);
          if (!checksums || !checksums.has(file)) errs.push(`${where}: lines of file entry ${file}, not in its checksums`);
          let prev = -1;
          for (let k = 0; k < nl; k++) {
            const lo = sub.readUInt32LE(o + 12 + 8 * k);
            if (lo < prev || lo >= size) errs.push(`${where}: a line at +${lo.toString(16)} of the function at ${off.toString(16)} (size ${size})`);
            prev = lo;
          }
          o += bl;
        }
      }
      a += 8 + ((len + 3) & ~3);
    }
    for (const [w, f] of siteFiles) if (!checksums || !checksums.has(f)) errs.push(`${w}: a file entry ${f}, not in the checksums`);
  });
  out.sites = sites;
  out.inlinees = inlinees;

  // the publics' and globals' symbol records and hashes
  const recs = S[symStream] || Buffer.alloc(0);
  const recAt = new Set();
  for (let a = 0; a < recs.length;) {
    const len = recs.readUInt16LE(a), kind = recs.readUInt16LE(a + 2);
    if ((len + 2) % 4) errs.push(`symbol records: length ${len} at ${a}`);
    recAt.add(a);
    const tf = symbolTypeFields(kind);
    if (!tf) errs.push(`symbol records: unknown kind ${kindName(kind)} at ${a}`);
    else for (const o of tf) { const v = recs.readUInt32LE(a + o); if (!typeOk(v, tpi.end)) errs.push(`symbol records: type 0x${v.toString(16)} at ${a} does not resolve`); }
    if (kind === 0x1125 || kind === 0x1127) {
      const off = recs.readUInt32LE(a + 8), m = recs.readUInt16LE(a + 12) - 1;
      if (!procs.get(m) || !procs.get(m).has(off)) errs.push(`symbol records: a procedure reference to ${m}:${off} which is none`);
    }
    a += len + 2;
  }
  const nameOf = (a) => {
    const kind = recs.readUInt16LE(a + 2);
    const at = { 0x110e: 14, 0x1125: 14, 0x1127: 14, 0x110c: 14, 0x110d: 14, 0x1112: 14, 0x1113: 14, 0x1108: 8 }[kind];
    if (at !== undefined) return cstr(recs, a + at);
    if (kind === 0x1107) return cstr(recs, a + 8 + numeric(recs, a + 8)[1]);
    return null;
  };
  const gsi = (h, what, list) => {
    if (h.readUInt32LE(0) !== 0xffffffff || h.readUInt32LE(4) !== 0xeffe0000 + 19990810) { errs.push(`${what}: hash header`); return; }
    const hrSize = h.readUInt32LE(8), bSize = h.readUInt32LE(12);
    const nrec = hrSize / 8;
    const bitmap = h.subarray(16 + hrSize, 16 + hrSize + 516);
    const starts = [];
    for (let o = 16 + hrSize + 516; o < 16 + hrSize + bSize; o += 4) starts.push(h.readUInt32LE(o) / 12);
    // each bucket in use (its bit set) holds the records from its start to the next's
    const buckets = [];
    for (let b = 0; b < 4096; b++) if ((bitmap.readUInt32LE(4 * (b >> 5)) >>> (b & 31)) & 1) buckets.push(b);
    if (buckets.length !== starts.length) errs.push(`${what}: ${buckets.length} buckets in use, ${starts.length} starts`);
    buckets.forEach((b, k) => {
      const end = k + 1 < starts.length ? starts[k + 1] : nrec;
      for (let j = starts[k]; j < end; j++) {
        const off = h.readUInt32LE(16 + 8 * j) - 1;
        if (!recAt.has(off)) { errs.push(`${what}: record ${off} is not a record's start`); continue; }
        const name = nameOf(off);
        if (!name) { errs.push(`${what}: record ${off} has no name`); continue; }
        if (hashV1(name) % 4096 !== b) errs.push(`${what}: "${name}" in bucket ${b}, not ${hashV1(name) % 4096}`);
        list.push({ name: name.toString(), kind: recs.readUInt16LE(off + 2), off });
      }
    });
    if (starts.length && starts[0] !== 0) errs.push(`${what}: the first bucket starts at ${starts[0]}`);
  };
  if (S[gsiStream]) gsi(S[gsiStream], 'globals', out.globals); else errs.push('no globals stream');
  if (S[psiStream]) {
    const ps = S[psiStream], hl = ps.readUInt32LE(0), al = ps.readUInt32LE(4);
    gsi(ps.subarray(28, 28 + hl), 'publics', out.publics);
    // the address map: the public records in address order
    let prev = [0, -1];
    for (let o = 28 + hl; o < 28 + hl + al; o += 4) {
      const r = ps.readUInt32LE(o);
      if (!recAt.has(r) || recs.readUInt16LE(r + 2) !== 0x110e) { errs.push(`publics: address map entry ${r} is no S_PUB32`); continue; }
      const off = recs.readUInt32LE(r + 8), sec = recs.readUInt16LE(r + 12);
      if (sec < prev[0] || (sec === prev[0] && off < prev[1])) errs.push('publics: the address map is out of order');
      prev = [sec, off];
    }
  } else errs.push('no publics stream');
  void opts;
  return out;
}

if (process.argv[1] && process.argv[1].endsWith('pdb.mjs') && process.argv[2]) {
  const r = checkPDB(fs.readFileSync(process.argv[2]));
  console.log(`streams ${r.streams.length}, types ${r.types.length}, ids ${r.ids.length}, modules ${r.modules.length}, ` +
    `globals ${r.globals.length}, publics ${r.publics.length}`);
  if (process.argv[3]) for (const g of r.globals) if (g.name.includes(process.argv[3])) console.log(`global ${g.name} ${kindName(g.kind)}`);
  for (const e of r.errs.slice(0, 50)) console.log(e);
  if (r.errs.length > 50) console.log(`... ${r.errs.length} errors`);
  process.exitCode = r.errs.length ? 1 : 0;
}
