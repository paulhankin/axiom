/* # is no operator in an object-like macro; ## is. */
#define H # x
#define HH a ## b
#define s(x) #x
#define xs(x) s(x)
xs(H) HH
