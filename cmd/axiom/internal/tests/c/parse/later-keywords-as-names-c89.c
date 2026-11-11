/* flags: --c-std=c89 */
/* Before C99, inline, restrict and bool are ordinary identifiers. */
int inline;
int restrict = 2;
int bool(int typeof) {
    return typeof * restrict;
}
