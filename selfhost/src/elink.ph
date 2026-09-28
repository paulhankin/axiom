// =============================================================================
// elink.ph - links ELF relocatable objects (from x64.ph / a64.ph, read with
// elf.read) into a dynamically linked Linux executable: position-dependent,
// eagerly bound (DF_BIND_NOW), needing libc.so.6 and the libraries the program
// names. The entry point is our own _start, which calls
// __libc_start_main(main, argc, argv, 0, 0, rtld_fini, stack_end).
//
// Layout (one 64 KiB-aligned PT_LOAD per permission; file offsets and
// addresses agree modulo the page size):
//   R   ELF and program headers, .interp, .dynsym, .dynstr, .hash, .rela.dyn,
//       .rodata
//   RX  _start, .plt, .text
//   RW  .dynamic, .got, .data (and .data.rel.ro), .tdata, then .bss
// Thread-local .tdata/.tbss form the PT_TLS template (local-exec model).
// =============================================================================
import "bin.ph" as bn;
import "elf.ph" as elf;
import "util.ph" as u;

const C_TEXT: i32 = 0;
const C_RODATA: i32 = 1;
const C_DATA: i32 = 2;
const C_TDATA: i32 = 3;
const C_TBSS: i32 = 4;
const C_BSS: i32 = 5;
const N_CLASSES: usize = 6;

const BASE: u64 = 0x400000;
const PAGE: u64 = 0x10000;

// A global (non-local) symbol of the link.
struct GSym {
    name: []u8,
    obj: i32,  // defining object, -1: none
    sym: u32,
    weak: bool,     // every reference / the definition is weak
    dynamic: bool,  // from a shared library
    dynidx: u32,    // .dynsym index
    got: i32,       // GOT slot, -1: none
    plt: i32,       // PLT stub, -1: none
    is_data: bool,  // a data object in its library
    lib: i32,       // the library defining it
    ver: []u8,      // its version there ("": unversioned)
    vidx: u32,      // .gnu.version index (1: unversioned)
}

struct Place {
    cls: i32,
    obj: u32,
    sec: u32,
    off: u64,
}

// A shared library: its SONAME and defined dynamic symbols.
struct Lib {
    soname: []u8,
    syms: u.SymMap.[u8],    // name -> 1 function/other, 2 data object
    vers: u.SymMap.[[]u8],  // name -> its default version (if versioned)
    used: bool,
    known: bool,  // its symbols were read
}

let mut M: u32 = 0;
let mut objs: u.List.[*mut elf.Obj] = u.List.[*mut elf.Obj].{ data: @ptr_from_int(*mut *mut elf.Obj, 0), len: 0,
                                                              cap: 0 };
let mut gsyms: u.List.[GSym] = u.List.[GSym].{ data: @ptr_from_int(*mut GSym, 0), len: 0, cap: 0 };
let mut gmap: u.SymMap.[u32] = u.SymMap.[u32].{ keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut u32, 0),
                                                cap: 0, len: 0 };
let mut libs: u.List.[*mut Lib] = u.List.[*mut Lib].{ data: @ptr_from_int(*mut *mut Lib, 0), len: 0, cap: 0 };
let mut sec_addr: u.List.[[]mut u64] = u.List.[[]mut u64].{ data: @ptr_from_int(*mut []mut u64, 0), len: 0, cap: 0 };
let mut local_got: u.List.[[]mut i32] = u.List.[[]mut i32].{ data: @ptr_from_int(*mut []mut i32, 0), len: 0, cap: 0 };
let mut got_obj: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };  // the symbol of each slot
let mut got_sym: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };
let mut got_dyn: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };   // dynsym index + 1, 0: static
let mut plt_syms: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };  // gsym index per stub
let mut dyn_syms: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };  // gsym index per dynsym - 1
let mut n_abs_dyn: usize = 0;
let mut errors: u32 = 0;

let mut tls_addr: u64 = 0;
let mut tls_memsz: u64 = 0;
let mut tls_align: u64 = 1;
let mut got_addr: u64 = 0;
let mut plt_addr: u64 = 0;

fn err(msg: []u8) void {
    u.eprint(u.f1("phalange: link error: %s\n", msg));
    errors += 1;
}

fn is_x64() bool {
    return M == elf.EM_X86_64;
}

fn align_up(x: u64, a: u64) u64 {
    if a <= 1 {
        return x;
    }
    return (x + a - 1) & ~(a - 1);
}

fn sec_class(s: *elf.Sec) i32 {
    if s.kind == elf.S_TEXT {
        return C_TEXT;
    }
    if s.kind == elf.S_RODATA {
        if u.starts(s.name, ".data.rel.ro") {
            return C_DATA;
        }
        return C_RODATA;
    }
    if s.kind == elf.S_DATA {
        return C_DATA;
    }
    if s.kind == elf.S_BSS {
        return C_BSS;
    }
    if s.kind == elf.S_TDATA {
        return C_TDATA;
    }
    return C_TBSS;
}

// -----------------------------------------------------------------------------
// Shared libraries
// -----------------------------------------------------------------------------
fn lib_dirs() [][]u8 {
    if is_x64() {
        let d = u.newn.[[]u8](5);
        d[0] = "/lib/x86_64-linux-gnu";
        d[1] = "/usr/lib/x86_64-linux-gnu";
        d[2] = "/lib64";
        d[3] = "/usr/lib64";
        d[4] = "/usr/lib";
        return d;
    }
    let d = u.newn.[[]u8](5);
    d[0] = "/lib/aarch64-linux-gnu";
    d[1] = "/usr/lib/aarch64-linux-gnu";
    d[2] = "/usr/aarch64-linux-gnu/lib";
    d[3] = "/lib64";
    d[4] = "/usr/lib64";
    return d;
}

fn find_file(name: []u8) []u8 {
    for dir in lib_dirs() {
        let p = u.f2("%s/%s", dir, name);
        if u.file_exists(p) {
            return p;
        }
    }
    return u.nosym();
}

