# Axiom Agent-Fit Plan

This plan follows a review of the whole repository against the design mandate: a
performant systems language that machine agents write, read and repair. The review found
the performance half well supported and the agent half mostly unmeasured. This document
plans seven changes that close the gaps the review found. An eighth, a measured corpus of
agent tasks, is planned separately.

It complements [`language-improvements.md`](language-improvements.md), which changes the
language's expressive power. This plan changes what happens when an agent gets something
wrong, what it can look up, and how much the project has to maintain.

Every number below was measured on main at `92b8dd88` on one x86-64 Linux machine.

---

## Summary

| # | Workstream | Breaking? | Size | Depends on |
|---|---|---|---|---|
| **F** | One compiler: bootstrap from a pinned release, freeze the C bootstrap | No (process) | Medium | A release that contains phases 0 to 5 |
| **A** | Diagnostics: one error per mistake, fixes for idioms from other languages | No | Medium | F (otherwise every change lands twice) |
| **E** | `axiom doc`: signatures and documentation from the command line | No | Small | None |
| **C** | Memory lifetime: an accurate claim, and the sanitizer on by default in tests | Tests only | Small | None |
| **D** | One iterator protocol, and `while let` | Yes (stdlib API) | Medium | F |
| **B** | Integer overflow traps by default | Yes (language) | Large | F, issue #72 |
| **G** | Standard library tiers; the C compiler marked experimental | Yes (import paths) | Large, mechanical | D, E |

### Recommended order

```
  [F: one compiler] ──► [A: diagnostics] ──► [D: iterators + while let] ──► [B: overflow traps]
                                                        │
  [E: axiom doc]  ──────────────────────────────────────┼──► [G: library tiers]
  [C: lifetime claim + sanitizer in tests]              │
  [G2: C compiler marked experimental]  (any time)
```

F comes first because A, D and B all change the front end, and today every front-end
change must land in two compilers with identical diagnostics. E, C and G2 touch no
front-end code and can start at once. D and B are breaking and should share one release
window with phases 2 to 4 of the language plan if that window is still open, and
otherwise form the next one.

---

## F. One Compiler

### Problem

The bootstrap compiler (`cmd/axiom/bootstrap/`, 20.6k lines of C on LLVM 18) has four
jobs today:

1. It builds stage 1 of the compiler.
2. It mirrors the front end. Every language change lands in both compilers with identical
   diagnostics, and `difftest.sh` enforces that.
3. Its LLVM-generated code is the reference for the backend fuzzer (`fuzz/run.sh`).
4. It is the default compiler of `run.mjs`.

Only the first job is essential, and a released compiler can do it. The second is a tax
on every change in this plan and in the language plan. The self-hosted compiler already
has features the bootstrap does not: the formatter, the language server and the C front
end.

Nothing is lost in code quality. Built by LLVM at `-O2` and by itself at `-O2`, the
compiler runs at the same speed (0.90x to 1.02x the time on self-compiles), and the
LLVM build takes 98 s against 19 s.

### Design

- **F1. Seed from a release.** `cmd/axiom/seed.toml` pins one release: its version and
  the SHA-256 of each platform's archive. Releases are already deterministic and publish
  `SHA256SUMS`. `bootstrap.sh` obtains the seed through the code path `axiom update`
  uses, verifies the hash, and keeps it in `$AXIOM_CACHE`. `AXIOM_SEED=<path>` names a
  local compiler instead, for offline builds. The stages become: seed builds stage 1,
  stage 1 builds stage 2, stage 2 builds stage 3, and stages 2 and 3 must be identical.
- **F2. The feature rule.** The compiler's sources, and the 29 standard library files
  they import, use only what the seed accepts. A new language feature becomes usable in
  the compiler once a release containing it is pinned. The bootstrap itself checks this:
  if the seed cannot build main, the build fails. A change of meaning (workstream B) needs
  sources that are correct under both the old and the new meaning for one release.
- **F3. Freeze the C bootstrap.** Tag the last commit at which it builds main
  (`bootstrap-final`) and delete `cmd/axiom/bootstrap/` from main. A new
  `docs/bootstrap-chain.md` records, for each release, the seed that built it and its
  hashes, so the chain from C and LLVM to any release can be replayed.
- **F4. Replace the references.**
  - The diagnostics comparison in `difftest.sh` goes away: there is one front end.
  - The backend fuzzer compares the backends with each other (wasm32 under node, x86-64,
    AArch64 under qemu) and `-O0` with `-O2` and `-O3`, as most of its runs already do.
  - What is lost is an independent implementation of lowering: a front-end bug is now
    shared by every backend. The mitigation is tests whose expected output comes from
    outside the compiler, as `tests/run/checked_widths.ax` does (exact arithmetic,
    generated).
- **F5. Clean up.** `run.mjs` defaults to `cmd/axiom/build/axiom`, and the
  `selfhost-only` directive disappears. `CLAUDE.md`, `CONTRIBUTING.md`,
  `docs/install.md` and `docs/releasing.md` drop the dual-compiler rule and the LLVM 18
  requirement. Building from source then needs only a seed and node.

### Status

Done. The Problem above describes the tree as it was.

- F1, F2: `cmd/axiom/seed.toml` pins the seed `0.3.0-seed.1` (the compiler of commit
  `4fea9e94`, made by `cmd/axiom/seed.sh`); `bootstrap.sh` builds from it, or from
  `AXIOM_SEED`, and fails with neither. The rule is in `CLAUDE.md`, `CONTRIBUTING.md`
  and `docs/bootstrap-chain.md`.
- F3: `cmd/axiom/bootstrap/` is deleted; `8d7656f7` is the last commit that has it, and
  `docs/bootstrap-chain.md` says how the chain is replayed from there. With it went the
  C sources of the runtime that only it linked (`std/internal/runtime/native_host.c`,
  `test_main.c`, `profile.c`, `trap_format.h`).
- F4: `difftest.sh` runs the suite in its configurations and the fuzzer; the fuzzer's
  reference is the WebAssembly build's output; `bench.sh` compares with the compiler
  `BASE` names.
