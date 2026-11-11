/* flags: --c-std=c23 */
/* expect-error: C1083 */
/* The limit is a constant expression without defined (6.10.4.2). */
int a[] = {
#embed "inc/abc.bin" limit(defined X)
};
