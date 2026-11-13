/* Aliases (a GNU dialect) of a static function and an object, strong and
   weak, which other units of the program call and read. */
static int impl(int x) { return x + 1; }
int api(int) __attribute__((alias("impl")));
int wapi(int) __attribute__((weak, alias("impl")));
int wapi2(int) __attribute__((weak, alias("impl")));
int data_impl = 5;
extern int data_alias __attribute__((alias("data_impl")));
int a_self(void) { return api(1) + data_alias; }
