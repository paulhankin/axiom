/* An object with static storage duration cannot have a VLA type. */
/* expect-error: C3081 */
int f(int n) { static int a[n]; return a[0]; }
int main(void) { return f(1); }
