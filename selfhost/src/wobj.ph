// =============================================================================
// wobj.ph - WebAssembly compilation units: the in-memory form produced by the
// wasm code generator, and its encoding as a relocatable object file in the
// LLVM / wasm-ld format (tool-conventions/Linking.md), so that objects work
// with our own linker (wlink.ph) and with wasm-ld (for linking C code).
// =============================================================================
import "util.ph" as u;
import "bin.ph" as bn;

// Value types
pub const I32: u8 = 0x7f;
pub const I64: u8 = 0x7e;
pub const F32: u8 = 0x7d;
pub const F64: u8 = 0x7c;
pub const V128: u8 = 0x7b;

// Symbol kinds and flags
pub const SYM_FUNC: u8 = 0;
pub const SYM_DATA: u8 = 1;
pub const SYM_GLOBAL: u8 = 2;
pub const SYM_TABLE: u8 = 5;
pub const F_WEAK: u32 = 0x1;
pub const F_LOCAL: u32 = 0x2;
pub const F_HIDDEN: u32 = 0x4;
pub const F_UNDEFINED: u32 = 0x10;
pub const F_EXPORTED: u32 = 0x20;
pub const F_EXPLICIT_NAME: u32 = 0x40;
pub const F_NO_STRIP: u32 = 0x80;
pub const F_TLS: u32 = 0x100;

// Relocation types
pub const R_FUNCTION_INDEX_LEB: u8 = 0;
pub const R_TABLE_INDEX_SLEB: u8 = 1;
pub const R_TABLE_INDEX_I32: u8 = 2;
pub const R_MEMORY_ADDR_LEB: u8 = 3;
pub const R_MEMORY_ADDR_SLEB: u8 = 4;
pub const R_MEMORY_ADDR_I32: u8 = 5;
pub const R_TYPE_INDEX_LEB: u8 = 6;
pub const R_GLOBAL_INDEX_LEB: u8 = 7;
pub const R_MEMORY_ADDR_LEB64: u8 = 14;
pub const R_MEMORY_ADDR_SLEB64: u8 = 15;
pub const R_MEMORY_ADDR_I64: u8 = 16;
pub const R_TABLE_INDEX_SLEB64: u8 = 18;
pub const R_TABLE_INDEX_I64: u8 = 19;
pub const R_TABLE_NUMBER_LEB: u8 = 20;

pub fn reloc_has_addend(t: u8) bool {
    return (t == R_MEMORY_ADDR_LEB) or (t == R_MEMORY_ADDR_SLEB) or (t == R_MEMORY_ADDR_I32) or
           (t == R_MEMORY_ADDR_LEB64) or (t == R_MEMORY_ADDR_SLEB64) or (t == R_MEMORY_ADDR_I64);
}

pub struct Reloc {
    ty: u8,
    off: u32,     // in the function body / segment data
    sym: u32,     // symbol index (type index for R_TYPE_INDEX_LEB)
    addend: i64,
}

pub struct Sym {
    kind: u8,
    flags: u32,
    name: []u8,
    index: u32,   // FUNC: function index; GLOBAL: global index; TABLE: table index
    seg: u32,     // DATA (defined)
    off: u64,
    size: u64,
}

pub struct Import {
    module: []u8,
    name: []u8,
    ty: u32,      // FUNC: type index; GLOBAL: value type
    mutable: bool,
}

pub struct Func {
    ty: u32,
    body: bn.Bytes,        // locals and code, ending with `end` (no size prefix)
    relocs: u.List.[Reloc],
    sym: u32,
    name: []u8,
}

pub struct Global {
    vt: u8,
    mutable: bool,
    init: i64,
}

pub struct Seg {
    name: []u8,
    align: u32,    // log2
    data: bn.Bytes,
    relocs: u.List.[Reloc],
    tls: bool,
}

pub struct Export {
    name: []u8,
    func: u32,     // function index
}

pub struct Unit {
    wasm64: bool,
    types: u.List.[[]u8],          // encoded function types (0x60 params results)
    fimports: u.List.[Import],     // function imports: function indices 0..n-1
    gimports: u.List.[Import],     // global imports
    funcs: u.List.[*mut Func],
    globals: u.List.[Global],
    segs: u.List.[*mut Seg],
    syms: u.List.[Sym],
    exports: u.List.[Export],
    uses_table: bool,
    shared_memory: bool,
    symmap: u.SymMap.[u32],        // name -> symbol index + 1 (non-local symbols)
}

pub fn new_unit(wasm64: bool) *mut Unit {
    let un = u.new.[Unit]();
    un.wasm64 = wasm64;
    return un;
}

