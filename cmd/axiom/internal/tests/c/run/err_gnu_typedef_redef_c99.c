/* flags: --c-std=gnu99 */
/* expect-error: C3002 */
/* A typedef declared again with another type stays an error in a GNU
   dialect. */
typedef int T;
typedef long T;
int main(void) { return 0; }
