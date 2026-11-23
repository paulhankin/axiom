# An object of the library, of LIB_2.0 (an assembly source's .symver)
        .data
        .globl value_v2
        .type value_v2, @object
        .size value_v2, 4
value_v2:
        .long 7
        .symver value_v2, value@@LIB_2.0
