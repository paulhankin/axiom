# A tour of Axiom

A guided walk through the language in a few pages. Every program here is a file in
[`docs/tour/`](tour) that the test suite builds and runs, so what you read is what the
compiler accepts. The [specification](spec.md) has the rules; this page has the shape of
things. [`docs/install.md`](install.md) says how to get the compiler.

Axiom is a small systems language with no hidden control flow (no exceptions,
destructors or implicit allocation), checked arithmetic and indexing, and explicit
memory. It compiles to native code for x86-64 and AArch64 (Linux, macOS, Windows) and to
WebAssembly.

## Hello

```axiom
import "std/io" as io;

export fn main() i32 {
    io.stdout().write_all("hello, world\n") catch return 1;
    return 0;
}
```

A program is a module: a file of declarations. `export fn main() i32` is the entry
point. `import "std/io" as io;` brings in a standard library module under a name of
your choosing; every use is qualified (`io.stdout()`), so there is nothing to look up.

`write_all` can fail, so its result is an error union, `!void`. An error must be
handled: `catch return 1` returns 1 from `main` if it is one. Nothing can be ignored by
accident.

```sh
axiom build hello.ax --target=native -o hello && ./hello
axiom build hello.ax -o hello.wasm                 # WebAssembly; node <std>/internal/runtime/host.mjs hello.wasm
```

## Values

```axiom
import "std/fmt" as fmt;

const LIMIT: u32 = 100;

export fn main() i32 {
    let answer: i32 = 42;    // immutable; a literal alone does not fix a type, so say it
    let mut count: u32 = 0;  // mutable
    count += 1;
    let ratio: f64 = 2.5 * 4.0;
    let big: u8 = 250;
    let wrapped = big +% 10;  // 4: wrapping is the default meaning of +, and +% says so
    let name = "Ada";         // a []u8: a slice of bytes
    fmt.print("{} {} {} {} {} {}\n", [fmt.int(answer), fmt.int(count), fmt.float(ratio), fmt.int(wrapped),
                                      fmt.str(name), fmt.int(LIMIT)]) catch return 1;
    return 0;
}
```

```
42 1 10 4 Ada 100
```

`let` binds an immutable name, `let mut` a mutable one. A literal has no type of its
own, so a declaration with only a literal names its type; `count += 1` then knows what
to do. Integer types are `u8` … `u64`, `i8` … `i64`, `usize` and `isize`; floats are
`f32` and `f64`.

Integer arithmetic wraps by default, and never silently changes type: `big +% 10` says
the wrap is intended. `axiom build --overflow=trap` makes every `+`, `-` and `*` trap on
overflow instead, to find the unintended ones (spec §7.6). `const` declares a
compile-time constant, usable as an array length.

`std/fmt` formats values through templates: `{}` takes the next argument, and the
arguments are wrapped (`fmt.int`, `fmt.str`, `fmt.float`) because Axiom has no
variadic or dynamically typed arguments.

## Arrays and slices

```axiom
import "std/fmt" as fmt;
import "std/str" as str;

fn total(xs: []u32) u32 {
    let mut t: u32 = 0;
    for x in xs {
        t += x;
    }
    return t;
}

export fn main() i32 {
    let mut squares: [5]u32 = [0; 5];  // an array: a value with a fixed length
    for i in 0..squares.len {
        squares[i] = @int_cast(u32, i * i);
    }
    let tail = squares[2..];  // a slice: a pointer and a length into the array
    fmt.print("{} {} {}\n", [fmt.int(total(squares[..])), fmt.int(total(tail)), fmt.int(tail.len)]) catch return 1;
    let line = "key=value";  // a string literal is a []u8 too
    let eq = str.index_of_byte(line, '=') catch return 1;
    let key = line[..eq];
    let value = line[eq + 1..];
    fmt.print("{} / {} / {}\n", [fmt.str(key), fmt.str(value), fmt.boolean(str.eq(key, "key"))]) catch return 1;
    return 0;
}
```

```
30 29 3
key / value / true
```