- F5: `run.mjs` tests `cmd/axiom/build/axiom` by default and has no skip that depends
  on the compiler; the `selfhost-only` and `lower-selfhost` directives are gone; the
  docs describe one compiler and no LLVM.
- Verified: the fixed point from the pinned seed, also with `CROSS=1` (the
  AArch64-hosted compiler under qemu generates the same code), and the suite in the
  configurations of `difftest.sh`.

Not done:

- The tag `bootstrap-final` on `8d7656f7` (the document names the commit instead).
- `docs/bootstrap-chain.md` records each release's commit and seed, not yet its hashes
  (they are in each release's `SHA256SUMS`).
- F4's mitigation for the lost independent lowering (more tests whose expected output
  comes from outside the compiler) is not started.

### Risks

- **A bad seed.** If a release miscompiles the compiler, pin the previous one. The fixed
  point (stage 2 equals stage 3) catches most such cases.
- **Trusting the seed.** Deterministic releases, the pinned hashes and the recorded chain
  answer this.
- **Platforms without a release.** They are built by cross-compiling from one that has
  one, as releases are today.

### Verification

`bootstrap.sh` reaches the fixed point from the pinned seed, and with `CROSS=1` on the
other architecture. The full suite passes with no reference to the bootstrap compiler.

---

## A. Diagnostics: One Error Per Mistake

### Problem

An agent that has written other languages makes predictable mistakes. Each one should
cost one diagnostic and, where the intent is clear, one machine-applicable fix. Probing
45 such mistakes against the compiler gave these results:

| Written | Meant | Errors today | First message |
|---|---|---|---|
| `if !ok {` | `not ok` | 7 | expected expression, found '!' |
| `a \|\| b` | `a or b` | 8 | operator '\|' cannot be applied to bool |
| `a && b` | `a and b` | 1 | operands of '&' have different types: bool and *bool |
| `P{ x: 1 }` | `P.{ x: 1 }` | 2 | expected a value, found a type |
| `fn f() -> T` | `fn f() T` | 5 | expected type, found '-' |
| `fn f(): T` | `fn f() T` | 4 | expected type, found ':' |
| `i++` | `i += 1` | 2 | result of type i32 is discarded |
| `c ? a : b` | an `if` statement | 2 | type mismatch: expected i32, found bool |
| `x as i32` | `@int_cast(i32, x)` | 4 | type mismatch |
| `try f()` and `f()?` | `f()!` | 2 and 1 | unknown name 'try' |
| `null`, `None`, `Some(x)` | `.none`, `.some(x)` | 1 to 2 | unknown name 'None' |
| `C::R` | `C.R` | 2 | expected a value, found a type |
| `var i`, `mut i` | `let mut i` | 4 and 5 | unknown name 'var' |
| `s.len()` | `s.len` | 1 | cannot call a value of type usize |
| `let r = match v { ... }` | a `match` statement | 2 | expected expression, found 'match' |
| `for (init; cond; step)` | `while` | 12 | expected identifier, found '(' |
| `0..=n` | `0..n + 1` | 7 | cannot infer the type of the range |
| `*p` | `p.*` | 3 | expected expression, found '*' |
| `fn id<T>(x: T)` | `fn id[T](x: T)` | 4 | expected '(', found '<' |
| `Vec<u32>`, `Vec[u32]` | `Vec.[u32]` | 10 and 2 | operators '<' and '>' cannot be mixed |
| `'abc'` | `"abc"` | 5 | malformed character literal |
| `for l in str.lines(s)` | see workstream D | 1 | 'for x in' needs an array or slice |
| `fmt.println(...)` | `fmt.print` | 1 | module has no declaration 'println' |

None of these names the Axiom spelling. In twelve of the 45 probes, a cascade error (`E0620`,
"result is discarded") carries a fix marked `machine`. Applying it would be wrong, which
breaks the guarantee in spec §16.2 that a machine fix yields a program without the
diagnostic and keeps the intended meaning.

Correct diagnostics already exist for shadowing, unused results, a missing `;`, a
missing return type and unannotated literals.

### Design

- **A1. Stop the cascade.**
  - A declaration with a syntax error is *poisoned*. Its signature is still checked, so
    other functions that call it check normally, but its body is not.
  - The parser reports at most one `E0100` per declaration. When statement-level
    recovery (`sync_stmt`) fails, it skips to the next token that starts a top-level
    declaration.
  - No fix is ever attached to a diagnostic inside a poisoned declaration. Spec §16.2
    gains this rule.
- **A2. Recognise the idioms.** The parser's error path may look ahead as far as it
  likes: the grammar stays LL(1), because only a program that is already wrong takes the
  path. Each row of the table gets a dedicated diagnostic that names the Axiom form.
  - *Parser, machine fix:* prefix `!`; adjacent `&&` and `||` (the lexer records
    adjacency); `Type{` where a block cannot start; `->` or `:` before a return type;
    `++` and `--`; `::`; `var` and a leading `mut`; `try`; `..=`; prefix `*`;
    angle-bracket generics; a missing `.` before `[` in a generic application; a
    multi-character literal in single quotes.
  - *Parser, suggestion only:* the ternary operator, a C-style `for`, and `match` or
    `if` used as an expression. Their rewrites need a choice the compiler cannot make.
  - *Checker, with types known:* postfix `?` on an error union; `null` where a `?T` is
    expected; `None`, `Some`, `Ok` and `Err` as unknown names; `.len()` on a slice or
    array; `as` after an expression (the fix picks `@as` when the conversion is lossless
    and offers `@int_cast` as a suggestion otherwise); `+` on two slices (a note naming
    `str.concat`).
  - New codes are `E0160` to `E0179`, one per idiom, so that a code's meaning stays
    fixed (§16.3).
- **A3. Nearest names.** `E0202` (unknown name) and `E0404` (no such field or method)
  gain a note listing the closest names in scope, or in the module or type, with their
  signatures. The fix is a suggestion when exactly one name is within edit distance 2.

### Status

Done. The Problem above describes the tree as it was.

