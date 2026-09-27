# Generates a Phalange program exercising many operations over edge values.
import random, sys
seed = int(sys.argv[1]) if len(sys.argv) > 1 else 1
random.seed(seed)
out = []
P = out.append
P('import host "env" { fn print_i64(x: i64) void; fn print_u64(x: u64) void; fn print_f64(x: f64) void; }')
INTS = [('u8',8,False),('i8',8,True),('u16',16,False),('i16',16,True),('u32',32,False),('i32',32,True),('u64',64,False),('i64',64,True)]
def vals(bits, signed):
    if signed:
        lo, hi = -(1<<(bits-1)), (1<<(bits-1))-1
    else:
        lo, hi = 0, (1<<bits)-1
    v = {0, 1, hi, lo, 2, 3, 7}
    if signed: v |= {-1, -2, lo+1, hi-1}
    else: v |= {hi-1, hi//2, hi//2+1}
    while len(v) < 12: v.add(random.randint(lo, hi))
    return sorted(v)
def lit(x, t):
    if x < 0: return '-%d' % (-x) if x != -(1<<63) else '-9223372036854775807 - 1'
    return str(x)
N = {}
for t, bits, s in INTS:
    vs = vals(bits, s)
    N[t] = len(vs)
    P('let mut v_%s: [%d]%s = [%s];' % (t, len(vs), t, ', '.join(('(%s)' % lit(x,t)) if x < 0 else lit(x,t) for x in vs)))
P('let mut cnt: [8]u32 = [0, 1, 3, 7, 8, 15, 31, 63];')
P('let mut cnt64: [3]u64 = [64, 65, 1000];')
FV = [0.0, -0.0, 1.0, -1.0, 0.5, 1.5, 2.5, -2.5, 3.7, -3.7, 1e10, -1e10, 1e300, 123456789.125]
P('let mut v_f64: [%d]f64 = [%s];' % (len(FV), ', '.join(('(%r)' % x) if x < 0 or str(x).startswith('-') else repr(x) for x in FV)))
P('let mut v_f32: [%d]f32 = [%s];' % (len(FV)-1, ', '.join(('(%r)' % x) if str(x).startswith('-') else repr(x) for x in FV[:-2]+[1e5])))
def pr(expr, t):
    if t.startswith('i'): return 'print_i64(@as(i64, %s));' % expr
    if t.startswith('u'): return 'print_u64(@as(u64, %s));' % expr
    if t == 'bool': return 'print_u64(@as(u64, @as(u8, 0)) + (if_b(%s)));' % expr
for t, bits, s in INTS:
    P('fn t_%s(a: %s, b: %s) void {' % (t, t, t))
    P('    print_u64(9000001);')
    for op in ['+%', '-%', '*%', '&', '|', '^']:
        P('    %s' % pr('a %s b' % op, t))
    for op in ['==', '!=', '<', '<=', '>', '>=']:
        P('    print_u64(bv(a %s b));' % op)
    P('    if b != 0 {')
    if s:
        P('        if not ((a == @min_value(%s)) and (b == -1)) {' % t)
        P('            %s' % pr('a / b', t)); P('            %s' % pr('a % b', t)); P('        }')
    else:
        P('        %s' % pr('a / b', t)); P('        %s' % pr('a % b', t))
    P('    }')
    P('    %s' % pr('@min(a, b)', t)); P('    %s' % pr('@max(a, b)', t))
    P('    %s' % pr('@add_sat(a, b)', t)); P('    %s' % pr('@sub_sat(a, b)', t)); P('    %s' % pr('@mul_sat(a, b)', t))
    P('    let x = @add_checked(a, b) catch { print_u64(777); return; };'); P('    %s' % pr('x', t))
    P('}')
    P('fn u_%s(a: %s) void {' % (t, t))
    P('    print_u64(9000002);')
    for f in ['@clz(a)', '@ctz(a)', '@popcount(a)', '~a', '-%a']:
        P('    %s' % pr(f, t))
    if s:
        P('    if a != @min_value(%s) { %s }' % (t, pr('@abs(a)', t)))
    P('    for k in cnt { %s %s %s %s }' % (pr('a << k', t), pr('a >> k', t), pr('@rotl(a, k)', t), pr('@rotr(a, k)', t)))
    P('    for k in cnt64 { %s %s }' % (pr('a << k', t), pr('a >> k', t)))
    # conversions
    for t2, b2, s2 in INTS:
        if t2 == t: continue
        P('    %s' % pr('@truncate(%s, a)' % t2, t2) if b2 < bits else '    %s' % pr('@as(%s, a)' % t2, t2) if (b2 > bits and (s2 or not s)) else '    // skip')
    P('    pf(@float_from_int(f64, a));')
    P('    pf(@float_cast(f64, @float_from_int(f32, a)));')
    P('}')
P('fn bv(c: bool) u64 { if c { return 1; } return 0; }')
for ft in ['f64', 'f32']:
    P('fn f_%s(a: %s, b: %s) void {' % (ft, ft, ft))
    P('    print_u64(9000003);')
    P('    let x = a; let y = b;')
    for e in ['x + y', 'x - y', 'x * y', 'x / y', '@min(x, y)', '@max(x, y)', '@copysign(x, y)', '@rem(x, y)', '@fma(x, y, x)']:
        P('    pf(@float_cast(f64, %s));' % e)
    for op in ['==', '!=', '<', '<=', '>', '>=']:
        P('    print_u64(bv(x %s y));' % op)
    P('}')
    P('fn g_%s(a: %s) void {' % (ft, ft))
    P('    print_u64(9000004);')
    for e in ['@sqrt(@abs(a))', '@floor(a)', '@ceil(a)', '@trunc(a)', '@nearest(a)', '@abs(a)', '-a']:
        P('    pf(@float_cast(f64, %s));' % e)
    for t2, b2, s2 in INTS:
        P('    %s' % pr('@int_from_float(%s, a)' % t2, t2))
    P('    pf(@float_cast(f64, @float_cast(f32, @float_cast(f64, a))));')
    P('}')
P('fn pf(x: f64) void { if x != x { print_u64(12345); } else { print_u64(@bit_cast(u64, x)); } }')
# SIMD
VT = [('i8x16','i8',16),('u8x16','u8',16),('i16x8','i16',8),('u16x8','u16',8),('i32x4','i32',4),('u32x4','u32',4),('i64x2','i64',2),('u64x2','u64',2)]
for vt, lt, n in VT:
    P('fn pv_%s(v: %s) void {' % (vt, vt))
    for i in range(n): P('    %s' % pr('@extract_lane(v, %d)' % i, lt))
    P('}')
    P('fn mk_%s(o: usize) %s { return @vec(%s, %s); }' % (vt, vt, vt, ', '.join('v_%s[(o + %d) %% %d]' % (lt, i, N[lt]) for i in range(n))))
    P('fn s_%s(a: %s, b: %s) void {' % (vt, vt, vt))
    P('    print_u64(9000005);')
    for op in ['+%', '-%', '&', '|', '^'] + (['*%'] if lt[1:] != '8' else []):
        P('    pv_%s(a %s b);' % (vt, op))
    P('    pv_%s(~a);' % vt)
    for f in ['eq', 'ne', 'lt', 'le', 'gt', 'ge']:
        P('    pv_%s(@bit_cast(%s, @lanes_%s(a, b)));' % (vt, vt, f))
    if lt[1:] != '64':
        P('    pv_%s(@min(a, b)); pv_%s(@max(a, b));' % (vt, vt))
    if lt[1:] in ('8', '16'):
        P('    pv_%s(@add_sat(a, b)); pv_%s(@sub_sat(a, b));' % (vt, vt))
    if lt.startswith('i'):
        P('    pv_%s(@abs(a));' % vt)
    for k in [0, 1, 5, int(lt[1:]) - 1, int(lt[1:]), 100]:
        P('    pv_%s(a << @as(u32, %d)); pv_%s(a >> @as(u32, %d));' % (vt, k, vt, k))
    P('    for k in cnt { pv_%s(a << k); pv_%s(a >> k); }' % (vt, vt))
    P('    %s' % pr('@reduce_add(a)', lt))
    P('    print_u64(@as(u64, @bitmask(a)));')
    P('    print_u64(bv(@any_true(a))); print_u64(bv(@all_true(a)));')
    P('    pv_%s(@splat(@extract_lane(a, 1)));' % vt)
    P('    pv_%s(@replace_lane(a, %d, @extract_lane(b, 0)));' % (vt, n - 1))
    idx = [random.randint(0, 2*n-1) for _ in range(n)]
    P('    pv_%s(@shuffle(a, b, [%s]));' % (vt, ', '.join(map(str, idx))))
    P('    pv_%s(@select(@bit_cast(%s, @lanes_lt(a, b)), a, b));' % (vt, vt))
    P('}')
P('fn pvf(v: f32x4) void { pf(@float_cast(f64, @extract_lane(v, 0))); pf(@float_cast(f64, @extract_lane(v, 1))); pf(@float_cast(f64, @extract_lane(v, 2))); pf(@float_cast(f64, @extract_lane(v, 3))); }')
P('fn pvd(v: f64x2) void { pf(@extract_lane(v, 0)); pf(@extract_lane(v, 1)); }')
P('fn sf(a: f32x4, b: f32x4, c: f64x2, d: f64x2) void {')
for e in ['a + b', 'a - b', 'a * b', 'a / b', '@min(a, b)', '@max(a, b)', '@sqrt(@abs(a))', '@floor(a)', '@ceil(a)', '@trunc(a)', '@nearest(a)', '-a', '@fma(a, b, a)']:
    P('    pvf(%s);' % e)
for e in ['c + d', 'c - d', 'c * d', 'c / d', '@min(c, d)', '@max(c, d)', '@sqrt(@abs(c))', '@floor(c)', '@nearest(c)', '-c', '@fma(c, d, c)']:
    P('    pvd(%s);' % e)
P('    pf(@float_cast(f64, @reduce_add(a))); pf(@reduce_add(c)); pf(@float_cast(f64, @dot(a, b)));')
P('    print_u64(@as(u64, @bitmask(@lanes_lt(a, b)))); print_u64(@as(u64, @bitmask(@lanes_ge(c, d))));')
P('}')
P('fn sw(a: u8x16, idx: u8x16) void { pv_u8x16(@swizzle(a, idx)); }')
P('fn dt(a: i16x8, b: i16x8) void { pv_i32x4(@dot(a, b)); }')
P('export fn run() void {')
for t, bits, s in INTS:
    P('    for i in 0..v_%s.len { u_%s(v_%s[i]); for j in 0..v_%s.len { t_%s(v_%s[i], v_%s[j]); } }' % (t,t,t,t,t,t,t))
for ft in ['f64', 'f32']:
    P('    for i in 0..v_%s.len { g_%s(v_%s[i]); for j in 0..v_%s.len { f_%s(v_%s[i], v_%s[j]); } }' % (ft,ft,ft,ft,ft,ft,ft))
for vt, lt, n in VT:
    P('    for i in @as(usize, 0)..6 { s_%s(mk_%s(i), mk_%s(i + 3)); }' % (vt, vt, vt))
P('    for i in @as(usize, 0)..4 { sw(mk_u8x16(i), mk_u8x16(i + 5)); dt(mk_i16x8(i), mk_i16x8(i + 2)); }')
P('    for i in @as(usize, 0)..5 {')
P('        let a = @vec(f32x4, v_f32[i], v_f32[i + 1], v_f32[i + 2], v_f32[i + 3]);')
P('        let b = @vec(f32x4, v_f32[i + 4], v_f32[i + 5], v_f32[i + 6], v_f32[i + 7]);')
P('        let c = @vec(f64x2, v_f64[i], v_f64[i + 3]);')
P('        let d = @vec(f64x2, v_f64[i + 5], v_f64[i + 7]);')
P('        sf(a, b, c, d);')
P('    }')
P('}')
print('\n'.join(out))
