/* host.h - the output functions the test host provides to the C tests
   (the same as the Axiom tests': cmd/axiom/internal/tests/run.mjs for
   WebAssembly, cmd/axiom/internal/tests/native_rt.c for native). Each
   prints its value and a newline. */
#ifndef HOST_H
#define HOST_H

#include <stddef.h>

void print_i64(__axiom_int64 x);
void print_u64(unsigned __axiom_int64 x);
void print_f64(double x);
void print_str(const char *p, size_t n);
void print_bool(int b);

#endif
