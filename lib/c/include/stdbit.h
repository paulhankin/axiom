/* stdbit.h - bit and byte utilities (C23 7.18), for the Axiom compiler's C
   front end. The functions are inline functions of this header over the
   compiler's builtins, and the type-generic macros the builtins themselves
   (each takes any unsigned integer type but bool, and counts in its width). */
#ifndef __AXIOM_STDBIT_H
#define __AXIOM_STDBIT_H

#if __STDC_VERSION__ < 202311L
#error "<stdbit.h> is a header of C23"
#endif

#define __STDC_VERSION_STDBIT_H__ 202311L

/* (the targets are little-endian) */
#define __STDC_ENDIAN_LITTLE__ 1234
#define __STDC_ENDIAN_BIG__ 4321
#define __STDC_ENDIAN_NATIVE__ __STDC_ENDIAN_LITTLE__


/* stdc_leading_zeros */
static inline unsigned int stdc_leading_zeros_uc(unsigned char __v) { return __builtin_stdc_leading_zeros(__v); }
static inline unsigned int stdc_leading_zeros_us(unsigned short __v) { return __builtin_stdc_leading_zeros(__v); }
static inline unsigned int stdc_leading_zeros_ui(unsigned int __v) { return __builtin_stdc_leading_zeros(__v); }
static inline unsigned int stdc_leading_zeros_ul(unsigned long __v) { return __builtin_stdc_leading_zeros(__v); }
static inline unsigned int stdc_leading_zeros_ull(unsigned long long __v) { return __builtin_stdc_leading_zeros(__v); }
#define stdc_leading_zeros(value) __builtin_stdc_leading_zeros(value)

/* stdc_leading_ones */
static inline unsigned int stdc_leading_ones_uc(unsigned char __v) { return __builtin_stdc_leading_ones(__v); }
static inline unsigned int stdc_leading_ones_us(unsigned short __v) { return __builtin_stdc_leading_ones(__v); }
static inline unsigned int stdc_leading_ones_ui(unsigned int __v) { return __builtin_stdc_leading_ones(__v); }
static inline unsigned int stdc_leading_ones_ul(unsigned long __v) { return __builtin_stdc_leading_ones(__v); }
static inline unsigned int stdc_leading_ones_ull(unsigned long long __v) { return __builtin_stdc_leading_ones(__v); }
#define stdc_leading_ones(value) __builtin_stdc_leading_ones(value)

/* stdc_trailing_zeros */
static inline unsigned int stdc_trailing_zeros_uc(unsigned char __v) { return __builtin_stdc_trailing_zeros(__v); }
static inline unsigned int stdc_trailing_zeros_us(unsigned short __v) { return __builtin_stdc_trailing_zeros(__v); }
static inline unsigned int stdc_trailing_zeros_ui(unsigned int __v) { return __builtin_stdc_trailing_zeros(__v); }
static inline unsigned int stdc_trailing_zeros_ul(unsigned long __v) { return __builtin_stdc_trailing_zeros(__v); }
static inline unsigned int stdc_trailing_zeros_ull(unsigned long long __v) { return __builtin_stdc_trailing_zeros(__v); }
#define stdc_trailing_zeros(value) __builtin_stdc_trailing_zeros(value)

/* stdc_trailing_ones */
static inline unsigned int stdc_trailing_ones_uc(unsigned char __v) { return __builtin_stdc_trailing_ones(__v); }
static inline unsigned int stdc_trailing_ones_us(unsigned short __v) { return __builtin_stdc_trailing_ones(__v); }
static inline unsigned int stdc_trailing_ones_ui(unsigned int __v) { return __builtin_stdc_trailing_ones(__v); }
static inline unsigned int stdc_trailing_ones_ul(unsigned long __v) { return __builtin_stdc_trailing_ones(__v); }
static inline unsigned int stdc_trailing_ones_ull(unsigned long long __v) { return __builtin_stdc_trailing_ones(__v); }
#define stdc_trailing_ones(value) __builtin_stdc_trailing_ones(value)

