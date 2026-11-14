/* In a GNU dialect 'defined' that macro replacement makes in a #if
   expression is evaluated (6.10.1p4 leaves it undefined; under an ISO
   standard it is C1044): its operand is the tokens after it, not replaced. */
/* flags: --c-std=gnu17 */
#define HAVE_A (defined(A) || defined B)
#define B 0
#if HAVE_A
yes_b
#endif
#undef B
#if HAVE_A
no
#else
neither
#endif