// Reads the dynamic symbols (and DT_SONAME) of a shared object. False if it
// is not an ELF shared object of this machine (a linker script, say).
fn read_lib(l: *mut Lib, path: []u8) bool {
    let d = u.read_file(path);
    if u.isnos(d) or (d.len < 64) or not u.str_eq(d[0..4], "\x7fELF") or (d[4] != 2) {
        return false;
    }
    if (elf.rd16(d, 16) != 3) or (elf.rd16(d, 18) != @as(u64, M)) {
        return false;
    }
    let shoff = @truncate(usize, elf.rd64(d, 40));
    let shnum = @truncate(usize, elf.rd16(d, 60));
    // .gnu.version (per symbol) and .gnu.version_d (index -> name)
    let mut versym_off: usize = 0;
    let mut vnames = u.newn.[[]u8](64);
    for i in 0..shnum {
        let h = shoff + (i * 64);
        let ty = elf.rd32(d, h + 4);
        let off = @truncate(usize, elf.rd64(d, h + 24));
        if ty == 0x6fff_ffff {
            versym_off = off;
        } else if ty == 0x6fff_fffd {
            let strh = shoff + (@truncate(usize, elf.rd32(d, h + 40)) * 64);
            let stroff = @truncate(usize, elf.rd64(d, strh + 24));
            let n = @truncate(usize, elf.rd32(d, h + 44));  // sh_info: number of entries
            let mut e = off;
            for k in 0..n {
                let ndx = @truncate(usize, elf.rd16(d, e + 4));
                let aux = e + @truncate(usize, elf.rd32(d, e + 12));
                if ndx < vnames.len {
                    vnames[ndx] = u.intern(elf.cstr_at(d, stroff + @truncate(usize, elf.rd32(d, aux))));
                }
                let next = @truncate(usize, elf.rd32(d, e + 16));
                if (next == 0) or (k + 1 == n) {
                    break;
                }
                e += next;
            }
        }
    }
    for i in 0..shnum {
        let h = shoff + (i * 64);
        let ty = elf.rd32(d, h + 4);
        if (ty != 11) and (ty != 6) {  // SHT_DYNSYM, SHT_DYNAMIC
            continue;
        }
        let off = @truncate(usize, elf.rd64(d, h + 24));
        let size = @truncate(usize, elf.rd64(d, h + 32));
        let strh = shoff + (@truncate(usize, elf.rd32(d, h + 40)) * 64);
        let stroff = @truncate(usize, elf.rd64(d, strh + 24));
        if ty == 6 {
            let mut k: usize = 0;
            while (k + 16) <= size {
                if elf.rd64(d, off + k) == 14 {  // DT_SONAME
                    l.soname = u.dup(elf.cstr_at(d, stroff + @truncate(usize, elf.rd64(d, off + k + 8))));
                }
                k += 16;
            }
            continue;
        }
        let n = size / 24;
        for j in 1..n {
            let e = off + (j * 24);
            let shndx = elf.rd16(d, e + 6);
            let bind = d[e + 4] >> 4;
            if (shndx == 0) or (bind == elf.B_LOCAL) {
                continue;
            }
            let mut vs: u64 = 1;
            if versym_off != 0 {
                vs = elf.rd16(d, versym_off + (j * 2));
            }
            if (vs & 0x8000) != 0 {
                continue;  // a hidden (old) version: not what new references bind to
            }
            let name = u.intern(elf.cstr_at(d, stroff + @truncate(usize, elf.rd32(d, e))));
            let mut kind: u8 = 1;
            if (d[e + 4] & 15) == elf.T_OBJECT {
                kind = 2;
            }
            l.syms.put(name, kind);
            let vi = @truncate(usize, vs & 0x7fff);
            if (vi >= 2) and (vi < vnames.len) and not u.isnos(vnames[vi]) {
                l.vers.put(name, vnames[vi]);
            }
        }
    }
    l.known = true;
    return true;
}

fn add_lib(soname: []u8, file: []u8) *mut Lib {
    let l = u.new.[Lib]();
    l.soname = soname;
    let p = find_file(file);
    if not u.isnos(p) {
        if not read_lib(l, p) {
            return u.nil.[Lib]();
        }
    }
    libs.push(l);
    return l;
}

// -----------------------------------------------------------------------------
// Symbols
// -----------------------------------------------------------------------------
fn gsym_index(name: []u8) u32 {
    let k = u.intern(name);
    let prev = gmap.get(k);
    if prev != 0 {
        return prev - 1;
    }
    gsyms.push(GSym.{ name: k, obj: -1, sym: 0, weak: true, dynamic: false, dynidx: 0, got: -1, plt: -1, is_data: false,
                      lib: -1, ver: "", vidx: 1 });
    gmap.put(k, @int_cast(u32, gsyms.len));
    return @int_cast(u32, gsyms.len - 1);
}

fn collect_symbols() void {
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for si in 0..o.syms.len {
            let s = o.syms.get(si);
            if (s.bind == elf.B_LOCAL) or (s.name.len == 0) {
                continue;
            }
            let gi = gsym_index(s.name);
            let g = gsyms.at(@as(usize, gi));
            if s.sec < 0 {
                if s.bind != elf.B_WEAK {
                    g.weak = false;
                }
                continue;
            }
            if g.obj < 0 {
                g.obj = @int_cast(i32, oi);
                g.sym = @int_cast(u32, si);
                if s.bind != elf.B_WEAK {
                    g.weak = false;
                }
            } else if s.bind != elf.B_WEAK {
                let prev = objs.get(@as(usize, @bit_cast(u32, g.obj))).syms.get(@as(usize, g.sym));
                if prev.bind == elf.B_WEAK {
                    g.obj = @int_cast(i32, oi);
                    g.sym = @int_cast(u32, si);
                } else {
                    err(u.f1("duplicate symbol '%s'", s.name));
                }
                g.weak = false;
            }
        }
    }
    // Undefined symbols come from the shared libraries.
    let mut verified = true;
    for i in 0..libs.len {
        if not libs.get(i).known {
            verified = false;
        }
    }
    for gi in 0..gsyms.len {
        let g = gsyms.at(gi);
        if g.obj >= 0 {
            continue;
        }
        let mut found = false;
        for i in 0..libs.len {
            let l = libs.get(i);
            let k = l.syms.get(g.name);
            if k != 0 {
                l.used = true;
                found = true;
                g.is_data = k == 2;
                g.lib = @int_cast(i32, i);
                g.ver = l.vers.get(g.name);
                break;
            }
        }
        if found or not verified {
            g.dynamic = true;
        } else if not g.weak {
            err(u.f1("undefined symbol '%s'", g.name));
        }
        if g.dynamic {
            dyn_syms.push(@int_cast(u32, gi));
            g.dynidx = @int_cast(u32, dyn_syms.len);
        }
    }
}

