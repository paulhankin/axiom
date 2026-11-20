/* flags: --c-std=gnu17 --c-warnings=error */
/* The GNU attributes that only inform another compiler's checks (access,
   nonstring, no_sanitize, tls_model, ...) are known (__has_attribute) and
   taken without a diagnostic; the program is as without them. */
#include "host.h"
#if !__has_attribute(access) || !__has_attribute(nonstring) || !__has_attribute(tls_model)
#error "hint attributes not known"
#endif
__attribute__((access(read_only, 1, 2))) static int sum(const char *p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) s += p[i];
    return s;
}
__attribute__((nonstring)) static char tag[4] = "abcd";
static __thread int tl __attribute__((tls_model("initial-exec"))) = 3;
__attribute__((no_sanitize("address"), no_stack_protector, nocf_check)) static int three(void) { return tl; }
__attribute__((__no_sanitize_address__, zero_call_used_regs("used"))) static int four(void) { return 4; }
int main(void) {
    print_i64(sum(tag, 4) + three() + four());
    return 0;
}
/* expect: 401 */
