/* flags: --c-std=c99 */
/* expect-error: C1021 */
/* C99 6.4.2.1p3: a universal character name for a digit (of Annex D) cannot
   begin an identifier; inside one it can. */
int x\u0660;
int \u0660x;
