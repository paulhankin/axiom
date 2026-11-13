/* flags: --c-std=gnu17 */
/* expect-warning: C4112 */
/* pop_options without push_options, and an extension the target does not
   have, are ignored with C4112. */
#pragma GCC pop_options
#pragma GCC target("no-such-extension")
int x;
