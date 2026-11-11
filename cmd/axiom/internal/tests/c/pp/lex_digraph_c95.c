/* Digraphs are punctuators from C95 on (spelt as written). */
/* flags: --c-std=c95 */
#define s(x) #x
#define xs(x) s(x)
xs(<: :> <% %> %: %:%: %:% <::>)
