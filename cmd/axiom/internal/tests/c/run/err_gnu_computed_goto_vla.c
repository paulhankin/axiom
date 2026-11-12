/* flags: --c-std=gnu11 */
/* expect-error: C3901 */
int main(int argc, char **argv) {
    (void)argv;
    void *t = &&end;
    int v[argc];
    v[0] = 0;
    goto *t;
end:
    return v[0];
}
