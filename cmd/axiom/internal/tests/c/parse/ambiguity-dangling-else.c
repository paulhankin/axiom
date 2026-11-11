/* An else belongs to the nearest if. */
int f(int a, int b) {
    if (a)
        if (b)
            return 1;
        else
            return 2;
    return 3;
}
