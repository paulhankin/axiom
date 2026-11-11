/* C23: true and false in #if, #elifdef and #elifndef, __has_include, __has_c_attribute. */
/* flags: --c-std=c23 */
#if true && !false
ok1
#endif
#define D
#ifdef U
#elifdef D
ok2
#endif
#ifdef U
#elifndef U2
ok3
#endif
#if 0
#elifdef D
ok4
#else
#endif
#if __has_include(<stddef.h>) && __has_include("inc/vers2.h") && !__has_include("nope.h")
ok5
#endif
#define HDR <limits.h>
#if __has_include(HDR)
ok6
#endif
#if __has_c_attribute(nodiscard) == 202003L && __has_c_attribute(__deprecated__) == 201904L && __has_c_attribute(noreturn) == 202202L
ok7
#endif
#if !__has_c_attribute(vendor::attr) && !__has_c_attribute(unknown) && __has_c_attribute(_Noreturn) && __has_c_attribute(maybe_unused) == 202106L
ok8
#endif
#if __has_c_attribute(fallthrough) == 201904L && __has_c_attribute(unsequenced) == 202207L && __has_c_attribute(reproducible) == 202207L
ok9
#endif
#if defined(__has_include) && defined __has_embed && defined(__has_c_attribute)
ok10
#endif
#ifdef __has_include
ok11
#endif
