/* A hidden declaration of what no object of the link defines. */
__attribute__((visibility("hidden"))) extern int nowhere;
int get(void) { return nowhere; }
