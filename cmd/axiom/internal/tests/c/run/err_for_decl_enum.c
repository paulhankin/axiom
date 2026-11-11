/* flags: --c-std=c99 */
/* expect-error: C3006 */
/* 6.8.5p3: a for statement's declaration declares only objects, not
   enumeration constants. */
int main(void) { for (enum { B } y = B; y; ) break; return 0; }
