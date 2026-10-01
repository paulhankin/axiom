// =============================================================================
// std/net/http.ph - an HTTP/1.1 server and client, over TCP or TLS.
//
// A server:
//
//     fn hello(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
//         let name = req.query_param("name") catch "world";
//         res.header("Content-Type", "text/plain")!;
//         fmt.write(res.writer(), "hello, {}\n", [fmt.str(name)])!;
//     }
//
//     let mut app = App.{ ... };
//     let mut r = http.Router.[App].init(&mut app);
//     r.get("/hello", hello)!;
//     r.get("/users/{id}", show_user)!;      // req.param("id")
//     r.get("/static/*", files)!;            // req.param("*")
//     r.serve("0.0.0.0", 8080, http.Options.default())!;
//
// or, without a router, http.serve(hostname, port, handler, ctx, options).
// Handlers run on a pool of threads (Options.threads), each serving one
// connection at a time; the context pointer is shared by all of them, so
// synchronize what it points at (std/sync). A Request and everything it
// points at lives until the handler returns.
//
// The response body is buffered and sent with a Content-Length, unless it
// grows past 64 KiB or the handler calls flush(), when it is streamed
// (chunked for HTTP/1.1). A handler that fails before anything was sent
// produces a 500 response.
//
// The server speaks HTTP/1.0 and 1.1 with keep-alive and pipelining,
// chunked request bodies and `Expect: 100-continue`, and limits the size of
// request heads and bodies (431, 413) and how long a connection may idle.
//
// A client:
//
//     let mut resp = http.get("http://example.com/")!;
//     defer resp.deinit();
//     io.stdout().print(resp.body)!;
//
// https:// URLs are fetched over TLS 1.3 (std/net/tls), checking the server's
// certificate against the system's roots unless ClientOptions.with_tls
// gives another configuration. A server serves HTTPS when made with
// server_tls (or Router.serve_tls) and a tls.ServerConfig.
//
// The server and the client are threads and blocking sockets over std/net,
// so they run natively and on WebAssembly under node alike.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../fmt.ph" as fmt;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../net.ph" as net;
import "../os/process.ph" as process;
import "../str.ph" as strs;
import "../sync.ph" as sync;
import "../thread.ph" as thread;
import "../time.ph" as time;
import "hpack.ph" as hpack;
import "http2.ph" as http2;
import "tls.ph" as tls;
import "url.ph" as urls;

error {
    InvalidArgument,
    BadRequest,
    BadResponse,
    BadUrl,
    TooLarge,
    HeadTooLarge,
    NotFound,
    Closed,
    InvalidHeader,
    OutOfMemory,
    NetError,
    TimedOut,
    Unsupported,
}

pub struct Header {
    name: []u8,
    value: []u8,
}

/// Case-insensitive header lookup: the first header called `name`.
pub fn find_header(headers: []Header, name: []u8) ![]u8 {
    for h in headers {
        if strs.eq_ignore_case(h.name, name) {
            return h.value;
        }
    }
    return error.NotFound;
}

/// Whether a comma-separated header value (such as Connection) contains
/// `token`, ignoring case.
fn has_token(value: []u8, token: []u8) bool {
    let mut it = strs.split(value, ",");
    loop {
        let t = it.next() catch break;
        if strs.eq_ignore_case(strs.trim(t), token) {
            return true;
        }
    }
    return false;
}

/// The standard reason phrase for a status code ("" if unknown).
pub fn reason_phrase(status: u16) []u8 {
    match status {
        100  => { return "Continue"; }
        101  => { return "Switching Protocols"; }
        200  => { return "OK"; }
        201  => { return "Created"; }
        202  => { return "Accepted"; }
        204  => { return "No Content"; }
        206  => { return "Partial Content"; }
        301  => { return "Moved Permanently"; }
        302  => { return "Found"; }
        303  => { return "See Other"; }
        304  => { return "Not Modified"; }
        307  => { return "Temporary Redirect"; }
        308  => { return "Permanent Redirect"; }
        400  => { return "Bad Request"; }
        401  => { return "Unauthorized"; }
        403  => { return "Forbidden"; }
        404  => { return "Not Found"; }
        405  => { return "Method Not Allowed"; }
        408  => { return "Request Timeout"; }
        409  => { return "Conflict"; }
        411  => { return "Length Required"; }
        413  => { return "Content Too Large"; }
        414  => { return "URI Too Long"; }
        415  => { return "Unsupported Media Type"; }
        422  => { return "Unprocessable Content"; }
        429  => { return "Too Many Requests"; }
        431  => { return "Request Header Fields Too Large"; }
        500  => { return "Internal Server Error"; }
        501  => { return "Not Implemented"; }
        502  => { return "Bad Gateway"; }
        503  => { return "Service Unavailable"; }
        504  => { return "Gateway Timeout"; }
        505  => { return "HTTP Version Not Supported"; }
        else => { return ""; }
    }
}

// -----------------------------------------------------------------------------
// Percent-encoding
// -----------------------------------------------------------------------------

fn hex_value(c: u8) u32 {
    if (c >= '0') and (c <= '9') {
        return @as(u32, c - '0');
    }
    let l = c | 0x20;
    if (l >= 'a') and (l <= 'f') {
        return @as(u32, l - 'a') + 10;
    }
    return 16;
}

/// Decodes %XX escapes (and, if `plus_space`, '+' as a space, as in query
/// strings) into out, which must be at least as long as s. Fails with
/// BadRequest on a malformed escape. (std/net/url has the rest of
/// percent-encoding.)
pub fn percent_decode(s: []u8, out: []mut u8, plus_space: bool) ![]u8 {
    return urls.percent_decode(s, out, plus_space) catch return error.BadRequest;
}

/// Writes s with everything but unreserved characters (letters, digits and
/// -._~) percent-encoded, for use in a path segment or query value.
pub fn percent_encode(w: io.Writer, s: []u8) !void {
    urls.encode_to(w, s, urls.Component.COMPONENT)!;
}

/// The value of `name` in a query string ("a=1&b=x%20y"), still encoded.
pub fn query_raw(query: []u8, name: []u8) ![]u8 {
    return urls.query_value(query, name);
}

// -----------------------------------------------------------------------------
// Buffered connection reading (shared by the server and the client)
// -----------------------------------------------------------------------------

/// A connection, plain or TLS.
/// A connection, plain or over TLS (for std/net/websocket, and upgraded
/// connections).
pub struct Transport {
    tcp: net.TcpStream,
    tls: *mut tls.Conn,  // when secure
    secure: bool,

    pub fn plain(s: net.TcpStream) Transport {
        return Transport.{ tcp: s, tls: @ptr_from_int(*mut tls.Conn, 8), secure: false };
    }

    pub fn over_tls(c: *mut tls.Conn) Transport {
        return Transport.{ tcp: c.stream, tls: c, secure: true };
    }

    pub fn read(self: Transport, buf: []mut u8) !usize {
        if self.secure {
            return self.tls.read(buf);
        }
        return self.tcp.read(buf);
    }

    pub fn write_all(self: Transport, bytes: []u8) !void {
        if self.secure {
            return self.tls.write_all(bytes);
        }
        return self.tcp.write_all(bytes);
    }

    pub fn close(self: Transport) void {
        if self.secure {
            self.tls.close();
        } else {
            self.tcp.close();
        }
    }
}

struct Conn {
    stream: Transport,
    buf: []mut u8,
    start: usize,  // unread data is buf[start..end]
    end: usize,
    pin: usize,     // buf[..pin] holds the current head: never moved
    limit: usize,   // the most buf may grow to
    deadline: i64,  // monotonic ns by which the current message must arrive (0: none)
    h2: bool,       // the client sent HTTP/2's preface

    fn init(stream: Transport, limit: usize) !Conn {
        let b = mem.alloc.[u8](@min(16384, limit))!;
        return Conn.{ stream: stream, buf: b, start: 0, end: 0, pin: 0, limit: limit, deadline: 0, h2: false };
    }

    fn deinit(self: *mut Conn) void {
        mem.free(self.buf);
    }

    /// Reads more data. 0 means the peer closed the connection.
    fn fill(self: *mut Conn) !usize {
        if self.end == self.buf.len {
            if self.start > self.pin {
                // make room by moving unread data down to the pinned head
                let n = self.end - self.start;
                @mem_copy(self.buf[self.pin..self.pin + n], self.buf[self.start..self.end]);
                self.start = self.pin;
                self.end = self.pin + n;
            } else if (self.pin == 0) and (self.buf.len < self.limit) {
                self.buf = mem.realloc(self.buf, @min(self.buf.len * 2, self.limit))!;
            } else {
                return error.HeadTooLarge;
            }
        }
        // (a peer dripping bytes must not hold the connection forever: each
        // read has the socket's timeout, the whole message has this)
        if (self.deadline != 0) and (time.Instant.now().ns > self.deadline) {
            return error.TimedOut;
        }
        let n = self.stream.read(self.buf[self.end..])!;
        self.end += n;
        return n;
    }

    /// Starts a new message: unread data moves to the front.
    fn reset(self: *mut Conn) void {
        let n = self.end - self.start;
        if (self.start > 0) and (n > 0) {
            @mem_copy(self.buf[..n], self.buf[self.start..self.end]);
        }
        self.start = 0;
        self.end = n;
        self.pin = 0;
    }

    /// Waits for a whole head (up to and including the blank line) and
    /// returns its length from `start`; 0 if the peer closed the connection
    /// before sending anything.
    fn read_head(self: *mut Conn) !usize {
        let mut scanned: usize = 0;
        loop {
            // blank lines before a message are ignored (RFC 9112 section 2.2)
            while (scanned == 0) and (self.start < self.end) and
                  ((self.buf[self.start] == '\r') or (self.buf[self.start] == '\n')) {
                self.start += 1;
            }
            let data = self.buf[self.start..self.end];
            let mut i = scanned;
            while i < data.len {
                if data[i] == '\n' {
                    if ((i + 1) < data.len) and (data[i + 1] == '\n') {
                        return i + 2;
                    }
                    if ((i + 2) < data.len) and (data[i + 1] == '\r') and (data[i + 2] == '\n') {
                        return i + 3;
                    }
                }
                i += 1;
            }
            scanned = @max(data.len, 2) - 2;
            let n = self.fill()!;
            if n == 0 {
                if self.end == self.start {
                    return 0;
                }
                return error.Closed;
            }
        }
    }

