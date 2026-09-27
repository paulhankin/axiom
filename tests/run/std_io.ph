// std/io
import "std/mem" as mem;
import "std/io" as io;

fn err_of[T](r: !T) []u8 {
    _ = r catch |e| {
        return @error_name(e);
    };
    return "ok";
}

fn demo() !void {
    let out = io.stdout();
    out.println("hello from std/io")!;                  // expect: hello from std/io
    out.print_i64(-9223372036854775807 - 1)!;
    out.print("\n")!;                                   // expect: -9223372036854775808
    out.print_u64(@max_value(u64))!;
    out.print_byte('\n')!;                              // expect: 18446744073709551615
    out.print_hex(0xdeadbeef)!;
    out.print(" ")!;
    out.print_bool(true)!;
    out.print("\n")!;                                   // expect: deadbeef true
    out.print_f64(3.14159, 3)!;
    out.print(" ")!;
    out.print_f64(-2.0, 0)!;
    out.print(" ")!;
    out.print_f64(0.9996, 3)!;
    out.print(" ")!;
    out.print_f64(1.5e20, 1)!;
    out.print("\n")!;                                   // expect: 3.142 -2 1.000 1.5e20

    let mut storage: [64]u8 = [0; 64];
    let mut bw = io.BufferWriter.init(storage[..]);
    let w = bw.writer();
    w.print("x=")!;
    w.print_i64(42)!;
    out.println(bw.written())!;                         // expect: x=42

    let mut sr = io.SliceReader.init("line one\r\nline two\nlast");
    let r = sr.reader();
    let mut lb: [32]u8 = [0; 32];
    out.println(r.read_line(lb[..])!)!;                 // expect: line one
    out.println(r.read_line(lb[..])!)!;                 // expect: line two
    out.println(r.read_line(lb[..])!)!;                 // expect: last
    out.println(err_of(r.read_line(lb[..])))!;         // expect: EndOfStream

    io.stdout().print_i64(io.parse_i64("-123")!)!;
    out.print("\n")!;                                   // expect: -123

    // files
    let path = "/tmp/phalange_std_io_test.txt";
    let f = io.create(path)!;
    let mut buffered = io.BufWriter.init(f.writer());
    let fw = buffered.writer();
    for i in @as(u64, 0)..1000 {
        fw.print_u64(i)!;
        fw.print("\n")!;
    }
    buffered.flush()!;
    f.close();
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();
    let g = io.open(path)!;
    let all = g.reader().read_all(arena.allocator())!;
    g.close();
    out.print_u64(@as(u64, all.len))!;
    out.print("\n")!;                                   // expect: 3890
    out.println(err_of(io.open("/tmp/definitely/not/here")))!;  // expect: NotFound
}

export fn run() void {
    demo() catch @trap();
}
