/* flags: --c-std=c23 */
/* expect-error: C1016 */
/* the preprocessor's arithmetic is intmax_t's: a wb constant wider than 64 bits */
#if 0x10000000000000000wb
#endif
int main(void) { return 0; }