- Verification step 2 came first: `run.mjs` applies each `machine` fix of the errors suite
  (`errors/` and `errors/idioms/`) alone, checks the result again, and requires the
  diagnostic gone and no error added. On the probes it rejected nine of the sixteen
  cascade `E0620` fixes; the other seven removed their diagnostic and were wrong all the
  same. A1 removed all sixteen.
- A1: a function, method or other top-level declaration with a syntax error is poisoned
  (one `E0100`, its body unchecked, no fixes in it; spec §16.2). Recovery stops at a
  column-1 token that starts a top-level declaration, and a declaration-only keyword
  first on its line inside a block ends the block (a missing `}`).
- A2: `E0160` to `E0179`, one per idiom (`&&` and `||` share `E0161`, `try` and `?`
  share `E0167`, `fn id<T>` and `Vec<T>` share `E0176`; `E0179` is `+` on slices). The
  parser builds the Axiom form's tree after a machine-fixed idiom, so the rest checks as
  written. `errors/idioms/` has a file per form, each exactly one error.
- A3: `E0202` and `E0404` have a note per closest name (at most three, with signatures),
  and a suggestion when exactly one is within two edits.
- Verified on the probes of the table (32 programs, one per form; the `Errors today`
  column was measured the same way): every row now gives one error, with the stated code.
  117 errors became 32; 24 of the 32 carry a machine fix, and each fixed program checks
  with no error. The tour and `examples/` give no diagnostics; the full suite passes.

| Probe | Before | After |
|---|---|---|
| `if !ok {` | 7 | 1, `E0160` |
| `a \|\| b` / `a && b` | 5 / 1 | 1, `E0161` |
| `P{ x: 1 }` | 2 | 1, `E0162` |
| `fn f() -> T` / `fn f(): T` | 5 / 4 | 1, `E0163` |
| `i++` / `i--` | 2 / 2 | 1, `E0164` |
| `c ? a : b` | 2 | 1, `E0165` (note, no fix) |
| `x as i32` (lossless / lossy) | 4 / 4 | 1, `E0166` (machine / suggestion) |
| `try f()` / `f()?` | 2 / 2 | 1, `E0167` |
| `null` / `None` / `Some(x)` | 2 / 1 / 1 | 1, `E0168` |
| `C::R` | 4 | 1, `E0169` |
| `var i` / `mut i` | 5 / 6 | 1, `E0170` |
| `s.len()` | 1 | 1, `E0171` |
| `let r = match v { ... }` / `if` | 3 / 3 | 1, `E0172` (note, no fix) |
| `for (init; cond; step)` | 12 | 1, `E0173` (note, no fix) |
| `0..=n` | 7 | 1, `E0174` |
| `*p` | 4 | 1, `E0175` |
| `fn id<T>(x: T)` / `Pair<u32>` | 4 / 5 | 1, `E0176` |
| `Pair[u32]` | 7 | 1, `E0177` |
| `'abc'` | 6 | 1, `E0178` |
| `for l in str.lines(s)` | 1 | 1, `E0412` (workstream D) |
| `fmt.println(...)` | 2 | 1, `E0202`, suggesting `print` |
| `a + b` on slices | 1 | 1, `E0179` (note naming `str.concat`) |

Decided, where the plan left a choice or was not quite right:

- `null` stays reserved: as a name it is still `E0003`; as a value it is `E0168`.
- `Ok(x)` and `Err(e)` lose the wrapper by a machine fix only where a `!T` is expected
  (`Err` only around `error.X`); elsewhere the fix is a suggestion. A lossy `x as T`
  between floats, or between a float and an integer, has no fix.
- `+` on slices, the conditional operator, a C-style `for` and `if` or `match` as an
  expression have notes only: their rewrite needs a name, a type or an allocator. The
  last three poison their declaration.
- The plan's `E0404` is "no such field or method"; the spec's table listed only its other
  meaning (a `void` element), and now lists both.
- Not recognised, conservatively: `f<A, B>(x)` in an expression (a `,` before `(` is two
  comparisons as arguments, `f(a < b, c > (d))`); a nested `Vec<Vec<u32>>` (the `>>`);
  `a[i++]`; `Pair[Foo]` in an expression, with a type of the program's own, which the
  parser cannot tell from indexing; a `for` over an iterator (workstream D).

### Verification

- A new directory `cmd/axiom/internal/tests/errors/idioms/` holds one file per row. Each
  must produce exactly one error, with the stated code.
- A new step in `run.mjs` applies every `machine` fix in the whole errors suite and runs
  `axiom check` again. The original diagnostic must be gone. This enforces the §16.2
  guarantee for every fix, old and new.
- The tour's programs and the examples still produce no diagnostics.

---

## E. `axiom doc`

### Problem

An agent that needs the signature of `str.split` has to read `std/str.ax` (991 lines) or
search for it. There is no documentation command. The language server answers the same
question, but only over its protocol. About 64% of the standard library's public
functions (2,607 of 4,044) have a `///` comment.

### Design

```
axiom doc std                         # the modules, one line each
axiom doc std/str                     # a module: its public items, one line each
axiom doc std/str.split               # an item: signature, documentation, errors, location
axiom doc std/collections/vec.Vec     # a type: fields and methods
axiom doc pkg:geometry                # a package's entry module
axiom doc --search=split [std/]       # public items whose name or documentation matches
```

- **Output.** Human output is compact text meant to be read into a context window: a
  module listing shows each item's signature and the first sentence of its
  documentation. An item's output expands a named error set into its names, since that
  is what a caller's `match` needs.
- **JSON.** `--format=agent-json` gives the same content under the schema `axiom-doc/1`.
- **Implementation.** A new `cmd/axiom/src/doc.ax` over the parsed tree, reusing the
  signature printing of `ide.ax`'s `documentSymbol` and `hover`. It parses without
  checking, so it answers in milliseconds, and it works on a file that does not compile.
- **Module summaries.** The first comment block of a file is its module's summary, a
  convention most of the standard library already follows.
