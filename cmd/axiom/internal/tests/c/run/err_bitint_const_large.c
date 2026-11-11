/* flags: --c-std=c23 */
/* expect-error: C3211 */
/* needs 65 bits as a signed _BitInt */
int x = 0xFFFFFFFFFFFFFFFFwb;
int main(void) { return 0; }
