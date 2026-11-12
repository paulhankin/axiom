## C

- `<string.h>`: the functions C23 asks of a freestanding implementation (all but
  `strdup`, `strndup`, `strcoll`, `strxfrm` and `strerror`), `memccpy` and
  `memset_explicit` included, and in C23 the generic `memchr`, `strchr`, `strpbrk`,
  `strrchr` and `strstr`. A program gets the ones it uses and does not define itself;
  `memcpy`, `memset`, `strlen` and the others of memory work a word at a time.
- The typedefs of `<stddef.h>` and `<stdint.h>` are the platform's types, not only of its
  widths: `size_t` is `unsigned long` on wasm32, `int64_t` is `long long` on macOS and
  WebAssembly, `intmax_t` and the `int_fast` types are the platform library's, and
  `max_align_t` is aligned as the platform's (16 bytes on Linux and WebAssembly). A
  program that relied on the old choices (in `_Generic` or a `printf` format) may need
  updating.
- Bit-fields are laid out as the target's ABI lays them out: on AArch64 Linux unnamed
  bit-fields add to a structure's alignment, and on Windows bit-fields of different type
  sizes no longer share a storage unit. Structures with bit-fields change size there.
- Variadic and other arguments aligned to more than 16 bytes are passed as the platforms'
  C ABIs pass them (x86-64 Linux, macOS), and `_BitInt` arguments and results from other
  compilers are read correctly.
- C23: enumerations with the same tag in different scopes are compatible when their
  constants and fixed underlying types agree; pointers to arrays whose elements differ in
  qualifiers are operands of `?:` (the result has both's qualifiers).
- A variadic function defined in one translation unit and called from an earlier one no
  longer crashes the compiler.
- The front end needs about half the memory it did: the tokens are given back once
  parsed, and the syntax tree once the IR is made.
