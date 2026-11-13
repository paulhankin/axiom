/* With the compiler's C library (--c-lib=axiom) an object that refers to a
   name of the system C library's own implementation (here the stream
   internals its <stdio.h> expands putc_unlocked to) cannot be linked: the
   program would have two C libraries. */
/* expect-error: E9004 */
/* expect-stderr: refers to '__overflow' */
int sys_put(int c);
int main(void) { return sys_put('x'); }
