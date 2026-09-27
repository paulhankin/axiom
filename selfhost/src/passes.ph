// =============================================================================
// passes.ph - the optimization pipeline over a module's IR (after lowering).
//
//   -O0   mem2reg, constant folding, CFG simplification, dead code
//   -O1+  also instruction combining (combine.ph), jump threading
//         (thread.ph), scalar replacement of aggregates (sroa.ph) and global
//         value numbering (gvn.ph)
//   -O2+  also interprocedural constant propagation and inlining
//         (inline.ph), bottom-up over the call graph, each function simplified
//         again after its calls are inlined
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "sroa.ph" as sroa;
import "gvn.ph" as gvn;
import "inline.ph" as inl;
import "thread.ph" as thr;

fn cleanup(f: *mut ir.Func, level: i32) void {
    let mut rounds: i32 = 0;
    loop {
        let a = opt.fold_combine(f, level > 0);
        let mut c = opt.simplify_cfg(f);
        if (level > 0) and thr.jump_thread(f) {
            c = true;
        }
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
        rounds = 0;
        while (rounds < 3) and gvn.gvn(f) {
            cleanup(f, level);
            rounds += 1;
        }
        if gvn.dse(f) {
            _ = gvn.gvn(f);
            cleanup(f, level);
        }
    }
    opt.dce(f);
}

fn resimplify(f: *mut ir.Func) void {
    optimize_func(f, 2);
}

pub fn optimize_module(m: *mut ir.Module, level: i32) void {
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if not ir.is_decl(f) {
            optimize_func(f, level);
        }
    }
    if level >= 2 {
        // twice: inlining exposes constant arguments (a comparison function
        // passed down through an inlined call), which make new direct calls
        for _round in @as(i32, 0)..2 {
            let consts = inl.ipcp(m);
            for i in 0..consts.len {
                optimize_func(consts.get(i), level);
            }
            inl.inline_module(m, resimplify);
        }
    }
}
