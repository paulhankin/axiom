/* A function that neither C nor Axiom defines, on Linux where the C
   library does not have it either: the link reports it (E9004, after the
   linker's "undefined symbol"). (WebAssembly imports it from the host, and
   Windows and macOS from the system's libraries.) */
/* linux-only */
/* expect-error: E9004 */
/* expect-stderr: undefined symbol 'no_such_function_anywhere' */
int no_such_function_anywhere(int);
int main(void) { return no_such_function_anywhere(1); }
