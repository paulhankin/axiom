/* flags: --c-std=c17 */
/* expect-error: C2100 */
/* u8 character constants are C23's (before, u8 and a character constant). */
int c = u8'a';
