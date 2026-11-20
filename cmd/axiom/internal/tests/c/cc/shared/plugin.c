/* A plugin: it calls a function of the program that loads it, which
   nothing in the program calls (-rdynamic exports it). */
int host_only(int x);
int plugin_run(int x) { return host_only(x) + 1; }
