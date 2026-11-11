/* expect-error: C3022 */
/* char ** does not convert to const char ** (the pointed-to types differ) */
int main(void) { char *s = 0; char **pp = &s; const char **cpp = pp; return cpp != 0; }
