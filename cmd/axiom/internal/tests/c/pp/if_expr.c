/* #if: the unsigned rules, short-circuit evaluation, ?:, character constants, defined. */
#if -1 < 0u
bad1
#else
ok1
#endif
#if -1 < 0
ok2
#endif
#if 0 && (1/0)
bad3
#elif 1 || (1/0)
ok3
#endif
#if 1 ? 2 : (1/0)
ok4
#endif
#if (0 ? -1 : 0u) > 0
bad5
#elif (1 ? -1 : 0u) > 0
ok5
#endif
#if 'a' == 97 && '\377' < 0
ok6
#endif
#if defined X || defined(Y)
bad7
#else
ok7
#endif
#define X
#ifdef X
ok8
#endif
#if UNDEF == 0 && !UNDEF2
ok9
#endif
#if 0x7fffffffffffffff > 0 && 0xffffffffffffffff == -1
ok10
#endif
#if (2 || 1/0) == 1 && (0 && 1/0) == 0
ok11
#endif
#if ~0u == 18446744073709551615u
ok12
#endif
#if 10 % 3 == 1 && -7 / 2 == -3 && -7 % 2 == -1 && (1 << 62) > 0 && (-8 >> 1) == -4
ok13
#endif
#if 1
# if 0
#  error no
# elif 0
# else
ok14
# endif
#endif
#if 0
#if 1
bad
#else
bad
#endif
#elif 0
#else
ok15
#endif
#ifndef Y
ok16
#endif