    /// Takes up to `max` buffered or newly read bytes.
    fn take(self: *mut Conn, max: usize) ![]u8 {
        if self.start == self.end {
            if self.fill()! == 0 {
                return error.Closed;
            }
        }
        let n = @min(max, self.end - self.start);
        let out = self.buf[self.start..self.start + n];
        self.start += n;
        return out;
    }

    /// Appends exactly n bytes to out, growing it as they arrive (a peer
    /// announcing a large body must send it before we allocate it).
    fn read_into(self: *mut Conn, out: *mut strs.String, n: usize) !void {
        let mut left = n;
        while left > 0 {
            let part = self.take(@min(left, 65536))!;
            out.push_str(part)!;
            left -= part.len;
        }
    }

    /// Reads exactly dst.len bytes.
    fn read_exact(self: *mut Conn, dst: []mut u8) !void {
        let mut got: usize = 0;
        while got < dst.len {
            let part = self.take(dst.len - got)!;
            @mem_copy(dst[got..got + part.len], part);
            got += part.len;
        }
    }

    /// One line without its line ending (at most a few KiB).
    fn read_line(self: *mut Conn) ![]u8 {
        loop {
            let data = self.buf[self.start..self.end];
            let nl = strs.index_of_byte(data, '\n') catch data.len;
            if nl < data.len {
                self.start += nl + 1;
                let mut line = data[..nl];
                if (line.len > 0) and (line[line.len - 1] == '\r') {
                    line = line[..line.len - 1];
                }
                return line;
            }
            if data.len > 8192 {
                return error.BadRequest;
            }
            if self.fill()! == 0 {
                return error.Closed;
            }
        }
    }

    /// Everything until the peer closes the connection, appended to out.
    fn read_to_end(self: *mut Conn, out: *mut strs.String, max: usize) !void {
        loop {
            if self.start < self.end {
                let part = self.buf[self.start..self.end];
                if (out.len + part.len) > max {
                    return error.TooLarge;
                }
                out.push_str(part)!;
                self.start = self.end;
            }
            // only a clean close ends the body: a TLS connection cut without
            // close_notify fails here (it may be a truncation attack)
            if self.fill()! == 0 {
                return;
            }
        }
    }

    /// A chunked body (RFC 9112 section 7.1), appended to out; trailers are
    /// read and dropped.
    fn read_chunked(self: *mut Conn, out: *mut strs.String, max: usize) !void {
        loop {
            let line = self.read_line()!;
            let semi = strs.index_of_byte(line, ';') catch line.len;
            // chunk-size = 1*HEXDIG, then optional whitespace before extensions
            let size_text = trim_trailing_blanks(line[..semi]);
            if (size_text.len == 0) or (size_text.len > ((2 * @size_of(usize)) - 1)) {
                return error.BadRequest;
            }
            let mut size: usize = 0;
            for c in size_text {
                let d = hex_value(c);
                if d > 15 {
                    return error.BadRequest;
                }
                size = (size * 16) + @as(usize, d);
            }
            if size == 0 {
                // trailers until a blank line (a bounded number)
                for _k in @as(usize, 0)..MAX_TRAILERS {
                    let t = self.read_line()!;
                    if t.len == 0 {
                        return;
                    }
                }
                return error.HeadTooLarge;
            }
            if (out.len + size) > max {
                return error.TooLarge;
            }
            self.read_into(out, size)!;
            if self.read_line()!.len != 0 {
                return error.BadRequest;
            }
        }
    }
}

fn is_token_char(c: u8) bool {
    if strs.is_alnum(c) {
        return true;
    }
    let specials = "!#$%&'*+-.^_`|~";
    for s in specials {
        if c == s {
            return true;
        }
    }
    return false;
}

/// Parses header lines ("Name: value") into out; returns how many.
pub fn parse_headers(lines: []u8, out: []mut Header) !usize {
    let mut n: usize = 0;
    let mut it = strs.lines(lines);
    loop {
        let line = it.next() catch break;
        if line.len == 0 {
            break;
        }
        if (line[0] == ' ') or (line[0] == '\t') {
            return error.BadRequest;  // obsolete line folding
        }
        let colon = strs.index_of_byte(line, ':') catch return error.BadRequest;
        let name = line[..colon];
        if name.len == 0 {
            return error.BadRequest;
        }
        for c in name {
            if not is_token_char(c) {
                return error.BadRequest;
            }
        }
        if n == out.len {
            return error.HeadTooLarge;
        }
        let value = strs.trim_bytes(line[colon + 1..], " \t");
        for c in value {
            // (a CR, NUL or other control byte can be read differently
            // by a proxy in front: reject it)
            if ((c < 0x20) and (c != '\t')) or (c == 0x7f) {
                return error.BadRequest;
            }
        }
        out[n] = Header.{ name: name, value: value };
        n += 1;
    }
    return n;
}

fn trim_trailing_blanks(s: []u8) []u8 {
    let mut e = s.len;
    while (e > 0) and ((s[e - 1] == ' ') or (s[e - 1] == '\t')) {
        e -= 1;
    }
    return s[..e];
}

/// Trailer lines accepted after a chunked body.
const MAX_TRAILERS: usize = 100;

/// How a message body is delimited.
struct Framing {
    chunked: bool,
    length: usize,
    has_length: bool,
}

fn framing(headers: []Header) !Framing {
    let mut f = Framing.{ chunked: false, length: 0, has_length: false };
    for h in headers {
        if strs.eq_ignore_case(h.name, "Transfer-Encoding") {
            // chunked must be the last coding; no others are supported
            if f.chunked or not strs.eq_ignore_case(h.value, "chunked") {
                return error.Unsupported;
            }
            f.chunked = true;
        } else if strs.eq_ignore_case(h.name, "Content-Length") {
            // 1*DIGIT: no sign, no spaces
            if (h.value.len == 0) or (h.value.len > 18) {
                return error.BadRequest;
            }
            for c in h.value {
                if (c < '0') or (c > '9') {
                    return error.BadRequest;
                }
            }
            let v = fmt.parse_int.[u64](h.value) catch return error.BadRequest;
            if v > @as(u64, @max_value(usize) / 2) {
                return error.TooLarge;
            }
            let len = @truncate(usize, v);
            if f.has_length and (len != f.length) {
                return error.BadRequest;
            }
            f.length = len;
            f.has_length = true;
        }
    }
    if f.chunked and f.has_length {
        return error.BadRequest;  // a request smuggling risk (RFC 9112 6.3)
    }
    return f;
}

// -----------------------------------------------------------------------------
// Requests
// -----------------------------------------------------------------------------

pub struct Param {
    name: []u8,
    value: []u8,
}

const MAX_PARAMS: usize = 8;

/// Memory that lives until the current request is answered, and the route
/// parameters.
struct Scratch {
    blocks: vec.Vec.[[]mut u8],
    params: [8]Param,
    nparams: usize,

    fn alloc(self: *mut Scratch, n: usize) ![]mut u8 {
        let b = mem.alloc.[u8](n)!;
        self.blocks.push(b) catch |e| {
            mem.free(b);
            return e;
        };
        return b;
    }

    fn clear(self: *mut Scratch) void {
        for b in self.blocks.items() {
            mem.free(b);
        }
        self.blocks.clear();
        self.nparams = 0;
    }
}

/// HTTP versions of a request (HTTP/2 requests are served as HTTP_1_1).
pub enum Version: u8 {
    HTTP_1_0 = 0,
    HTTP_1_1 = 1,
}

pub struct Request {
    method: []u8,  // "GET", "POST", ...
    target: []u8,  // as sent: "/search?q=x"
    path: []u8,    // the target before any '?', still percent-encoded
    query: []u8,   // the target after '?', or ""
    version: Version,
    headers: []Header,
    body: []u8,
    scratch: *mut Scratch,

    /// The first header called `name` (case-insensitive), or NotFound.
    pub fn header(self: *Request, name: []u8) ![]u8 {
        return find_header(self.headers, name);
    }

    /// A route parameter, such as "id" for the pattern "/users/{id}", or "*"
    /// for the rest of the path after a trailing "/*". Percent-decoded.
    pub fn param(self: *Request, name: []u8) ![]u8 {
        let sc = self.scratch;
        for i in 0..sc.nparams {
            if @mem_eq(sc.params[i].name, name) {
                return sc.params[i].value;
            }
        }
        return error.NotFound;
    }

    /// The decoded value of `name` in the query string, or NotFound.
    pub fn query_param(self: *Request, name: []u8) ![]u8 {
        let raw = query_raw(self.query, name)!;
        let out = self.scratch.alloc(raw.len)!;
        return percent_decode(raw, out, true);
    }

    /// The decoded path.
    pub fn decoded_path(self: *Request) ![]u8 {
        let out = self.scratch.alloc(self.path.len)!;
        return percent_decode(self.path, out, false);
    }
}

// -----------------------------------------------------------------------------
// Responses
// -----------------------------------------------------------------------------

/// Bodies bigger than this are streamed rather than buffered.
const STREAM_THRESHOLD: usize = 65536;

pub struct Response {
    stream: Transport,
    status: u16,
    head: strs.String,  // "Name: value\r\n" for each header the handler set
    body: strs.String,  // buffered, not yet sent
    version: Version,
    head_only: bool,  // a HEAD request: no body is sent
    keep_alive: bool,
    started: bool,  // the status line and headers have been sent
    chunked: bool,
    has_type: bool,
    upgraded: bool,       // the handler took the connection over
    h2: *mut http2.Conn,  // for HTTP/2 (is_h2): the connection and stream
    h2_id: u32,
    is_h2: bool,

    pub fn set_status(self: *mut Response, status: u16) void {
        self.status = status;
    }

    /// Adds a header. Content-Length, Transfer-Encoding and Date are set by
    /// the server; "Connection: close" ends the connection after this
    /// response. Fails if called after the head was sent, or if the name or
    /// value is malformed.
    pub fn header(self: *mut Response, name: []u8, value: []u8) !void {
        if self.started or (name.len == 0) {
            return error.InvalidHeader;
        }
        for c in name {
            if not is_token_char(c) {
                return error.InvalidHeader;
            }
        }
        for c in value {
            if (c == '\r') or (c == '\n') or (c == 0) {
                return error.InvalidHeader;
            }
        }
        if strs.eq_ignore_case(name, "Content-Length") or strs.eq_ignore_case(name, "Transfer-Encoding") {
            return error.InvalidHeader;
        }
        if strs.eq_ignore_case(name, "Content-Type") {
            self.has_type = true;
        }
        if strs.eq_ignore_case(name, "Connection") and has_token(value, "close") {
            self.keep_alive = false;
            return;
        }
        self.head.push_str(name)!;
        self.head.push_str(": ")!;
        self.head.push_str(value)!;
        self.head.push_str("\r\n")!;
    }

