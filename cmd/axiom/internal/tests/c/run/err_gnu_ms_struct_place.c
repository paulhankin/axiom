/* flags: --target=x86_64 --c-std=gnu17 */
/* check-only */
/* ms_struct appertains to structures and unions only (elsewhere C4052,
   ignored); a malformed #pragma ms_struct is C4112. */
int x __attribute__((ms_struct)); /* expect-warning: C4052 */
#pragma ms_struct sideways /* expect-warning: C4112 */
int main(void) { return 0; }
