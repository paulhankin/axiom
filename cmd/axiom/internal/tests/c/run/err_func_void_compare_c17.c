/* expect-error: C3020 */
/* In an ISO mode a pointer to void is not compared with a function pointer
   (6.5.9p2). */
static int one(void) { return 1; }
int main(void) {
    void *p = 0;
    return p == one;
}
