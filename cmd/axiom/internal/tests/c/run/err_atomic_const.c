/* flags: --c-std=c11 */
/* expect-error: C3021 */
/* a const atomic object is not modifiable */
const _Atomic int c = 1;
int main(void) { c += 1; return 0; }