pub fn type_index(un: *mut Unit, params: []u8, results: []u8) u32 {
    let mut b = bn.new_bytes();
    b.u8_(0x60);
    b.uleb(@as(u64, params.len));
    b.put(params);
    b.uleb(@as(u64, results.len));
    b.put(results);
    let enc = b.view();
    for i in 0..un.types.len {
        if u.str_eq(un.types.get(i), enc) {
            b.free();
            return @int_cast(u32, i);
        }
    }
    un.types.push(u.dup(enc));
    b.free();
    return @int_cast(u32, un.types.len - 1);
}

pub fn add_sym(un: *mut Unit, s: Sym) u32 {
    un.syms.push(s);
    let idx = @int_cast(u32, un.syms.len - 1);
    if ((s.flags & F_LOCAL) == 0) and (s.name.len > 0) {
        un.symmap.put(u.intern(s.name), idx + 1);
    }
    return idx;
}

pub fn find_sym(un: *mut Unit, name: []u8) i64 {
    let k = un.symmap.get(u.intern(name));
    if k == 0 {
        return -1;
    }
    return @as(i64, k) - 1;
}

// -----------------------------------------------------------------------------
// Encoding as an object file
// -----------------------------------------------------------------------------
fn section(out: *mut bn.Bytes, id: u8, content: *bn.Bytes) void {
    out.u8_(id);
    out.uleb(@as(u64, content.len()));
    out.put(content.view());
}

fn custom(out: *mut bn.Bytes, name: []u8, content: *bn.Bytes) void {
    let mut c = bn.new_bytes();
    c.name(name);
    c.put(content.view());
    section(out, 0, &c);
    c.free();
}

fn write_relocs(r: *mut bn.Bytes, rs: *u.List.[Reloc], base: u32) void {
    for i in 0..rs.len {
        let x = rs.get(i);
        r.u8_(x.ty);
        r.uleb(@as(u64, x.off + base));
        r.uleb(@as(u64, x.sym));
        if reloc_has_addend(x.ty) {
            r.sleb(x.addend);
        }
    }
}

