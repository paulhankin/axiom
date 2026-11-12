/* Two units define one name: C3064 (6.9p5), also beside Axiom modules. */
/* uses: ../print.ax */
/* expect-error: C3064 */
int shared = 1;
int main(void) { return shared; }
