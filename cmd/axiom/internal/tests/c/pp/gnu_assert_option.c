/* flags: --c-std=gnu17 -Amode=fast -Acolor(red) -Acolor=blue -A-color=red -A size(big) -A- -Aafter=yes */
/* Assertions from the command line (a GNU dialect): -Apred=answer and
   -Apred(answer) assert, -A-pred=answer takes one answer back, -A- every
   assertion before it, the target's included. */
#if !#mode && !#color && !#size && !#cpu && !#machine
one
#endif
#if #after(yes)
two
#endif
