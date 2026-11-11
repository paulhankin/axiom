/* Character constants: plain ones are int, a single char's value as char (signed here). */
/* flags: --emit=c-tokens --c-std=c23 --c-char=signed */
/* expect-warning: C4102 */
'a' '\0' '\n' '\a' '\b' '\f' '\r' '\t' '\v' '\\' '\'' '\"' '\?' '"' '\177' '\200' '\377' '\x7f' '\xff' '\x0ff'
'ab' 'abcd' L'a' L'\xffffffff' u'a' u'\xffff' U'\U0010FFFF' u8'a' u8'\xff' L'é' u'é' U'😀'
