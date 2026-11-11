/* flags: --c-std=c89 */
/* expect-error: C2001 */
/* Digraphs are C95's: in C89 '<:' is '<' and ':' (the error says so). */
int a<:2:>;
int main(void) { return 0; }
