# 0005: Further cuts to the compiler's memory

- Type: feature
- Status: open
- Area: compiler (selfhost)

An -O2 build of the self-hosted compiler now peaks at about 420 MB (from
2 GB: shared constants, per-function scratch for the backends, recycled IR;
commits fafd682 and 5989ef5). What is left is mostly live data. By phase,
the main arena grows by about 77 MB in parsing and checking, 150 MB in IR
generation, 20 MB in lowering, 30 MB in optimization.

Ideas, roughly by expected gain:

- Smaller `ir.Value` (about 300 bytes): rarely used fields (`mask`, `ext`,
  `name`, `aty`, `fty`, the backend scratch fields) could move to side tables
  or a union. Every instruction pays for them.
- Release the front end after IR generation: parse and check into an arena
  of their own and free it once codegen has made the IR (nothing in the IR
  should point into it; `AXIOM_POISON` can prove it).
- Optimize each function as soon as it is generated ("streaming"), so its
  discarded IR is recycled before the next function is made. A patch exists
  (lowering per function, `Func.ready`); it was correct but did not lower
  the peak by itself, since most of IR generation's growth is not
  instructions. Worth revisiting with the item below.
- String constants: `ir.cstring` makes an aggregate with one pointer per
  byte (16 bytes per character with its list); a byte-string constant kind
  would make them 1 byte per character. Stack-trace call-site records make
  ~25,000 such strings in this build.
- IR of a function compiled by the backend is dead; it could be recycled
  too (this would not lower the peak today, which comes before the backend).
