/* flags: --c-std=c23 */
/* expect-error: C1084 */
#embed "inc/abc.bin" vendor::thing(1)
