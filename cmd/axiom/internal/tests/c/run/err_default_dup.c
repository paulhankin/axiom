/* expect-error: C3041 */
int main(void) { switch (1) { default: default: break; } return 0; }
