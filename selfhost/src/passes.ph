// =============================================================================
// passes.ph - the optimization pipeline over a module's IR (after lowering).
//
//   -O0   mem2reg, constant folding, CFG simplification, dead code
//   -O1+  also instruction combining (combine.ph), jump threading and
//         if-conversion (thread.ph), scalar replacement of aggregates (sroa.ph),
//         global value numbering, dead stores, bounds and loop-invariant code
//         motion (gvn.ph)
//   -O2+  also interprocedural constant propagation and inlining
//         (inline.ph), bottom-up over the call graph, each function simplified
//         again after its calls are inlined
// =============================================================================
import "gvn.ph" as gvn;
import "inline.ph" as inl;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "outline.ph" as outl;
import "sroa.ph" as sroa;
import "target.ph" as tg;
import "thread.ph" as thr;
import "util.ph" as u;

fn cleanup(f: *mut ir.Func, level: i32) void {
    let mut rounds: i32 = 0;
    loop {
        let a = opt.fold_combine(f, level > 0);
        let mut c = opt.simplify_cfg(f);
        if (level > 0) and thr.jump_thread(f) {
            c = true;
        }
        if (level > 0) and thr.if_convert(f) {
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
        if gvn.ss_opt(f) {
            cleanup(f, level);
        }
        rounds = 0;
        while (rounds < 8) and gvn.loop_idioms(f) {
            cleanup(f, level);
            rounds += 1;
        }
        if gvn.licm(f) {
            cleanup(f, level);
            _ = gvn.gvn(f);
            cleanup(f, level);
        }
        opt.dce(f);
        _ = gvn.sink(f);
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
        // uncommon paths that make calls leave small functions (to inline)
        let mut changed = u.List.[*mut ir.Func].{ data: u.nil.[*mut ir.Func](), len: 0, cap: 0 };
        if outl.outline_module(m, &mut changed) {
            for i in 0..changed.len {
                optimize_func(changed.get(i), level);
            }
        }
        u.free(@ptr_cast(*mut void, changed.data));
        // twice: inlining exposes constant arguments (a comparison function
        // passed down through an inlined call), which make new direct calls
        for _round in @as(i32, 0)..2 {
            let consts = inl.ipcp(m);
            for i in 0..consts.len {
                optimize_func(consts.get(i), level);
            }
            inl.inline_module(m, resimplify);
        }
        for i in 0..m.funcs.len {
            let f = m.funcs.get(i);
            if ir.is_decl(f) {
                continue;
            }
            let split = gvn.split_loops(f);
            if split or gvn.reduce_addresses(f, tg.is_wasm()) {
                let mut rounds: i32 = 0;
                while (rounds < 3) and gvn.gvn(f) {
                    cleanup(f, level);
                    rounds += 1;
                }
                if gvn.licm(f) {
                    cleanup(f, level);
                    _ = gvn.gvn(f);
                    cleanup(f, level);
                }
                opt.dce(f);
            }
        }
    }
}
