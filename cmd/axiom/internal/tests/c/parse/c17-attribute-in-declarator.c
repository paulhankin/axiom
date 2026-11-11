/* flags: --c-std=c17 */
/* expect-error: C2130 */
/* expect-error: C2130 */
int x [[vendor::a]];
int *[[vendor::b]] p;