`[5]u32` is an array, a value with a fixed length that is copied on assignment. `[]u32`
is a slice: a pointer and a length into memory that something else owns. `squares[..]`
and `squares[2..]` make slices of an array; `tail.len` is the length. Indexing is
bounds-checked: `xs[i]` with `i >= xs.len` traps, deterministically, on every target.

Strings are `[]u8`, bytes in UTF-8. `std/str` has the usual searches, splits and
comparisons; `std/unicode` deals with code points and text segmentation.

## Structs, unions and enums

```axiom
import "std/fmt" as fmt;

struct Point {
    x: f64,
    y: f64,

    fn dist2(self: Point, o: Point) f64 {
        let dx = self.x - o.x;
        let dy = self.y - o.y;
        return dx * dx + dy * dy;
    }
}

union Shape {
    Circle: struct { center: Point, radius: f64 },
    Rect: struct { min: Point, max: Point },
    Empty,

    fn area(self: Shape) f64 {
        match self {
            .Circle(c) => { return 3.141592653589793 * c.radius * c.radius; }
            .Rect(r)   => { return (r.max.x - r.min.x) * (r.max.y - r.min.y); }
            .Empty     => { return 0.0; }
        }
    }
}

enum Color: u8 {
    RED,
    GREEN,
    BLUE,
}

fn name(c: Color) []u8 {
    match c {
        .RED   => { return "red"; }
        .GREEN => { return "green"; }
        .BLUE  => { return "blue"; }
    }
}

export fn main() i32 {
    let origin = Point.{ x: 0.0, y: 0.0 };
    let shapes: [3]Shape = [
        Shape.Circle(.{ center: origin, radius: 1.0 }),
        Shape.Rect(.{ min: origin, max: Point.{ x: 2.0, y: 3.0 } }),
        Shape.Empty,
    ];
    for s in shapes {
        fmt.print("{:.2}\n", [fmt.float(s.area())]) catch return 1;
    }
    let d = origin.dist2(Point.{ x: 3.0, y: 4.0 });
    fmt.print("{} {}\n", [fmt.str(name(Color.GREEN)), fmt.float(d)]) catch return 1;
    return 0;
}
```

```
3.14
6.00
0.00
green 25
```

A `struct` has fields in declaration order and the layout you would expect, and can
hold methods; `self` is passed by value, or as `*Self` or `*mut Self` for methods that
should not copy or that change the value. `Point.{ x: 0.0, y: 0.0 }` is a struct
literal.

