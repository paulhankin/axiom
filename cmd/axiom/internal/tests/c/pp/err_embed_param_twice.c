/* flags: --c-std=c23 */
/* expect-error: C1083 */
/* An embed parameter appears at most once (6.10.4.1). */
int a[] = {
#embed "inc/abc.bin" limit(1) __limit__(2)
};