    /// Appends to the body.
    pub fn write(self: *mut Response, bytes: []u8) !void {
        self.body.push_str(bytes)!;
        if self.body.len >= STREAM_THRESHOLD {
            self.flush()!;
        }
    }

    /// A Writer for the body (for fmt.write, json.Writer, ...).
    pub fn writer(self: *mut Response) io.Writer {
        return io.Writer.{ ctx: @ptr_cast(*mut void, self), write_fn: response_write };
    }

    /// Sets the status and a text/plain body in one go.
    pub fn text(self: *mut Response, status: u16, body: []u8) !void {
        self.status = status;
        if not self.has_type {
            self.header("Content-Type", "text/plain; charset=utf-8")!;
        }
        self.write(body)!;
    }

    /// Sends what is buffered now and streams the rest of the body.
    pub fn flush(self: *mut Response) !void {
        if self.is_h2 {
            if not self.started {
                self.h2_head(false, 0, false)!;
            }
            if (self.body.len > 0) and not self.head_only {
                self.h2.send_data(self.h2_id, self.body.bytes(), false)!;
            }
            self.body.clear();
            return;
        }
        if not self.started {
            if self.version == Version.HTTP_1_0 {
                // no chunked encoding in HTTP/1.0: the body ends at close
                self.keep_alive = false;
            } else {
                self.chunked = true;
            }
            self.send_head(0, false, "")!;
        }
        if (self.body.len == 0) or self.head_only {
            self.body.clear();
            return;
        }
        if self.chunked {
            // one write per chunk: small separate writes stall on Nagle's
            // algorithm and delayed ACKs
            let mut b: [20]u8 = [0; 20];
            let size = fmt.format_uint(b[..18], @as(u64, self.body.len), 16, false);
            let mut chunk = strs.String.with_capacity(self.body.len + 24)!;
            defer chunk.deinit();
            chunk.push_str(size)!;
            chunk.push_str("\r\n")!;
            chunk.push_str(self.body.bytes())!;
            chunk.push_str("\r\n")!;
            self.stream.write_all(chunk.bytes())!;
        } else {
            self.stream.write_all(self.body.bytes())!;
        }
        self.body.clear();
    }

    /// Takes the connection over for another protocol (as std/net/websocket
    /// does): sends "101 Switching Protocols" with the headers set so far,
    /// and returns the connection. The handler then speaks the new protocol
    /// on it until it returns, when the server closes it.
    pub fn upgrade(self: *mut Response) !Transport {
        if self.started or self.is_h2 {
            return error.InvalidHeader;
        }
        self.status = 101;
        self.keep_alive = true;
        self.send_head(0, false, "")!;
        self.upgraded = true;
        return self.stream;
    }

    /// Sends the status line and headers, followed by `body` in the same
    /// write.
    fn send_head(self: *mut Response, length: usize, with_length: bool, body: []u8) !void {
        let mut h = strs.String.with_capacity(256 + self.head.len + body.len)!;
        defer h.deinit();
        let w = h.writer();
        if self.version == Version.HTTP_1_0 {
            h.push_str("HTTP/1.0 ")!;
        } else {
            h.push_str("HTTP/1.1 ")!;
        }
        fmt.write(w, "{} {}\r\n", [fmt.int(self.status), fmt.str(reason_phrase(self.status))])!;
        let mut db: [32]u8 = [0; 32];
        fmt.write(w, "Date: {}\r\n", [fmt.str(time.now().format_http(db[..]))])!;
        h.push_str(self.head.bytes())!;
        if (not self.has_type) and ((length > 0) or self.chunked) {
            h.push_str("Content-Type: text/plain; charset=utf-8\r\n")!;
        }
        let no_body = ((self.status >= 100) and (self.status < 200)) or (self.status == 204) or (self.status == 304);
        if with_length and not no_body {
            fmt.write(w, "Content-Length: {}\r\n", [fmt.int(length)])!;
        }
        if self.chunked {
            h.push_str("Transfer-Encoding: chunked\r\n")!;
        }
        if not self.keep_alive {
            h.push_str("Connection: close\r\n")!;
        } else if self.version == Version.HTTP_1_0 {
            h.push_str("Connection: keep-alive\r\n")!;
        }
        h.push_str("\r\n")!;
        h.push_str(body)!;
        self.started = true;
        self.stream.write_all(h.bytes())!;
    }

    // HTTP/2: the status and headers as a HEADERS frame (the connection's
    // own headers do not exist in HTTP/2, and names are lower-case)
    fn h2_head(self: *mut Response, with_length: bool, length: usize, end: bool) !void {
        let mut store = strs.String.init();
        defer store.deinit();
        let mut spans = vec.Vec.[[4]usize].init();
        defer spans.deinit();
        let mut sb: [8]u8 = [0; 8];
        let st = fmt.format_int(sb[..], self.status);
        store.push_str(":status")!;
        store.push_str(st)!;
        spans.push([0, 7, 7, st.len])!;
        let mut lines = strs.lines(self.head.bytes());
        while true {
            let line = lines.next() catch break;
            let c = strs.index_of(line, ": ") catch continue;
            let name = line[..c];
            if strs.eq_ignore_case(name, "Connection") or strs.eq_ignore_case(name, "Keep-Alive") or
               strs.eq_ignore_case(name, "Upgrade") or strs.eq_ignore_case(name, "Transfer-Encoding") {
                continue;
            }
            let na = store.len;
            for ch in name {
                store.push(strs.to_lower_byte(ch))!;
            }
            let va = store.len;
            store.push_str(strs.trim_right(line[c + 2..]))!;
            spans.push([na, name.len, va, store.len - va])!;
        }
        let mut db: [32]u8 = [0; 32];
        let date = time.now().format_http(db[..]);
        let da = store.len;
        store.push_str("date")!;
        store.push_str(date)!;
        spans.push([da, 4, da + 4, date.len])!;
        if not self.has_type and (with_length and (length > 0)) {
            let ta = store.len;
            store.push_str("content-typetext/plain; charset=utf-8")!;
            spans.push([ta, 12, ta + 12, 25])!;
        }
        if with_length {
            let mut lb: [24]u8 = [0; 24];
            let l = fmt.format_int(lb[..], length);
            let la = store.len;
            store.push_str("content-length")!;
            store.push_str(l)!;
            spans.push([la, 14, la + 14, l.len])!;
        }
        let mut hs = vec.Vec.[hpack.Header].init();
        defer hs.deinit();
        let b = store.bytes();
        for sp in spans.items() {
            hs.push(hpack.Header.{ name: b[sp[0]..sp[0] + sp[1]], value: b[sp[2]..sp[2] + sp[3]] })!;
        }
        self.h2.send_headers(self.h2_id, hs.items(), end)!;
        self.started = true;
    }

    /// Completes the response after the handler returns.
    fn finish(self: *mut Response) !void {
        if self.is_h2 {
            let no_body = (self.status == 204) or (self.status == 304) or self.head_only;
            if not self.started {
                let n = self.body.len;
                self.h2_head(true, n, no_body or (n == 0))!;
                if not no_body and (n > 0) {
                    self.h2.send_data(self.h2_id, self.body.bytes(), true)!;
                }
            } else {
                let mut rest: []u8 = "";
                if not no_body {
                    rest = self.body.bytes();
                }
                self.h2.send_data(self.h2_id, rest, true)!;
            }
            self.body.clear();
            return;
        }
        if not self.started {
            let n = self.body.len;
            let no_body = (self.status == 204) or (self.status == 304);
            let mut body: []u8 = "";
            if (not self.head_only) and not no_body {
                body = self.body.bytes();
            }
            self.send_head(n, true, body)!;
            self.body.clear();
            return;
        }
        self.flush()!;
        if self.chunked and not self.head_only {
            self.stream.write_all("0\r\n\r\n")!;
        }
    }
}

fn response_write(ctx: *mut void, bytes: []u8) !usize {
    let r = @ptr_cast(*mut Response, ctx);
    r.write(bytes)!;
    return bytes.len;
}

// -----------------------------------------------------------------------------
// The server
// -----------------------------------------------------------------------------

pub struct Options {
    threads: u32,           // connections served at once, one per thread
    max_head_bytes: usize,  // request line and headers
    max_body_bytes: usize,
    idle_timeout_ms: u32,     // how long any one read may wait
    request_timeout_ms: u32,  // how long a whole request may take to arrive

    pub fn default() Options {
        return Options.{
            threads: 8,
            max_head_bytes: 65536,
            max_body_bytes: 8388608,
            idle_timeout_ms: 30000,
            request_timeout_ms: 60000,
        };
    }
}

/// A listening server; run() serves until stop().
pub struct Server[C] {
    listener: net.TcpListener,
    handler: fn(*mut C, *Request, *mut Response) !void,
    ctx: *mut C,
    opts: Options,
    stopping: u32,
    tls_config: *tls.ServerConfig,
    secure: bool,

    /// The port it listens on (useful after listening on port 0).
    pub fn port(self: *Self) u16 {
        return self.listener.port();
    }

    /// Serves connections on Options.threads threads (this one included)
    /// until stop() is called.
    pub fn run(self: *mut Self) !void {
        let extra = @as(usize, @max(self.opts.threads, 1)) - 1;
        let ts = mem.alloc.[thread.Thread.[*mut Self]](@max(extra, 1))!;
        defer mem.free(ts);
        let mut started: usize = 0;
        for i in 0..extra {
            ts[i] = thread.spawn(accept_loop.[C], self) catch break;
            started += 1;
        }
        accept_loop(self);
        for i in 0..started {
            ts[i].join();
        }
    }

    /// Stops accepting connections; run() returns once the connections in
    /// progress have finished their current request. May be called from a
    /// handler or any other thread.
    pub fn stop(self: *mut Self) void {
        if @atomic_xchg(&mut self.stopping, 1) == 0 {
            self.listener.close();
        }
    }
}

/// Listens on hostname:port (hostname "" or "0.0.0.0" for every address; port 0
/// for any free port). Call run() to serve.
pub fn server[C](hostname: []u8, port: u16, handler: fn(*mut C, *Request, *mut Response) !void, ctx: *mut C,
                 opts: Options) !Server.[C] {
    let l = net.listen(hostname, port)!;
    return Server.[C].{
        listener: l,
        handler: handler,
        ctx: ctx,
        opts: opts,
        stopping: 0,
        tls_config: @ptr_from_int(*tls.ServerConfig, 8),
        secure: false,
    };
}

