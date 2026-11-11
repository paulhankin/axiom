/* flags: --c-std=c99 */
/* expect-error: C3006 */
/* 6.8.5p3: a for statement's declaration declares only objects, not a tag. */
int main(void) { for (struct s *p = 0; p; ) break; return 0; }
