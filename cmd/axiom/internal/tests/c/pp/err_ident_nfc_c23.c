/* flags: --c-std=c23 */
/* expect-error: C1005 */
/* C23 6.4.2.1: an identifier is in Normalization Form C (here: e and a combining acute accent, decomposed). */
int e\u0301;
