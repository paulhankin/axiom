# 0100: Self-hosted inliner: a one-time initialisation inlined into a hot wrapper keeps the wrapper out of line

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: inline)

the benchmark's `sanity_check(board, out)` (the benchmark `axiom/src/eval/eval.ax`) is
`sanity_check_with_endgame-table(board, out, bo.default_db())`, and `default_db` checks a `Once`
inline and loads the databases out of line (`load_default_db`, as Rust's
`OnceLock::get_or_init`, whose initialisation is `#[cold]`):

```
pub inline fn default_db() *Endgame-tableDatabase {
    if not DEFAULT_ONCE.done() { load_default_db(); }
    return &DEFAULT_DB;
}
fn load_default_db() void { DEFAULT_ONCE.call(load_default_once, &mut DEFAULT_DB); }
```

The self-hosted inliner finds `load_default_db` (with `Once.call` inlined: a cas, the
call, a store, a notify and a wait loop) under `SMALL` (calls cost 3 + arguments) and
inlines it into `sanity_check`, which then is too big to be inlined into its callers. Every
0-ply evaluation pays for a call, a frame of three saved registers and the `Once` test
twice: about 21 instructions per call, 43k of 5.3M instructions in five 1-ply positions
(0.8%). The LLVM-built port keeps `load_default_db` out of line and inlines `sanity_check`.

Possible approaches: a cost for what a callee does not run on its common path (a loop
around `@atomic_wait`, a call through a pointer) so that `SMALL` means cheap to run, not
few instructions; or treat a call that is only reached when an atomic flag is not yet set
as cold (the shape of every lazy initialisation); or a `cold` attribute in the language
(the IR has `Func.cold`, set only by the outliner).
