## C

- The target's conventional predefined macros are defined in every standard and dialect:
  the processor (`__x86_64__`, `__aarch64__`, `__wasm32__` ..., `_M_X64` on Windows), the
  system (`__linux__`, `__unix__`, `__ELF__`; `__APPLE__`, `__MACH__`; `_WIN32`, `_WIN64`;
  and in a GNU dialect `unix` and `linux`), the data model (`__LP64__`, `__ILP32__`), the
  types (`__SIZEOF_INT__`, `__INT_MAX__`, `__SIZE_TYPE__`, `__INT64_C`, `__FLT_MAX__` ...,
  as the compiler's headers have them), the byte order, the processor extensions of `--cpu`
  and `--features` (`__SSE2__`, `__AVX2__`, `__ARM_NEON`, `__ARM_FEATURE_*`,
  `__wasm_simd128__` ...), `__OPTIMIZE__` and `__PIC__`. A program that defined some of them
  with `-D` gets the same values; `-U` and `-D` still change them. Programs that include
  intrinsics headers (`<immintrin.h>`, `<arm_neon.h>`) when such a macro is defined do not
  build, the compiler having none: `-U__SSE2__` (or the macro they test) chooses their
  portable code. No macro names a compiler (`__GNUC__`, `__clang__`, `_MSC_VER`).
