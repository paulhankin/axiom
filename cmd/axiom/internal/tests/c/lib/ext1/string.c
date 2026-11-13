/* Annex K's <string.h> (C11 K.3.7): each function's result, its stores,
   and every runtime-constraint violation, which calls the handler (here
   one that prints its message and error) and then does what K.3.7 says
   it does after one. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

static int calls;

static void handler(const char *restrict msg, void *restrict ptr, errno_t error) {
    calls++;
    printf("handler: %s (%s, %s)\n", msg, ptr == NULL ? "null" : "ptr",
           error == EINVAL ? "EINVAL" : error == ERANGE ? "ERANGE" : "other");
}

static void show(const char *name, const char *s, size_t n) {
    size_t i;
    printf("%s:", name);
    for (i = 0; i < n; i++)
        printf(" %d", (unsigned char)s[i]);
    printf("\n");
}

int main(void) {
    char a[8], b[16], *t, *ptr;
    rsize_t max;
    errno_t r;
    constraint_handler_t old = set_constraint_handler_s(handler);
    printf("old is abort: %d\n", old == abort_handler_s);
    printf("RSIZE_MAX: %d\n", RSIZE_MAX == SIZE_MAX >> 1);

    /* memcpy_s: copies; n > s1max zeroes all of s1 */
    r = memcpy_s(a, sizeof a, "abcdef", 6);
    printf("memcpy_s %d %.6s\n", r, a);
    memset(a, 'x', sizeof a);
    r = memcpy_s(a, 4, "abcdef", 6);
    show("memcpy_s too long", a, 8);
    r = memcpy_s(a, sizeof a, NULL, 1);
    printf("memcpy_s null: %d\n", r != 0);
    r = memcpy_s(a, (rsize_t)-1, "a", 1);
    printf("memcpy_s huge: %d\n", r != 0);
    strcpy(b, "0123456789");
    r = memcpy_s(b + 2, 8, b, 4);
    printf("memcpy_s overlap: %d [%s]\n", r != 0, b);
    /* memmove_s: overlap is fine */
    strcpy(b, "0123456789");
    r = memmove_s(b + 2, 8, b, 4);
    printf("memmove_s %d %s\n", r, b);
    r = memmove_s(b, 2, "xyz", 3);
    show("memmove_s too long", b, 3);

    /* strcpy_s */
    r = strcpy_s(a, sizeof a, "hello");
    printf("strcpy_s %d %s\n", r, a);
    r = strcpy_s(a, sizeof a, "12345678");
    printf("strcpy_s too long %d [%s]\n", r != 0, a);
    r = strcpy_s(a, 0, "x");
    r = strcpy_s(NULL, 5, "x");
    strcpy(a, "abc");
    r = strcpy_s(a, 4, a + 1);
    printf("strcpy_s overlap %d [%s]\n", r != 0, a);

    /* strncpy_s: at most n, a null after; n >= s1max needs a short s2 */
    memset(a, 'x', sizeof a);
    r = strncpy_s(a, sizeof a, "hello", 3);
    show("strncpy_s 3", a, 5);
    r = strncpy_s(a, 5, "hello", 10);
    printf("strncpy_s n >= s1max, long %d [%s]\n", r != 0, a);
    r = strncpy_s(a, 6, "hello", 10);
    printf("strncpy_s %d [%s]\n", r, a);
    r = strncpy_s(a, 3, "hello", 2);
    printf("strncpy_s n < s1max %d [%s]\n", r, a);
    /* the example of K.3.7.1.4p6 */
    {
        char src1[100] = "hello", src2[7] = {'g', 'o', 'o', 'd', 'b', 'y', 'e'};
        char dst1[6], dst2[5], dst3[5];
        int r1, r2, r3;
        r1 = strncpy_s(dst1, 6, src1, 100);
        r2 = strncpy_s(dst2, 5, src2, 7);
        r3 = strncpy_s(dst3, 5, src2, 4);
        printf("example: %d %s, %d %d, %d %s\n", r1, dst1, r2 != 0, dst2[0], r3, dst3);
    }

    /* strcat_s */
    strcpy(a, "ab");
    r = strcat_s(a, sizeof a, "cde");
    printf("strcat_s %d %s\n", r, a);
    r = strcat_s(a, sizeof a, "fgh");
    printf("strcat_s too long %d [%s]\n", r != 0, a);
    memset(a, 'x', sizeof a);
    r = strcat_s(a, sizeof a, "");
    printf("strcat_s no null %d [%d]\n", r != 0, a[0]);
    /* strncat_s and the example of K.3.7.2.2p6 */
    {
        char s1[100] = "good", s2[6] = "hello", s3[6] = "hello", s4[7] = "abc", s5[1000] = "bye";
        int r1, r2, r3, r4;
        r1 = strncat_s(s1, 100, s5, 1000);
        r2 = strncat_s(s2, 6, "", 1);
        r3 = strncat_s(s3, 6, "X", 2);
        r4 = strncat_s(s4, 7, "defghijklmn", 3);
        printf("strncat_s: %d %s, %d %s, %d [%d], %d %s\n", r1, s1, r2, s2, r3 != 0, s3[0], r4, s4);
    }

    /* strtok_s: the example of K.3.7.3.1p9 */
    {
        static char str1[] = "?a???b,,,#c";
        static char str2[] = "\t";
        char *ptr1, *ptr2;
        rsize_t max1 = sizeof str1, max2 = sizeof str2;
        t = strtok_s(str1, &max1, "?", &ptr1);
        printf("1 [%s] %d\n", t, (int)max1);
        t = strtok_s(NULL, &max1, ",", &ptr1);
        printf("2 [%s] %d\n", t, (int)max1);
        t = strtok_s(str2, &max2, " \t", &ptr2);
        printf("3 %d %d\n", t == NULL, (int)max2);
        t = strtok_s(NULL, &max1, "#,", &ptr1);
        printf("4 [%s] %d\n", t, (int)max1);
        t = strtok_s(NULL, &max1, "?", &ptr1);
        printf("5 %d\n", t == NULL);
    }
    strcpy(b, "abcdef ghi");
    max = 4;
    t = strtok_s(b, &max, " ", &ptr);
    printf("strtok_s no end within: %d %d %s\n", t == NULL, (int)max, b);
    max = 5;
    t = strtok_s(NULL, &max, " ", NULL);

    /* memset_s */
    r = memset_s(a, sizeof a, 'q', 3);
    show("memset_s", a, 4);
    r = memset_s(a, 4, 'z', 5);
    show("memset_s n > smax", a, 5);

    /* strerror_s, strerrorlen_s: the whole message, or cut with "..." */
    r = strerror_s(b, sizeof b, ERANGE);
    printf("strerror_s %d %s %d\n", r, b, (int)strerrorlen_s(ERANGE) == (int)strlen(strerror(ERANGE)));
    r = strerror_s(a, sizeof a, ERANGE);
    printf("strerror_s cut %d %s\n", r != 0, a);
    r = strerror_s(a, 3, ERANGE);
    printf("strerror_s 3 %d %s\n", r != 0, a);
    r = strerror_s(a, 0, ERANGE);

    /* strnlen_s */
    printf("strnlen_s %d %d %d %d\n", (int)strnlen_s("abc", 10), (int)strnlen_s("abc", 2), (int)strnlen_s(NULL, 5),
           (int)strnlen_s("abc", 0));

    /* ignore_handler_s: no message */
    set_constraint_handler_s(ignore_handler_s);
    r = strcpy_s(NULL, 1, "a");
    printf("ignored %d, handler calls %d\n", r != 0, calls);
    printf("previous %d\n", set_constraint_handler_s(NULL) == ignore_handler_s);
    printf("default %d\n", set_constraint_handler_s(NULL) == abort_handler_s);
    return 0;
}

