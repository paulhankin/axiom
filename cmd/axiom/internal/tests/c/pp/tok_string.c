/* String literals: the code units of each prefix; adjacent ones joined. */
/* flags: --emit=c-tokens --c-std=c23 */
"" "abc" "a" "b" "\x12" "3" "é" "\u00e9" "\U0001F600" u8"é" u8"😀"
;
u"é😀" ; U"é😀" ; L"é😀" ; L"a" "b" ; "a" L"b" ; u8"a" "b" ; "a" u"b" "c" ; U"\xffffffff"
; "\0\1\12\123\1234" ; "a\
b"