/// Like server, but serving HTTPS with the given certificate and key, which
/// must outlive the server.
pub fn server_tls[C](hostname: []u8, port: u16, handler: fn(*mut C, *Request, *mut Response) !void, ctx: *mut C,
                     opts: Options, cfg: *tls.ServerConfig) !Server.[C] {
    let mut s = server(hostname, port, handler, ctx, opts)!;
    s.tls_config = cfg;
    s.secure = true;
    return s;
}

/// Listens on hostname:port and serves forever.
pub fn serve[C](hostname: []u8, port: u16, handler: fn(*mut C, *Request, *mut Response) !void, ctx: *mut C,
                opts: Options) !void {
    let mut s = server(hostname, port, handler, ctx, opts)!;
    s.run()!;
}

fn accept_loop[C](s: *mut Server.[C]) void {
    loop {
        let conn = s.listener.accept() catch {
            if @atomic_load(&s.stopping) != 0 {
                return;
            }
            thread.sleep_ms(10);  // e.g. out of file descriptors
            continue;
        };
        if @atomic_load(&s.stopping) != 0 {
            conn.close();
            return;
        }
        conn.set_timeout(s.opts.idle_timeout_ms) catch {
            conn.close();
            continue;
        };
        let mut t = Transport.plain(conn);
        if s.secure {
            // the handshake closes the socket if it fails
            t = Transport.over_tls(tls.server(conn, s.tls_config) catch continue);
        }
        if s.secure and strs.eq(t.tls.protocol(), "h2") {
            serve_h2(s, t, "");
        } else {
            serve_connection(s, t);
        }
        t.close();
    }
}

/// Sends a short error response and returns false (close the connection).
fn reject(stream: Transport, status: u16) bool {
    let mut b: [160]u8 = [0; 160];
    let msg = fmt.bufprint(b[..],
                           "HTTP/1.1 {} {}\r\nContent-Type: text/plain\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}\n",
                           [
                               fmt.int(status),
                               fmt.str(reason_phrase(status)),
                               fmt.int(reason_phrase(status).len + 1),
                               fmt.str(reason_phrase(status)),
                           ]) catch return false;
    stream.write_all(msg) catch return false;
    return false;
}

fn status_for(e: error) u16 {
    match e {
        error.HeadTooLarge => { return 431; }
        error.TooLarge     => { return 413; }
        error.Unsupported  => { return 501; }
        error.TimedOut     => { return 408; }
        else               => { return 400; }
    }
}

fn serve_connection[C](s: *mut Server.[C], stream: Transport) void {
    let mut c = Conn.init(stream, s.opts.max_head_bytes) catch return;
    defer c.deinit();
    let mut body = strs.String.init();
    defer body.deinit();
    let mut scratch = new_scratch();
    defer scratch.blocks.deinit();
    loop {
        let keep = serve_request(s, &mut c, &mut body, &mut scratch) catch false;
        scratch.clear();
        if c.h2 {
            // the preface was read as a request head: give HTTP/2 all of it
            let mut pre = strs.String.init();
            defer pre.deinit();
            pre.push_str("PRI * HTTP/2.0\r\n\r\n") catch return;
            pre.push_str(c.buf[c.start..c.end]) catch return;
            serve_h2(s, c.stream, pre.bytes());
            return;
        }
        if (not keep) or (@atomic_load(&s.stopping) != 0) {
            return;
        }
        c.reset();
        if body.capacity() > 1048576 {
            body.deinit();
        }
        body.clear();
    }
}

fn transport_read(ctx: *mut void, buf: []mut u8) !usize {
    let t = @ptr_cast(*mut Transport, ctx);
    return t.read(buf)!;
}

fn transport_write(ctx: *mut void, bytes: []u8) !usize {
    let t = @ptr_cast(*mut Transport, ctx);
    t.write_all(bytes)!;
    return bytes.len;
}

// HTTP/2: the requests of a connection, one at a time (the others wait,
// their frames handled meanwhile).
fn serve_h2[C](s: *mut Server.[C], stream: Transport, prefill: []u8) void {
    let mut t = stream;
    let r = io.Reader.{ ctx: @ptr_cast(*mut void, &mut t), read_fn: transport_read };
    let w = io.Writer.{ ctx: @ptr_cast(*mut void, &mut t), write_fn: transport_write };
    let mut h = http2.Conn.init(r, w, true, prefill) catch return;
    defer h.deinit();
    h.max_body = s.opts.max_body_bytes;
    h.handshake() catch return;
    let mut scratch = new_scratch();
    defer scratch.blocks.deinit();
    let mut hs = vec.Vec.[Header].init();
    defer hs.deinit();
    loop {
        if @atomic_load(&s.stopping) != 0 {
            h.goaway(http2.NO_ERROR) catch {};
            return;
        }
        let id = h.step() catch return;
        if id == 0 {
            // the client said goodbye, and nothing is left to answer
            if h.got_goaway and (h.streams.len == 0) {
                return;
            }
            continue;
        }
        let st = h.stream(id) catch continue;
        if st.reset {
            h.drop_stream(id);
            continue;
        }
        let method = st.header(":method");
        let target = st.header(":path");
        if (method.len == 0) or (target.len == 0) or strs.eq(method, "CONNECT") {
            h.reset_stream(id, http2.PROTOCOL_ERROR) catch return;
            h.drop_stream(id);
            continue;
        }
        // the headers as HTTP/1's (with Host from :authority)
        hs.clear();
        let authority = st.header(":authority");
        if authority.len > 0 {
            hs.push(Header.{ name: "host", value: authority }) catch return;
        }
        for x in st.headers() {
            if (x.name.len > 0) and (x.name[0] != ':') {
                hs.push(Header.{ name: x.name, value: x.value }) catch return;
            }
        }
        let q = strs.index_of_byte(target, '?') catch target.len;
        let mut query: []u8 = "";
        if q < target.len {
            query = target[q + 1..];
        }
        let req = Request.{ method: method, target: target, path: target[..q], query: query, version: Version.HTTP_1_1,
                            headers: hs.items(), body: st.body.bytes(), scratch: &mut scratch };
        let mut res = Response.{ stream: t, status: 200, head: strs.String.init(), body: strs.String.init(),
                                 version: Version.HTTP_1_1, head_only: @mem_eq(method, "HEAD"), keep_alive: true,
                                 started: false, chunked: false, has_type: false, upgraded: false, h2: &mut h,
                                 h2_id: id, is_h2: true };
        s.handler(s.ctx, &req, &mut res) catch {
            if not res.started {
                res.status = 500;
                res.head.clear();
                res.body.clear();
                res.has_type = false;
                res.write("Internal Server Error\n") catch {};
            }
        };
        let ok = res.finish();
        res.head.deinit();
        res.body.deinit();
        scratch.clear();
        h.drop_stream(id);
        ok catch |e| {
            if e != error.StreamReset {
                return;
            }
        };
    }
}

/// Serves one request; false if the connection should close.
fn serve_request[C](s: *mut Server.[C], c: *mut Conn, body: *mut strs.String, scratch: *mut Scratch) !bool {
    c.deadline = time.Instant.now().ns + (@as(i64, s.opts.request_timeout_ms) * 1000000);
    let head_len = c.read_head() catch |e| {
        if (e == error.Closed) or (e == error.NetError) {
            return false;
        }
        if (e == error.TimedOut) and (c.end == c.start) {
            return false;  // an idle keep-alive connection
        }
        return reject(c.stream, status_for(e));
    };
    if head_len == 0 {
        return false;
    }
    let head = c.buf[c.start..c.start + head_len];
    c.start += head_len;
    c.pin = c.start;
    // the request line
    let line_end = strs.index_of_byte(head, '\n')!;
    let mut rl = head[..line_end];
    if (rl.len > 0) and (rl[rl.len - 1] == '\r') {
        rl = rl[..rl.len - 1];
    }
    let sp1 = strs.index_of_byte(rl, ' ') catch return reject(c.stream, 400);
    let rest = rl[sp1 + 1..];
    let sp2 = strs.index_of_byte(rest, ' ') catch return reject(c.stream, 400);
    let method = rl[..sp1];
    let target = rest[..sp2];
    let proto = rest[sp2 + 1..];
    if (method.len == 0) or (target.len == 0) {
        return reject(c.stream, 400);
    }
    for ch in method {
        if not is_token_char(ch) {
            return reject(c.stream, 400);
        }
    }
    for ch in target {
        if (ch <= 0x20) or (ch == 0x7F) {
            return reject(c.stream, 400);
        }
    }
    if @mem_eq(method, "PRI") and @mem_eq(target, "*") and @mem_eq(proto, "HTTP/2.0") {
        c.h2 = true;  // (serve_connection continues in HTTP/2)
        return false;
    }
    let mut version = Version.HTTP_1_1;
    if @mem_eq(proto, "HTTP/1.0") {
        version = Version.HTTP_1_0;
    } else if not @mem_eq(proto, "HTTP/1.1") {
        if strs.starts_with(proto, "HTTP/") {
            return reject(c.stream, 505);
        }
        return reject(c.stream, 400);
    }
    let mut hs: [100]Header = [Header.{ name: "", value: "" }; 100];
    let nh = parse_headers(head[line_end + 1..], hs[..]) catch |e| {
        return reject(c.stream, status_for(e));
    };
    let headers = hs[..nh];
    // exactly one Host (RFC 9112 3.2)
    let mut hosts: usize = 0;
    for h in headers {
        if strs.eq_ignore_case(h.name, "Host") {
            hosts += 1;
        }
    }
    if (hosts > 1) or ((version == Version.HTTP_1_1) and (hosts == 0)) {
        return reject(c.stream, 400);
    }
    let fr = framing(headers) catch |e| {
        return reject(c.stream, status_for(e));
    };
    if fr.chunked and (version == Version.HTTP_1_0) {
        return reject(c.stream, 400);  // (HTTP/1.0 has no transfer codings: faulty framing)
    }
    // the body
    if fr.has_length and (fr.length > s.opts.max_body_bytes) {
        return reject(c.stream, 413);
    }
    if (fr.chunked or (fr.length > 0)) and has_token(find_header(headers, "Expect") catch "", "100-continue") {
        c.stream.write_all("HTTP/1.1 100 Continue\r\n\r\n") catch return false;
    }
    if fr.chunked {
        c.read_chunked(body, s.opts.max_body_bytes) catch |e| {
            return reject(c.stream, status_for(e));
        };
    } else if fr.length > 0 {
        c.read_into(body, fr.length) catch return false;
    }
    // keep-alive
    let conn_hdr = find_header(headers, "Connection") catch "";
    let mut keep = version == Version.HTTP_1_1;
    if has_token(conn_hdr, "close") {
        keep = false;
    } else if (version == Version.HTTP_1_0) and has_token(conn_hdr, "keep-alive") {
        keep = true;
    }
    let q = strs.index_of_byte(target, '?') catch target.len;
    let mut query: []u8 = "";
    if q < target.len {
        query = target[q + 1..];
    }
    let req = Request.{
        method: method,
        target: target,
        path: target[..q],
        query: query,
        version: version,
        headers: headers,
        body: body.bytes(),
        scratch: scratch,
    };
    let mut res = Response.{
        stream: c.stream,
        status: 200,
        head: strs.String.init(),
        body: strs.String.init(),
        version: version,
        head_only: @mem_eq(method, "HEAD"),
        keep_alive: keep,
        started: false,
        chunked: false,
        has_type: false,
        upgraded: false,
        h2: @ptr_from_int(*mut http2.Conn, 8),
        h2_id: 0,
        is_h2: false,
    };
    defer res.head.deinit();
    defer res.body.deinit();
    s.handler(s.ctx, &req, &mut res) catch {
        if res.started {
            return false;  // too late to report it: drop the connection
        }
        res.status = 500;
        res.head.clear();
        res.body.clear();
        res.has_type = false;
        res.write("Internal Server Error\n") catch return false;
    };
    if res.upgraded {
        return false;  // the handler has had the connection
    }
    res.finish() catch return false;
    return res.keep_alive;
}

