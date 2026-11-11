/* Universal character names and UTF-8 in identifiers (C99 on; C23 by XID_Start and XID_Continue). */
/* flags: --c-std=c23 */
#define s(x) #x
#define caf\u00e9 1
café caf\u00e9 caf\U000000E9 αβ s(caf\u00e9) x́
