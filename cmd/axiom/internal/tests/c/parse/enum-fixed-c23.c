/* flags: --c-std=c23 */
/* Enumerations with a fixed underlying type (C23); a bit-field of an
   enumeration type is not one. */
enum E : unsigned char { A, B = 200 };
enum F : const long;
enum G : long long { C };
enum H { X };
struct s { enum H h : 3; };
