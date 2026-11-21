/* open with O_CREAT and no mode: the headers' check under _FORTIFY_SOURCE
   (a call left after inlining of a function defined nowhere) fails the build */
#include <fcntl.h>
int main(void) { return open("x.txt", O_WRONLY | O_CREAT) < 0; }
