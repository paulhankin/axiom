/* flags: --c-std=gnu17 */
/* Assertions (a GNU dialect): #assert, #unassert of one answer and of all,
   #pred(answer) and #pred in #if expressions (not macro-expanded; answers
   compared token by token, whitespace ignored), the target's cpu, machine
   and system predicates. */
#define vax notvax
#assert machine(vax)
#assert answer(a b  c)
#assert answer(x (y) z)
#if #machine(vax) && #machine && !#nothing && #answer(a   b c) && #answer(x(y)z) && !#answer(a b)
one
#endif
#unassert machine(vax)
#if #machine(vax) || #machine(notvax)
bad
#endif
#if #cpu && #machine
two
#endif
#unassert answer
#if #answer
bad
#endif
#assert answer(again)
#if #answer(again) ? 1 : 0
three
#endif
