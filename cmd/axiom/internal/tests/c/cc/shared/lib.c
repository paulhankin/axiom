/* A shared library: lib_api is exported (the version script), lib_inner
   is not, and host_value is the program's (bound when it is loaded). */
int host_value(void);
int lib_inner(int x) { return x + 1; }
int lib_api(int x) { return lib_inner(x) * host_value(); }
