string "" "abc" "a" "b" "\x12" "3" "é" "\u00e9" "\U0001F600" u8"é" u8"😀" prefix=2 units=97,98,99,97,98,18,51,195,169,195,169,240,159,152,128,195,169,240,159,152,128
punct ; id=33
string u"é😀" prefix=3 units=233,55357,56832
punct ; id=33
string U"é😀" prefix=4 units=233,128512
punct ; id=33
string L"é😀" prefix=1 units=233,128512
punct ; id=33
string L"a" "b" prefix=1 units=97,98
punct ; id=33
string "a" L"b" prefix=1 units=97,98
punct ; id=33
string u8"a" "b" prefix=2 units=97,98
punct ; id=33
string "a" u"b" "c" prefix=3 units=97,98,99
punct ; id=33
string U"\xffffffff" prefix=4 units=4294967295
punct ; id=33
string "\0\1\12\123\1234" prefix=0 units=0,1,10,83,83,52
punct ; id=33
string "ab" prefix=0 units=97,98
eof 