fn new_scratch() Scratch {
    return Scratch.{ blocks: vec.Vec.[[]mut u8].init(), params: [Param.{ name: "", value: "" }; 8], nparams: 0 };
}

fn has_header(headers: []Header, name: []u8) bool {
    _ = find_header(headers, name) catch return false;
    return true;
}

// -----------------------------------------------------------------------------
// Router
// -----------------------------------------------------------------------------

struct Route[C] {
    method: []u8,
    pattern: []u8,
    handler: fn(*mut C, *Request, *mut Response) !void,
}

/// Dispatches requests by method and path pattern. Patterns are paths
/// whose segments may be `{name}` (any one segment, see Request.param) and
/// whose last segment may be `*` (the rest of the path). The first route
/// that matches wins; HEAD requests use GET routes. Unmatched paths get
/// 404, and paths that match only for other methods get 405.
pub struct Router[C] {
    routes: vec.Vec.[Route.[C]],
    ctx: *mut C,

    pub fn init(ctx: *mut C) Self {
        return Self.{ routes: vec.Vec.[Route.[C]].init(), ctx: ctx };
    }

    pub fn deinit(self: *mut Self) void {
        self.routes.deinit();
    }

    /// Adds a route. The method and pattern must outlive the router (string
    /// literals do).
    pub fn route(self: *mut Self, method: []u8, pattern: []u8, h: fn(*mut C, *Request, *mut Response) !void) !void {
        self.routes.push(Route.[C].{ method: method, pattern: pattern, handler: h })!;
    }

    pub fn get(self: *mut Self, pattern: []u8, h: fn(*mut C, *Request, *mut Response) !void) !void {
        self.route("GET", pattern, h)!;
    }

    pub fn post(self: *mut Self, pattern: []u8, h: fn(*mut C, *Request, *mut Response) !void) !void {
        self.route("POST", pattern, h)!;
    }

    pub fn put(self: *mut Self, pattern: []u8, h: fn(*mut C, *Request, *mut Response) !void) !void {
        self.route("PUT", pattern, h)!;
    }

    pub fn delete(self: *mut Self, pattern: []u8, h: fn(*mut C, *Request, *mut Response) !void) !void {
        self.route("DELETE", pattern, h)!;
    }

    /// The handler for Server: routes one request.
    pub fn handle(self: *mut Self, req: *Request, res: *mut Response) !void {
        let mut path_matched = false;
        let mut allow = strs.String.init();
        defer allow.deinit();
        for r in self.routes.items() {
            if not match_pattern(r.pattern, req.path, req.scratch)! {
                continue;
            }
            let method_ok = @mem_eq(r.method, req.method) or (@mem_eq(req.method, "HEAD") and @mem_eq(r.method, "GET"));
            if method_ok {
                return r.handler(self.ctx, req, res);
            }
            path_matched = true;
            if allow.len > 0 {
                allow.push_str(", ")!;
            }
            allow.push_str(r.method)!;
        }
        if path_matched {
            res.header("Allow", allow.bytes())!;
            res.text(405, "Method Not Allowed\n")!;
            return;
        }
        res.text(404, "Not Found\n")!;
    }

    /// Listens on hostname:port and serves requests with this router forever.
    pub fn serve(self: *mut Self, hostname: []u8, port: u16, opts: Options) !void {
        let mut s = server(hostname, port, Self.handle, self, opts)!;
        s.run()!;
    }

    /// Listens on hostname:port and serves HTTPS with this router forever.
    pub fn serve_tls(self: *mut Self, hostname: []u8, port: u16, opts: Options, cfg: *tls.ServerConfig) !void {
        let mut s = server_tls(hostname, port, Self.handle, self, opts, cfg)!;
        s.run()!;
    }
}

/// Whether path matches pattern; the parameters are stored (decoded) in
/// the request's scratch.
fn match_pattern(pattern: []u8, path: []u8, req: *mut Scratch) !bool {
    req.nparams = 0;
    let mut pi = strs.split(pattern, "/");
    let mut si = strs.split(path, "/");
    loop {
        let p = pi.next() catch {
            // the pattern is used up: so must the path be
            _ = si.next() catch return true;
            return false;
        };
        if @mem_eq(p, "*") {
            let rest = si.remainder();
            _ = si.next() catch "";
            return add_param(req, "*", rest);
        }
        let seg = si.next() catch return false;
        if (p.len >= 2) and (p[0] == '{') and (p[p.len - 1] == '}') {
            if seg.len == 0 {
                return false;
            }
            if not add_param(req, p[1..p.len - 1], seg)! {
                return false;
            }
            continue;
        }
        if not @mem_eq(p, seg) {
            return false;
        }
    }
}

fn add_param(req: *mut Scratch, name: []u8, raw: []u8) !bool {
    if req.nparams == MAX_PARAMS {
        return false;
    }
    let out = req.alloc(raw.len)!;
    let v = percent_decode(raw, out, false) catch return false;
    // a parameter names something inside its place: no NUL, no "..", and
    // (except for "*") no "/" smuggled in as %2F
    if strs.contains(v, "\x00") or (not @mem_eq(name, "*") and strs.contains(v, "/")) {
        return false;
    }
    let mut segs = strs.split(v, "/");
    loop {
        let seg = segs.next() catch break;
        if @mem_eq(seg, "..") {
            return false;
        }
    }
    req.params[req.nparams] = Param.{ name: name, value: v };
    req.nparams += 1;
    return true;
}

// -----------------------------------------------------------------------------
// The client
// -----------------------------------------------------------------------------

/// The parts of an http:// or https:// URL.
pub struct Url {
    hostname: []u8,
    port: u16,
    target: []u8,  // path and query, "/" if empty
    secure: bool,  // https
}

/// Parses "http[s]://hostname[:port][/path][?query]" (with std/net/url;
/// no user information or IPv6 addresses, and the scheme in lowercase).
pub fn parse_url(text: []u8) !Url {
    let u = urls.parse(text) catch return error.BadUrl;
    let secure = strs.eq(u.scheme, "https");
    if (not secure and not strs.eq(u.scheme, "http")) or not u.has_authority or u.has_userinfo or u.ip_literal {
        return error.BadUrl;
    }
    // the target is the path and query, which end the text but for a fragment
    let mut end = text.len;
    if u.has_fragment {
        end -= u.fragment.len + 1;
    }
    let mut target = text[end - u.path.len..end];
    if u.has_query {
        target = text[(end - u.query.len) - (u.path.len + 1)..end];
        if u.path.len == 0 {
            return error.BadUrl;  // "http://h?q"
        }
    }
    if target.len == 0 {
        target = "/";
    }
    let mut port: u16 = 80;
    if secure {
        port = 443;
    }
    if u.has_port {
        port = u.port_number() catch return error.BadUrl;  // (also for an empty port)
    }
    if u.hostname.len == 0 {
        return error.BadUrl;
    }
    // a host name or an IPv4 address: letters, digits, '-' and '.'
    for ch in u.hostname {
        if not (strs.is_alnum(ch) or (ch == '-') or (ch == '.')) {
            return error.BadUrl;
        }
    }
    return Url.{ hostname: u.hostname, port: port, target: target, secure: secure };
}

let H2_ALPN: [2][]u8 = ["h2", "http/1.1"];

fn request_h2(t: *mut Transport, u: Url, method: []u8, headers: []Header, body: []u8,
              opts: ClientOptions) !ClientResponse {
    let r = io.Reader.{ ctx: @ptr_cast(*mut void, t), read_fn: transport_read };
    let w = io.Writer.{ ctx: @ptr_cast(*mut void, t), write_fn: transport_write };
    let mut auth = strs.String.init();
    defer auth.deinit();
    auth.push_str(u.hostname)!;
    if ((u.port != 80) and not u.secure) or ((u.port != 443) and u.secure) {
        fmt.write(auth.writer(), ":{}", [fmt.int(u.port)])!;
    }
    let mut scheme: []u8 = "http";
    if u.secure {
        scheme = "https";
    }
    let mut c = http2.Client.init(r, w, auth.bytes(), scheme)!;
    defer c.deinit();
    c.conn.max_body = opts.max_body_bytes;
    // names in lower case, for HTTP/2
    let mut store = strs.String.init();
    defer store.deinit();
    for x in headers {
        for ch in x.name {
            store.push(strs.to_lower_byte(ch))!;
        }
    }
    let mut hs = vec.Vec.[hpack.Header].init();
    defer hs.deinit();
    let mut at: usize = 0;
    let lower = store.bytes();
    for x in headers {
        hs.push(hpack.Header.{ name: lower[at..at + x.name.len], value: x.value })!;
        at += x.name.len;
    }
    let mut resp = c.request(method, u.target, hs.items(), body)!;
    defer resp.deinit();
    // a ClientResponse owning copies
    let mut total: usize = 0;
    for x in resp.headers {
        total += x.name.len + x.value.len;
    }
    let hm = mem.alloc.[u8](total)!;
    let hdrs = mem.alloc.[Header](resp.headers.len) catch |e| {
        mem.free(hm);
        return e;
    };
    let mut k: usize = 0;
    for i in @as(usize, 0)..resp.headers.len {
        let x = resp.headers[i];
        @mem_copy(hm[k..k + x.name.len], x.name);
        @mem_copy(hm[k + x.name.len..k + x.name.len + x.value.len], x.value);
        hdrs[i] = Header.{ name: hm[k..k + x.name.len], value: hm[k + x.name.len..k + x.name.len + x.value.len] };
        k += x.name.len + x.value.len;
    }
    let mut rb = strs.String.init();
    rb.push_str(resp.body) catch |e| {
        mem.free(hm);
        mem.free(hdrs);
        return e;
    };
    return ClientResponse.{ status: resp.status, reason: reason_phrase(resp.status), headers: hdrs, body: rb.bytes(),
                            head_mem: hm, header_mem: hdrs, body_mem: rb };
}

