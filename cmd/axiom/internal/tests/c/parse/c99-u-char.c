/* flags: --c-std=c99 */
/* expect-error: C2100 */
/* u'' and U'' are C11's (before, the prefix is an identifier). */
int c = u'a';
