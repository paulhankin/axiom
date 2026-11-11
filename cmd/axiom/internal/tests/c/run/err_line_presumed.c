/* expect-error: C3001 */
/* Diagnostics show the position #line presents (the presumed file and
   line), and the agent-json format the physical one with it (run.mjs
   checks both). */
int a;
#line 100 "gen.y"
int b = c;
#line 7
int d = e;
int main(void) { return 0; }
