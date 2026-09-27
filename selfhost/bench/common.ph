// Output for the benchmarks: one checksum line.
import "std/io" as io;

pub fn result(name: []u8, x: u64) void {
    let mut buf: [24]u8 = [0; 24];
    let w = io.stdout();
    w.print(name) catch return;
    w.print(": ") catch return;
    w.println(io.fmt_u64(buf[..], x)) catch return;
}
