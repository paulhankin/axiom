/* flags: --c-std=gnu11 */
/* __builtin_frame_address of a level above 65535 (C3090). */
/* expect-error: C3090 */
int main(void) { return __builtin_frame_address(70000) != 0; }
