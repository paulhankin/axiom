/* Objects alone, and none defines main: C3063. */
/* main-object: ours */
/* expect-error: C3063 */
int object_function(void) { return 1; }
