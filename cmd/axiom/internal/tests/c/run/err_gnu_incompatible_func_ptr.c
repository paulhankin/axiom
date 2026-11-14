/* flags: --c-std=gnu17 */
/* expect-error: C3022 */
/* In a GNU dialect pointers to incompatible function types stay errors
   (only object pointers are converted with a warning), as do _Atomic
   pointed-to types. */
static int f(int x) { return x; }
int main(void) {
    long (*g)(long) = f;
    _Atomic int a = 0;
    int *p = &a;
    return (int)g(0) + *p;
}
