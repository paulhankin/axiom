/* flags: --c-std=c11 */
/* Constants and string literals, with their values, bases and suffixes. */
unsigned long long i[] = {0, 42, 052, 0x2A, 42u, 42l, 42ul, 42LL, 42ull, 0XffU};
double d[] = {1.5, 1e3, .5, 2.f, 1.0L, 0x1p4};
int c[] = {'a', '\n', '\x41', '\101', L'b', u'c', U'd'};
char *s1 = "ab\tc";
char *s2 = "con" "cat" "enated";
int *s3 = (int *)L"wide";
