#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/wasmtab.ax, the WebAssembly assembler's table
of the instructions that take a fixed signature (the numeric, memory,
vector and atomic instructions), from the lists below, written from the
WebAssembly specification's binary-format opcode tables (Core
Specification 2.0 section 5.4, and the threads, relaxed SIMD, tail-call and
exception-handling extensions):

    python3 cmd/internal/tools/gen_wasm_table.py > cmd/axiom/src/asm/wasmtab.ax && axiom fmt cmd/axiom/src/asm/wasmtab.ax
    python3 cmd/internal/tools/gen_wasm_table.py --test > cmd/axiom/internal/tests/c/run/asm_wasm_table.c

(--test: the test that uses every instruction of the table in an asm
statement, whose module the engine validates when it loads it.)

A line reads `name code imm sig feature`:
- code: the opcode in hex; with a prefix byte (fc, fd, fe) first, the rest
  is the LEB128 number after it (`fd10c`: 0xfd then 268);
- imm: the immediate: `-` none, `mN` a memarg of natural alignment N bytes,
  `LN` a memarg and a lane index (N-byte lanes), `lN` a lane index of N
  lanes, `c` a constant of the result's type, `V` v128.const, `S` the 16
  lane indices of i8x16.shuffle, `z` one zero byte (memory.size, memory.grow,
  memory.fill, atomic.fence), `Z` two (memory.copy);
- sig: the operand types, `:`, the result types: i i32, I i64, f f32, F f64,
  v v128, p an address (i32, or i64 on memory64);
- feature: `-` the core instruction set, s simd128, r relaxed-simd, b
  bulk-memory, n nontrapping-fptoint, x sign-ext, a atomics.
