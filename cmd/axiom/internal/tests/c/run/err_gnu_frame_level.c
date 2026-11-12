/* flags: --c-std=gnu11 */
/* expect-error: C3090 */
int main(void) { return __builtin_frame_address(1) != 0; }
