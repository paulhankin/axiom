/* Digraphs are punctuators from C95 on (spelt as written). */
/* flags: --c-std=c89 */
#define s(x) #x
#define xs(x) s(x)
xs(<: :> <% %> %: %:%: %:% <::>)
