/* setlocale (C17 7.11.1.1). The locales: "C" and "POSIX" (a single-byte
   encoding), "C.UTF-8" and any name whose codeset (after its last '.',
   before an "@modifier") is "UTF-8" or "utf8" in any case, such as
   "en_US.UTF-8": each has the "C" locale's conventions, with UTF-8 as
   LC_CTYPE's encoding. "" is the environment's locale for the category,
   as POSIX defines it: $LC_ALL, else $LC_<category>, else $LANG, the first
   that is set and not empty, else "C". A query (a null pointer) of LC_ALL
   when the categories differ gives "LC_COLLATE=...;LC_CTYPE=...;..."
   (each category's name), which setlocale(LC_ALL, ...) accepts back. */
#include <axc/text.h>
#include <locale.h>

static const char *const category_names[6] = {"LC_ALL", "LC_COLLATE", "LC_CTYPE", "LC_MONETARY",
                                              "LC_NUMERIC", "LC_TIME"};

static int lower(int c) {
    return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

static int same(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

/* whether s[0..n) is t, ignoring case */
static int same_fold(const char *s, int n, const char *t) {
    int i;
    for (i = 0; i < n; i++)
        if (t[i] == 0 || lower((unsigned char)s[i]) != t[i])
            return 0;
    return t[n] == 0;
}

/* -1 if the first n bytes of s are not a locale's name, else whether its
   encoding is UTF-8 */
static int kind(const char *s, int n) {
    int i, dot = -1, at = n;
    if (n == 0 || n >= __AXC_LOCALE_NAME)
        return -1;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c <= ' ' || c >= 0x7f || c == ';' || c == '=')
            return -1;
        if (c == '@' && at == n)
            at = i;
        if (c == '.' && at == n)
            dot = i;
    }
    if ((n == 1 && s[0] == 'C') ||
        (n == 5 && s[0] == 'P' && s[1] == 'O' && s[2] == 'S' && s[3] == 'I' && s[4] == 'X'))
        return 0;
    if (dot >= 0 && (same_fold(s + dot + 1, at - dot - 1, "utf-8") || same_fold(s + dot + 1, at - dot - 1, "utf8")))
        return 1;
    return -1;
}

static int length(const char *s) {
    int n = 0;
    while (s[n] && n < __AXC_LOCALE_NAME)
        n++;
    return n;
}

/* the environment's name for category c (1 to 5) */
static const char *from_environment(int c) {
    const char *v = getenv("LC_ALL");
    if (v && *v)
        return v;
    v = getenv(category_names[c]);
    if (v && *v)
        return v;
    v = getenv("LANG");
    if (v && *v)
        return v;
    return "C";
}

static void copy(char *d, const char *s, int n) {
    int i;
    for (i = 0; i < n; i++)
        d[i] = s[i];
    d[n] = 0;
}

static char *query(struct __axc_locale *L, int category) {
    int c, k = 0;
    if (category != LC_ALL)
        return L->names[category];
    for (c = 2; c <= 5; c++)
        if (!same(L->names[c], L->names[1]))
            break;
    if (c > 5)
        return L->names[1];
    for (c = 1; c <= 5; c++) {
        const char *p;
        if (c > 1)
            L->all[k++] = ';';
        for (p = category_names[c]; *p; p++)
            L->all[k++] = *p;
        L->all[k++] = '=';
        for (p = L->names[c]; *p; p++)
            L->all[k++] = *p;
    }
    L->all[k] = 0;
    return L->all;
}

/* the names of a composite name ("LC_COLLATE=a;..."): each category's
   in start[c], length len[c]; 0 if it is not one */
static int composite(const char *s, const char *start[6], int len[6]) {
    int c, seen = 0;
    for (;;) {
        const char *e = s;
        int n;
        while (*e && *e != '=' && *e != ';')
            e++;
        if (*e != '=')
            return 0;
        for (c = 1; c <= 5; c++) {
            const char *name = category_names[c];
            int i = 0;
            while (name[i] && s + i < e && name[i] == s[i])
                i++;
            if (name[i] == 0 && s + i == e)
                break;
        }
        if (c > 5 || (seen & (1 << c)))
            return 0;
        seen |= 1 << c;
        s = e + 1;
        e = s;
        while (*e && *e != ';')
            e++;
        n = (int)(e - s);
        if (kind(s, n) < 0)
            return 0;
        start[c] = s;
        len[c] = n;
        if (*e == 0)
            break;
        s = e + 1;
    }
    return seen == 0x3e;
}

char *setlocale(int category, const char *locale) {
    struct __axc_locale *L = __axc_locale();
    const char *start[6];
    int len[6];
    int c, first, last;
    if (category < LC_ALL || category > LC_TIME)
        return 0;
    if (!locale)
        return query(L, category);
    first = category == LC_ALL ? 1 : category;
    last = category == LC_ALL ? 5 : category;
    /* (every category's name is checked before any is changed) */
    if (category == LC_ALL && composite(locale, start, len)) {
    } else {
        for (c = first; c <= last; c++) {
            start[c] = *locale ? locale : from_environment(c);
            len[c] = length(start[c]);
            if (kind(start[c], len[c]) < 0)
                return 0;
        }
    }
    for (c = first; c <= last; c++)
        copy(L->names[c], start[c], len[c]);
    L->utf8 = kind(L->names[LC_CTYPE], length(L->names[LC_CTYPE]));
    return query(L, category);
}