- **Coverage.** A new warning, `W0320`, reports a public item without documentation. It
  is off by default and turned on for the core library (workstream G), where the test
  suite requires none.

### Verification

Golden outputs for a small fixture module and for `std/str`, in both formats. `run.mjs`
checks that every module listed by `axiom doc std` has a summary.

### Status

Done, but for turning `W0320` on for the core library, which is workstream G's.

- `cmd/axiom/src/doc.ax`: every form of the design above, and `axiom doc <file.ax>`,
  `<file.ax>.<item>`, a directory (`std/net/` lists the directory, `std/net` is the
  module), a package directory, and `--search` under any of them. Signatures are
  `ide.ax`'s (`sig_of`, as hover shows them), documentation its `doc_comment`. An
  item's output expands a named error set, through the sets it includes and across
  imports. An item that is not found is `E0202` with the module's similar names.
  `--format=agent-json` is `axiom-doc/1` (spec §16.10; `docs/cli.md`).
- Measured on one x86-64 Linux machine (median of 15 runs): `axiom doc std/str` 1.6 ms
  (5.9 kB of output), `axiom doc std/str.split` 1.6 ms, `axiom doc std` 32 ms (212
  modules, 28.6 kB: it reads every file of `std/` to find the part files), `axiom doc
  --search=split` 0.35 s (it parses every module).