/* stdc_first_leading_zero */
static inline unsigned int stdc_first_leading_zero_uc(unsigned char __v) { return __builtin_stdc_first_leading_zero(__v); }
static inline unsigned int stdc_first_leading_zero_us(unsigned short __v) { return __builtin_stdc_first_leading_zero(__v); }
static inline unsigned int stdc_first_leading_zero_ui(unsigned int __v) { return __builtin_stdc_first_leading_zero(__v); }
static inline unsigned int stdc_first_leading_zero_ul(unsigned long __v) { return __builtin_stdc_first_leading_zero(__v); }
static inline unsigned int stdc_first_leading_zero_ull(unsigned long long __v) { return __builtin_stdc_first_leading_zero(__v); }
#define stdc_first_leading_zero(value) __builtin_stdc_first_leading_zero(value)

/* stdc_first_leading_one */
static inline unsigned int stdc_first_leading_one_uc(unsigned char __v) { return __builtin_stdc_first_leading_one(__v); }
static inline unsigned int stdc_first_leading_one_us(unsigned short __v) { return __builtin_stdc_first_leading_one(__v); }
static inline unsigned int stdc_first_leading_one_ui(unsigned int __v) { return __builtin_stdc_first_leading_one(__v); }
static inline unsigned int stdc_first_leading_one_ul(unsigned long __v) { return __builtin_stdc_first_leading_one(__v); }
static inline unsigned int stdc_first_leading_one_ull(unsigned long long __v) { return __builtin_stdc_first_leading_one(__v); }
#define stdc_first_leading_one(value) __builtin_stdc_first_leading_one(value)

/* stdc_first_trailing_zero */
static inline unsigned int stdc_first_trailing_zero_uc(unsigned char __v) { return __builtin_stdc_first_trailing_zero(__v); }
static inline unsigned int stdc_first_trailing_zero_us(unsigned short __v) { return __builtin_stdc_first_trailing_zero(__v); }
static inline unsigned int stdc_first_trailing_zero_ui(unsigned int __v) { return __builtin_stdc_first_trailing_zero(__v); }
static inline unsigned int stdc_first_trailing_zero_ul(unsigned long __v) { return __builtin_stdc_first_trailing_zero(__v); }
static inline unsigned int stdc_first_trailing_zero_ull(unsigned long long __v) { return __builtin_stdc_first_trailing_zero(__v); }
#define stdc_first_trailing_zero(value) __builtin_stdc_first_trailing_zero(value)

/* stdc_first_trailing_one */
static inline unsigned int stdc_first_trailing_one_uc(unsigned char __v) { return __builtin_stdc_first_trailing_one(__v); }
static inline unsigned int stdc_first_trailing_one_us(unsigned short __v) { return __builtin_stdc_first_trailing_one(__v); }
static inline unsigned int stdc_first_trailing_one_ui(unsigned int __v) { return __builtin_stdc_first_trailing_one(__v); }
static inline unsigned int stdc_first_trailing_one_ul(unsigned long __v) { return __builtin_stdc_first_trailing_one(__v); }
static inline unsigned int stdc_first_trailing_one_ull(unsigned long long __v) { return __builtin_stdc_first_trailing_one(__v); }
#define stdc_first_trailing_one(value) __builtin_stdc_first_trailing_one(value)

/* stdc_count_zeros */
static inline unsigned int stdc_count_zeros_uc(unsigned char __v) { return __builtin_stdc_count_zeros(__v); }
static inline unsigned int stdc_count_zeros_us(unsigned short __v) { return __builtin_stdc_count_zeros(__v); }
static inline unsigned int stdc_count_zeros_ui(unsigned int __v) { return __builtin_stdc_count_zeros(__v); }
static inline unsigned int stdc_count_zeros_ul(unsigned long __v) { return __builtin_stdc_count_zeros(__v); }
static inline unsigned int stdc_count_zeros_ull(unsigned long long __v) { return __builtin_stdc_count_zeros(__v); }
#define stdc_count_zeros(value) __builtin_stdc_count_zeros(value)

