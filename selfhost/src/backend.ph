// =============================================================================
// backend.ph - from IR to an object file: target-independent lowering and
// optimization (lower.ph, opt.ph), then the target's code generator.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "target.ph" as tg;
import "lower.ph" as lower;
import "passes.ph" as passes;
import "wasm.ph" as wa;
import "wobj.ph" as wo;
import "x64.ph" as x64;
import "a64.ph" as a64;
import "elf.ph" as elf;

pub const EMIT_OBJ: i32 = 0;
pub const EMIT_IR: i32 = 1;
pub const EMIT_ASM: i32 = 2;

pub fn emit_module(m: *mut ir.Module, opt_level: i32, emit: i32, path: []u8) bool {
    let raw = (emit == EMIT_IR) and not u.isnos(u.env("PHALANGE_IR_RAW"));
    ir.merge_same_named(m);
    if not raw {
        lower.lower_module(m);
        passes.optimize_module(m, opt_level);
    }
    let dump = u.env("PHALANGE_DUMP_IR");  // debugging: the optimized IR of every compilation
    if not u.isnos(dump) {
        _ = u.write_file(dump, ir.module_str(m));
    }
    if emit == EMIT_IR {
        if not u.write_file(path, ir.module_str(m)) {
            u.eprint(u.f1("phalange: cannot write %s\n", path));
            return false;
        }
        return true;
    }
    if tg.is_wasm() {
        let un = wa.compile(m, tg.g_arch == tg.ARCH_WASM64);
        if u.isnil(un) {
            return false;
        }
        if not u.write_file(path, wo.write_object(un)) {
            u.eprint(u.f1("phalange: cannot write %s\n", path));
            return false;
        }
        return true;
    }
    if tg.g_arch == tg.ARCH_X64 {
        let o = x64.compile(m);
        if u.isnil(o) {
            return false;
        }
        if not u.write_file(path, elf.write(o)) {
            u.eprint(u.f1("phalange: cannot write %s\n", path));
            return false;
        }
        return true;
    }
    if tg.g_arch == tg.ARCH_A64 {
        let o = a64.compile(m);
        if u.isnil(o) {
            return false;
        }
        if not u.write_file(path, elf.write(o)) {
            u.eprint(u.f1("phalange: cannot write %s\n", path));
            return false;
        }
        return true;
    }
    u.eprint("phalange: code generation for this target is not implemented yet\n");
    return false;
}
