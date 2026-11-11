/* Before C23 #elifdef is no directive: in a skipped group it is ignored. */
/* flags: --c-std=c17 */
#if 0
#elifdef X
bad
#else
ok
#endif
#if true
bad
#else
ok: true is 0 before C23
#endif