/* stdc_count_ones */
static inline unsigned int stdc_count_ones_uc(unsigned char __v) { return __builtin_stdc_count_ones(__v); }
static inline unsigned int stdc_count_ones_us(unsigned short __v) { return __builtin_stdc_count_ones(__v); }
static inline unsigned int stdc_count_ones_ui(unsigned int __v) { return __builtin_stdc_count_ones(__v); }
static inline unsigned int stdc_count_ones_ul(unsigned long __v) { return __builtin_stdc_count_ones(__v); }
static inline unsigned int stdc_count_ones_ull(unsigned long long __v) { return __builtin_stdc_count_ones(__v); }
#define stdc_count_ones(value) __builtin_stdc_count_ones(value)

/* stdc_has_single_bit */
static inline bool stdc_has_single_bit_uc(unsigned char __v) { return __builtin_stdc_has_single_bit(__v); }
static inline bool stdc_has_single_bit_us(unsigned short __v) { return __builtin_stdc_has_single_bit(__v); }
static inline bool stdc_has_single_bit_ui(unsigned int __v) { return __builtin_stdc_has_single_bit(__v); }
static inline bool stdc_has_single_bit_ul(unsigned long __v) { return __builtin_stdc_has_single_bit(__v); }
static inline bool stdc_has_single_bit_ull(unsigned long long __v) { return __builtin_stdc_has_single_bit(__v); }
#define stdc_has_single_bit(value) __builtin_stdc_has_single_bit(value)

/* stdc_bit_width */
static inline unsigned int stdc_bit_width_uc(unsigned char __v) { return __builtin_stdc_bit_width(__v); }
static inline unsigned int stdc_bit_width_us(unsigned short __v) { return __builtin_stdc_bit_width(__v); }
static inline unsigned int stdc_bit_width_ui(unsigned int __v) { return __builtin_stdc_bit_width(__v); }
static inline unsigned int stdc_bit_width_ul(unsigned long __v) { return __builtin_stdc_bit_width(__v); }
static inline unsigned int stdc_bit_width_ull(unsigned long long __v) { return __builtin_stdc_bit_width(__v); }
#define stdc_bit_width(value) __builtin_stdc_bit_width(value)

/* stdc_bit_floor */
static inline unsigned char stdc_bit_floor_uc(unsigned char __v) { return __builtin_stdc_bit_floor(__v); }
static inline unsigned short stdc_bit_floor_us(unsigned short __v) { return __builtin_stdc_bit_floor(__v); }
static inline unsigned int stdc_bit_floor_ui(unsigned int __v) { return __builtin_stdc_bit_floor(__v); }
static inline unsigned long stdc_bit_floor_ul(unsigned long __v) { return __builtin_stdc_bit_floor(__v); }
static inline unsigned long long stdc_bit_floor_ull(unsigned long long __v) { return __builtin_stdc_bit_floor(__v); }
#define stdc_bit_floor(value) __builtin_stdc_bit_floor(value)

/* stdc_bit_ceil */
static inline unsigned char stdc_bit_ceil_uc(unsigned char __v) { return __builtin_stdc_bit_ceil(__v); }
static inline unsigned short stdc_bit_ceil_us(unsigned short __v) { return __builtin_stdc_bit_ceil(__v); }
static inline unsigned int stdc_bit_ceil_ui(unsigned int __v) { return __builtin_stdc_bit_ceil(__v); }
static inline unsigned long stdc_bit_ceil_ul(unsigned long __v) { return __builtin_stdc_bit_ceil(__v); }
static inline unsigned long long stdc_bit_ceil_ull(unsigned long long __v) { return __builtin_stdc_bit_ceil(__v); }
#define stdc_bit_ceil(value) __builtin_stdc_bit_ceil(value)

#endif