pub fn write_object(un: *mut Unit) []u8 {
    let mut out = bn.new_bytes();
    out.put("\x00asm");
    out.u32_(1);
    let mut nsec: u32 = 0;
    // type
    let mut c = bn.new_bytes();
    c.uleb(@as(u64, un.types.len));
    for i in 0..un.types.len {
        c.put(un.types.get(i));
    }
    section(&mut out, 1, &c);
    nsec += 1;
    c.free();
    // import: memory, table, globals, functions
    c = bn.new_bytes();
    let mut nimp = 1 + un.gimports.len + un.fimports.len;
    if un.uses_table {
        nimp += 1;
    }
    c.uleb(@as(u64, nimp));
    c.name("env");
    c.name("__linear_memory");
    c.u8_(2);
    let mut lim: u8 = 0;
    if un.wasm64 {
        lim = 4;
    }
    c.u8_(lim);
    c.uleb(0);
    if un.uses_table {
        c.name("env");
        c.name("__indirect_function_table");
        c.u8_(1);
        c.u8_(0x70);
        c.u8_(0);
        c.uleb(0);
    }
    for i in 0..un.gimports.len {
        let g = un.gimports.get(i);
        c.name(g.module);
        c.name(g.name);
        c.u8_(3);
        c.u8_(@truncate(u8, g.ty));
        c.u8_(u.pick(g.mutable, "\x01", "\x00")[0]);
    }
    for i in 0..un.fimports.len {
        let f = un.fimports.get(i);
        c.name(f.module);
        c.name(f.name);
        c.u8_(0);
        c.uleb(@as(u64, f.ty));
    }
    section(&mut out, 2, &c);
    nsec += 1;
    c.free();
    // function
    c = bn.new_bytes();
    c.uleb(@as(u64, un.funcs.len));
    for i in 0..un.funcs.len {
        c.uleb(@as(u64, un.funcs.get(i).ty));
    }
    section(&mut out, 3, &c);
    nsec += 1;
    c.free();
    // global
    if un.globals.len > 0 {
        c = bn.new_bytes();
        c.uleb(@as(u64, un.globals.len));
        for i in 0..un.globals.len {
            let g = un.globals.get(i);
            c.u8_(g.vt);
            c.u8_(u.pick(g.mutable, "\x01", "\x00")[0]);
            if g.vt == I64 {
                c.u8_(0x42);
            } else {
                c.u8_(0x41);
            }
            c.sleb(g.init);
            c.u8_(0x0b);
        }
        section(&mut out, 6, &c);
        nsec += 1;
        c.free();
    }
    // export (functions with export names)
    if un.exports.len > 0 {
        c = bn.new_bytes();
        c.uleb(@as(u64, un.exports.len));
        for i in 0..un.exports.len {
            let e = un.exports.get(i);
            c.name(e.name);
            c.u8_(0);
            c.uleb(@as(u64, e.func));
        }
        section(&mut out, 7, &c);
        nsec += 1;
        c.free();
    }
    // datacount
    c = bn.new_bytes();
    c.uleb(@as(u64, un.segs.len));
    section(&mut out, 12, &c);
    nsec += 1;
    c.free();
    // code (relocation offsets are relative to the section contents)
    c = bn.new_bytes();
    c.uleb(@as(u64, un.funcs.len));
    let mut code_relocs = u.List.[Reloc].{ data: u.nil.[Reloc](), len: 0, cap: 0 };
    for i in 0..un.funcs.len {
        let f = un.funcs.get(i);
        c.uleb(@as(u64, f.body.len()));
        let base = @int_cast(u32, c.len());
        c.put(f.body.view());
        for k in 0..f.relocs.len {
            let mut r = f.relocs.get(k);
            r.off += base;
            code_relocs.push(r);
        }
    }
    let code_sec = nsec;
    section(&mut out, 10, &c);
    nsec += 1;
    c.free();
    // data
    c = bn.new_bytes();
    c.uleb(@as(u64, un.segs.len));
    let mut data_relocs = u.List.[Reloc].{ data: u.nil.[Reloc](), len: 0, cap: 0 };
    let mut seg_offs = u.List.[u32].{ data: u.nil.[u32](), len: 0, cap: 0 };
    for i in 0..un.segs.len {
        let sg = un.segs.get(i);
        c.u8_(0);  // active, memory 0
        if un.wasm64 {
            c.u8_(0x42);
        } else {
            c.u8_(0x41);
        }
        c.sleb(0);
        c.u8_(0x0b);
        c.uleb(@as(u64, sg.data.len()));
        let base = @int_cast(u32, c.len());
        seg_offs.push(base);
        c.put(sg.data.view());
        for k in 0..sg.relocs.len {
            let mut r = sg.relocs.get(k);
            r.off += base;
            data_relocs.push(r);
        }
    }
    let data_sec = nsec;
    section(&mut out, 11, &c);
    nsec += 1;
    c.free();
    // linking
    c = bn.new_bytes();
    c.uleb(2);
    // symbol table
    let mut st = bn.new_bytes();
    st.uleb(@as(u64, un.syms.len));
    for i in 0..un.syms.len {
        let s = un.syms.get(i);
        st.u8_(s.kind);
        st.uleb(@as(u64, s.flags));
        if s.kind == SYM_DATA {
            st.name(s.name);
            if (s.flags & F_UNDEFINED) == 0 {
                st.uleb(@as(u64, s.seg));
                st.uleb(s.off);
                st.uleb(s.size);
            }
        } else {
            st.uleb(@as(u64, s.index));
            if ((s.flags & F_UNDEFINED) == 0) or ((s.flags & F_EXPLICIT_NAME) != 0) {
                st.name(s.name);
            }
        }
    }
    c.u8_(8);
    c.uleb(@as(u64, st.len()));
    c.put(st.view());
    st.free();
    // segment info
    if un.segs.len > 0 {
        st = bn.new_bytes();
        st.uleb(@as(u64, un.segs.len));
        for i in 0..un.segs.len {
            let sg = un.segs.get(i);
            st.name(sg.name);
            st.uleb(@as(u64, sg.align));
            let mut fl: u64 = 0;
            if sg.tls {
                fl = 2;
            }
            st.uleb(fl);
        }
        c.u8_(5);
        c.uleb(@as(u64, st.len()));
        c.put(st.view());
        st.free();
    }
    custom(&mut out, "linking", &c);
    c.free();
    if code_relocs.len > 0 {
        c = bn.new_bytes();
        c.uleb(@as(u64, code_sec));
        c.uleb(@as(u64, code_relocs.len));
        write_relocs(&mut c, &code_relocs, 0);
        custom(&mut out, "reloc.CODE", &c);
        c.free();
    }
    if data_relocs.len > 0 {
        c = bn.new_bytes();
        c.uleb(@as(u64, data_sec));
        c.uleb(@as(u64, data_relocs.len));
        write_relocs(&mut c, &data_relocs, 0);
        custom(&mut out, "reloc.DATA", &c);
        c.free();
    }
    // target features, so wasm-ld accepts the features we use
    c = bn.new_bytes();
    let feats = ["bulk-memory", "mutable-globals", "nontrapping-fptoint", "sign-ext", "simd128"];
    c.uleb(@as(u64, feats.len));
    for f in feats {
        c.u8_('+');
        c.name(f);
    }
    custom(&mut out, "target_features", &c);
    c.free();
    u.free(@ptr_cast(*mut void, code_relocs.data));
    u.free(@ptr_cast(*mut void, data_relocs.data));
    u.free(@ptr_cast(*mut void, seg_offs.data));
    let r = u.dup(out.view());
    out.free();
    return r;
}