/// A response received by the client. It owns its memory: call deinit.
pub struct ClientResponse {
    status: u16,
    reason: []u8,
    headers: []Header,
    body: []u8,
    head_mem: []mut u8,
    header_mem: []mut Header,
    body_mem: strs.String,

    pub fn header(self: *ClientResponse, name: []u8) ![]u8 {
        return find_header(self.headers, name);
    }

    pub fn deinit(self: *mut ClientResponse) void {
        mem.free(self.head_mem);
        mem.free(self.header_mem);
        self.body_mem.deinit();
    }
}

/// Client limits and timeouts.
pub struct ClientOptions {
    timeout_ms: u32,
    max_body_bytes: usize,
    tls_config: *tls.ClientConfig,  // for https (see with_tls)
    has_tls_config: bool,
    http2: bool,  // offer HTTP/2 (ALPN "h2"; for http://, assume it)

    pub fn default() ClientOptions {
        return ClientOptions.{
            timeout_ms: 30000,
            max_body_bytes: 1073741824,
            tls_config: @ptr_from_int(*tls.ClientConfig, 8),
            has_tls_config: false,
            http2: false,
        };
    }

    /// These options, with HTTP/2 when the server speaks it: https://
    /// servers that agree to "h2" by ALPN; http:// ones are assumed to
    /// speak it ("prior knowledge"), so use this only for those that do.
    pub fn with_http2(self: ClientOptions) ClientOptions {
        let mut o = self;
        o.http2 = true;
        return o;
    }

    /// These options with https:// connections using cfg (which must
    /// outlive the requests) rather than the system's root certificates.
    pub fn with_tls(self: ClientOptions, cfg: *tls.ClientConfig) ClientOptions {
        let mut o = self;
        o.tls_config = cfg;
        o.has_tls_config = true;
        return o;
    }
}

// The configuration for https:// by default, trusting the system's roots,
// made the first time it is needed.
let mut SYSTEM_TLS_LOCK: sync.Mutex = sync.Mutex.{ state: 0 };
let mut SYSTEM_TLS: usize = 0;

pub fn system_tls() !*tls.ClientConfig {
    SYSTEM_TLS_LOCK.lock();
    defer SYSTEM_TLS_LOCK.unlock();
    if SYSTEM_TLS == 0 {
        // kept until the program ends: not a leak
        mem.leaks_ignored(true);
        defer mem.leaks_ignored(false);
        let cfg = tls.ClientConfig.system()!;
        let p = mem.create(cfg) catch |e| {
            let mut c = cfg;
            c.deinit();
            return e;
        };
        SYSTEM_TLS = @int_from_ptr(p);
    }
    return @ptr_from_int(*tls.ClientConfig, SYSTEM_TLS);
}

pub fn get(url: []u8) !ClientResponse {
    return request("GET", url, no_headers(), "", ClientOptions.default());
}

pub fn post(url: []u8, content_type: []u8, body: []u8) !ClientResponse {
    let hs: [1]Header = [Header.{ name: "Content-Type", value: content_type }];
    return request("POST", url, hs[..], body, ClientOptions.default());
}

/// An empty header list, for request().
pub fn no_headers() []Header {
    return @slice_from_ptr(@ptr_from_int(*Header, 8), 0);
}

/// Sends one request on a new connection and reads the whole response.
/// Host, Content-Length and "Connection: close" are added.
pub fn request(method: []u8, url: []u8, headers: []Header, body: []u8, opts: ClientOptions) !ClientResponse {
    let u = parse_url(url)!;
    // nothing a caller passes may change the request's framing
    if method.len == 0 {
        return error.InvalidArgument;
    }
    for ch in method {
        if not is_token_char(ch) {
            return error.InvalidArgument;
        }
    }
    for x in headers {
        if (x.name.len == 0) or strs.eq_ignore_case(x.name, "Host") or strs.eq_ignore_case(x.name, "Content-Length") or
           strs.eq_ignore_case(x.name, "Transfer-Encoding") or strs.eq_ignore_case(x.name, "Connection") {
            return error.InvalidArgument;
        }
        for ch in x.name {
            if not is_token_char(ch) {
                return error.InvalidArgument;
            }
        }
        for ch in x.value {
            if ((ch < 0x20) and (ch != '\t')) or (ch == 0x7f) {
                return error.InvalidArgument;
            }
        }
    }
    let tcp = net.connect(u.hostname, u.port)!;
    tcp.set_timeout(opts.timeout_ms) catch |e| {
        tcp.close();
        return e;
    };
    let mut stream = Transport.plain(tcp);
    let mut use_h2 = opts.http2 and not u.secure;
    if u.secure {
        let mut cfg = opts.tls_config;
        if not opts.has_tls_config {
            cfg = system_tls() catch |e| {
                tcp.close();
                return e;
            };
        }
        // (offering h2 by ALPN on a copy of the configuration)
        let mut c2 = cfg.*;
        if opts.http2 {
            c2.alpn = H2_ALPN[..];
        }
        stream = Transport.over_tls(tls.client(tcp, u.hostname, &c2)!);  // closes tcp on failure
        use_h2 = opts.http2 and strs.eq(stream.tls.protocol(), "h2");
    }
    defer stream.close();
    if use_h2 {
        return request_h2(&mut stream, u, method, headers, body, opts)!;
    }
    // the request head
    let mut h = strs.String.init();
    defer h.deinit();
    let w = h.writer();
    fmt.write(w, "{} {} HTTP/1.1\r\nHost: {}", [fmt.str(method), fmt.str(u.target), fmt.str(u.hostname)])!;
    if ((u.port != 80) and not u.secure) or ((u.port != 443) and u.secure) {
        fmt.write(w, ":{}", [fmt.int(u.port)])!;
    }
    h.push_str("\r\nConnection: close\r\n")!;
    if (body.len > 0) or @mem_eq(method, "POST") or @mem_eq(method, "PUT") {
        fmt.write(w, "Content-Length: {}\r\n", [fmt.int(body.len)])!;
    }
    for x in headers {
        fmt.write(w, "{}: {}\r\n", [fmt.str(x.name), fmt.str(x.value)])!;
    }
    h.push_str("\r\n")!;
    h.push_str(body)!;
    stream.write_all(h.bytes())!;
    // the response
    let mut c = Conn.init(stream, 1048576)!;
    let mut head_len: usize = 0;
    let mut status: u16 = 0;
    loop {
        head_len = c.read_head() catch |e| {
            c.deinit();
            return e;
        };
        if head_len == 0 {
            c.deinit();
            return error.BadResponse;
        }
        status = parse_status(c.buf[c.start..c.start + head_len]) catch |e| {
            c.deinit();
            return e;
        };
        if (status >= 100) and (status < 200) and (status != 101) {
            // an interim response: skip it
            c.start += head_len;
            c.reset();
            continue;
        }
        break;
    }
    let head = c.buf[c.start..c.start + head_len];
    c.start += head_len;
    c.pin = c.start;
    let line_end = strs.index_of_byte(head, '\n')!;
    let hmem = mem.alloc.[Header](100) catch |e| {
        c.deinit();
        return e;
    };
    let nh = parse_headers(head[line_end + 1..], hmem) catch |e| {
        c.deinit();
        mem.free(hmem);
        return e;
    };
    let mut rb = strs.String.init();
    let has_body = not (@mem_eq(method, "HEAD") or (status == 204) or (status == 304));
    if has_body {
        read_response_body(&mut c, hmem[..nh], &mut rb, opts.max_body_bytes) catch |e| {
            c.deinit();
            mem.free(hmem);
            rb.deinit();
            return e;
        };
    }
    let mut reason = head[..line_end];
    if (reason.len > 0) and (reason[reason.len - 1] == '\r') {
        reason = reason[..reason.len - 1];
    }
    reason = reason[@min(reason.len, 13)..];
    return ClientResponse.{
        status: status,
        reason: reason,
        headers: hmem[..nh],
        body: rb.bytes(),
        head_mem: c.buf,
        header_mem: hmem,
        body_mem: rb,
    };
}

fn read_response_body(c: *mut Conn, headers: []Header, out: *mut strs.String, max: usize) !void {
    let fr = framing(headers) catch return error.BadResponse;
    if fr.chunked {
        c.read_chunked(out, max)!;
    } else if fr.has_length {
        if fr.length > max {
            return error.TooLarge;
        }
        c.read_into(out, fr.length)!;
    } else {
        c.read_to_end(out, max)!;
    }
}

/// The status code from "HTTP/1.1 200 OK".
fn parse_status(head: []u8) !u16 {
    if (head.len < 12) or not strs.starts_with(head, "HTTP/1.") or (head[8] != ' ') {
        return error.BadResponse;
    }
    let code = fmt.parse_int.[u16](head[9..12]) catch return error.BadResponse;
    if (code < 100) or (code > 999) {
        return error.BadResponse;
    }
    return code;
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/net/http.ph)
// -----------------------------------------------------------------------------