A `union` is a tagged union: a value holds exactly one variant, with or without a
payload. `match` takes it apart, binding the payload (`.Circle(c)`), and must cover
every variant. This is the language's whole polymorphism: no inheritance, interfaces or
vtables. Where behaviour must vary at run time, a struct holds function pointers
(`std/io`'s `Writer` is one).

An `enum` is a set of named integer constants with a declared representation.

## Errors

```axiom
import "std/fmt" as fmt;

error { Empty, NotANumber, TooBig }

fn parse(s: []u8) !u32 {
    if s.len == 0 {
        return error.Empty;
    }
    let mut v: u32 = 0;
    for c in s {
        if (c < '0') or (c > '9') {
            return error.NotANumber;
        }
        v = @mul_checked(v, 10) catch return error.TooBig;
        v = @add_checked(v, @as(u32, c - '0')) catch return error.TooBig;
    }
    return v;
}

// `!` hands an error straight to the caller.
fn sum(a: []u8, b: []u8) !u32 {
    return parse(a)! + parse(b)!;
}

export fn main() i32 {
    let inputs: [4][]u8 = ["12", "", "x1", "99999999999"];
    for s in inputs {
        let v = parse(s) catch |e| {
            fmt.print("{:?}: {}\n", [fmt.str(s), fmt.str(@error_name(e))]) catch return 1;
            continue;
        };
        fmt.print("{:?} = {}\n", [fmt.str(s), fmt.int(v)]) catch return 1;
    }
    let total = sum("40", "2") catch 0;  // a default instead of a handler
    fmt.print("{}\n", [fmt.int(total)]) catch return 1;
    return 0;
}
```

```
"12" = 12
"": Empty
"x1": NotANumber
"99999999999": TooBig
42
```

An error is a 16-bit code, one of the names declared in `error { … }` blocks anywhere
in the program. A function that can fail returns `!T`: a `T` or an error. The caller
chooses: postfix `!` hands an error to its own caller, `catch` with a value supplies a
default, and `catch |e| { … }` handles it, with `return`, `break` or `continue`
allowed inside. There is no unwinding and nothing happens behind your back.
`@mul_checked` and `@add_checked` are arithmetic that reports overflow as an error
instead of wrapping.

`defer S` runs `S` when the enclosing block ends, however it ends; `errdefer S` runs
only when the function returns an error, to undo partial work (spec §8.8).

## Memory

```axiom
import "std/collections/map" as map;
import "std/collections/vec" as vec;
import "std/fmt" as fmt;
import "std/hash" as hash;
import "std/mem" as mem;

export fn main() i32 {
    // Memory comes from the one global allocator, by an explicit call, and
    // goes back with mem.free; `defer` runs that call when the block ends.
    let buf = mem.alloc.[f64](4) catch return 1;
    defer mem.free(buf);
    for i in 0..buf.len {
        buf[i] = @float_from_int(f64, i) / 2.0;
    }
    // A growable vector and a hash map, both built on mem.alloc.
    let mut primes = vec.Vec.[u32].init();
    defer primes.deinit();
    let first: [4]u32 = [2, 3, 5, 7];
    for p in first {
        primes.push(p) catch return 1;
    }
    let mut ages = map.Map.[[]u8, u32].init(hash.hash_bytes, hash.eq_bytes);
    defer ages.deinit();
    ages.put("ada", 36) catch return 1;
    ages.put("alan", 41) catch return 1;
    fmt.print("{} {} {} {}\n", [fmt.float(buf[3]), fmt.int(primes.items().len), fmt.int(primes.get(3)),
                                fmt.int(ages.get("alan") catch 0)]) catch return 1;
    return 0;
}
```

```
1.5 4 7 41
```

No construct allocates. Memory comes from `std/mem`, the one global allocator, through
calls you can see, and goes back with `mem.free`; `defer mem.free(buf)` is the idiom.
The collections (`std/collections`: `vec`, `map`, `set`, `deque`, …) are built on it
and own what they allocate until `deinit`. A `Map` takes its hash and equality
functions when made, because the language has no traits to find them by.

Raw pointers (`*T`, `*mut T`) exist for the allocator, the runtime and C interop, and
are the one place the checks stop: a pointer must stay inside the object it came from
(spec §12.1). Everything else is checked.

## Generics

```axiom
import "std/fmt" as fmt;

// A generic function: T is any type with an order. Each use is compiled
// separately for its types (monomorphization); nothing is decided at run time.
fn largest[T: Ord](xs: []T) T {
    let mut best = xs[0];
    for x in xs {
        if x > best {
            best = x;
        }
    }
    return best;
}

// A constant parameter: the array's length is part of the type.
fn mean[const N: usize](xs: [N]f64) f64 {
    let mut t: f64 = 0.0;
    for x in xs {
        t += x;
    }
    return t / @float_from_int(f64, N);
}

// A generic type, with methods.
struct Pair[A, B] {
    first: A,
    second: B,

    fn swap(self: Pair.[A, B]) Pair.[B, A] {
        return Pair.[B, A].{ first: self.second, second: self.first };
    }
}

export fn main() i32 {
    let ints: [4]i32 = [3, -1, 9, 4];
    let xs: [3]f64 = [1.0, 2.0, 6.0];
    let p = Pair.[u8, []u8].{ first: 1, second: "one" }.swap();
    // T and N are deduced from the arguments; largest.[u8] gives T explicitly
    fmt.print("{} {} {} {} {}\n", [fmt.int(largest(ints[..])), fmt.int(largest.[u8]("pear")), fmt.float(mean(xs)),
                                   fmt.str(p.first), fmt.int(p.second)]) catch return 1;
    return 0;
}
```

```
9 114 3 one 1
```

Functions and types take type parameters and integer constant parameters. Each use is
compiled for its arguments (monomorphization); there is no run-time dispatch and no
code runs at compile time. A constraint (`T: Ord`, `T: Num`) says which operators a
type parameter must support, so a generic function is checked once, when it is
written. Arguments are deduced from the call where they can be, or written as
`largest.[u8](…)`.

## Tests

```axiom
import "std/str" as str;

pub fn is_palindrome(s: []u8) bool {
    let mut i: usize = 0;
    let mut j = s.len;
    while i + 1 < j {
        j -= 1;
        if s[i] != s[j] {
            return false;
        }
        i += 1;
    }
    return true;
}

test "palindromes" {
    @expect(is_palindrome("level"));
    @expect(is_palindrome(""));
    @expect(not is_palindrome("levels"));
}

test "str helpers" {
    @expect_eq(str.index_of_byte("a=b", '=')!, 1);
    @expect(str.eq(str.trim(" x "), "x"));
}

test "indexing past the end traps" expect trap(T0001) {
    let xs: [2]u8 = [1, 2];
    let i: usize = 2;
    _ = xs[i];
}
```

`test` blocks live next to the code, see the module's private items, and run with
`axiom test tests.ax` (`--target=native` for native code; the default is WebAssembly,
which needs Node.js), each in its own process. They are type-checked in every build, so
they cannot rot, but add nothing to a normal build. `@expect` and `@expect_eq` are the
assertions; `expect trap(T0001)` makes a test that must trap, since traps are defined
behaviour and so testable.

```
$ axiom test tests.ax --target=native
ok   palindromes (3 ms)
ok   str helpers (3 ms)
ok   indexing past the end traps (5 ms)

3 passed, 0 failed
```

## SIMD

```axiom
import "std/fmt" as fmt;

// Four f32 lanes at a time. The same code runs on x86-64 (SSE/AVX),
// AArch64 (NEON) and WebAssembly (SIMD), with the same results.
fn dot(a: []f32, b: []f32) f32 {
    let mut acc: f32x4 = @splat(0.0);
    let mut i: usize = 0;
    while i + 4 <= a.len {
        acc = acc + @load(f32x4, a[i..i + 4]) * @load(f32x4, b[i..i + 4]);
        i += 4;
    }
    // the tail: lanes past the end read as zero and are never touched in memory
    acc = acc + @load_partial(f32x4, a[i..]) * @load_partial(f32x4, b[i..]);
    return @reduce_add(acc);
}

export fn main() i32 {
    let a: [6]f32 = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
    let b: [6]f32 = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0];
    let v = @vec(i32x4, 1, 2, 3, 4);
    let w = v * @splat(10) + v;  // lane-wise: 11 22 33 44
    fmt.print("{} {} {}\n", [fmt.float32(dot(a[..], b[..])), fmt.int(@extract_lane(w, 2)),
                             fmt.int(@reduce_add(w))]) catch return 1;
    return 0;
}
```

```
21 33 110
```

Vectors are primitive types (`f32x4`, `i32x4`, `u8x16`, … up to 512 bits) with
lane-wise arithmetic and builtins for loading, storing, shuffling and reducing them.
Their meaning is WebAssembly's on every target, so a kernel gives the same results on
x86-64, AArch64 and the web; `axiom build --target=native` or `--cpu=x86-64-v4` picks
the processor extensions to use, and `@load_partial` reads a tail without running past
the end of a slice.

## Building for a target

```sh
axiom build prog.ax --target=native -O2 -o prog            # this machine
axiom build prog.ax --target=x86_64 -O2 -o prog            # any x86-64 Linux machine
axiom build prog.ax --target=aarch64-macos -O2 -o prog     # Apple silicon
axiom build prog.ax --target=x86_64-windows -O2 -o prog.exe
axiom build prog.ax -O2 -o prog.wasm                       # WebAssembly (wasm32)
axiom build prog.ax --sanitize=address --target=native     # use after free, double free, leaks
axiom profile prog.ax --target=native -- args              # where the time goes
```

Native programs that link no C code are static executables: the compiler has its own
backends and linkers. `axiom new <name>` makes a package (an `axiom.toml` and a
`src/lib.ax`), and `axiom build`, `check` and `test` take a package directory
(spec §4.6). [`docs/cli.md`](cli.md) lists every command and flag; the
[`examples/`](../examples) directory has complete programs, from a word counter to a
pinball table.