// expect: old is abort: 1
// expect: RSIZE_MAX: 1
// expect: memcpy_s 0 abcdef
// expect: handler: memcpy_s: n is greater than s1max (null, ERANGE)
// expect: memcpy_s too long: 0 0 0 0 120 120 120 120
// expect: handler: memcpy_s: s2 is a null pointer (null, EINVAL)
// expect: memcpy_s null: 1
// expect: handler: memcpy_s: s1max is greater than RSIZE_MAX (null, ERANGE)
// expect: memcpy_s huge: 1
// expect: handler: memcpy_s: the objects overlap (null, EINVAL)
// expect: memcpy_s overlap: 1 [01]
// expect: memmove_s 0 0101236789
// expect: handler: memmove_s: n is greater than s1max (null, ERANGE)
// expect: memmove_s too long: 0 0 48
// expect: strcpy_s 0 hello
// expect: handler: strcpy_s: s2 does not fit in s1max characters (null, ERANGE)
// expect: strcpy_s too long 1 []
// expect: handler: strcpy_s: s1max is zero (null, ERANGE)
// expect: handler: strcpy_s: s1 is a null pointer (null, EINVAL)
// expect: handler: strcpy_s: the objects overlap (null, EINVAL)
// expect: strcpy_s overlap 1 []
// expect: strncpy_s 3: 104 101 108 0 120
// expect: handler: strncpy_s: s2 does not fit in s1max characters (null, ERANGE)
// expect: strncpy_s n >= s1max, long 1 []
// expect: strncpy_s 0 [hello]
// expect: strncpy_s n < s1max 0 [he]
// expect: handler: strncpy_s: s2 does not fit in s1max characters (null, ERANGE)
// expect: example: 0 hello, 1 0, 0 good
// expect: strcat_s 0 abcde
// expect: handler: strcat_s: s2 does not fit after s1's string (null, ERANGE)
// expect: strcat_s too long 1 []
// expect: handler: strcat_s: s1 has no null character in s1max characters (null, ERANGE)
// expect: strcat_s no null 1 [0]
// expect: handler: strncat_s: s2 does not fit after s1's string (null, ERANGE)
// expect: strncat_s: 0 goodbye, 0 hello, 1 [0], 0 abcdef
// expect: 1 [a] 9
// expect: 2 [??b] 5
// expect: 3 1 1
// expect: 4 [c] 1
// expect: 5 1
// expect: handler: strtok_s: no end of the token within *s1max characters (null, EINVAL)
// expect: strtok_s no end within: 1 4 abcdef ghi
// expect: handler: strtok_s: ptr is a null pointer (null, EINVAL)
// expect: memset_s: 113 113 113 120
// expect: handler: memset_s: n is greater than smax (null, ERANGE)
// expect: memset_s n > smax: 122 122 122 122 120
// expect: strerror_s 34 Result out o... 1
// expect: strerror_s cut 1 Resu...
// expect: strerror_s 3 1 Re
// expect: handler: strerror_s: maxsize is zero (null, ERANGE)
// expect: strnlen_s 3 2 0 0
// expect: ignored 1, handler calls 18
// expect: previous 1
// expect: default 1
