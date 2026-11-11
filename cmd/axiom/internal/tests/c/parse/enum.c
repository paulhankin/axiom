/* flags: --c-std=c99 */
/* Enumerations, with values and a trailing comma (C99). */
enum color { RED, GREEN = 4, BLUE = GREEN + 1, };
enum color c = BLUE;
enum { ONE = 1 } one;
