/* A floating constant must be in the range of its type. */
/* expect-error: C1018 */
double x = 1e309;
