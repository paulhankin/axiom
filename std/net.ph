// =============================================================================
// std/net.ph - TCP sockets and IPv4 addresses.
//
// Sockets use the Phalange host ABI (import module "phalange"); see
// runtime/host.mjs and runtime/native_host.c. All operations block.
// =============================================================================

import "io.ph" as io;

error { ConnectionRefused, AddressInUse, InvalidAddress, NetError }

import host "phalange" {
    fn ph_tcp_connect(name: *u8, name_len: usize, port: u32) i32;
    fn ph_tcp_listen(name: *u8, name_len: usize, port: u32) i32;
    fn ph_tcp_accept(listener: i32) i32;
    fn ph_tcp_local_port(handle: i32) i32;
    fn ph_sock_read(handle: i32, p: *mut u8, n: usize) isize;
    fn ph_sock_write(handle: i32, p: *u8, n: usize) isize;
    fn ph_sock_close(handle: i32) void;
}

fn host_error(code: i32) error { match code {
    -2   => { return error.ConnectionRefused; }
    -3   => { return error.AddressInUse; }
    -4   => { return error.InvalidAddress; }
    else => { return error.NetError; }
}
}

// -----------------------------------------------------------------------------
// IPv4 addresses (host byte order: 127.0.0.1 == 0x7F000001)
// -----------------------------------------------------------------------------

/// Parses dotted-quad notation, e.g. "192.168.0.1".
pub fn parse_ipv4(s: []u8) !u32 {
    let mut addr: u32 = 0;
    let mut part: u32 = 0;
    let mut digits: u32 = 0;
    let mut dots: u32 = 0;
    for c in s {
        if c == '.' {
            if (digits == 0) or (dots == 3) {
                return error.InvalidAddress;
            }
            addr = (addr << 8) | part;
            part = 0;
            digits = 0;
            dots += 1;
        } else if (c >= '0') and (c <= '9') {
            if (digits > 0) and (part == 0) {
                return error.InvalidAddress;  // no leading zeros
            }
            part = (part * 10) + @as(u32, c - '0');
            digits += 1;
            if (part > 255) or (digits > 3) {
                return error.InvalidAddress;
            }
        } else {
            return error.InvalidAddress;
        }
    }
    if (dots != 3) or (digits == 0) {
        return error.InvalidAddress;
    }
    return (addr << 8) | part;
}

/// Writes dotted-quad notation into buf (needs 15 bytes) and returns it.
pub fn format_ipv4(buf: []mut u8, addr: u32) []u8 {
    let mut n: usize = 0;
    let mut shift: u32 = 24;
    loop {
        let octet = (addr >> shift) & 255;
        let mut tmp: [20]u8 = [0; 20];
        let d = io.fmt_u64(tmp[..], @as(u64, octet));
        @mem_copy(buf[n..n + d.len], d);
        n += d.len;
        if shift == 0 {
            break;
        }
        buf[n] = '.';
        n += 1;
        shift -= 8;
    }
    return buf[..n];
}

// -----------------------------------------------------------------------------
// TCP
// -----------------------------------------------------------------------------

pub struct TcpStream {
    handle: i32,

    /// Reads some bytes; 0 means the peer closed the connection.
    pub fn read(self: TcpStream, buf: []mut u8) !usize {
        let n = ph_sock_read(self.handle, buf.ptr, buf.len);
        if n < 0 {
            return error.NetError;
        }
        return @int_cast(usize, n);
    }

    pub fn write(self: TcpStream, bytes: []u8) !usize {
        let n = ph_sock_write(self.handle, bytes.ptr, bytes.len);
        if n < 0 {
            return error.NetError;
        }
        return @int_cast(usize, n);
    }

    pub fn write_all(self: TcpStream, bytes: []u8) !void {
        self.writer().write_all(bytes)!;
    }

    pub fn reader(self: TcpStream) io.Reader {
        return io.Reader.{ ctx: handle_ctx(self.handle), read_fn: stream_read };
    }

    pub fn writer(self: TcpStream) io.Writer {
        return io.Writer.{ ctx: handle_ctx(self.handle), write_fn: stream_write };
    }

    pub fn close(self: TcpStream) void {
        ph_sock_close(self.handle);
    }
}

pub struct TcpListener {
    handle: i32,

    /// The local port (useful after listening on port 0).
    pub fn port(self: TcpListener) u16 {
        return @int_cast(u16, ph_tcp_local_port(self.handle));
    }

    /// Waits for the next incoming connection.
    pub fn accept(self: TcpListener) !TcpStream {
        let h = ph_tcp_accept(self.handle);
        if h < 0 {
            return host_error(h);
        }
        return TcpStream.{ handle: h };
    }

    pub fn close(self: TcpListener) void {
        ph_sock_close(self.handle);
    }
}

/// Connects to hostname:port. `hostname` is a name or a dotted-quad address.
pub fn connect(hostname: []u8, port: u16) !TcpStream {
    let h = ph_tcp_connect(hostname.ptr, hostname.len, @as(u32, port));
    if h < 0 {
        return host_error(h);
    }
    return TcpStream.{ handle: h };
}

/// Listens on hostname:port; port 0 picks a free port (see TcpListener.port).
pub fn listen(hostname: []u8, port: u16) !TcpListener {
    let h = ph_tcp_listen(hostname.ptr, hostname.len, @as(u32, port));
    if h < 0 {
        return host_error(h);
    }
    return TcpListener.{ handle: h };
}

fn handle_ctx(h: i32) *mut void {
    return @ptr_from_int(*mut void, @int_cast(usize, h));
}

fn stream_read(ctx: *mut void, buf: []mut u8) !usize {
    let s = TcpStream.{ handle: @int_cast(i32, @int_from_ptr(ctx)) };
    return s.read(buf);
}

fn stream_write(ctx: *mut void, bytes: []u8) !usize {
    let s = TcpStream.{ handle: @int_cast(i32, @int_from_ptr(ctx)) };
    return s.write(bytes);
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/net.ph)
// -----------------------------------------------------------------------------

test "ipv4 round trip" {
    let a = parse_ipv4("10.0.255.1")!;
    @expect_eq(a, 0x0A00FF01);
    let mut buf: [15]u8 = [0; 15];
    @expect_eq(format_ipv4(buf[..], a), "10.0.255.1");
}

test "ipv4 rejects malformed input" {
    let bad: [5][]u8 = ["1.2.3", "1.2.3.4.5", "256.1.1.1", "01.2.3.4", "a.b.c.d"];
    for s in bad {
        _ = parse_ipv4(s) catch continue;
        @trap();
    }
}
