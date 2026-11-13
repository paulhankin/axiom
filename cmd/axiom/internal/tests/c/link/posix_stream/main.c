/* With the compiler's C library, an object calling a POSIX function on the
   system library's streams (fileno) cannot be linked; with
   --c-lib=system it can (system_lib/). */
/* expect-error: E9004 */
/* expect-stderr: calls 'fileno' */
int sys_fd(void);
int main(void) { return sys_fd(); }
