/* Decimal floating constants are a C23 feature. */
/* flags: --c-std=c17 */
/* expect-error: C1019 */
x = 1.0df;
