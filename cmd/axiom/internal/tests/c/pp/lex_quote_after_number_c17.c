/* C17: a pp-number followed by a character constant stays two tokens. */
/* flags: --c-std=c17 */
#define S(x) #x
S(1'2')
