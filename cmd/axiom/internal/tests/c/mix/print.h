/* print.h - the output functions of the mixed-language tests, which
   print.ax (next to this file) defines in Axiom. */
#ifndef PRINT_H
#define PRINT_H

#include <stddef.h>

void print_i64(__axiom_int64 x);
void print_u64(unsigned __axiom_int64 x);
void print_bool(_Bool b);
void print_str(const char *p, size_t n);
void print_cstr(const char *p);
void print_f64(double x);

#endif