// -----------------------------------------------------------------------------
// Decoding (objects written by write_object)
// -----------------------------------------------------------------------------
fn read_relocs(r: *mut bn.Reader, out: *mut u.List.[Reloc]) void {
    let n = r.uleb();
    for i in @as(u64, 0)..n {
        let t = r.u8_();
        let off = @truncate(u32, r.uleb());
        let sym = @truncate(u32, r.uleb());
        let mut add: i64 = 0;
        if reloc_has_addend(t) {
            add = r.sleb();
        }
        out.push(Reloc.{ ty: t, off: off, sym: sym, addend: add });
    }
}

// Returns nil if the bytes are not an object this compiler wrote.
pub fn read_object(d: []u8) *mut Unit {
    let mut r = bn.reader(d);
    if (d.len < 8) or not u.str_eq(d[0..4], "\x00asm") {
        return u.nil.[Unit]();
    }
    r.pos = 8;
    let un = new_unit(false);
    let mut code_start: usize = 0;
    let mut data_start: usize = 0;
    let mut code_relocs = u.List.[Reloc].{ data: u.nil.[Reloc](), len: 0, cap: 0 };
    let mut data_relocs = u.List.[Reloc].{ data: u.nil.[Reloc](), len: 0, cap: 0 };
    let mut ftypes = u.List.[u32].{ data: u.nil.[u32](), len: 0, cap: 0 };
    let mut body_offs = u.List.[usize].{ data: u.nil.[usize](), len: 0, cap: 0 };
    let mut seg_offs = u.List.[usize].{ data: u.nil.[usize](), len: 0, cap: 0 };
    let mut seg_names = u.List.[[]u8].{ data: u.nil.[[]u8](), len: 0, cap: 0 };
    let mut seg_aligns = u.List.[u32].{ data: u.nil.[u32](), len: 0, cap: 0 };
    let mut seg_tls = u.List.[bool].{ data: u.nil.[bool](), len: 0, cap: 0 };
    while not r.at_end() and not r.bad {
        let id = r.u8_();
        let size = @truncate(usize, r.uleb());
        let start = r.pos;
        let end = start + size;
        if id == 1 {
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                let s = r.pos;
                _ = r.u8_();
                let np = @truncate(usize, r.uleb());
                _ = r.bytes(np);
                let nr = @truncate(usize, r.uleb());
                _ = r.bytes(nr);
                un.types.push(u.dup(d[s..r.pos]));
            }
        } else if id == 2 {
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                let m = r.name();
                let nm = r.name();
                let kind = r.u8_();
                if kind == 0 {
                    let t = @truncate(u32, r.uleb());
                    un.fimports.push(Import.{ module: m, name: nm, ty: t, mutable: false });
                } else if kind == 1 {
                    _ = r.u8_();
                    let fl = r.u8_();
                    _ = r.uleb();
                    if (fl & 1) != 0 {
                        _ = r.uleb();
                    }
                    un.uses_table = true;
                } else if kind == 2 {
                    let fl = r.u8_();
                    un.wasm64 = (fl & 4) != 0;
                    _ = r.uleb();
                    if (fl & 1) != 0 {
                        _ = r.uleb();
                    }
                } else if kind == 3 {
                    let vt = r.u8_();
                    let mu = r.u8_() != 0;
                    un.gimports.push(Import.{ module: m, name: nm, ty: @as(u32, vt), mutable: mu });
                }
            }
        } else if id == 3 {
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                ftypes.push(@truncate(u32, r.uleb()));
            }
        } else if id == 6 {
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                let vt = r.u8_();
                let mu = r.u8_() != 0;
                _ = r.u8_();
                let init = r.sleb();
                _ = r.u8_();
                un.globals.push(Global.{ vt: vt, mutable: mu, init: init });
            }
        } else if id == 7 {
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                let nm = r.name();
                _ = r.u8_();
                let fi = @truncate(u32, r.uleb());
                un.exports.push(Export.{ name: nm, func: fi });
            }
        } else if id == 10 {
            code_start = start;
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                let sz = @truncate(usize, r.uleb());
                body_offs.push(r.pos - start);
                let f = u.new.[Func]();
                f.ty = ftypes.get(@truncate(usize, i));
                f.body.put(r.bytes(sz));
                un.funcs.push(f);
            }
        } else if id == 11 {
            data_start = start;
            let n = r.uleb();
            for i in @as(u64, 0)..n {
                _ = r.u8_();
                _ = r.u8_();
                _ = r.sleb();
                _ = r.u8_();
                let sz = @truncate(usize, r.uleb());
                seg_offs.push(r.pos - start);
                let sg = u.new.[Seg]();
                sg.data.put(r.bytes(sz));
                un.segs.push(sg);
            }
        } else if id == 0 {
            let nm = r.name();
            if u.str_eq(nm, "linking") {
                _ = r.uleb();
                while r.pos < end {
                    let sub = r.u8_();
                    let ssz = @truncate(usize, r.uleb());
                    let send = r.pos + ssz;
                    if sub == 8 {
                        let n = r.uleb();
                        for i in @as(u64, 0)..n {
                            let mut s = Sym.{ kind: r.u8_(), flags: 0, name: "", index: 0, seg: 0, off: 0, size: 0 };
                            s.flags = @truncate(u32, r.uleb());
                            if s.kind == SYM_DATA {
                                s.name = r.name();
                                if (s.flags & F_UNDEFINED) == 0 {
                                    s.seg = @truncate(u32, r.uleb());
                                    s.off = r.uleb();
                                    s.size = r.uleb();
                                }
                            } else {
                                s.index = @truncate(u32, r.uleb());
                                if ((s.flags & F_UNDEFINED) == 0) or ((s.flags & F_EXPLICIT_NAME) != 0) {
                                    s.name = r.name();
                                } else if (s.kind == SYM_FUNC) and (@as(usize, s.index) < un.fimports.len) {
                                    s.name = un.fimports.get(@as(usize, s.index)).name;
                                } else if (s.kind == SYM_GLOBAL) and (@as(usize, s.index) < un.gimports.len) {
                                    s.name = un.gimports.get(@as(usize, s.index)).name;
                                }
                            }
                            _ = add_sym(un, s);
                        }
                    } else if sub == 5 {
                        let n = r.uleb();
                        for i in @as(u64, 0)..n {
                            seg_names.push(r.name());
                            seg_aligns.push(@truncate(u32, r.uleb()));
                            seg_tls.push((r.uleb() & 2) != 0);
                        }
                    }
                    r.pos = send;
                }
            } else if u.str_eq(nm, "reloc.CODE") {
                _ = r.uleb();
                read_relocs(&mut r, &mut code_relocs);
            } else if u.str_eq(nm, "reloc.DATA") {
                _ = r.uleb();
                read_relocs(&mut r, &mut data_relocs);
            }
        }
        r.pos = end;
    }
    if r.bad {
        return u.nil.[Unit]();
    }
    // distribute relocations to functions and segments
    for i in 0..code_relocs.len {
        let x = code_relocs.get(i);
        let off = @as(usize, x.off);
        let mut k = body_offs.len;
        while (k > 0) and (body_offs.get(k - 1) > off) {
            k -= 1;
        }
        if k == 0 {
            continue;
        }
        let f = un.funcs.get(k - 1);
        f.relocs.push(Reloc.{ ty: x.ty, off: @int_cast(u32, off - body_offs.get(k - 1)), sym: x.sym, addend: x.addend });
    }
    for i in 0..data_relocs.len {
        let x = data_relocs.get(i);
        let off = @as(usize, x.off);
        let mut k = seg_offs.len;
        while (k > 0) and (seg_offs.get(k - 1) > off) {
            k -= 1;
        }
        if k == 0 {
            continue;
        }
        let sg = un.segs.get(k - 1);
        sg.relocs.push(Reloc.{ ty: x.ty, off: @int_cast(u32, off - seg_offs.get(k - 1)), sym: x.sym, addend: x.addend });
    }
    for i in 0..un.segs.len {
        let sg = un.segs.get(i);
        if i < seg_names.len {
            sg.name = seg_names.get(i);
            sg.align = seg_aligns.get(i);
            sg.tls = seg_tls.get(i);
        }
    }
    // function names and symbols
    for i in 0..un.syms.len {
        let s = un.syms.get(i);
        if (s.kind == SYM_FUNC) and ((s.flags & F_UNDEFINED) == 0) {
            let fi = @as(usize, s.index) - un.fimports.len;
            if fi < un.funcs.len {
                un.funcs.get(fi).sym = @int_cast(u32, i);
                un.funcs.get(fi).name = s.name;
            }
        }
    }
    return un;
}
