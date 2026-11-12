/* Reading standard input (C17 7.21.7, 7.21.6.4): scanf, getchar, fgets
   and ungetc on stdin, a matching failure leaving the character in the
   stream, and the end of the file (EOF from then on, 7.21.7.1p3). */
/* stdin: 12 34 abc */
/* stdin: second line */
/* stdin: x */
#include <stdio.h>

int main(void) {
    int a = 0, b = 0, c, d, e, r;
    char buf[32];
    r = scanf("%d%d", &a, &b);
    printf("%d %d %d\n", r, a, b);
    r = scanf("%d", &a);
    c = getchar();
    printf("matching failure: %d %c\n", r, c);
    ungetc('A', stdin);
    c = getchar();
    d = getchar();
    printf("ungetc: %c%c\n", c, d);
    printf("rest: [%s]", fgets(buf, sizeof buf, stdin) != NULL ? buf : "");
    printf("[%s]", fgets(buf, sizeof buf, stdin) != NULL ? buf : "");
    c = getchar();
    d = getchar();
    printf("%c %d", c, d == '\n');
    c = getchar();
    d = feof(stdin) != 0;
    e = getchar();
    printf(" %d %d %d\n", c == EOF, d, e == EOF);
    printf("%d\n", scanf("%d", &a));
    clearerr(stdin);
    printf("%d\n", feof(stdin));
    return 0;
}
/* expect: 2 12 34 */
/* expect: matching failure: 0 a */
/* expect: ungetc: Ab */
/* expect: rest: [c */
/* expect: ][second line */
/* expect: ]x 1 1 1 1 */
/* expect: -1 */
/* expect: 0 */
