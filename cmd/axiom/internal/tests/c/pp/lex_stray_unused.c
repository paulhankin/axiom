/* A stray character is a token; it is an error only if it is left in the text. */
#define s(x) #x
#define drop(x)
s(@ $ `) drop(@ $ ` \)
