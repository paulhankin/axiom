/* flags: --c-std=gnu23 */
/* expect-error: C3231 */
/* __bf16 with _Float16: neither holds the other's values, no common type */
int main(void) { __bf16 a = 1; _Float16 b = 2; return (int)(a * b); }
