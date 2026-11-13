/* This compiler's object: aliases (a GNU dialect) of a static function and
   a static object, and a weak alias, each a symbol of its own. */
static int sdata = 41;
extern int pub_data __attribute__((alias("sdata")));
static int sfn(void) { return 3; }
int pub_fn(void) __attribute__((alias("sfn")));
int pub_weak(void) __attribute__((weak, alias("sfn")));
