/* Every kind of statement. */
int f(int n) {
    int i, s = 0;
    ;
    {
        s = 1;
    }
    if (n)
        s = 2;
    if (n) {
        s = 3;
    } else if (n > 1) {
        s = 4;
    } else
        s = 5;
    switch (n) {
    case 1:
        s = 6;
        break;
    case 2:
    case 3:
        s = 7;
    default:
        s = 8;
    }
    while (n)
        n--;
    do
        n++;
    while (n < 10);
    for (i = 0; i < n; i++)
        continue;
    for (;;)
        break;
    goto end;
end:
    return s;
}
void g(void) {
    return;
}
