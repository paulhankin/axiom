/* The target's macros may be undefined and defined again, with -U and -D
   (-D replaces one) and in the source; under an ISO standard linux and
   unix are the program's names. */
/* flags: --target=x86_64 -U__linux__ -D__x86_64__=2 -Dlinux=3 -D__SIZE_TYPE__=int */
__linux__ __x86_64__ linux unix __SIZE_TYPE__
#undef __ELF__
#define __unix__ 5
#define __LP64__ 64
#undef __SSE2__
#define __SSE2__ 2
__ELF__ __unix__ __LP64__ __SSE2__
#undef __CHAR_BIT__
#define __CHAR_BIT__ 9
#define unix 6
__CHAR_BIT__ unix
