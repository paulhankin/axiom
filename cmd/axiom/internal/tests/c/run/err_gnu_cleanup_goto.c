/* flags: --c-std=gnu11 */
/* expect-error: C3083 */
static void done(int *p) { (void)p; }
int main(void) {
    goto in;
    {
        int x __attribute__((cleanup(done))) = 1;
    in:
        return x;
    }
}
