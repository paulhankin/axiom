// =============================================================================
// passes.ph - the optimization pipeline over a module's IR (after lowering).
//
//   -O0   mem2reg, constant folding, CFG simplification, dead code
//   -O1+  also instruction combining (combine.ph) and scalar replacement of
//         aggregates (sroa.ph)
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "sroa.ph" as sroa;

fn cleanup(f: *mut ir.Func, level: i32) void {
    let mut rounds: i32 = 0;
    loop {
        let a = opt.fold_combine(f, level > 0);
        let c = opt.simplify_cfg(f);
        rounds += 1;
        if not (a or c) or (rounds > 8) {
            break;
        }
    }
}

pub fn optimize_func(f: *mut ir.Func, level: i32) void {
    opt.mem2reg(f);
    cleanup(f, level);
    if level > 0 {
        let mut rounds: i32 = 0;
        while (rounds < 6) and sroa.sroa(f) {
            opt.mem2reg(f);
            cleanup(f, level);
            rounds += 1;
        }
    }
    opt.dce(f);
}

pub fn optimize_module(m: *mut ir.Module, level: i32) void {
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if not ir.is_decl(f) {
            optimize_func(f, level);
        }
    }
}