// The global a symbol of an object stands for, or -1 for local symbols.
fn global_of(oi: usize, si: u32) i32 {
    let s = objs.get(oi).syms.at(@as(usize, si));
    if (s.bind == elf.B_LOCAL) or (s.name.len == 0) {
        return -1;
    }
    return @int_cast(i32, gmap.get(u.intern(s.name))) - 1;
}

fn sym_addr_in(oi: usize, si: u32) u64 {
    let s = objs.get(oi).syms.at(@as(usize, si));
    if s.sec < 0 {
        return 0;
    }
    return sec_addr.get(oi)[@as(usize, @bit_cast(u32, s.sec))] + s.value;
}

// The address of a symbol (0 for dynamic and missing weak ones).
fn sym_addr(oi: usize, si: u32) u64 {
    let gi = global_of(oi, si);
    if gi < 0 {
        return sym_addr_in(oi, si);
    }
    let g = gsyms.at(@int_cast(usize, gi));
    if g.obj < 0 {
        return 0;
    }
    return sym_addr_in(@as(usize, @bit_cast(u32, g.obj)), g.sym);
}

fn is_dynamic(oi: usize, si: u32) bool {
    let gi = global_of(oi, si);
    return (gi >= 0) and gsyms.at(@int_cast(usize, gi)).dynamic;
}

fn is_got_reloc(ty: u32) bool {
    if is_x64() {
        return (ty == 9) or (ty == 41) or (ty == 42);  // GOTPCREL, GOTPCRELX, REX_GOTPCRELX
    }
    return (ty == elf.R_AARCH64_ADR_GOT_PAGE) or (ty == elf.R_AARCH64_LD64_GOT_LO12_NC);
}

fn is_call_reloc(ty: u32) bool {
    if is_x64() {
        return (ty == elf.R_X86_64_PLT32) or (ty == elf.R_X86_64_PC32);
    }
    return (ty == elf.R_AARCH64_CALL26) or (ty == elf.R_AARCH64_JUMP26);
}

fn is_abs64(ty: u32) bool {
    if is_x64() {
        return ty == elf.R_X86_64_64;
    }
    return ty == elf.R_AARCH64_ABS64;
}

fn new_got(oi: usize, si: u32, dynidx: u32) i32 {
    got_obj.push(@int_cast(u32, oi));
    got_sym.push(si);
    got_dyn.push(dynidx);
    return @int_cast(i32, got_dyn.len - 1);
}

fn got_slot_g(gi: u32, oi: usize, si: u32) i32 {
    let g = gsyms.at(@as(usize, gi));
    if g.got < 0 {
        let mut d: u32 = 0;
        if g.dynamic {
            d = g.dynidx;
        }
        g.got = new_got(oi, si, d);
    }
    return g.got;
}

// The GOT slot of a symbol, allocated on first use.
fn got_slot(oi: usize, si: u32) i32 {
    let gi = global_of(oi, si);
    if gi >= 0 {
        return got_slot_g(@bit_cast(u32, gi), oi, si);
    }
    let lg = local_got.get(oi);
    if lg[@as(usize, si)] == 0 {
        lg[@as(usize, si)] = new_got(oi, si, 0) + 1;
    }
    return lg[@as(usize, si)] - 1;
}

fn plt_stub(gi: u32) i32 {
    let g = gsyms.at(@as(usize, gi));
    if g.plt < 0 {
        g.plt = @int_cast(i32, plt_syms.len);
        plt_syms.push(gi);
    }
    return g.plt;
}

