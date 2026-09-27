// Error unions, !, catch and error names.
import "../lib/io.ph" as io;

error { InvalidInput, TooBig }

fn parse_digit(c: u8) !u32 {
    if (c < '0') or (c > '9') {
        return error.InvalidInput;
    }
    return @as(u32, c - '0');
}

fn parse(s: []u8) !u32 {
    let mut total: u32 = 0;
    for c in s {
        total = total * 10 + parse_digit(c)!;
        if total > 1000 {
            return error.TooBig;
        }
    }
    return total;
}

fn check(x: u32) !void {
    if x == 13 {
        return error.InvalidInput;
    }
}

export fn run() void {
    io.int(@as(i64, parse("123") catch 0));      // expect: 123
    io.int(@as(i64, parse("12x") catch 7));      // expect: 7
    let r = parse("99999") catch |e| {
        io.str(@error_name(e));                  // expect: TooBig
        return;
    };
    io.int(@as(i64, r));
}
