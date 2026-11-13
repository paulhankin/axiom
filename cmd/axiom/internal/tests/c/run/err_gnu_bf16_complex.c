/* flags: --c-std=gnu23 */
/* expect-error: C3005 */
/* __bf16 has no complex type */
_Complex __bf16 z;
int main(void) { return 0; }
