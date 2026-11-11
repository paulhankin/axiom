/* A trigraph # starts a directive; ??/ before a new-line splices lines. */
/* flags: --c-std=c89 */
??=define X 1 ??/
+ 2
X
