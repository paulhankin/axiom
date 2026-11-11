/* flags: --c-std=c17 */
/* expect-error: C1014 */
/* Digit separators are C23's: before, 1'000 is a pp-number and an unterminated character constant. */
int x = 1'000;
