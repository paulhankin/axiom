/* flags: --c-std=c11 */
/* expect-error: C2113 */
int f(a) register a; { return a; }
