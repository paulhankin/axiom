/* flags: --c-std=c11 */
/* expect-error: C2001 */
/* _Alignof takes a type name; its expression form is not standard */
int y;
int main(void) { return (int)_Alignof(y); }