- Module summaries: every one of the 212 modules `axiom doc std` lists (the `.ax` files
  outside `internal/` and `testdata/` directories that are not part files) already had
  one, so none was added; `run.mjs` checks all of them. Some summaries' first sentence
  is long (the C runtime's modules).
- `W0320`, off by default, is turned on by `--warn=W0320` (for `build`, `check` and
  `test`), for the modules whose tests would run (spec §17.2). It treats only `///` as
  documentation, as hover does. The registry in spec §16.3 now lists the warnings, and
  its test in `run.mjs` checks `W` codes as it checks `E` codes; `W0601` and `W0631`,
  which no test asserted, have one (`errors/unreachable_warn.ax`).
- By `axiom doc`'s count, 3,039 of std's 5,330 public items and methods (57%) have a
  `///` comment: what `W0320` would report today for all of std is the other 2,291.
- Tests: golden outputs (`cmd/axiom/internal/tests/doc/`) for the fixture module
  `shapes.ax` (a part file, an error set from another module, an item of each kind) and
  for `std/str`, in both formats (`AXIOM_UPDATE_DOC=1` rewrites them); a module with
  syntax and type errors; `pkg:`; what is not found; `W0320` off and on
  (`errors/undocumented_warn.ax`).

Not done:

- `W0320` is not turned on for any part of `std/` (workstream G).
- The language server does not use `doc.ax` (hover has its own, which shows the
  doc comment but not the expanded error set).

---

## C. Memory Lifetime: An Accurate Claim

### Problem

The README says there is "no undefined behavior outside raw pointers". This program has
no raw pointer, compiles without a warning, and reads freed memory twice on a native
target:

```axiom
let s = first(&v);          // a slice of v's buffer
for i in @as(u32, 0)..1000 {
    v.push(i) catch return 1;   // the buffer moves
}
let buf = mem.alloc.[u32](4) catch return 1;
mem.free(buf);
let x = s[0] + buf[0];      // both are uses after free
```

`--sanitize=address` reports it correctly, but it is off by default. Phase 3 of the
language plan closed the copy and escape holes (`nocopy`, `E0710`). It did not, and was
not meant to, track heap lifetimes.

A borrow checker would close the gap and is out of scope: it contradicts the small,
context-free language the mandate asks for. This workstream makes the claim true as
stated, and makes the remaining errors likely to be caught.

### Design

- **C1. State it accurately.** The README, spec §1 and the tour say: every operation is
  checked or defined except two rules a program must keep itself. A raw pointer stays
  inside its object, and memory is not used after it is freed (§12.1). The second is
  caught by the sanitizer.
- **C2. The sanitizer is on in `axiom test`.** `axiom test` builds with
  `--sanitize=address` unless given `--sanitize=none`. A program that keeps the rules
  behaves the same either way, so tests still test what ships. Before the switch,
  measure the suite's time under the sanitizer on both test targets; the 64 MiB
  quarantine may need a smaller default for tests that run in parallel.
- **C3. A local warning for the plain cases.** A new warning, `W0711`, covers two shapes
  inside one function:
  - a local slice or pointer used after it was passed to `mem.free`;
  - a slice or pointer obtained from a container through a `*Self` method and used after
    a `*mut Self` method was called on the same container.

  It is a heuristic and will have false positives (a `set` that does not move the
  buffer). It lands off by default. It is enabled only if its false-positive rate over
  `std/`, `examples/` and the compiler is low enough to act on, and that number goes in
  the commit that enables it.

### Verification

The program above is a test: it fails under `axiom test` with `T0012`, and `W0711` fires
at both uses once enabled. The standard library's own tests pass under the sanitizer.

### Status

Done, but for turning `W0711` on, which waits on the owner's decision with the numbers
below. The Problem above describes the tree as it was.

- C1: the README, spec §1 (and the definitions before it), §12.1 and the tour say that
  every operation is checked or defined except two rules a program keeps itself: a raw
  pointer stays inside its object, and memory is not used after it is freed, through a
  pointer or a slice. §12.1 had said that only a raw pointer could break the access
  rule. `SECURITY.md`, `docs/security.md` and `docs/comparison.md` made the old claim
  too, and say the new one.
- C2: `axiom test` builds with `--sanitize=address` unless given `--sanitize=none`
  (spec §16.8, §17.2; `docs/cli.md`). The sanitizer works on wasm32, x86-64 and
  AArch64 Linux and x86-64 Windows (run here under an emulator and an emulation
  layer), and builds for macOS and AArch64 Windows; for wasm64 (whose host fails a
  sanitized program, and runs no wasm64 tests at all), a C program or an Axiom program
  with C units, `axiom test` quietly builds without it. A sanitized test's default time
  limit is 30 s.
- Measured before the switch, on one x86-64 Linux machine, two at a time, compile
  included: the standard library's tests and the self-tests (154 files) took 43.0 s
  natively and 60.6 s on wasm32 without the sanitizer, and 91.7 s and 112.3 s with
  it, two svg tests timing out on wasm. The sanitizer now sets and checks shadow memory
  a page at a time and caches which allocation sites are `std/mem`'s: 75.9 s and
  96.2 s (1.8 and 1.6 times), nothing timing out.
- Quarantine: a test keeps 16 MiB of freed memory, other programs 64 MiB. The wasm test
  runner holds up to eight instances at once; at 64 MiB its peak memory was 579 MB for
  `std/graphics/svg` (358 at 16 MiB, 320 at 8), 555 for `std/net/http` (466, 448).
  Time did not change.
- Every std test and self-test passes under the sanitizer, natively and on wasm32: it
  found no use after free, overflow or leak in them. The compiler's own tests
  (`axiom test cmd/axiom/src/version.ax`) reported its arenas as leaks; `util.ax`'s
  allocation functions now mark their memory as kept on purpose.
- C3: `W0711` (`cmd/axiom/src/lifetime.ax`, `--warn=W0711`, spec §16.3) reports the
  two shapes, after checking, in statement order with branches merged. Over std/ (each
  of its 271 modules as the root), the 18 examples and the compiler, 19,662
  functions: 563 views taken from a container, 380 frees of a local, 124 changes of a
  container after a view of it was taken, and 3 warnings. One is a meant double free
  in a test that expects the trap (`std/mem/tests.ax`); two are false: a node pointer
  kept across a `*mut Self` method that does not move the nodes
  (`std/text/html/tree_builder.ax:748`), and a slice a function made from `&order` (a
  new allocation, not a view) used after `order.deinit()`
  (`cmd/axiom/src/outline.ax:431`). That rate came after three refinements, each
  measured: an element that is a pointer (`List.get`) is not a view, nor is an error
  union of a slice (most often a new allocation), and a `*mut Self` method that returns
  a view is an accessor. `mem.realloc` is not a free. First there were 44 warnings.
- Tests: `testing/lifetime.ax` is the program above (fails with `T0012` at line 25;
  `W0711` at both uses with the flag, none without); `errors/lifetime_warn.ax` has each
  shape and the cases not to report; `testing/leaks.ax` tests the sanitizer as the
  default; a case checks `--sanitize=none` and the wasm64 fallback, and
  `c/mix/test_with_c` the fallback for C units.

Not done:

- `W0711` is not on by default. Two false positives in about 650k lines is a low rate,
  but the warning is narrow: a use through another variable, a field or another
  function is not seen, so it catches the plainest cases only.
- wasm64: a program built with `--sanitize=address` fails in the WebAssembly host
  (`Cannot convert undefined to a BigInt`), and `axiom test --target=wasm64` fails in
  its test runner with or without the sanitizer.

---

## D. One Iterator Protocol

### Problem

`for` walks slices, arrays and ranges only. The standard library's 30 or so iterators use
five different shapes for `next`:

| Shape | Where |
|---|---|
| `next() ?T` | `str` (`Split`, `Lines`, `Fields`, `Utf8Iter`), `unicode`, `net/url`, `protobuf` |
| `next() E!?T` | `yaml` events, `regex` matches, `mime`, `protobuf` |
| `next() E!T`, with the end reported as an error or a sentinel | `csv`, `tar`, `der`, `x509`, `cookie`, `xml`, `html` tokenizer, `os/path` |
| `next(key: *mut K, val: *mut V) bool` | `Map`, `Set`, `Lru`, `TreeMap` |
| `next() bool`, a cursor read through fields | `html` and `markdown` walkers |

An agent has to look up each one. The loop that drives them is three lines of
boilerplate (`loop { let x = it.next() ?? break; ... }`).

### Design

- **D1. The protocol.** An iterator has `pub fn next(self: *mut Self) ?T`, or
  `E!?T` if producing an item can fail. The end is always `.none`. It is never an
  error, a sentinel or a `bool`. A cursor that is read through its fields names its step
  `advance`, not `next`.
- **D2. `while let`.**

  ```
  WhileStmt ::= Label? "while" ( Expr | "let" Patterns "=" Expr ) Block
  ```

  This mirrors `if let`, stays LL(1) and adds no keyword. The loop runs while the
  pattern matches:

  ```axiom
  let mut lines = str.lines(text);
  while let .some(line) = lines.next() {
      ...
  }
  while let .some(ev) = events.next()! {     // a fallible iterator
      ...
  }
  ```

  The call to `next` is written out, so the rule of no implicit calls holds.
- **D3. `for` does not drive iterators.** `for x in it` calling `it.next()` would be the
  language's first implicit call. Instead, `E0412` on a `for` over a value whose type
  has a conforming `next` carries a fix that rewrites the loop to `while let`. The fix
  is `machine` when the operand is a mutable place, and a suggestion when a `let mut`
  must be introduced for it.
- **D4. Migrate the standard library.**
  - The collections return an entry of pointers: `next() ?Entry.[K, V]` with
    `key: *K` and `value: *mut V`, so nothing is copied. `cmd/axiom/bench/hashmap.ax`
    must not get slower.
  - The error-as-end iterators become `E!?T`, and their `Done` or `Eof` errors leave
    their error sets.
  - The walkers rename `next` to `advance`.
- **D5. Keep it that way.** `run.mjs` checks that every public method named `next` in
  the library has one of the two signatures.

### Status

Done. The Problem above describes the tree as it was.

- D2: `while let` (spec §8.4) is parsed as `loop { match e { P => { A } else => {
  break; } } }`, so it shares `if let`'s pattern checking and lowering, and `break`,
  `continue` and labels need nothing new; `axiom fmt` and the language server needed
  no change. Tests: `run/while_let.ax` (labels, `break` and `continue`, nesting,
  `defer`, a fallible `next()!`, several patterns, `return` from the body),
  `errors/while_let_*.ax`, and a case in the formatter's `layout` test. In passing,
  an arm whose binding is refused (an invalid scrutinee, `E0634`, `E0635`) still
  declares it, so `if let`, `while let` and `match` add no `E0202` after the error.
- D3: `E0412` on a `for` over a value whose type has a conforming `next` says that
  `for` does not call `next` and has a fix writing the `while let`: `machine` for a
  mutable variable, a pointer to the iterator or a field of one when `next` cannot
  fail; otherwise a suggestion (with `!` for a fallible `next`), which declares
  `let mut it = ...;` first (before the loop's label) for any other operand. The
  `str.lines` probe of workstream A now gives one `E0412` with that suggestion; the
  machine-fix re-check passes on `errors/for_iterator*.ax`.
- D4: the table above, checked, missed four: `html.Children` and `xml.Elements`
  (`NotFound` at the end), `der.Reader` (`Corrupt` at the end) and `cron.Schedule`
  (not an iterator). Every public `next` now follows D1:
  - Already conforming, unchanged: `str` (`Split`, `Lines`, `Fields`, `Utf8Iter`),
    `unicode` (`Graphemes`, `Words`, `Sentences`, `linebreak.Breaks`), `net/url`
    `Query`, `regex` `Matches`, `yaml` `Events`, `protobuf` (`Varints`, `Fixed`,
    `Reader`), `mime` `Multipart`.
  - Collections: `Map`, `MapDyn`, `Lru`, `LruDyn`, `TreeMap`, `TreeMapDyn` give
    `?map.Entry.[K, V]` (`key: *K`, `value: *mut V`); `Set`, `SetDyn` and the new
    `Map.keys()` give `?*K` (`Iter.next_key` is gone).
  - Error or sentinel at the end, now `E!?T` or `?T`: `tar`, `csv`, `der`, `xml`
    `Reader` (`TokenKind.END_DOCUMENT` removed), `html` `Tokenizer` (`EOF` is now the
    tree builder's alone), `x509` `PemIter`; and, as nothing else can fail, `?T`:
    `cookie` `Pairs`, `os/path` `Elements`, `xml` `Elements`, `html` `Children`.
    `Done` left `TarError`, `CsvError`, `DerError`, `CookieError`, `OsPathError` and
    `FsError` (it was in the last only to pass on `os/path`'s).
  - Cursors: the `html` and `markdown` `Walker`s' step is `advance`.
  - `cron.Schedule.next` and `prev`, which take a time and are not iterators, became
    `next_utc` and `prev_utc`, beside `next_in` and `prev_in`.
  - Callers updated in std, `examples/`, the tests and the docs; 119 loops in std,
    `examples/` and the tests became `while let` (none in the compiler's sources or
    the std modules it imports, which the seed builds). The compiler itself calls
    none of the changed methods.
  - `cmd/axiom/bench/hashmap.ax`: its machine code is the same before and after (the
    binaries differ only in stack-trace source text); CPU time, median of 21
    alternating runs, 201 ms before and 205 ms after, which is noise on this shared
    machine. A loop over a million-entry map, 400 times: `main` is the same
    instructions with `next(&mut k, &mut v)` and with `?Entry` (the entry is kept in
    registers), 1447 ms and 1460 ms.
- D5: `run.mjs` (section `spec §15.3`) checks every public method named `next` in
  std; no exemption was needed (the free function `next` in `std/c/rt/dfp.ax` is not
  a method).

Decided, where the plan left a choice:

- `Map.iter` (and `Lru`'s and `TreeMap`'s) take `*mut Self`, since the entries hold
  `*mut V`; `Map.keys()` iterates through `*Self`. `Lru` and `TreeMap` reuse
  `map.Entry`; `lru.Entry` stays the by-value entry that `put`, `remove` and
  `pop_lru` give back.
- `der.Reader.next()` iterates the elements of a constructed value; `element()`
  reads one whatever its tag, for code that knows the structure.

Left: several std error sets still list a `Done` that nothing returns any more
(`X509Error`, `TlsError`, `HtmlError`, `FmtError`, `JsonError` and others); taking
it out changes their exhaustive matches, which is separate work.

### Decided

`for` does not drive iterators (D3). It is what an agent writes first, and the D3 fix
makes that mistake cost one round; the mandate's rule of no implicit calls keeps no
exception.

---

## B. Integer Overflow Traps by Default

### Problem

`+`, `-` and `*` wrap silently, and `+%`, `-%` and `*%` mean the same thing. So the
language has two spellings for one operation, and the naive spelling hides bugs. Two
cases from the review:

- A decimal parser written as `v = v * 10 + digit` accepts `99999999999` and returns a
  wrong number.
- The compiler itself, built with `--overflow=trap`, traps at `-O0` on its first
  self-compile: `elfpack.ax:58` computes `x - y` and relies on the wrap.

Spec §7.6 chose wrapping because a check on every operation "was the largest remaining
cost of compute-heavy code". On x86-64 that is no longer so:

| Program (x86-64, `-O2`) | Trap / wrap, CPU time |
|---|---|
| Eight Axiom benchmarks, geometric mean | 1.015 |
| The worst of them (`alloc_tree`) | 1.14 |
| The other seven | 0.99 to 1.01 |
| Compiler self-compile | 0.99 |
| Compiler binary size | 1.07 |

Not yet measured: AArch64 (when this was written the branch on flags was x86-64 only;
issue #72 has since given AArch64 its own), WebAssembly (no overflow flag, so each check
is a compare and a branch), and integer reductions the vectorizer handles today.

### Design

- **B1. The meaning.** `+`, `-`, `*`, unary `-`, `@abs` and `MIN / -1` trap (`T0002`)
  when the result does not fit, in every build and at every optimization level. The
  `%`-suffixed operators wrap, and are the only way to ask for a wrap. Vector
  arithmetic, atomics and shifts are unchanged. Tested and shipped programs still
  behave the same, which was §7.6's argument against a debug-only check.
- **B2. An escape for measurement.** `--overflow=wrap` stays as a flag, documented as a
  measuring aid and not a dialect. `--overflow=saturate` is removed: `@add_sat` and its
  siblings say it at the operation.
- **B3. A decision gate.** Before the default changes:
  - land issue #72, so AArch64 branches on flags;
  - measure the benchmark set and the self-compile on AArch64 and wasm32;
  - add integer reduction kernels to `cmd/axiom/bench/` and measure them.

  The proposed bar is a geometric mean within 3% on each target, and no program more
  than 15% slower that cannot be fixed by writing `+%` where a wrap is intended. If
  WebAssembly misses the bar, the gate decides whether it blocks the change or wasm
  gets optimizer work first.
- **B4. Make the tree clean first.** A new mode, `--overflow=report`, logs each distinct
  source location that overflows and keeps going, so one run of the suite finds every
  site. Each site is then either a bug to fix or an intended wrap to spell with `%`.
  Hashes, checksums, codecs and random number generators will account for most.
  This step is not breaking and is worth doing even if the gate says no.
- **B5. The optimizer.** Checks are removed where a range is known. GVN already tracks
  bounds, and the common case is an induction variable below a limit. A reduction
  written with `+%` vectorizes as today. One written with `+` does not, because the
  trap depends on the order of the additions. The spec says so.
- **B6. Bootstrapping.** Under F2 the compiler's sources must be correct in both
  meanings for one release. B4 guarantees that: a program that never overflows behaves
  the same in both.

### Verification

The whole suite, the standard library's tests, the examples and both fuzzers pass with
trapping on. Benchmark numbers for the three targets go in the commit that changes the
default.

### Status

B4 and B3's measurements are done; B1, B2, B5 and B6 are not started, and the default
is unchanged (wrap). The gate's decision is the owner's.

- **`--overflow=report`** (spec §7.6, `docs/cli.md`): the program wraps, and the first
  time a site that `trap` checks overflows it writes
  `file:line:col: integer overflow (T0002), wrapped: <code>` to stderr, or appends it to
  the file `AXIOM_OVERFLOW_LOG` names. The report is `std/internal/runtime/overflow.ax`,
  added to programs built in that mode and called from a cold block after the check.
  It runs on x86-64 and AArch64 Linux, x86-64 Windows, wasm32 and wasm64 (macOS: built,
  not run). On WebAssembly it imports the host's `ax_write`, `ax_open` and `ax_getenv`.
  `run.mjs --overflow=report` (or `trap`) builds the suite's Axiom programs and tests in
  that mode, except tests whose own flags name one, and lists the sites at the end.
- **B4, the tree is clean.** Runs with a compiler built with `--overflow=report`: the
  whole suite on wasm32 (-O0, -O2) and natively (-O0, -O2); `tests/run/` on AArch64
  (qemu) and x86-64 Windows (Wine); the std tests on wasm32 and x86-64; the examples
  (`cmd/axiom/examples.sh`); the benchmarks on all three targets; and self-compiles for
  x86-64, AArch64, wasm32, Windows and macOS at -O0 and -O2. The last runs report no
  site. A compiler built with `--overflow=trap` compiles itself at -O0 and -O2.
  - Intended wraps, now `+%` / `-%`: in the compiler, `elfpack.ax` (`diff`, `plus`: the
    `-O0` trap the review found), `util.ax` (`U64Map.del`'s cyclic distances),
    `c/cgen_va.ax` (an alignment mask), `dwarf.ax` and `codeview.ax` (`LAZY + 1` is 0,
    "no symbol"), `mlink.ax` (the sizing pass of the compact unwind table), `switches.ax`
    (a span negative past 2^63); in the tests, hashes of results and byte counts; in
    std, a test's made-up script names (`graphics/shape.ax`). `ir_verify.ax` computed a
    wrapped `k - 3` it then discarded; it no longer does. The compiler's machine code
    is unchanged (`.text` and `.rodata` identical with `--stack-traces=off`; with
    traces only site records differ), and the compilers before and after build the
    same compiler.
  - Tests of the default's wrapping (`overflow_wrap.ax`, `overflow_release.ax`,
    `div_const.ax`, whose `MIN / -1` has no wrapping operator) name `--overflow=wrap`.
  - Bugs: `std/mem`'s count of bytes freed in a decay epoch wrapped on wasm32 past
    4 GiB (and with no clock the epoch never ends), so the allocator kept too little
    free memory; it saturates now, with a test. `tests/run/tailrec.ax`'s combinatorial
    index ran `r` below 0 and ended only because `n` wrapped round to it; its input is
    now one that ends.
- **B3, the measurements.** CPU time, trap / wrap, medians of alternating runs, all at
  `-O2` with the compiler that has issue #72. x86-64: 11 runs on a quiet machine (two
  earlier runs on a loaded one gave geomeans of 1.053 and 1.057, `alloc_tree` 1.13 and
  1.45: these numbers move by several percent). AArch64: qemu user time, 7 runs, only
  indicative (no hardware here, and the qemu here counts no instructions; its costs
  are not a core's).
  wasm32: node, 7 runs. The self-compile builds the compiler for x86-64 at `-O2`
  (`AXIOM_JOBS=1`; on wasm32 with `--units=module`, since `--units=program` runs out of
  wasm32's 4 GiB).

  | Program | x86-64 | AArch64 (qemu) | wasm32 |
  |---|---|---|---|
  | 13 benchmarks of `cmd/axiom/bench/`, geometric mean | 1.035 | 1.039 | 0.997 |
  | the 7 that are not `alloc_*` | 0.99 | 0.98 | 0.98 |
  | the 6 `alloc_*` | 1.09 | 1.12 | 1.02 |
  | worst | `alloc_tree` 1.27 | `alloc_threads` 1.35 | `vecpush` 1.12 |
  | over 1.15 besides | none | `alloc_churn` 1.27 | none |
  | compiler self-compile | 1.00 | 1.05 | 1.01 |
  | `reduce_sum`, `+` / `+%` | 7.14 / 0.99 | 1.19 / 0.99 | 1.46 / 0.99 |
  | `reduce_dot`, `+ *` / `+% *%` | 1.29 / 1.00 | 2.97 / 0.98 | 1.84 / 1.09 |
  | `reduce_hist`, `+` / `+%` | 0.83 / 1.09 | 1.55 / 1.01 | 1.04 / 0.98 |

  Against the bar (geometric mean within 3% on each target, no program more than 15%
  slower that `+%` cannot fix): wasm32 meets it. x86-64 misses the mean by half a
  percent and AArch64 (indicatively) by one, both from the allocator benchmarks, whose
  checks are std/mem's own arithmetic, which does not wrap: `+%` cannot fix them, and
  bounds the optimizer could use (B5) or a check-free spelling in the allocator could.
  The reductions written with `+` lose their vectorization (`reduce_sum` on x86-64 is
  7 times slower) and with `%` operators cost nothing, as B5 says.
  `reduce_hist` with `+` ran faster trapping than wrapping on x86-64 in every run (0.81
  to 0.83); not investigated (the loop's code differs in more than the check).

---

## G. Library Tiers, and the C Compiler as an Experiment

### Problem

`std/` is 387k lines. What a program needs to start is small: a hello-world imports 6
files (3.5k lines), and the compiler itself imports 29 (73k lines, of which 49k are
Unicode tables). The rest is of very uneven kind:

| Area | Lines |
|---|---|
| `graphics/` (images, fonts, shaping, PDF, SVG, charts, GPU) | 85k |
| `unicode/` | 57k |
| `audio/` (seven codecs) | 49k |
| `c/rt/` (the C runtime library) | 42k |
| `text/`, `encoding/`, `net/`, `compress/`, `crypto/`, `math/`, `archive/` | 110k |
| `collections/`, `mem`, `str`, `fmt`, `io`, `hash`, `time`, `sync`, `thread` | about 15k |

Everything is equally "standard", so nothing can be promised of any of it. The rules
this plan adds (documentation, the iterator protocol, sanitized tests) and the ones the
language plan adds (narrow error sets, `?T` for absence) cannot be enforced on 387k
lines at once. An agent looking for a hash map also has to search past an AV1 decoder.

### Design

- **G1. Three tiers, visible in the import path.**

  | Tier | Root | Contents | Promise |
  |---|---|---|---|
  | Core | `std/` | `mem`, `str`, `fmt`, `io`, `os` (files, paths, processes, cpu), `collections`, `hash`, `math` (scalar), `time`, `thread`, `sync`, `unicode` (UTF-8 and the tables the compiler needs), `diag`, `log`, `encoding/json`, `encoding/binary`, `net` (sockets), the digests (`md5`, `sha1`, `sha2`, `sha3`, as `hash/...`) | Every rule below is enforced by the suite |
  | Libraries | `lib/` | the rest of `net` (`http`, `tls`, `dns`, ...), the rest of `crypto` (ciphers, signatures, certificates), `compress`, `archive`, the rest of `encoding`, `text`, `unicode/collate` and segmentation, `math/bigint` and matrices | Tested and fuzzed as today; rules adopted module by module |
  | Media | `lib/` | `graphics`, `audio`, `os/window`, `os/cocoa`, `os/win32`, `os/gamepad` | Tested as today |

  The compiler resolves `import "lib/..."` from a `lib/` directory beside `std/`, by
  the mechanism it uses for `std/`. Releases ship both. The change to `modload.ax` is
  small, and the rest is a mechanical rename that `axiom fmt` can apply to import
  lines.
- **G2. What core promises.** The suite enforces, for `std/` only:
  - every public item is documented (`W0320`, workstream E);
  - iterators follow the protocol (workstream D);
  - absence is `?T` and error sets are narrow (language plan, phase 4);
  - tests pass under the sanitizer (workstream C) and with overflow trapping
    (workstream B).
- **G3. The C compiler is an experiment.** In concrete terms:
  - The README, `docs/c.md`, `docs/cli.md` and `axiom cc --help` say so. Release notes
    give it its own section.
  - The C runtime library (`std/c/rt/`, 42k lines) moves beside the front end, to
    `cmd/axiom/src/c/rt/` or `lib/c/`. `std/c.ax`, the helpers for calling C from
    Axiom, stays in core. The compiler's imports of `std/c/rt/ldouble` move with it.
  - Its tests keep running on every change: they have found real backend bugs. They
    are reported as their own group, and a release of the language does not wait on
    them.
  - Headline performance numbers come from Axiom programs. `cmd/axiom/bench/c/` stays
    as a stress test of the backend.
  - Work on the C front end does not delay workstreams A to F.

### Decided

- **The AAC decoder stays, as a recorded exception.** It is under the LGPL, and
  `CLAUDE.md` forbids adding copyleft code. `CLAUDE.md`'s licensing section gains the
  exception by name: the decoder (`audio/aac.ax` and `audio/internal/aac/`) keeps its
  license, a program includes it only by importing it, and nothing else under a
  copyleft license is added. It moves to `lib/` with the rest of `audio`.
- **Media stays in this repository**, under `lib/`. That keeps language migrations
  atomic while the language is still making breaking changes.
- **JSON, sockets and the digests are core.** `encoding/json` and `net.ax` (sockets)
  stay in `std/`. The digest functions (`md5`, `sha1`, `sha2`, `sha3`) move from
  `crypto/` to `std/hash/`, and `encoding/binary`, which they import, stays in `std/`.
  So core imports only core, and the compiler imports only core, with no exceptions.
  `run.mjs` checks both.

### Verification

The suite passes after the rename. `axiom doc std` lists only core, and reports no
undocumented public item.

---

## Not in This Plan

- **A corpus of agent tasks**, with compile rounds to a passing build and tokens spent
  as its measures. It is planned separately. Until it exists, workstream A's idiom table
  is the fixed test of progress.
- **Implicit lossless widening** (`u32` to `u64` without `@as`). The review counted it
  as friction. It is left out because it changes §6.1's closed list of conversions,
  which deserves the corpus's numbers first.
- **The compiler's own style**: 550 `threadlocal` globals, 4 tagged unions against 601
  structs, and about 1,950 `nil()`/`isnil()` uses. Phase 5.2 of the language plan
  already covers this.
