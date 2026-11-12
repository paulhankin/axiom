/* flags: --c-std=gnu11 */
/* expect-error: C3091 */
/* expect-warning: C4123 */
void bad_call(void) __attribute__((error("do not call this")));
void warn_call(void) __attribute__((warning("careful")));
void warn_call(void) { }
int main(void) { warn_call(); bad_call(); return 0; }
