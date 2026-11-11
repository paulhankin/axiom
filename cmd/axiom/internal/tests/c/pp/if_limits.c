/* #if arithmetic is intmax_t and uintmax_t: 64 bits in every standard. */
/* flags: --c-std=c89 */
#if 0x7fffffffffffffff + 0 == 9223372036854775807 && 0xffffffffffffffff + 1 == 0
ok1
#endif
#if 0x8000000000000000 > 0 && -1 > 0u && (0u - 1) == 0xffffffffffffffff
ok2
#endif
#if 18446744073709551615 == -1
ok3 C89: a decimal constant too large for long is unsigned long
#endif
#if (-9223372036854775807 - 1) < 0 && -9223372036854775807 - 1 == 0x8000000000000000
ok4
#endif
#if ~0 == -1 && ~0u == 0xffffffffffffffff && (1 ? -1 : 0u) == 0xffffffffffffffff
ok5
#endif
#if (0x7fffffffffffffff >> 62) == 1 && (-1 >> 63) == -1 && (1u << 63) == 0x8000000000000000
ok6
#endif