test "urls and encoding" {
    let u = parse_url("http://example.com:8080/a/b?x=1#frag")!;
    @expect_eq(u.hostname, "example.com");
    @expect_eq(u.port, 8080);
    @expect_eq(u.target, "/a/b?x=1");
    @expect_eq(parse_url("http://h")!.target, "/");
    @expect_eq(parse_url("http://h#frag")!.target, "/");
    @expect_eq(parse_url("http://h/p?q=1#frag")!.target, "/p?q=1");
    @expect_eq(parse_url("http://h")!.port, 80);
    @expect(not parse_url("http://h")!.secure);
    let v = parse_url("https://h.example/x")!;
    @expect(v.secure);
    @expect_eq(v.port, 443);
    @expect_eq(v.hostname, "h.example");
    @expect_eq(parse_url("https://h:8443")!.port, 8443);
    _ = parse_url("ftp://h/") catch |e| {
        @expect_eq(e, error.BadUrl);
        let mut out: [32]u8 = [0; 32];
        @expect_eq(percent_decode("a%20b+c%2F", out[..], true)!, "a b c/");
        @expect_eq(percent_decode("a+b", out[..], false)!, "a+b");
        @expect_eq(query_raw("a=1&bb=x%20y&c", "bb")!, "x%20y");
        @expect_eq(query_raw("a=1&bb=x%20y&c", "c")!, "");
        let mut s = strs.String.init();
        defer s.deinit();
        percent_encode(s.writer(), "a b/\u{e9}")!;
        @expect_eq(s.bytes(), "a%20b%2F%C3%A9");
        return;
    };
    @trap();
}

test "patterns" {
    let mut sc = new_scratch();
    defer sc.blocks.deinit();
    defer sc.clear();
    let req = Request.{
        method: "GET",
        target: "",
        path: "",
        query: "",
        version: Version.HTTP_1_1,
        headers: no_headers(),
        body: "",
        scratch: &mut sc,
    };
    @expect(match_pattern("/", "/", &mut sc)!);
    @expect(not match_pattern("/", "/x", &mut sc)!);
    @expect(match_pattern("/users/{id}", "/users/4%202", &mut sc)!);
    @expect_eq(req.param("id")!, "4 2");
    @expect(not match_pattern("/users/{id}", "/users/", &mut sc)!);
    @expect(not match_pattern("/users/{id}", "/users/1/x", &mut sc)!);
    @expect(match_pattern("/static/*", "/static/css/a.css", &mut sc)!);
    @expect_eq(req.param("*")!, "css/a.css");
    @expect(match_pattern("/a/{x}/b/{y}", "/a/1/b/2", &mut sc)!);
    @expect_eq(req.param("y")!, "2");
}

struct TestApp {
    hits: u32,
}

fn t_hello(app: *mut TestApp, req: *Request, res: *mut Response) !void {
    _ = @atomic_add(&mut app.hits, 1);
    let name = req.query_param("name") catch "world";
    fmt.write(res.writer(), "hello, {}", [fmt.str(name)])!;
}

fn t_user(app: *mut TestApp, req: *Request, res: *mut Response) !void {
    res.header("Content-Type", "application/json")!;
    fmt.write(res.writer(), "{{\"id\":\"{}\"}}", [fmt.str(req.param("id")!)])!;
}

fn t_echo(app: *mut TestApp, req: *Request, res: *mut Response) !void {
    res.set_status(201);
    res.header("X-Length", fmt.bufprint(req.scratch.alloc(20)!, "{}", [fmt.int(req.body.len)])!)!;
    res.write(req.body)!;
}

fn t_big(app: *mut TestApp, req: *Request, res: *mut Response) !void {
    for i in @as(u32, 0)..10000 {
        fmt.write(res.writer(), "line {:05}\n", [fmt.int(i)])!;
    }
}

fn t_fail(app: *mut TestApp, req: *Request, res: *mut Response) !void {
    res.write("partial")!;
    return error.NotFound;
}

fn t_run(s: *mut Server.[Router.[TestApp]]) void {
    s.run() catch @trap();
}

/// Sends raw bytes and reads until the server closes the connection.
fn raw_exchange(port: u16, text: []u8) ![]mut u8 {
    let st = net.connect("127.0.0.1", port)!;
    defer st.close();
    st.write_all(text)!;
    return st.reader().read_all();
}

test "server and client" {
    let mut app = TestApp.{ hits: 0 };
    let mut r = Router.[TestApp].init(&mut app);
    defer r.deinit();
    r.get("/hello", t_hello)!;
    r.get("/users/{id}", t_user)!;
    r.post("/echo", t_echo)!;
    r.get("/big", t_big)!;
    r.get("/fail", t_fail)!;
    let mut opts = Options.default();
    opts.threads = 2;
    opts.max_head_bytes = 4096;
    let mut srv = server("127.0.0.1", 0, Router.[TestApp].handle, &mut r, opts)!;
    let port = srv.port();
    let th = thread.spawn(t_run, &mut srv)!;
    let mut ub: [64]u8 = [0; 64];
    let base = fmt.bufprint(ub[..], "http://127.0.0.1:{}", [fmt.int(port)])!;
    let mut url: [128]u8 = [0; 128];

    let mut a = get(fmt.bufprint(url[..], "{}/hello?name=A%20B", [fmt.str(base)])!)!;
    @expect_eq(a.status, 200);
    @expect_eq(a.reason, "OK");
    @expect_eq(a.body, "hello, A B");
    @expect_eq(a.header("content-type")!, "text/plain; charset=utf-8");
    a.deinit();

    let mut b = get(fmt.bufprint(url[..], "{}/users/42", [fmt.str(base)])!)!;
    @expect_eq(b.body, "{\"id\":\"42\"}");
    @expect_eq(b.header("Content-Type")!, "application/json");
    b.deinit();

    let mut c = post(fmt.bufprint(url[..], "{}/echo", [fmt.str(base)])!, "text/plain", "some data")!;
    @expect_eq(c.status, 201);
    @expect_eq(c.body, "some data");
    @expect_eq(c.header("x-length")!, "9");
    c.deinit();

    let mut d = get(fmt.bufprint(url[..], "{}/nope", [fmt.str(base)])!)!;
    @expect_eq(d.status, 404);
    d.deinit();
    let mut e = post(fmt.bufprint(url[..], "{}/hello", [fmt.str(base)])!, "text/plain", "")!;
    @expect_eq(e.status, 405);
    @expect_eq(e.header("Allow")!, "GET");
    e.deinit();

    // streamed (chunked) response
    let mut f = get(fmt.bufprint(url[..], "{}/big", [fmt.str(base)])!)!;
    @expect_eq(f.body.len, 110000);
    @expect_eq(f.header("Transfer-Encoding")!, "chunked");
    @expect(strs.ends_with(f.body, "line 09999\n"));
    f.deinit();

    let mut g = get(fmt.bufprint(url[..], "{}/fail", [fmt.str(base)])!)!;
    @expect_eq(g.status, 500);
    @expect_eq(g.body, "Internal Server Error\n");
    g.deinit();

    // pipelining and keep-alive on one connection, HEAD, a chunked upload
    let out = raw_exchange(port,
                           "GET /hello HTTP/1.1\r\nHost: x\r\n\r\nHEAD /hello?name=z HTTP/1.1\r\nHost: x\r\n\r\nPOST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n3\r\nabc\r\n2;ext=1\r\nde\r\n0\r\n\r\n")!;
    defer mem.free(out);
    @expect_eq(strs.count(out, "HTTP/1.1 200 OK"), 2);
    @expect_eq(strs.count(out, "hello, world"), 1);
    @expect(strs.contains(out, "Content-Length: 8\r\n"));
    @expect(strs.contains(out, "HTTP/1.1 201 Created"));
    @expect(strs.ends_with(out, "\r\n\r\nabcde"));
    @expect(strs.contains(out, "Connection: close"));

    // HTTP/1.0 closes by default
    let o2 = raw_exchange(port, "GET /users/7 HTTP/1.0\r\n\r\n")!;
    defer mem.free(o2);
    @expect(strs.starts_with(o2, "HTTP/1.0 200 OK\r\n"));
    @expect(strs.ends_with(o2, "{\"id\":\"7\"}"));

    // malformed and oversized requests
    let o3 = raw_exchange(port, "GARBAGE\r\n\r\n")!;
    defer mem.free(o3);
    @expect(strs.starts_with(o3, "HTTP/1.1 400 Bad Request"));
    let o4 = raw_exchange(port, "GET / HTTP/1.1\r\n\r\n")!;
    defer mem.free(o4);
    @expect(strs.starts_with(o4, "HTTP/1.1 400 "));
    let huge = strs.repeat("X-Filler: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n", 100)!;
    defer mem.free(huge);
    let req_text = strs.concat("GET / HTTP/1.1\r\nHost: x\r\n", huge)!;
    defer mem.free(req_text);
    let o5 = raw_exchange(port, req_text)!;
    defer mem.free(o5);
    @expect(strs.starts_with(o5, "HTTP/1.1 431 "));
    let o6 = raw_exchange(port, "GET / HTTP/2.0\r\nHost: x\r\n\r\n")!;
    defer mem.free(o6);
    @expect(strs.starts_with(o6, "HTTP/1.1 505 "));
    let o7 = raw_exchange(port,
                          "POST /echo HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nTransfer-Encoding: chunked\r\n\r\n")!;
    defer mem.free(o7);
    @expect(strs.starts_with(o7, "HTTP/1.1 400 "));

    // framing that a proxy could read differently is refused (request smuggling)
    let smuggles: [9][]u8 = [
        "POST /echo HTTP/1.1\r\nHost: x\r\nContent-Length: +3\r\n\r\nabc",
        "POST /echo HTTP/1.1\r\nHost: x\r\nContent-Length: 3 3\r\n\r\nabc",
        "POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: \x0bchunked\r\n\r\n0\r\n\r\n",
        "POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n 3\r\nabc\r\n0\r\n\r\n",
        "POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFFFF\r\n",
        "POST /echo HTTP/1.0\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n",
        "GET /hello HTTP/1.1\r\nHost: x\r\nHost: y\r\n\r\n",
        "GET /hello HTTP/1.1\r\nHost: x\r\nX-A: a\rb\r\n\r\n",
        "GET /hello HTTP/1.1\r\nHost: x\r\nX-A: a\x00b\r\n\r\n",
    ];
    for sm in smuggles {
        let o = raw_exchange(port, sm)!;
        defer mem.free(o);
        @expect(strs.starts_with(o, "HTTP/1.1 4") or strs.starts_with(o, "HTTP/1.1 501"));
    }
    // route parameters cannot climb out of their place
    let o8 = raw_exchange(port, "GET /users/..%2Fetc HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")!;
    defer mem.free(o8);
    @expect(strs.starts_with(o8, "HTTP/1.1 404 "));
    let o9 = raw_exchange(port, "GET /users/%2E%2E HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")!;
    defer mem.free(o9);
    @expect(strs.starts_with(o9, "HTTP/1.1 404 "));
    // the client will not send requests its caller could have split
    let bad_url = fmt.bufprint(url[..], "{}/a HTTP/1.1\r\nX: 1\r\n\r\nGET /b", [fmt.str(base)])!;
    @expect(failed(get(bad_url)));
    let hdrs: [1]Header = [Header.{ name: "X-Evil", value: "1\r\nContent-Length: 0" }];
    @expect(failed(request("GET", fmt.bufprint(url[..], "{}/hello", [fmt.str(base)])!, hdrs[..], "",
                           ClientOptions.default())));
    let hdrs2: [1]Header = [Header.{ name: "Transfer-Encoding", value: "chunked" }];
    @expect(failed(request("GET", fmt.bufprint(url[..], "{}/hello", [fmt.str(base)])!, hdrs2[..], "",
                           ClientOptions.default())));
    @expect(failed(get("http://internal\x00.example.com/")));

    @expect_eq(@atomic_load(&app.hits), 3);
    srv.stop();
    th.join();
}

