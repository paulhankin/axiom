/* Decimal floating types are not supported. */
/* flags: --c-std=c23 */
/* expect-error: C1019 */
x = 1.0df;
