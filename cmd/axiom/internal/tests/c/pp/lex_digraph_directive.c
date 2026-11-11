/* %: starts a directive, and %: and %:%: are the # and ## operators. */
/* flags: --c-std=c95 */
%:define str(x) %:x
%:define cat(a, b) a %:%: b
str(hi) cat(x, y)
a<:1:> = <%2%>;
