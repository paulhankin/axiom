int lib_input(void);
void lib_report(int x);

int main(void) {
    int x = lib_input();
    lib_report(x * x);
    return 3;
}
