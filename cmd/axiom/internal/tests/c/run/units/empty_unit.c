/* A unit whose conditional inclusion leaves no declarations. */
#ifdef NOT_DEFINED_ANYWHERE
int never_compiled(void) { return 1; }
#endif
