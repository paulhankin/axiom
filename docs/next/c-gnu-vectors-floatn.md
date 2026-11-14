## C

- Vector types (`__attribute__((vector_size(n)))` and the vector modes) in the GNU
  dialects: the operators lane by lane, comparisons giving -1 and 0, a scalar with a
  vector, subscripts, initializers, casts between vectors of one size, `?:` with a vector
  condition, `__builtin_shuffle`, `__builtin_shufflevector` and
  `__builtin_convertvector`, from 2 to 4096 bytes of any integer or real floating
  lanes; generated with the target's vector instructions, and passed as each platform's
  ABI says: on x86-64 System V, vectors of 32 bytes in `ymm` registers when AVX is
  enabled (`--cpu=x86-64-v3`, `--features=+avx`) and of 64 bytes in `zmm` registers with
  AVX-512F (else in memory), as the psABI has it, so that libraries taking `__m256` or
  `__m512` values by value can be called (checked against the system's C compiler both
  ways).
- The interchange and extended floating types of C23 Annex H, in every mode:
  `_Float16`, `_Float32`, `_Float64`, `_Float128`, `_Float32x`, `_Float64x` (and
  `__float128` with the `q` suffix in a GNU dialect), their constants, conversions and
  ABI, `<float.h>`'s macros, and under `__STDC_WANT_IEC_60559_TYPES_EXT__` the
  `_Float16` and `_Float128` functions of `<math.h>` and `strtof16`, `strtof128`,
  `strfromf16`, `strfromf128` (binary128 on every target, whatever `long double` is),
  which `<tgmath.h>` chooses for such arguments; they convert to and from the decimal
  floating types, and the format checks know printf's and scanf's `H`, `D` and `DD`.
- The imaginary types of Annex G (`float`, `double` and `long double _Imaginary`,
  `imaginary` and `_Imaginary_I` in `<complex.h>`), with G.4's conversions and G.5's
  arithmetic; `2.0i` constants in a GNU dialect.
- Format strings are checked at calls of functions with the attribute `format` (the C
  library's `printf`, `scanf`, `strftime` and their kin in every mode): warnings C4130
  to C4135, chosen with `--c-warn-format=0|1|2` and named `-Wformat` ... by `#pragma GCC
  diagnostic`.
- Nested functions (GNU), whose addresses work through a pool of compiled trampolines
  (no executable stack); `__builtin_object_size` of the objects an expression shows;
  `__builtin_frame_address` and `__builtin_return_address` of any level;
  `__builtin_setjmp` and `__builtin_longjmp`; `__builtin_cpu_supports`, `__builtin_cpu_is`
  and `__builtin_cpu_init`; `__builtin_ia32_pause` and the fences on every target;
  enumerations of underlying type `__int128`; case ranges in a `switch` on `__int128`;
  `aligned` on a typedef; `cold` and `__builtin_expect` lay out the unlikely code apart;
  `visibility`, `section` and `#pragma GCC visibility` are recorded on the IR's functions
  and objects.