fn failed(r: !ClientResponse) bool {
    let mut v = r catch return true;
    v.deinit();
    return false;
}

test "https" {
    let cert = tls.test_certificate_pem()!;
    defer mem.free(cert);
    let key = tls.test_key_pem()!;
    defer mem.free(key);
    let mut scfg = tls.ServerConfig.from_pem(cert, key)!;
    defer scfg.deinit();
    let mut app = TestApp.{ hits: 0 };
    let mut r = Router.[TestApp].init(&mut app);
    defer r.deinit();
    r.get("/hello", t_hello)!;
    r.post("/echo", t_echo)!;
    r.get("/big", t_big)!;
    let mut opts = Options.default();
    opts.threads = 2;
    let mut srv = server_tls("127.0.0.1", 0, Router.[TestApp].handle, &mut r, opts, &scfg)!;
    let port = srv.port();
    let th = thread.spawn(t_run, &mut srv)!;
    let mut ccfg = tls.ClientConfig.with_roots_pem(cert)!;
    defer ccfg.deinit();
    let copts = ClientOptions.default().with_tls(&ccfg);
    let mut url: [128]u8 = [0; 128];

    let mut a = request("GET", fmt.bufprint(url[..], "https://localhost:{}/hello?name=tls", [fmt.int(port)])!,
                        no_headers(), "", copts)!;
    @expect_eq(a.status, 200);
    @expect_eq(a.body, "hello, tls");
    a.deinit();

    let mut b = request("POST", fmt.bufprint(url[..], "https://127.0.0.1:{}/echo", [fmt.int(port)])!, no_headers(),
                        "over tls", copts)!;
    @expect_eq(b.status, 201);
    @expect_eq(b.body, "over tls");
    b.deinit();

    let mut c = request("GET", fmt.bufprint(url[..], "https://localhost:{}/big", [fmt.int(port)])!, no_headers(), "",
                        copts)!;
    @expect_eq(c.body.len, 110000);
    c.deinit();

    // a client that doesn't trust the certificate; plain HTTP to a TLS port
    let mut other = tls.ClientConfig.with_roots_pem("")!;
    defer other.deinit();
    @expect(fails(request("GET", fmt.bufprint(url[..], "https://localhost:{}/hello", [fmt.int(port)])!, no_headers(),
                          "", ClientOptions.default().with_tls(&other))));
    // the system's roots don't include it either
    @expect(fails(get(fmt.bufprint(url[..], "https://localhost:{}/hello", [fmt.int(port)])!)));
    let mut plain_opts = ClientOptions.default();
    plain_opts.timeout_ms = 5000;
    @expect(fails(request("GET", fmt.bufprint(url[..], "http://localhost:{}/hello", [fmt.int(port)])!, no_headers(), "",
                          plain_opts)));

    srv.stop();
    th.join();
}

fn fails[T](r: !T) bool {
    _ = r catch return true;
    return false;
}

fn stream_read(ctx: *mut void, buf: []mut u8) !usize {
    let t = @ptr_cast(*mut net.TcpStream, ctx);
    return t.read(buf)!;
}

fn stream_write(ctx: *mut void, bytes: []u8) !usize {
    let t = @ptr_cast(*mut net.TcpStream, ctx);
    return t.write(bytes)!;
}

test "HTTP/2" {
    let mut app = TestApp.{ hits: 0 };
    let mut r = Router.[TestApp].init(&mut app);
    defer r.deinit();
    r.get("/hello", t_hello)!;
    r.post("/echo", t_echo)!;
    r.get("/big", t_big)!;
    let mut opts = Options.default();
    opts.threads = 2;
    opts.max_body_bytes = 8388608;
    let mut srv = server("127.0.0.1", 0, Router.[TestApp].handle, &mut r, opts)!;
    let port = srv.port();
    let th = thread.spawn(t_run, &mut srv)!;
    let mut url: [128]u8 = [0; 128];
    let h2 = ClientOptions.default().with_http2();
    // prior knowledge, on a plain connection
    let mut a = request("GET", fmt.bufprint(url[..], "http://127.0.0.1:{}/hello?name=h2", [fmt.int(port)])!,
                        no_headers(), "", h2)!;
    @expect_eq(a.status, 200);
    @expect_eq(a.body, "hello, h2");
    @expect_eq(a.header("content-type")!, "text/plain; charset=utf-8");
    a.deinit();
    let mut c = request("GET", fmt.bufprint(url[..], "http://127.0.0.1:{}/big", [fmt.int(port)])!, no_headers(), "",
                        h2)!;
    @expect_eq(c.body.len, 110000);
    c.deinit();
    // a body bigger than the windows: flow control has to open them
    let big = mem.alloc.[u8](3000000)!;
    defer mem.free(big);
    for k in @as(usize, 0)..big.len {
        big[k] = @truncate(u8, k * 13);
    }
    let mut e = request("POST", fmt.bufprint(url[..], "http://127.0.0.1:{}/echo", [fmt.int(port)])!, no_headers(), big,
                        h2)!;
    @expect(e.status == 201);
    @expect(@mem_eq(e.body, big));
    @expect_eq(e.header("x-length")!, "3000000");
    e.deinit();
    // several requests on one connection
    let mut tcp = net.connect("127.0.0.1", port)!;
    let rd = io.Reader.{ ctx: @ptr_cast(*mut void, &mut tcp), read_fn: stream_read };
    let wr = io.Writer.{ ctx: @ptr_cast(*mut void, &mut tcp), write_fn: stream_write };
    let mut cl = http2.Client.init(rd, wr, "127.0.0.1", "http")!;
    for k in @as(usize, 0)..5 {
        let mut resp = cl.request("GET", "/hello?name=again", no_h2_headers(), "")!;
        @expect((resp.status == 200) and strs.eq(resp.body, "hello, again"));
        resp.deinit();
        _ = k;
    }
    let mut nf = cl.request("GET", "/nowhere", no_h2_headers(), "")!;
    @expect(nf.status == 404);
    nf.deinit();
    cl.deinit();
    tcp.close();
    // curl, where it is installed (nghttp2's HTTP/2)
    let cargs: [6][]u8 = [
        "curl", "-s", "--http2-prior-knowledge", "-w", " %{http_version}", fmt.bufprint(
            url[..], "http://127.0.0.1:{}/hello?name=curl", [fmt.int(port)])!];
    let mut out = process.run(cargs[..]) catch process.Output.{ status: process.Status.{ code: -1, signal: 0 },
                                                                stdout: mem.alloc.[u8](0)!,
                                                                stderr: mem.alloc.[u8](0)! };
    if out.status.success() {
        @expect_eq(out.stdout, "hello, curl 2");
    }
    out.deinit();
    srv.stop();
    th.join();
}

fn no_h2_headers() []hpack.Header {
    return NO_H2[..0];
}

let NO_H2: [1]hpack.Header = [hpack.Header.{ name: "", value: "" }];

let H2_SERVER_ALPN: [2][]u8 = ["h2", "http/1.1"];

test "HTTP/2 over TLS" {
    let cert = tls.test_certificate_pem()!;
    defer mem.free(cert);
    let key = tls.test_key_pem()!;
    defer mem.free(key);
    let mut scfg = tls.ServerConfig.from_pem(cert, key)!;
    defer scfg.deinit();
    scfg.alpn = H2_SERVER_ALPN[..];
    let mut app = TestApp.{ hits: 0 };
    let mut r = Router.[TestApp].init(&mut app);
    defer r.deinit();
    r.get("/hello", t_hello)!;
    let mut opts = Options.default();
    opts.threads = 2;
    let mut srv = server_tls("127.0.0.1", 0, Router.[TestApp].handle, &mut r, opts, &scfg)!;
    let port = srv.port();
    let th = thread.spawn(t_run, &mut srv)!;
    let mut ccfg = tls.ClientConfig.with_roots_pem(cert)!;
    defer ccfg.deinit();
    let mut url: [128]u8 = [0; 128];
    // with and without h2 offered
    let mut a = request("GET", fmt.bufprint(url[..], "https://localhost:{}/hello?name=alpn", [fmt.int(port)])!,
                        no_headers(), "", ClientOptions.default().with_tls(&ccfg).with_http2())!;
    @expect_eq(a.body, "hello, alpn");
    a.deinit();
    let mut b = request("GET", fmt.bufprint(url[..], "https://localhost:{}/hello?name=h1", [fmt.int(port)])!,
                        no_headers(), "", ClientOptions.default().with_tls(&ccfg))!;
    @expect_eq(b.body, "hello, h1");
    b.deinit();
    let cargs2: [6][]u8 = [
        "curl", "-sk", "--http2", "-w", " %{http_version}", fmt.bufprint(
            url[..], "https://localhost:{}/hello?name=curl", [fmt.int(port)])!];
    let mut out = process.run(cargs2[..]) catch process.Output.{ status: process.Status.{ code: -1, signal: 0 },
                                                                 stdout: mem.alloc.[u8](0)!,
                                                                 stderr: mem.alloc.[u8](0)! };
    if out.status.success() {
        @expect_eq(out.stdout, "hello, curl 2");
    }
    out.deinit();
    srv.stop();
    th.join();
}
