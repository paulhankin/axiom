/* #line changes __LINE__ and __FILE__; its operands may come from macros. */
__LINE__
#line 100
__LINE__ __FILE__
#line 200 "other.c"
__LINE__ __FILE__
#define N 300
#define F "macro.c"
#line N F
__LINE__ __FILE__
#line 2147483647
__LINE__
/* a comment
   over lines */ #line 10 "a\\b.c" /* and
   another */
__LINE__ __FILE__
#define L __LINE__
L
#define f(x) x __LINE__
f(
1
)