The control, variable and parametric instructions (block, br, call,
local.get, select, ...) are the assembler's own (asm/wasm.ax).
"""

ROWS = []


def row(name, code, imm, sig, feat):
    ROWS.append('%s %s %s %s %s' % (name, code, imm, sig, feat))


def h(n):
    return '%x' % n


# memory (5.4.6)
for name, code, size, sig in [
        ('i32.load', 0x28, 4, 'p:i'), ('i64.load', 0x29, 8, 'p:I'), ('f32.load', 0x2a, 4, 'p:f'),
        ('f64.load', 0x2b, 8, 'p:F'), ('i32.load8_s', 0x2c, 1, 'p:i'), ('i32.load8_u', 0x2d, 1, 'p:i'),
        ('i32.load16_s', 0x2e, 2, 'p:i'), ('i32.load16_u', 0x2f, 2, 'p:i'), ('i64.load8_s', 0x30, 1, 'p:I'),
        ('i64.load8_u', 0x31, 1, 'p:I'), ('i64.load16_s', 0x32, 2, 'p:I'), ('i64.load16_u', 0x33, 2, 'p:I'),
        ('i64.load32_s', 0x34, 4, 'p:I'), ('i64.load32_u', 0x35, 4, 'p:I'), ('i32.store', 0x36, 4, 'pi:'),
        ('i64.store', 0x37, 8, 'pI:'), ('f32.store', 0x38, 4, 'pf:'), ('f64.store', 0x39, 8, 'pF:'),
        ('i32.store8', 0x3a, 1, 'pi:'), ('i32.store16', 0x3b, 2, 'pi:'), ('i64.store8', 0x3c, 1, 'pI:'),
        ('i64.store16', 0x3d, 2, 'pI:'), ('i64.store32', 0x3e, 4, 'pI:')]:
    row(name, h(code), 'm%d' % size, sig, '-')
row('memory.size', '3f', 'z', ':p', '-')
row('memory.grow', '40', 'z', 'p:p', '-')

# constants (5.4.7)
row('i32.const', '41', 'c', ':i', '-')
row('i64.const', '42', 'c', ':I', '-')
row('f32.const', '43', 'c', ':f', '-')
row('f64.const', '44', 'c', ':F', '-')

# comparisons
CMP_I = ['eq', 'ne', 'lt_s', 'lt_u', 'gt_s', 'gt_u', 'le_s', 'le_u', 'ge_s', 'ge_u']
CMP_F = ['eq', 'ne', 'lt', 'gt', 'le', 'ge']
row('i32.eqz', '45', '-', 'i:i', '-')
for k, n in enumerate(CMP_I):
    row('i32.' + n, h(0x46 + k), '-', 'ii:i', '-')
row('i64.eqz', '50', '-', 'I:i', '-')
for k, n in enumerate(CMP_I):
    row('i64.' + n, h(0x51 + k), '-', 'II:i', '-')
for k, n in enumerate(CMP_F):
    row('f32.' + n, h(0x5b + k), '-', 'ff:i', '-')
for k, n in enumerate(CMP_F):
    row('f64.' + n, h(0x61 + k), '-', 'FF:i', '-')

# arithmetic
UN_I = ['clz', 'ctz', 'popcnt']
BIN_I = ['add', 'sub', 'mul', 'div_s', 'div_u', 'rem_s', 'rem_u', 'and', 'or', 'xor', 'shl', 'shr_s', 'shr_u',
         'rotl', 'rotr']
UN_F = ['abs', 'neg', 'ceil', 'floor', 'trunc', 'nearest', 'sqrt']
BIN_F = ['add', 'sub', 'mul', 'div', 'min', 'max', 'copysign']
for t, s, base in [('i32', 'i', 0x67), ('i64', 'I', 0x79)]:
    for k, n in enumerate(UN_I):
        row('%s.%s' % (t, n), h(base + k), '-', '%s:%s' % (s, s), '-')
    for k, n in enumerate(BIN_I):
        row('%s.%s' % (t, n), h(base + 3 + k), '-', '%s%s:%s' % (s, s, s), '-')
for t, s, base in [('f32', 'f', 0x8b), ('f64', 'F', 0x99)]:
    for k, n in enumerate(UN_F):
        row('%s.%s' % (t, n), h(base + k), '-', '%s:%s' % (s, s), '-')
    for k, n in enumerate(BIN_F):
        row('%s.%s' % (t, n), h(base + 7 + k), '-', '%s%s:%s' % (s, s, s), '-')

# conversions
for name, code, sig in [
        ('i32.wrap_i64', 0xa7, 'I:i'), ('i32.trunc_f32_s', 0xa8, 'f:i'), ('i32.trunc_f32_u', 0xa9, 'f:i'),
        ('i32.trunc_f64_s', 0xaa, 'F:i'), ('i32.trunc_f64_u', 0xab, 'F:i'), ('i64.extend_i32_s', 0xac, 'i:I'),
        ('i64.extend_i32_u', 0xad, 'i:I'), ('i64.trunc_f32_s', 0xae, 'f:I'), ('i64.trunc_f32_u', 0xaf, 'f:I'),
        ('i64.trunc_f64_s', 0xb0, 'F:I'), ('i64.trunc_f64_u', 0xb1, 'F:I'), ('f32.convert_i32_s', 0xb2, 'i:f'),
        ('f32.convert_i32_u', 0xb3, 'i:f'), ('f32.convert_i64_s', 0xb4, 'I:f'), ('f32.convert_i64_u', 0xb5, 'I:f'),
        ('f32.demote_f64', 0xb6, 'F:f'), ('f64.convert_i32_s', 0xb7, 'i:F'), ('f64.convert_i32_u', 0xb8, 'i:F'),
        ('f64.convert_i64_s', 0xb9, 'I:F'), ('f64.convert_i64_u', 0xba, 'I:F'), ('f64.promote_f32', 0xbb, 'f:F'),
        ('i32.reinterpret_f32', 0xbc, 'f:i'), ('i64.reinterpret_f64', 0xbd, 'F:I'),
        ('f32.reinterpret_i32', 0xbe, 'i:f'), ('f64.reinterpret_i64', 0xbf, 'I:F')]:
    row(name, h(code), '-', sig, '-')
# sign extension
for name, code, sig in [('i32.extend8_s', 0xc0, 'i:i'), ('i32.extend16_s', 0xc1, 'i:i'),
                        ('i64.extend8_s', 0xc2, 'I:I'), ('i64.extend16_s', 0xc3, 'I:I'),
                        ('i64.extend32_s', 0xc4, 'I:I')]:
    row(name, h(code), '-', sig, 'x')
# saturating truncation
for k, (name, sig) in enumerate([
        ('i32.trunc_sat_f32_s', 'f:i'), ('i32.trunc_sat_f32_u', 'f:i'), ('i32.trunc_sat_f64_s', 'F:i'),
        ('i32.trunc_sat_f64_u', 'F:i'), ('i64.trunc_sat_f32_s', 'f:I'), ('i64.trunc_sat_f32_u', 'f:I'),
        ('i64.trunc_sat_f64_s', 'F:I'), ('i64.trunc_sat_f64_u', 'F:I')]):
    row(name, 'fc' + h(k), '-', sig, 'n')
# bulk memory (memory.init and data.drop name data segments, which C has not)
row('memory.copy', 'fca', 'Z', 'ppp:', 'b')
row('memory.fill', 'fcb', 'z', 'pip:', 'b')

# vector instructions (5.4.8): 0xfd and the number

def v(name, code, imm, sig, feat='s'):
    row(name, 'fd' + h(code), imm, sig, feat)


for name, code, size, sig in [
        ('v128.load', 0, 16, 'p:v'), ('v128.load8x8_s', 1, 8, 'p:v'), ('v128.load8x8_u', 2, 8, 'p:v'),
        ('v128.load16x4_s', 3, 8, 'p:v'), ('v128.load16x4_u', 4, 8, 'p:v'), ('v128.load32x2_s', 5, 8, 'p:v'),
        ('v128.load32x2_u', 6, 8, 'p:v'), ('v128.load8_splat', 7, 1, 'p:v'), ('v128.load16_splat', 8, 2, 'p:v'),
        ('v128.load32_splat', 9, 4, 'p:v'), ('v128.load64_splat', 10, 8, 'p:v'), ('v128.store', 11, 16, 'pv:'),
        ('v128.load32_zero', 92, 4, 'p:v'), ('v128.load64_zero', 93, 8, 'p:v')]:
    v(name, code, 'm%d' % size, sig)
v('v128.const', 12, 'V', ':v')
v('i8x16.shuffle', 13, 'S', 'vv:v')
v('i8x16.swizzle', 14, '-', 'vv:v')
for k, (name, s) in enumerate([('i8x16', 'i'), ('i16x8', 'i'), ('i32x4', 'i'), ('i64x2', 'I'), ('f32x4', 'f'),
                               ('f64x2', 'F')]):
    v(name + '.splat', 15 + k, '-', '%s:v' % s)
for name, code, lanes, sig in [
        ('i8x16.extract_lane_s', 21, 16, 'v:i'), ('i8x16.extract_lane_u', 22, 16, 'v:i'),
        ('i8x16.replace_lane', 23, 16, 'vi:v'), ('i16x8.extract_lane_s', 24, 8, 'v:i'),
        ('i16x8.extract_lane_u', 25, 8, 'v:i'), ('i16x8.replace_lane', 26, 8, 'vi:v'),
        ('i32x4.extract_lane', 27, 4, 'v:i'), ('i32x4.replace_lane', 28, 4, 'vi:v'),
        ('i64x2.extract_lane', 29, 2, 'v:I'), ('i64x2.replace_lane', 30, 2, 'vI:v'),
        ('f32x4.extract_lane', 31, 4, 'v:f'), ('f32x4.replace_lane', 32, 4, 'vf:v'),
        ('f64x2.extract_lane', 33, 2, 'v:F'), ('f64x2.replace_lane', 34, 2, 'vF:v')]:
    v(name, code, 'l%d' % lanes, sig)
for k, n in enumerate(CMP_I):
    v('i8x16.' + n, 35 + k, '-', 'vv:v')
for k, n in enumerate(CMP_I):
    v('i16x8.' + n, 45 + k, '-', 'vv:v')
for k, n in enumerate(CMP_I):
    v('i32x4.' + n, 55 + k, '-', 'vv:v')
for k, n in enumerate(CMP_F):
    v('f32x4.' + n, 65 + k, '-', 'vv:v')
for k, n in enumerate(CMP_F):
    v('f64x2.' + n, 71 + k, '-', 'vv:v')
v('v128.not', 77, '-', 'v:v')
v('v128.and', 78, '-', 'vv:v')
v('v128.andnot', 79, '-', 'vv:v')
v('v128.or', 80, '-', 'vv:v')
v('v128.xor', 81, '-', 'vv:v')
v('v128.bitselect', 82, '-', 'vvv:v')
v('v128.any_true', 83, '-', 'v:i')
for k, size in enumerate([1, 2, 4, 8]):
    v('v128.load%d_lane' % (size * 8), 84 + k, 'L%d' % size, 'pv:v')
for k, size in enumerate([1, 2, 4, 8]):
    v('v128.store%d_lane' % (size * 8), 88 + k, 'L%d' % size, 'pv:')
VEC = [
    (94, 'f32x4.demote_f64x2_zero', 'v:v'), (95, 'f64x2.promote_low_f32x4', 'v:v'),
    (96, 'i8x16.abs', 'v:v'), (97, 'i8x16.neg', 'v:v'), (98, 'i8x16.popcnt', 'v:v'),
    (99, 'i8x16.all_true', 'v:i'), (100, 'i8x16.bitmask', 'v:i'), (101, 'i8x16.narrow_i16x8_s', 'vv:v'),
    (102, 'i8x16.narrow_i16x8_u', 'vv:v'), (103, 'f32x4.ceil', 'v:v'), (104, 'f32x4.floor', 'v:v'),
    (105, 'f32x4.trunc', 'v:v'), (106, 'f32x4.nearest', 'v:v'), (107, 'i8x16.shl', 'vi:v'),
    (108, 'i8x16.shr_s', 'vi:v'), (109, 'i8x16.shr_u', 'vi:v'), (110, 'i8x16.add', 'vv:v'),
    (111, 'i8x16.add_sat_s', 'vv:v'), (112, 'i8x16.add_sat_u', 'vv:v'), (113, 'i8x16.sub', 'vv:v'),
    (114, 'i8x16.sub_sat_s', 'vv:v'), (115, 'i8x16.sub_sat_u', 'vv:v'), (116, 'f64x2.ceil', 'v:v'),
    (117, 'f64x2.floor', 'v:v'), (118, 'i8x16.min_s', 'vv:v'), (119, 'i8x16.min_u', 'vv:v'),
    (120, 'i8x16.max_s', 'vv:v'), (121, 'i8x16.max_u', 'vv:v'), (122, 'f64x2.trunc', 'v:v'),
    (123, 'i8x16.avgr_u', 'vv:v'), (124, 'i16x8.extadd_pairwise_i8x16_s', 'v:v'),
    (125, 'i16x8.extadd_pairwise_i8x16_u', 'v:v'), (126, 'i32x4.extadd_pairwise_i16x8_s', 'v:v'),
    (127, 'i32x4.extadd_pairwise_i16x8_u', 'v:v'), (128, 'i16x8.abs', 'v:v'), (129, 'i16x8.neg', 'v:v'),
    (130, 'i16x8.q15mulr_sat_s', 'vv:v'), (131, 'i16x8.all_true', 'v:i'), (132, 'i16x8.bitmask', 'v:i'),
    (133, 'i16x8.narrow_i32x4_s', 'vv:v'), (134, 'i16x8.narrow_i32x4_u', 'vv:v'),
    (135, 'i16x8.extend_low_i8x16_s', 'v:v'), (136, 'i16x8.extend_high_i8x16_s', 'v:v'),
    (137, 'i16x8.extend_low_i8x16_u', 'v:v'), (138, 'i16x8.extend_high_i8x16_u', 'v:v'),
    (139, 'i16x8.shl', 'vi:v'), (140, 'i16x8.shr_s', 'vi:v'), (141, 'i16x8.shr_u', 'vi:v'),
    (142, 'i16x8.add', 'vv:v'), (143, 'i16x8.add_sat_s', 'vv:v'), (144, 'i16x8.add_sat_u', 'vv:v'),
    (145, 'i16x8.sub', 'vv:v'), (146, 'i16x8.sub_sat_s', 'vv:v'), (147, 'i16x8.sub_sat_u', 'vv:v'),
    (148, 'f64x2.nearest', 'v:v'), (149, 'i16x8.mul', 'vv:v'), (150, 'i16x8.min_s', 'vv:v'),
    (151, 'i16x8.min_u', 'vv:v'), (152, 'i16x8.max_s', 'vv:v'), (153, 'i16x8.max_u', 'vv:v'),
    (155, 'i16x8.avgr_u', 'vv:v'), (156, 'i16x8.extmul_low_i8x16_s', 'vv:v'),
    (157, 'i16x8.extmul_high_i8x16_s', 'vv:v'), (158, 'i16x8.extmul_low_i8x16_u', 'vv:v'),
    (159, 'i16x8.extmul_high_i8x16_u', 'vv:v'), (160, 'i32x4.abs', 'v:v'), (161, 'i32x4.neg', 'v:v'),
    (163, 'i32x4.all_true', 'v:i'), (164, 'i32x4.bitmask', 'v:i'), (167, 'i32x4.extend_low_i16x8_s', 'v:v'),
    (168, 'i32x4.extend_high_i16x8_s', 'v:v'), (169, 'i32x4.extend_low_i16x8_u', 'v:v'),
    (170, 'i32x4.extend_high_i16x8_u', 'v:v'), (171, 'i32x4.shl', 'vi:v'), (172, 'i32x4.shr_s', 'vi:v'),
    (173, 'i32x4.shr_u', 'vi:v'), (174, 'i32x4.add', 'vv:v'), (177, 'i32x4.sub', 'vv:v'),
    (181, 'i32x4.mul', 'vv:v'), (182, 'i32x4.min_s', 'vv:v'), (183, 'i32x4.min_u', 'vv:v'),
    (184, 'i32x4.max_s', 'vv:v'), (185, 'i32x4.max_u', 'vv:v'), (186, 'i32x4.dot_i16x8_s', 'vv:v'),
    (188, 'i32x4.extmul_low_i16x8_s', 'vv:v'), (189, 'i32x4.extmul_high_i16x8_s', 'vv:v'),
    (190, 'i32x4.extmul_low_i16x8_u', 'vv:v'), (191, 'i32x4.extmul_high_i16x8_u', 'vv:v'),
    (192, 'i64x2.abs', 'v:v'), (193, 'i64x2.neg', 'v:v'), (195, 'i64x2.all_true', 'v:i'),
    (196, 'i64x2.bitmask', 'v:i'), (199, 'i64x2.extend_low_i32x4_s', 'v:v'),
    (200, 'i64x2.extend_high_i32x4_s', 'v:v'), (201, 'i64x2.extend_low_i32x4_u', 'v:v'),
    (202, 'i64x2.extend_high_i32x4_u', 'v:v'), (203, 'i64x2.shl', 'vi:v'), (204, 'i64x2.shr_s', 'vi:v'),
    (205, 'i64x2.shr_u', 'vi:v'), (206, 'i64x2.add', 'vv:v'), (209, 'i64x2.sub', 'vv:v'),
    (213, 'i64x2.mul', 'vv:v'), (214, 'i64x2.eq', 'vv:v'), (215, 'i64x2.ne', 'vv:v'),
    (216, 'i64x2.lt_s', 'vv:v'), (217, 'i64x2.gt_s', 'vv:v'), (218, 'i64x2.le_s', 'vv:v'),
    (219, 'i64x2.ge_s', 'vv:v'), (220, 'i64x2.extmul_low_i32x4_s', 'vv:v'),
    (221, 'i64x2.extmul_high_i32x4_s', 'vv:v'), (222, 'i64x2.extmul_low_i32x4_u', 'vv:v'),
    (223, 'i64x2.extmul_high_i32x4_u', 'vv:v'), (224, 'f32x4.abs', 'v:v'), (225, 'f32x4.neg', 'v:v'),
    (227, 'f32x4.sqrt', 'v:v'), (228, 'f32x4.add', 'vv:v'), (229, 'f32x4.sub', 'vv:v'),
    (230, 'f32x4.mul', 'vv:v'), (231, 'f32x4.div', 'vv:v'), (232, 'f32x4.min', 'vv:v'),
    (233, 'f32x4.max', 'vv:v'), (234, 'f32x4.pmin', 'vv:v'), (235, 'f32x4.pmax', 'vv:v'),
    (236, 'f64x2.abs', 'v:v'), (237, 'f64x2.neg', 'v:v'), (239, 'f64x2.sqrt', 'v:v'),
    (240, 'f64x2.add', 'vv:v'), (241, 'f64x2.sub', 'vv:v'), (242, 'f64x2.mul', 'vv:v'),
    (243, 'f64x2.div', 'vv:v'), (244, 'f64x2.min', 'vv:v'), (245, 'f64x2.max', 'vv:v'),
    (246, 'f64x2.pmin', 'vv:v'), (247, 'f64x2.pmax', 'vv:v'), (248, 'i32x4.trunc_sat_f32x4_s', 'v:v'),
    (249, 'i32x4.trunc_sat_f32x4_u', 'v:v'), (250, 'f32x4.convert_i32x4_s', 'v:v'),
    (251, 'f32x4.convert_i32x4_u', 'v:v'), (252, 'i32x4.trunc_sat_f64x2_s_zero', 'v:v'),
    (253, 'i32x4.trunc_sat_f64x2_u_zero', 'v:v'), (254, 'f64x2.convert_low_i32x4_s', 'v:v'),
    (255, 'f64x2.convert_low_i32x4_u', 'v:v')]
for code, name, sig in VEC:
    v(name, code, '-', sig)
# relaxed SIMD
for code, name, sig in [
        (0x100, 'i8x16.relaxed_swizzle', 'vv:v'), (0x101, 'i32x4.relaxed_trunc_f32x4_s', 'v:v'),
        (0x102, 'i32x4.relaxed_trunc_f32x4_u', 'v:v'), (0x103, 'i32x4.relaxed_trunc_f64x2_s_zero', 'v:v'),
        (0x104, 'i32x4.relaxed_trunc_f64x2_u_zero', 'v:v'), (0x105, 'f32x4.relaxed_madd', 'vvv:v'),
        (0x106, 'f32x4.relaxed_nmadd', 'vvv:v'), (0x107, 'f64x2.relaxed_madd', 'vvv:v'),
        (0x108, 'f64x2.relaxed_nmadd', 'vvv:v'), (0x109, 'i8x16.relaxed_laneselect', 'vvv:v'),
        (0x10a, 'i16x8.relaxed_laneselect', 'vvv:v'), (0x10b, 'i32x4.relaxed_laneselect', 'vvv:v'),
        (0x10c, 'i64x2.relaxed_laneselect', 'vvv:v'), (0x10d, 'f32x4.relaxed_min', 'vv:v'),
        (0x10e, 'f32x4.relaxed_max', 'vv:v'), (0x10f, 'f64x2.relaxed_min', 'vv:v'),
        (0x110, 'f64x2.relaxed_max', 'vv:v'), (0x111, 'i16x8.relaxed_q15mulr_s', 'vv:v'),
        (0x112, 'i16x8.relaxed_dot_i8x16_i7x16_s', 'vv:v'),
        (0x113, 'i32x4.relaxed_dot_i8x16_i7x16_add_s', 'vvv:v')]:
    v(name, code, '-', sig, 'r')

# atomics (the threads extension): 0xfe and the number
row('memory.atomic.notify', 'fe0', 'm4', 'pi:i', 'a')
row('memory.atomic.wait32', 'fe1', 'm4', 'piI:i', 'a')
row('memory.atomic.wait64', 'fe2', 'm8', 'pII:i', 'a')
row('atomic.fence', 'fe3', 'z', ':', 'a')
for k, (name, size, sig) in enumerate([
        ('i32.atomic.load', 4, 'p:i'), ('i64.atomic.load', 8, 'p:I'), ('i32.atomic.load8_u', 1, 'p:i'),
        ('i32.atomic.load16_u', 2, 'p:i'), ('i64.atomic.load8_u', 1, 'p:I'), ('i64.atomic.load16_u', 2, 'p:I'),
        ('i64.atomic.load32_u', 4, 'p:I'), ('i32.atomic.store', 4, 'pi:'), ('i64.atomic.store', 8, 'pI:'),
        ('i32.atomic.store8', 1, 'pi:'), ('i32.atomic.store16', 2, 'pi:'), ('i64.atomic.store8', 1, 'pI:'),
        ('i64.atomic.store16', 2, 'pI:'), ('i64.atomic.store32', 4, 'pI:')]):
    row(name, 'fe' + h(0x10 + k), 'm%d' % size, sig, 'a')
RMW = [('i32.atomic.rmw', 4, 'i'), ('i64.atomic.rmw', 8, 'I'), ('i32.atomic.rmw8', 1, 'i'),
       ('i32.atomic.rmw16', 2, 'i'), ('i64.atomic.rmw8', 1, 'I'), ('i64.atomic.rmw16', 2, 'I'),
       ('i64.atomic.rmw32', 4, 'I')]
for j, op in enumerate(['add', 'sub', 'and', 'or', 'xor', 'xchg', 'cmpxchg']):
    for k, (pre, size, s) in enumerate(RMW):
        sfx = '' if k < 2 else '_u'
        sig = 'p%s%s:%s' % (s, s, s) if op == 'cmpxchg' else 'p%s:%s' % (s, s)
        row('%s.%s%s' % (pre, op, sfx), 'fe' + h(0x1e + 7 * j + k), 'm%d' % size, sig, 'a')



def test():
    CT = {'i': 'int', 'I': 'long long', 'f': 'float', 'F': 'double', 'v': 'v4', 'p': 'char *'}
    CONST = {'i32.const': '-7', 'i64.const': '0x7fff_ffff_ffff', 'f32.const': '0x1p-149', 'f64.const': 'nan:0x8'}
    print('/* flags: --c-std=gnu17 --features=+relaxed-simd */')
    print('/* wasm-only */')
    print("/* Every instruction of the WebAssembly assembler's table (asm/wasmtab.ax)")
    print('   in an asm statement, with its immediates (memargs with offset and align,')
    print('   lanes, constants, v128.const, a shuffle): the module the program is must')
    print('   validate (the engine checks it when it loads it). Generated by')
    print('   cmd/internal/tools/gen_wasm_table.py --test; at -O0 and -O2, on wasm32')
    print('   and wasm64. */')
    out = ['typedef int v4 __attribute__((vector_size(16)));',
           'static char buf[64] __attribute__((aligned(16)));', 'volatile int zero;']
    calls = []
    for k, r in enumerate(ROWS):
        name, code, imm, sig, feat = r.split()
        ps, rs = sig.split(':')
        gets = ' '.join('local.get %%%d' % (i + len(rs)) for i in range(len(ps)))
        im = ''
        if imm[0] == 'm':
            im = 'offset=8 align=%s' % imm[1:]
        elif imm[0] == 'L':
            im = 'offset=0 0'
        elif imm[0] == 'l':
            im = '%d' % (int(imm[1:]) - 1)
        elif imm == 'c':
            im = CONST[name]
        elif imm == 'V':
            im = 'i16x8 1 2 3 4 5 6 7 -8'
        elif imm == 'S':
            im = ' '.join(str(i) for i in range(31, 15, -1))
        tmpl = '%s %s %s %s' % (gets, name, im, 'local.set %0' if rs else '')
        outs = ', '.join('"=r"(o%d)' % i for i in range(len(rs)))
        ins = ', '.join('"r"(a%d)' % i for i in range(len(ps)))
        decl = '; '.join('%s o%d' % (CT[c], i) for i, c in enumerate(rs))
        args = ', '.join('%s a%d' % (CT[c], i) for i, c in enumerate(ps)) or 'void'
        body = '%s; __asm__ volatile("%s" : %s : %s : "memory");' % (decl, tmpl, outs, ins)
        out.append('void f%d(%s) { /* %s */ %s }' % (k, args, name, body))
        vals = [{'i': '1', 'I': '1', 'f': '1', 'F': '1', 'v': '(v4){0}', 'p': 'buf'}[c] for c in ps]
        calls.append('    if (zero) f%d(%s);' % (k, ', '.join(vals)))
    out.append('int main(void) {')
    out += calls
    out.append('    return 0;\n}')
    print('\n'.join(out))


import sys
if '--test' in sys.argv:
    test()
    sys.exit(0)

print('''// =============================================================================
// asm/wasmtab.ax - the table of the WebAssembly assembler's instructions of a
// fixed signature (asm/wasm.ax): each line `name code imm sig feature`, as
// cmd/internal/tools/gen_wasm_table.py, which generates it from the
// WebAssembly specification's opcode tables, says how it reads: do not edit.
// =============================================================================''')
print('pub let TABLE: [%d][]u8 = [' % len(ROWS))
for r in ROWS:
    print('    "%s",' % r)
print('];')