// Which GOT slots, PLT stubs and dynamic relocations the relocations need.
fn scan_relocs() void {
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for k in 0..o.secs.len {
            let s = o.secs.get(k);
            for r in s.relocs.items() {
                if is_got_reloc(r.ty) {
                    _ = got_slot(oi, r.sym);
                } else if is_dynamic(oi, r.sym) {
                    let gi = @bit_cast(u32, global_of(oi, r.sym));
                    if is_call_reloc(r.ty) {
                        let g = gsyms.at(@as(usize, gi));
                        if g.is_data {
                            err(u.f1("'%s' is data in a shared library; it cannot be referenced directly", g.name));
                        }
                        // a GOT slot for the stub to jump through
                        _ = got_slot(oi, r.sym);
                        _ = plt_stub(gi);
                    } else if is_abs64(r.ty) {
                        if sec_class(s) != C_DATA {
                            err(u.f1("absolute reference to '%s' in read-only data", gsyms.at(@as(usize, gi)).name));
                        }
                        n_abs_dyn += 1;
                    } else {
                        err(u.f1("unsupported reference to shared library symbol '%s'", gsyms.at(@as(usize, gi)).name));
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
fn w16(b: []mut u8, off: u64, x: u64) void {
    let o = @truncate(usize, off);
    b[o] = @truncate(u8, x);
    b[o + 1] = @truncate(u8, x >> 8);
}

fn w32(b: []mut u8, off: u64, x: u64) void {
    let o = @truncate(usize, off);
    for k in @as(usize, 0)..4 {
        b[o + k] = @truncate(u8, x >> @as(u64, k * 8));
    }
}

fn w64(b: []mut u8, off: u64, x: u64) void {
    w32(b, off, x & 0xFFFF_FFFF);
    w32(b, off + 4, x >> 32);
}

fn r32(b: []u8, off: u64) u64 {
    let o = @truncate(usize, off);
    return @as(u64, b[o]) | (@as(u64, b[o + 1]) << 8) | (@as(u64, b[o + 2]) << 16) | (@as(u64, b[o + 3]) << 24);
}

fn check32(v: i64, what: []u8) void {
    if (v < -0x8000_0000) or (v > 0x7fff_ffff) {
        err(u.f1("relocation out of range (%s)", what));
    }
}

// AArch64 instruction fields.
fn a64_adrp(b: []mut u8, off: u64, p: u64, target: u64) void {
    let pages = (@bit_cast(i64, target & ~@as(u64, 0xfff)) - @bit_cast(i64, p & ~@as(u64, 0xfff))) >> 12;
    if (pages < -0x10_0000) or (pages >= 0x10_0000) {
        err("adrp out of range");
    }
    let v = @bit_cast(u64, pages) & 0x1f_ffff;
    let ins = (r32(b, off) & 0x9f00_001f) | ((v & 3) << 29) | ((v >> 2) << 5);
    w32(b, off, ins);
}

fn a64_imm12(b: []mut u8, off: u64, x: u64) void {
    let ins = (r32(b, off) & 0xffc0_03ff) | ((x & 0xfff) << 10);
    w32(b, off, ins);
}

fn a64_branch(b: []mut u8, off: u64, p: u64, target: u64) void {
    let d = @bit_cast(i64, target -% p);
    if (d < -0x800_0000) or (d >= 0x800_0000) {
        err("branch out of range");
    }
    let ins = (r32(b, off) & 0xfc00_0000) | ((@bit_cast(u64, d) >> 2) & 0x3ff_ffff);
    w32(b, off, ins);
}

fn elf_hash(name: []u8) u64 {
    let mut h: u64 = 0;
    for c in name {
        h = (h << 4) + @as(u64, c);
        let g = h & 0xf000_0000;
        if g != 0 {
            h = h ^ (g >> 24);
        }
        h = h & ~g;
    }
    return h;
}

fn tp_offset(addr: u64) i64 {
    let off = @bit_cast(i64, addr -% tls_addr);
    if is_x64() {
        return off - @bit_cast(i64, align_up(tls_memsz, tls_align));  // variant II: the block ends at tp
    }
    return off + @bit_cast(i64, align_up(16, tls_align));  // variant I: after the 16-byte TCB
}

struct DynRel {
    off: u64,
    info: u64,
    addend: i64,
}

fn apply_relocs(b: []mut u8, rels: *mut u.List.[DynRel]) void {
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for k in 0..o.secs.len {
            let s = o.secs.get(k);
            let base = sec_addr.get(oi)[k];
            let cls = sec_class(s);
            if (cls == C_BSS) or (cls == C_TBSS) {
                continue;
            }
            for r in s.relocs.items() {
                let p = base + r.off;
                let fo = file_of(p);
                let dynamic = is_dynamic(oi, r.sym);
                let mut sv = sym_addr(oi, r.sym);
                if dynamic and is_call_reloc(r.ty) {
                    let gi = @bit_cast(u32, global_of(oi, r.sym));
                    sv = plt_addr + (@as(u64, @bit_cast(u32, gsyms.at(@as(usize, gi)).plt)) * 16);
                }
                let a = r.addend;
                let t = r.ty;
                if is_got_reloc(t) {
                    let slot = got_slot(oi, r.sym);
                    let ga = got_addr + (@as(u64, @bit_cast(u32, slot)) * 8);
                    if is_x64() {
                        // mov r, [rip + GOT] -> lea r, [rip + sym] when static
                        if not dynamic and (t == 42) and (fo >= 2) and (b[@truncate(usize, fo - 2)] == 0x8b) {
                            b[@truncate(usize, fo - 2)] = 0x8d;
                            let v = @bit_cast(i64, sv +% @bit_cast(u64, a)) - @bit_cast(i64, p);
                            check32(v, "lea");
                            w32(b, fo, @bit_cast(u64, v) & 0xFFFF_FFFF);
                            continue;
                        }
                        let v = @bit_cast(i64, ga +% @bit_cast(u64, a)) - @bit_cast(i64, p);
                        check32(v, "GOT");
                        w32(b, fo, @bit_cast(u64, v) & 0xFFFF_FFFF);
                    } else if t == elf.R_AARCH64_ADR_GOT_PAGE {
                        a64_adrp(b, fo, p, ga);
                    } else {
                        a64_imm12(b, fo, (ga & 0xfff) >> 3);
                    }
                    continue;
                }
                let sa = sv +% @bit_cast(u64, a);
                if is_abs64(t) {
                    if dynamic {
                        let gi = @bit_cast(u32, global_of(oi, r.sym));
                        let di = @as(u64, gsyms.at(@as(usize, gi)).dynidx);
                        let mut rt: u64 = 257;  // R_AARCH64_ABS64
                        if is_x64() {
                            rt = 1;  // R_X86_64_64
                        }
                        rels.push(DynRel.{ off: p, info: (di << 32) | rt, addend: a });
                        w64(b, fo, 0);
                    } else {
                        w64(b, fo, sa);
                    }
                    continue;
                }
                if is_x64() {
                    if (t == elf.R_X86_64_PC32) or (t == elf.R_X86_64_PLT32) {
                        let v = @bit_cast(i64, sa) - @bit_cast(i64, p);
                        check32(v, "pc32");
                        w32(b, fo, @bit_cast(u64, v) & 0xFFFF_FFFF);
                    } else if t == elf.R_X86_64_TPOFF32 {
                        let v = tp_offset(sa);
                        check32(v, "tpoff32");
                        w32(b, fo, @bit_cast(u64, v) & 0xFFFF_FFFF);
                    } else if (t == 10) or (t == 11) {  // R_X86_64_32, 32S
                        w32(b, fo, sa & 0xFFFF_FFFF);
                    } else {
                        err(u.f1("unsupported relocation type %s", u.udec(@as(u64, t))));
                    }
                    continue;
                }
                if t == elf.R_AARCH64_PREL32 {
                    let v = @bit_cast(i64, sa) - @bit_cast(i64, p);
                    check32(v, "prel32");
                    w32(b, fo, @bit_cast(u64, v) & 0xFFFF_FFFF);
                } else if t == elf.R_AARCH64_ADR_PREL_PG_HI21 {
                    a64_adrp(b, fo, p, sa);
                } else if (t == elf.R_AARCH64_ADD_ABS_LO12_NC) or (t == elf.R_AARCH64_LDST8_ABS_LO12_NC) {
                    a64_imm12(b, fo, sa & 0xfff);
                } else if t == elf.R_AARCH64_LDST16_ABS_LO12_NC {
                    a64_imm12(b, fo, (sa & 0xfff) >> 1);
                } else if t == elf.R_AARCH64_LDST32_ABS_LO12_NC {
                    a64_imm12(b, fo, (sa & 0xfff) >> 2);
                } else if t == elf.R_AARCH64_LDST64_ABS_LO12_NC {
                    a64_imm12(b, fo, (sa & 0xfff) >> 3);
                } else if t == elf.R_AARCH64_LDST128_ABS_LO12_NC {
                    a64_imm12(b, fo, (sa & 0xfff) >> 4);
                } else if (t == elf.R_AARCH64_CALL26) or (t == elf.R_AARCH64_JUMP26) {
                    a64_branch(b, fo, p, sa);
                } else if t == elf.R_AARCH64_TLSLE_ADD_TPREL_HI12 {
                    let v = tp_offset(sa);
                    if (v < 0) or (v >= 0x100_0000) {
                        err("TLS offset out of range");
                    }
                    a64_imm12(b, fo, @bit_cast(u64, v) >> 12);
                } else if t == elf.R_AARCH64_TLSLE_ADD_TPREL_LO12_NC {
                    a64_imm12(b, fo, @bit_cast(u64, tp_offset(sa)) & 0xfff);
                } else {
                    err(u.f1("unsupported relocation type %s", u.udec(@as(u64, t))));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Layout and the executable
// -----------------------------------------------------------------------------
struct Seg {
    vaddr: u64,
    off: u64,
    filesz: u64,
    memsz: u64,
}

let mut segs: [3]Seg = [Seg.{ vaddr: 0, off: 0, filesz: 0, memsz: 0 }, Seg.{ vaddr: 0, off: 0, filesz: 0, memsz: 0 },
                        Seg.{ vaddr: 0, off: 0, filesz: 0, memsz: 0 }];

fn file_of(addr: u64) u64 {
    for k in @as(usize, 0)..3 {
        let sg = segs[k];
        if (addr >= sg.vaddr) and (addr < (sg.vaddr + sg.filesz)) {
            return sg.off + (addr - sg.vaddr);
        }
    }
    err("internal: address outside the file");
    return 0;
}

// Places the sections of class `cls` from address `at`; returns the end.
fn place(cls: i32, at: u64) u64 {
    let mut cur = at;
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for k in 0..o.secs.len {
            let s = o.secs.get(k);
            if sec_class(s) != cls {
                continue;
            }
            cur = align_up(cur, s.align);
            sec_addr.get(oi)[k] = cur;
            cur += elf.sec_size(s);
        }
    }
    return cur;
}

fn class_align(cls: i32) u64 {
    let mut a: u64 = 1;
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for k in 0..o.secs.len {
            let s = o.secs.get(k);
            if (sec_class(s) == cls) and (s.align > a) {
                a = s.align;
            }
        }
    }
    return a;
}

fn copy_class(b: []mut u8, cls: i32) void {
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for k in 0..o.secs.len {
            let s = o.secs.get(k);
            if sec_class(s) != cls {
                continue;
            }
            let d = s.data.view();
            if d.len == 0 {
                continue;
            }
            let fo = @truncate(usize, file_of(sec_addr.get(oi)[k]));
            for i in 0..d.len {
                b[fo + i] = d[i];
            }
        }
    }
}

fn next_seg(prev: *Seg, file_end: u64) Seg {
    let off = align_up(file_end, 64);
    let vaddr = align_up(prev.vaddr + prev.memsz, PAGE) + (off % PAGE);
    return Seg.{ vaddr: vaddr, off: off, filesz: 0, memsz: 0 };
}

fn phdr(h: *mut bn.Bytes, ty: u64, flags: u64, off: u64, vaddr: u64, filesz: u64, memsz: u64, al: u64) void {
    h.u32_(ty);
    h.u32_(flags);
    h.u64_(off);
    h.u64_(vaddr);
    h.u64_(vaddr);
    h.u64_(filesz);
    h.u64_(memsz);
    h.u64_(al);
}

const START_SIZE: u64 = 64;

fn write_start(b: []mut u8, start: u64, main_addr: u64, lsm_got: u64) void {
    let fo = file_of(start);
    if is_x64() {
        let code =
            "\x31\xed\x49\x89\xd1\x5e\x48\x89\xe2\x48\x83\xe4\xf0\x50\x54\x45\x31\xc0\x31\xc9\x48\x8d\x3d\x00\x00\x00\x00\xff\x15\x00\x00\x00\x00\xf4";
        for i in 0..code.len {
            b[@truncate(usize, fo) + i] = code[i];
        }
        w32(b, fo + 23, (main_addr -% (start + 27)) & 0xFFFF_FFFF);
        w32(b, fo + 29, (lsm_got -% (start + 33)) & 0xFFFF_FFFF);
        return;
    }
    let words: [14]u64 = [0xAA1F03FD, 0xAA1F03FE, 0xAA0003E5, 0xF94003E1, 0x910023E2, 0x910003E6, 0x90000000,
                          0x91000000, 0xAA1F03E3, 0xAA1F03E4, 0x90000010, 0xF9400210, 0xD63F0200, 0xD4200000];
    for i in @as(usize, 0)..14 {
        w32(b, fo + (@as(u64, i) * 4), words[i]);
    }
    a64_adrp(b, fo + 24, start + 24, main_addr);
    a64_imm12(b, fo + 28, main_addr & 0xfff);
    a64_adrp(b, fo + 40, start + 40, lsm_got);
    a64_imm12(b, fo + 44, (lsm_got & 0xfff) >> 3);
}

fn write_plt(b: []mut u8) void {
    for i in 0..plt_syms.len {
        let g = gsyms.at(@as(usize, plt_syms.get(i)));
        let stub = plt_addr + (@as(u64, i) * 16);
        let ga = got_addr + (@as(u64, @bit_cast(u32, g.got)) * 8);
        let fo = file_of(stub);
        if is_x64() {
            // jmp [rip + GOT]; ud2 padding
            b[@truncate(usize, fo)] = 0xff;
            b[@truncate(usize, fo) + 1] = 0x25;
            w32(b, fo + 2, (ga -% (stub + 6)) & 0xFFFF_FFFF);
            for k in @as(usize, 6)..16 {
                b[@truncate(usize, fo) + k] = 0xcc;
            }
        } else {
            // adrp x16, GOT; ldr x17, [x16, lo12]; br x17; nop
            w32(b, fo, 0x90000010);
            w32(b, fo + 4, 0xF9400211);
            w32(b, fo + 8, 0xD61F0220);
            w32(b, fo + 12, 0xD503201F);
            a64_adrp(b, fo, stub, ga);
            a64_imm12(b, fo + 4, (ga & 0xfff) >> 3);
        }
    }
}

// Links `inputs` into the executable `out`. `lib_names` are the libraries
// the program names (-l). 0: done; 1: errors (reported); 2: a library is not
// a plain shared object (the caller links with the system toolchain).
pub fn link(inputs: []*mut elf.Obj, machine: u32, lib_names: [][]u8, out: []u8) i32 {
    M = machine;
    for i in 0..inputs.len {
        objs.push(inputs[i]);
    }
    let libc = add_lib("libc.so.6", "libc.so.6");
    let libm = add_lib("libm.so.6", "libm.so.6");
    if u.isnil(libc) or u.isnil(libm) {
        err("cannot read the C library");
        return 1;
    }
    libc.used = true;
    for name in lib_names {
        let builtin = ["c", "m", "pthread", "dl", "rt"];
        let mut skip = false;
        for x in builtin {
            if u.str_eq(x, name) {
                skip = true;
            }
        }
        if skip {
            continue;
        }
        let file = u.f1("lib%s.so", name);
        let p = find_file(file);
        if u.isnos(p) {
            err(u.f1("cannot find library '%s'", name));
            continue;
        }
        let l = u.new.[Lib]();
        l.soname = file;
        if not read_lib(l, p) {
            return 2;
        }
        l.used = true;
        libs.push(l);
    }
    let lsm = gsym_index("__libc_start_main");
    gsyms.at(@as(usize, lsm)).weak = false;
    let mn = gsym_index("main");
    gsyms.at(@as(usize, mn)).weak = false;
    collect_symbols();
    if gsyms.at(@as(usize, mn)).obj < 0 {
        err("no 'main' function");
    }
    for oi in 0..objs.len {
        sec_addr.push(u.newn.[u64](objs.get(oi).secs.len + 1));
        local_got.push(u.newn.[i32](objs.get(oi).syms.len + 1));
    }
    scan_relocs();
    if errors != 0 {
        return 1;
    }
    let lsm_slot = got_slot_g(lsm, 0, 0);

    // .dynstr
    let mut dynstr = bn.new_bytes();
    dynstr.u8_(0);
    let mut needed = u.List.[u64].{ data: u.nil.[u64](), len: 0, cap: 0 };
    for i in 0..libs.len {
        let l = libs.get(i);
        if l.used {
            needed.push(@as(u64, dynstr.len()));
            dynstr.put(l.soname);
            dynstr.u8_(0);
        }
    }
    // Symbol versions: which versions of each library the symbols need.
    let mut vn_lib = u.List.[u32].{ data: u.nil.[u32](), len: 0, cap: 0 };
    let mut vn_name = u.List.[[]u8].{ data: u.nil.[[]u8](), len: 0, cap: 0 };
    let mut vn_off = u.List.[u64].{ data: u.nil.[u64](), len: 0, cap: 0 };
    for i in 0..dyn_syms.len {
        let g = gsyms.at(@as(usize, dyn_syms.get(i)));
        if u.isnos(g.ver) or (g.ver.len == 0) or (g.lib < 0) {
            continue;
        }
        let mut found: u32 = 0;
        for k in 0..vn_lib.len {
            if (vn_lib.get(k) == @bit_cast(u32, g.lib)) and u.same(vn_name.get(k), g.ver) {
                found = @int_cast(u32, k) + 2;
            }
        }
        if found == 0 {
            vn_lib.push(@bit_cast(u32, g.lib));
            vn_name.push(g.ver);
            vn_off.push(@as(u64, dynstr.len()));
            dynstr.put(g.ver);
            dynstr.u8_(0);
            found = @int_cast(u32, vn_lib.len) + 1;
        }
        g.vidx = found;
    }
    let lib_off = u.newn.[u64](libs.len + 1);
    let mut k_needed: usize = 0;
    for i in 0..libs.len {
        if libs.get(i).used {
            lib_off[i] = needed.get(k_needed);
            k_needed += 1;
        }
    }
    let mut n_vlibs: u64 = 0;
    for i in 0..libs.len {
        for k in 0..vn_lib.len {
            if vn_lib.get(k) == @int_cast(u32, i) {
                n_vlibs += 1;
                break;
            }
        }
    }
    let verneed_size = (n_vlibs * 16) + (@as(u64, vn_lib.len) * 16);
    let name_off = u.newn.[u64](dyn_syms.len + 1);
    for i in 0..dyn_syms.len {
        name_off[i] = @as(u64, dynstr.len());
        dynstr.put(gsyms.at(@as(usize, dyn_syms.get(i))).name);
        dynstr.u8_(0);
    }
    let interp = u.pick(is_x64(), "/lib64/ld-linux-x86-64.so.2", "/lib/ld-linux-aarch64.so.1");
    let has_tls = (class_align(C_TDATA) > 1) or (class_align(C_TBSS) > 1) or (place(C_TDATA, 0) > 0) or
                  (place(C_TBSS, 0) > 0);
    let mut phnum: u64 = 7;
    if has_tls {
        phnum = 8;
    }
    let nsyms = @as(u64, dyn_syms.len) + 1;
    let n_rela = @as(u64, n_abs_dyn);
    let mut n_got_dyn: u64 = 0;
    for i in 0..got_dyn.len {
        if got_dyn.get(i) != 0 {
            n_got_dyn += 1;
        }
    }

    // Segment 1 (R): headers, dynamic linking tables, .rodata.
    let interp_off = 64 + (phnum * 56);
    let dynsym_off = align_up(interp_off + @as(u64, interp.len) + 1, 8);
    let dynstr_off = dynsym_off + (nsyms * 24);
    let hash_off = align_up(dynstr_off + @as(u64, dynstr.len()), 8);
    let versym_off = align_up(hash_off + (4 * (2 + 1 + nsyms)), 2);
    let verneed_off = align_up(versym_off + (2 * nsyms), 8);
    let rela_off = align_up(verneed_off + verneed_size, 8);
    let rela_size = (n_rela + n_got_dyn) * 24;
    segs[0] = Seg.{ vaddr: BASE, off: 0, filesz: 0, memsz: 0 };
    let ro_end = place(C_RODATA, BASE + rela_off + rela_size);
    segs[0].filesz = ro_end - BASE;
    segs[0].memsz = segs[0].filesz;

    // Segment 2 (RX): _start, PLT, .text.
    segs[1] = next_seg(&segs[0], segs[0].filesz);
    let start = segs[1].vaddr;
    plt_addr = start + START_SIZE;
    let text_end = place(C_TEXT, plt_addr + (@as(u64, plt_syms.len) * 16));
    segs[1].filesz = text_end - segs[1].vaddr;
    segs[1].memsz = segs[1].filesz;

    // Segment 3 (RW): .dynamic, .got, .data, .tdata; .bss after the file.
    segs[2] = next_seg(&segs[1], segs[1].off + segs[1].filesz);
    let dyn_addr = segs[2].vaddr;
    let n_dyn = @as(u64, needed.len) + 16;
    got_addr = align_up(dyn_addr + (n_dyn * 16), 8);
    let data_end = place(C_DATA, got_addr + (@as(u64, got_dyn.len) * 8));
    tls_align = class_align(C_TDATA);
    if class_align(C_TBSS) > tls_align {
        tls_align = class_align(C_TBSS);
    }
    tls_addr = align_up(data_end, tls_align);
    let tdata_end = place(C_TDATA, tls_addr);
    let tbss_end = place(C_TBSS, tdata_end);
    tls_memsz = tbss_end - tls_addr;
    let mut file_end3 = data_end;
    if has_tls {
        file_end3 = tdata_end;
    }
    let bss_end = place(C_BSS, file_end3);
    segs[2].filesz = file_end3 - segs[2].vaddr;
    segs[2].memsz = bss_end - segs[2].vaddr;

    let total = segs[2].off + segs[2].filesz;
    let mut outb = bn.new_bytes();
    outb.zeros(@truncate(usize, total));
    let b = outb.view();

    // contents
    copy_class(b, C_RODATA);
    copy_class(b, C_TEXT);
    copy_class(b, C_DATA);
    copy_class(b, C_TDATA);
    let main_addr = sym_addr(@as(usize, @bit_cast(u32, gsyms.at(@as(usize, mn)).obj)), gsyms.at(@as(usize, mn)).sym);
    write_start(b, start, main_addr, got_addr + (@as(u64, @bit_cast(u32, lsm_slot)) * 8));
    write_plt(b);
    let mut rels = u.List.[DynRel].{ data: u.nil.[DynRel](), len: 0, cap: 0 };
    apply_relocs(b, &mut rels);
    let mut glob_dat: u64 = 1025;  // R_AARCH64_GLOB_DAT
    if is_x64() {
        glob_dat = 6;  // R_X86_64_GLOB_DAT
    }
    for i in 0..got_dyn.len {
        let at = got_addr + (@as(u64, i) * 8);
        let d = got_dyn.get(i);
        if d != 0 {
            rels.push(DynRel.{ off: at, info: (@as(u64, d) << 32) | glob_dat, addend: 0 });
        } else {
            w64(b, file_of(at), sym_addr(@as(usize, got_obj.get(i)), got_sym.get(i)));
        }
    }

    // .interp, .dynsym, .dynstr, .hash, .rela.dyn
    for i in 0..interp.len {
        b[@truncate(usize, interp_off) + i] = interp[i];
    }
    for i in 0..dyn_syms.len {
        let g = gsyms.at(@as(usize, dyn_syms.get(i)));
        let e = dynsym_off + ((@as(u64, i) + 1) * 24);
        w32(b, e, name_off[i]);
        let mut bind: u8 = elf.B_GLOBAL;
        if g.weak {
            bind = elf.B_WEAK;
        }
        let mut t: u8 = elf.T_FUNC;
        if g.is_data {
            t = elf.T_OBJECT;
        }
        b[@truncate(usize, e) + 4] = (bind << 4) | t;
    }
    let ds = dynstr.view();
    for i in 0..ds.len {
        b[@truncate(usize, dynstr_off) + i] = ds[i];
    }
    w32(b, hash_off, 1);          // nbucket
    w32(b, hash_off + 4, nsyms);  // nchain (the buckets and chains stay empty)
    // .gnu.version, .gnu.version_r
    for i in 0..dyn_syms.len {
        w16(b, versym_off + ((@as(u64, i) + 1) * 2), @as(u64, gsyms.at(@as(usize, dyn_syms.get(i))).vidx));
    }
    let mut vo = verneed_off;
    let mut left = n_vlibs;
    for i in 0..libs.len {
        let mut cnt: u64 = 0;
        for k in 0..vn_lib.len {
            if vn_lib.get(k) == @int_cast(u32, i) {
                cnt += 1;
            }
        }
        if cnt == 0 {
            continue;
        }
        left -= 1;
        w16(b, vo, 1);
        w16(b, vo + 2, cnt);
        w32(b, vo + 4, lib_off[i]);
        w32(b, vo + 8, 16);
        let mut next: u64 = 0;
        if left > 0 {
            next = 16 + (16 * cnt);
        }
        w32(b, vo + 12, next);
        let mut ao = vo + 16;
        let mut done: u64 = 0;
        for k in 0..vn_lib.len {
            if vn_lib.get(k) != @int_cast(u32, i) {
                continue;
            }
            done += 1;
            w32(b, ao, elf_hash(vn_name.get(k)));
            w16(b, ao + 4, 0);
            w16(b, ao + 6, @as(u64, k) + 2);
            w32(b, ao + 8, vn_off.get(k));
            let mut an: u64 = 0;
            if done < cnt {
                an = 16;
            }
            w32(b, ao + 12, an);
            ao += 16;
        }
        vo = ao;
    }
    for i in 0..rels.len {
        let r = rels.get(i);
        let e = rela_off + (@as(u64, i) * 24);
        w64(b, e, r.off);
        w64(b, e + 8, r.info);
        w64(b, e + 16, @bit_cast(u64, r.addend));
    }

    // .dynamic
    let mut dy = bn.new_bytes();
    for i in 0..needed.len {
        dy.u64_(1);  // DT_NEEDED
        dy.u64_(needed.get(i));
    }
    let tags: [9]u64 = [4, 5, 6, 10, 11, 7, 8, 9, 30];
    let vals: [9]u64 = [BASE + hash_off, BASE + dynstr_off, BASE + dynsym_off, @as(u64, ds.len), 24, BASE + rela_off,
                        @as(u64, rels.len) * 24, 24, 8];  // DF_BIND_NOW
    for i in @as(usize, 0)..9 {
        dy.u64_(tags[i]);
        dy.u64_(vals[i]);
    }
    if n_vlibs > 0 {
        dy.u64_(0x6fff_fff0);  // DT_VERSYM
        dy.u64_(BASE + versym_off);
        dy.u64_(0x6fff_fffe);  // DT_VERNEED
        dy.u64_(BASE + verneed_off);
        dy.u64_(0x6fff_ffff);  // DT_VERNEEDNUM
        dy.u64_(n_vlibs);
    }
    dy.u64_(0x6fff_fffb);  // DT_FLAGS_1: DF_1_NOW
    dy.u64_(1);
    dy.u64_(21);  // DT_DEBUG
    dy.u64_(0);
    dy.u64_(0);  // DT_NULL
    dy.u64_(0);
    let dv = dy.view();
    let dfo = file_of(dyn_addr);
    for i in 0..dv.len {
        b[@truncate(usize, dfo) + i] = dv[i];
    }

    // ELF header and program headers
    let mut h = bn.new_bytes();
    h.put("\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00");
    h.u16_(2);  // ET_EXEC
    h.u16_(@as(u64, M));
    h.u32_(1);
    h.u64_(start);
    h.u64_(64);  // e_phoff
    h.u64_(0);   // e_shoff
    h.u32_(0);
    h.u16_(64);
    h.u16_(56);
    h.u16_(phnum);
    h.u16_(64);
    h.u16_(0);
    h.u16_(0);
    phdr(&mut h, 6, 4, 64, BASE + 64, phnum * 56, phnum * 56, 8);  // PT_PHDR
    phdr(&mut h, 3, 4, interp_off, BASE + interp_off, @as(u64, interp.len) + 1, @as(u64, interp.len) + 1, 1);
    let flags: [3]u64 = [4, 5, 6];
    for k in @as(usize, 0)..3 {
        let sg = segs[k];
        phdr(&mut h, 1, flags[k], sg.off, sg.vaddr, sg.filesz, sg.memsz, PAGE);
    }
    phdr(&mut h, 2, 6, dfo, dyn_addr, n_dyn * 16, n_dyn * 16, 8);  // PT_DYNAMIC
    if has_tls {
        phdr(&mut h, 7, 4, segs[2].off + (tls_addr - segs[2].vaddr), tls_addr, tdata_end - tls_addr, tls_memsz,
             tls_align);
    }
    phdr(&mut h, 0x6474_e551, 6, 0, 0, 0, 0, 16);  // PT_GNU_STACK
    let hv = h.view();
    for i in 0..hv.len {
        b[i] = hv[i];
    }
    if errors != 0 {
        return 1;
    }
    // a symbol table (for debuggers and profilers), with section headers
    // over the segments for its symbols to refer to
    let mut st = bn.new_bytes();
    let mut stl = bn.new_bytes();  // the local symbols, then the others
    let mut stg = bn.new_bytes();
    let mut str = bn.new_bytes();
    str.u8_(0);
    stl.zeros(24);
    let mut nlocal: u64 = 1;
    for oi in 0..objs.len {
        let o = objs.get(oi);
        for si in 0..o.syms.len {
            let sy = o.syms.at(si);
            if (sy.sec < 0) or (sy.name.len == 0) or (sy.name[0] == '.') or
               ((sy.ty != elf.T_FUNC) and (sy.ty != elf.T_OBJECT)) {
                continue;
            }
            let k = o.secs.get(@as(usize, @bit_cast(u32, sy.sec))).kind;
            if (k == elf.S_TDATA) or (k == elf.S_TBSS) {
                continue;
            }
            let gi = global_of(oi, @int_cast(u32, si));
            if (gi >= 0) and (gsyms.at(@int_cast(usize, gi)).obj != @int_cast(i32, oi)) {
                continue;  // a copy the link did not keep
            }
            let a = sym_addr_in(oi, @int_cast(u32, si));
            let mut shndx: u64 = 0xfff1;
            if (a >= segs[1].vaddr) and (a < (segs[1].vaddr + segs[1].memsz)) {
                shndx = 1;
            } else if (a >= segs[0].vaddr) and (a < (segs[0].vaddr + segs[0].memsz)) {
                shndx = 2;
            } else if (a >= segs[2].vaddr) and (a < (segs[2].vaddr + segs[2].filesz)) {
                shndx = 3;
            } else if (a >= segs[2].vaddr) and (a < (segs[2].vaddr + segs[2].memsz)) {
                shndx = 4;
            }
            let mut e = &mut stg;
            if sy.bind == elf.B_LOCAL {
                e = &mut stl;
                nlocal += 1;
            }
            e.u32_(@as(u64, str.len()));
            e.u8_((sy.bind << 4) | sy.ty);
            e.u8_(0);
            e.u16_(shndx);
            e.u64_(a);
            e.u64_(sy.size);
            str.put(sy.name);
            str.u8_(0);
        }
    }
    st.put(stl.view());
    st.put(stg.view());
    let mut shs = bn.new_bytes();
    shs.put("\x00.text\x00.rodata\x00.data\x00.bss\x00.symtab\x00.strtab\x00.shstrtab\x00.dynamic\x00");
    let mut tail = bn.new_bytes();
    tail.put(b);
    let sym_off = align_up(@as(u64, tail.len()), 8);
    tail.zeros(@truncate(usize, sym_off) - tail.len());
    tail.put(st.view());
    let str_off = @as(u64, tail.len());
    tail.put(str.view());
    let shs_off = @as(u64, tail.len());
    tail.put(shs.view());
    let sh_off = align_up(@as(u64, tail.len()), 8);
    tail.zeros(@truncate(usize, sh_off) - tail.len());
    // name, type, flags, addr, off, size, link, info, align, entsize
    let shdr: [9][10]u64 = [
        [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        [1, 1, 6, segs[1].vaddr, segs[1].off, segs[1].filesz, 0, 0, 16, 0],
        [7, 1, 2, segs[0].vaddr, segs[0].off, segs[0].filesz, 0, 0, 8, 0],
        [15, 1, 3, segs[2].vaddr, segs[2].off, segs[2].filesz, 0, 0, 8, 0],
        [21, 8, 3, segs[2].vaddr + segs[2].filesz, segs[2].off + segs[2].filesz, segs[2].memsz - segs[2].filesz, 0, 0,
         8, 0],
        [26, 2, 0, 0, sym_off, @as(u64, st.len()), 6, nlocal, 8, 24],
        [34, 3, 0, 0, str_off, @as(u64, str.len()), 0, 0, 1, 0],
        [42, 3, 0, 0, shs_off, @as(u64, shs.len()), 0, 0, 1, 0],
        [52, 6, 3, dyn_addr, dfo, n_dyn * 16, 0, 0, 8, 16],
    ];
    for k in @as(usize, 0)..9 {
        let x = shdr[k];
        tail.u32_(x[0]);
        tail.u32_(x[1]);
        tail.u64_(x[2]);
        tail.u64_(x[3]);
        tail.u64_(x[4]);
        tail.u64_(x[5]);
        tail.u32_(x[6]);
        tail.u32_(x[7]);
        tail.u64_(x[8]);
        tail.u64_(x[9]);
    }
    let tb = tail.view();
    w64(tb, 40, sh_off);  // e_shoff
    w16(tb, 58, 64);      // e_shentsize
    w16(tb, 60, 9);       // e_shnum
    w16(tb, 62, 7);       // e_shstrndx
    if not u.write_file(out, tb) {
        err(u.f1("cannot write %s", out));
        return 1;
    }
    _ = u.chmod(u.cz(out), 0o755);
    return 0;
}
