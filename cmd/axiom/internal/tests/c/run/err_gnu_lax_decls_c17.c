/* flags: --c-std=c17 */
/* expect-error: C3022 */
/* expect-error: C3061 */
/* Under an ISO standard pointers to incompatible object types and main(int)
   are errors (an empty declaration at file scope too: parse/). */
int main(int argc) {
    long *q = &argc;
    return (int)*q;
}
