/* flags: --c-std=c99 */
/* expect-error: C3020 */
/* C99 6.5.8p2: a relational operator takes two pointers to object types or
   two to incomplete types, not one of each (C11 allows it). */
int a[3];
int (*p)[] = 0;
int (*q)[3] = &a;
int main(void) { return p < q; }
