/* flags: --c-std=gnu17 */
/* expect-error: C1087 */
/* #assert needs an answer, #unassert a predicate */
#assert machine
#unassert
#if #x(
#endif
