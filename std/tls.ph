// =============================================================================
// std/tls.ph - TLS 1.3 (RFC 8446), client and server.
//
//     let mut cfg = tls.ClientConfig.system()!;    // the OS's root CAs
//     defer cfg.deinit();
//     let conn = tls.connect("example.com", 443, &cfg)!;
//     defer conn.close();
//     conn.write_all("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n")!;
//     let n = conn.read(buf[..])!;                  // 0 at the end
//
//     let mut scfg = tls.ServerConfig.from_pem(cert_chain_pem, key_pem)!;
//     let conn = tls.server(stream, &scfg)!;       // after listener.accept()
//
// Only TLS 1.3 is spoken, with the cipher suites TLS_CHACHA20_POLY1305_SHA256,
// TLS_AES_128_GCM_SHA256 and TLS_AES_256_GCM_SHA384, key exchange over
// X25519, P-256 or P-384, and certificates signed with Ed25519, ECDSA
// (P-256, P-384) or RSA (PSS; PKCS #1 v1.5 within certificates). Clients
// verify the server's certificate chain and host name (std/crypto/x509)
// unless told not to. There is no session resumption or 0-RTT data, and a
// client asked for a certificate answers with none.
//
// A Conn is used by one thread at a time; ClientConfig and ServerConfig may
// be shared by many connections at once.
// =============================================================================

import "crypto/aes_gcm.ph" as aes_gcm;
import "crypto/chacha20poly1305.ph" as chacha;
import "crypto/curve25519.ph" as curve25519;
import "crypto/ec.ph" as ec;
import "crypto/hmac.ph" as hmac;
import "crypto/random.ph" as random;
import "crypto/sha2.ph" as sha2;
import "crypto/subtle.ph" as subtle;
import "crypto/x509.ph" as x509;
import "io.ph" as io;
import "mem.ph" as mem;
import "net.ph" as net;
import "str.ph" as strs;
import "thread.ph" as thread;
import "time.ph" as time;
import "vec.ph" as vec;

error {
    HandshakeFailure,
    DecodeError,
    UnexpectedMessage,
    BadRecordMac,
    RecordOverflow,
    ProtocolVersion,
    IllegalParameter,
    AlertReceived,
    BadCertificate,
    Closed,
    NoCertificate,
    InvalidArgument,
    TimedOut,
    OutOfMemory,
}

// Record content types
const CT_CHANGE_CIPHER_SPEC: u8 = 20;
const CT_ALERT: u8 = 21;
const CT_HANDSHAKE: u8 = 22;
const CT_APPLICATION_DATA: u8 = 23;

// Handshake message types
const HS_CLIENT_HELLO: u8 = 1;
const HS_SERVER_HELLO: u8 = 2;
const HS_NEW_SESSION_TICKET: u8 = 4;
const HS_ENCRYPTED_EXTENSIONS: u8 = 8;
const HS_CERTIFICATE: u8 = 11;
const HS_CERTIFICATE_REQUEST: u8 = 13;
const HS_CERTIFICATE_VERIFY: u8 = 15;
const HS_FINISHED: u8 = 20;
const HS_KEY_UPDATE: u8 = 24;
const HS_MESSAGE_HASH: u8 = 254;

// Extensions
const EXT_SERVER_NAME: u16 = 0;
const EXT_SUPPORTED_GROUPS: u16 = 10;
const EXT_SIGNATURE_ALGORITHMS: u16 = 13;
const EXT_ALPN: u16 = 16;
const EXT_SUPPORTED_VERSIONS: u16 = 43;
const EXT_COOKIE: u16 = 44;
const EXT_KEY_SHARE: u16 = 51;

// Cipher suites
const TLS_AES_128_GCM_SHA256: u16 = 0x1301;
const TLS_AES_256_GCM_SHA384: u16 = 0x1302;
const TLS_CHACHA20_POLY1305_SHA256: u16 = 0x1303;

// Groups
const GROUP_P256: u16 = 0x0017;
const GROUP_P384: u16 = 0x0018;
const GROUP_X25519: u16 = 0x001d;

// Signature schemes
const SIG_RSA_PKCS1_SHA256: u16 = 0x0401;
const SIG_RSA_PKCS1_SHA384: u16 = 0x0501;
const SIG_RSA_PKCS1_SHA512: u16 = 0x0601;
const SIG_ECDSA_P256_SHA256: u16 = 0x0403;
const SIG_ECDSA_P384_SHA384: u16 = 0x0503;
const SIG_RSA_PSS_SHA256: u16 = 0x0804;
const SIG_RSA_PSS_SHA384: u16 = 0x0805;
const SIG_RSA_PSS_SHA512: u16 = 0x0806;
const SIG_ED25519: u16 = 0x0807;

// Alerts
const ALERT_CLOSE_NOTIFY: u8 = 0;
const ALERT_UNEXPECTED_MESSAGE: u8 = 10;
const ALERT_BAD_RECORD_MAC: u8 = 20;
const ALERT_HANDSHAKE_FAILURE: u8 = 40;
const ALERT_BAD_CERTIFICATE: u8 = 42;
const ALERT_ILLEGAL_PARAMETER: u8 = 47;
const ALERT_RECORD_OVERFLOW: u8 = 22;
const ALERT_DECODE_ERROR: u8 = 50;
const ALERT_DECRYPT_ERROR: u8 = 51;
const ALERT_PROTOCOL_VERSION: u8 = 70;
const ALERT_INTERNAL_ERROR: u8 = 80;
const ALERT_MISSING_EXTENSION: u8 = 109;
const ALERT_NO_APPLICATION_PROTOCOL: u8 = 120;

const MAX_PLAINTEXT: usize = 16384;
const MAX_CIPHERTEXT: usize = 16384 + 256;

/// The HelloRetryRequest's special ServerHello.random.
let HRR_RANDOM: [32]u8 = [
    0xcf,
    0x21,
    0xad,
    0x74,
    0xe5,
    0x9a,
    0x61,
    0x11,
    0xbe,
    0x1d,
    0x8c,
    0x02,
    0x1e,
    0x65,
    0xb8,
    0x91,
    0xc2,
    0xa2,
    0x11,
    0x16,
    0x7a,
    0xbb,
    0x8c,
    0x5e,
    0x07,
    0x9e,
    0x09,
    0xe2,
    0xc8,
    0xa8,
    0x33,
    0x9c,
];

// -----------------------------------------------------------------------------
// Reading and writing message fields
// -----------------------------------------------------------------------------

/// A cursor over received bytes; every read fails with DecodeError when
/// the data runs out.
struct Cur {
    b: []u8,
    i: usize,

    fn get8(self: *mut Cur) !u8 {
        if self.i >= self.b.len {
            return error.DecodeError;
        }
        self.i += 1;
        return self.b[self.i - 1];
    }

    fn get16(self: *mut Cur) !u16 {
        let hi = self.get8()!;
        let lo = self.get8()!;
        return (@as(u16, hi) << 8) | @as(u16, lo);
    }

    fn get24(self: *mut Cur) !usize {
        let a = self.get8()!;
        let b = self.get16()!;
        return (@as(usize, a) << 16) | @as(usize, b);
    }

    fn bytes(self: *mut Cur, n: usize) ![]u8 {
        if n > (self.b.len - self.i) {
            return error.DecodeError;
        }
        self.i += n;
        return self.b[self.i - n..self.i];
    }

    fn vec8(self: *mut Cur) ![]u8 {
        let n = @as(usize, self.get8()!);
        return self.bytes(n);
    }

    fn vec16(self: *mut Cur) ![]u8 {
        let n = @as(usize, self.get16()!);
        return self.bytes(n);
    }

    fn vec24(self: *mut Cur) ![]u8 {
        let n = self.get24()!;
        return self.bytes(n);
    }

    fn done(self: *Cur) bool {
        return self.i >= self.b.len;
    }
}

fn cur(b: []u8) Cur {
    return Cur.{ b: b, i: 0 };
}

/// Appends big-endian integers and length-prefixed vectors.
struct Out {
    s: strs.String,

    fn put8(self: *mut Out, v: u8) !void {
        self.s.push(v)!;
    }

    fn put16(self: *mut Out, v: u16) !void {
        self.s.push(@truncate(u8, v >> 8))!;
        self.s.push(@truncate(u8, v))!;
    }

    fn put24(self: *mut Out, v: usize) !void {
        self.s.push(@truncate(u8, v >> 16))!;
        self.s.push(@truncate(u8, v >> 8))!;
        self.s.push(@truncate(u8, v))!;
    }

    fn put(self: *mut Out, b: []u8) !void {
        self.s.push_str(b)!;
    }

    /// Starts a vector with an n-byte length; finish it with close.
    fn open(self: *mut Out, n: usize) !usize {
        let at = self.s.len;
        for i in 0..n {
            self.s.push(0)!;
        }
        return at;
    }

    fn close(self: *mut Out, at: usize, n: usize) void {
        let len = (self.s.len - at) - n;
        let b = self.s.bytes();
        for i in 0..n {
            b[at + i] = @truncate(u8, len >> @truncate(u32, 8 * ((n - 1) - i)));
        }
    }

    fn bytes(self: *Out) []u8 {
        return self.s.bytes();
    }
}

fn out() Out {
    return Out.{ s: strs.String.init() };
}

// -----------------------------------------------------------------------------
// Cipher suites, keys and the key schedule
// -----------------------------------------------------------------------------

struct Suite {
    id: u16,
    hash: u8,  // sha2.SHA256 or sha2.SHA384
    key_len: usize,
}

fn suite_of(id: u16) !Suite {
    if id == TLS_CHACHA20_POLY1305_SHA256 {
        return Suite.{ id: id, hash: sha2.SHA256, key_len: 32 };
    }
    if id == TLS_AES_128_GCM_SHA256 {
        return Suite.{ id: id, hash: sha2.SHA256, key_len: 16 };
    }
    if id == TLS_AES_256_GCM_SHA384 {
        return Suite.{ id: id, hash: sha2.SHA384, key_len: 32 };
    }
    return error.HandshakeFailure;
}

/// HKDF-Expand-Label (RFC 8446 7.1).
fn expand_label(hash: u8, secret: []u8, label: []u8, context: []u8, out_buf: []mut u8) void {
    let mut info: [300]u8 = [0; 300];
    let n = out_buf.len;
    info[0] = @truncate(u8, n >> 8);
    info[1] = @truncate(u8, n);
    info[2] = @truncate(u8, 6 + label.len);
    @mem_copy(info[3..9], "tls13 ");
    @mem_copy(info[9..9 + label.len], label);
    let at = 9 + label.len;
    info[at] = @truncate(u8, context.len);
    @mem_copy(info[at + 1..(at + 1) + context.len], context);
    hmac.expand(hash, secret, info[..(at + 1) + context.len], out_buf);
}

/// One direction's record protection.
struct Cipher {
    suite: u16,
    active: bool,
    key: [32]u8,
    iv: [12]u8,
    seq: u64,
    gcm: aes_gcm.AesGcm,

    fn none() Cipher {
        return Cipher.{ suite: 0, active: false, key: [0; 32], iv: [0; 12], seq: 0,
                        gcm: aes_gcm.AesGcm.init("0123456789abcdef") catch @trap() };
    }

    /// Keys from a traffic secret.
    fn set(self: *mut Cipher, s: Suite, secret: []u8) !void {
        self.suite = s.id;
        self.active = true;
        self.seq = 0;
        @mem_set(self.key[..], 0);
        expand_label(s.hash, secret, "key", "", self.key[..s.key_len]);
        expand_label(s.hash, secret, "iv", "", self.iv[..]);
        if s.id != TLS_CHACHA20_POLY1305_SHA256 {
            self.gcm = aes_gcm.AesGcm.init(self.key[..s.key_len])!;
        }
    }

    fn nonce(self: *Cipher, out_buf: []mut u8) void {
        @mem_copy(out_buf[..12], self.iv[..]);
        for i in @as(usize, 0)..8 {
            out_buf[11 - i] ^= @truncate(u8, self.seq >> @truncate(u32, 8 * i));
        }
    }

    fn seal(self: *mut Cipher, aad: []u8, plain: []u8, sealed: []mut u8) !void {
        let mut n: [12]u8 = [0; 12];
        self.nonce(n[..]);
        if self.suite == TLS_CHACHA20_POLY1305_SHA256 {
            chacha.seal(self.key[..], n[..], aad, plain, sealed)!;
        } else {
            self.gcm.seal(n[..], aad, plain, sealed)!;
        }
        self.seq += 1;
    }

    fn open(self: *mut Cipher, aad: []u8, sealed: []u8, plain: []mut u8) !void {
        let mut n: [12]u8 = [0; 12];
        self.nonce(n[..]);
        if self.suite == TLS_CHACHA20_POLY1305_SHA256 {
            chacha.open(self.key[..], n[..], aad, sealed, plain) catch return error.BadRecordMac;
        } else {
            self.gcm.open(n[..], aad, sealed, plain) catch return error.BadRecordMac;
        }
        self.seq += 1;
    }
}

/// The running transcript hash, over the ClientHello (and anything before
/// the suite is known) and every handshake message after it.
struct Transcript {
    started: bool,
    pending: strs.String,  // messages seen before the hash was chosen
    h: sha2.Hasher,

    fn init() Transcript {
        return Transcript.{ started: false, pending: strs.String.init(), h: sha2.Hasher.init(sha2.SHA256) };
    }

    fn deinit(self: *mut Transcript) void {
        self.pending.deinit();
    }

    fn add(self: *mut Transcript, msg: []u8) !void {
        if self.started {
            self.h.update(msg);
        } else {
            self.pending.push_str(msg)!;
        }
    }

    fn start(self: *mut Transcript, hash: u8) void {
        self.h = sha2.Hasher.init(hash);
        self.h.update(self.pending.bytes());
        self.started = true;
    }

    /// The hash of everything so far (the running hash continues).
    fn current(self: *Transcript, out_buf: []mut u8) []u8 {
        let mut copy = self.h;
        copy.final(out_buf);
        return out_buf[..self.h.size()];
    }
}

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

/// Settings for clients.
pub struct ClientConfig {
    roots: x509.CertPool,
    verify: bool,  // check the server's certificate (keep this on)
    alpn: [][]u8,  // protocols to offer, e.g. ["h2", "http/1.1"]

    /// Trusts the operating system's root certificates.
    pub fn system() !ClientConfig {
        let mut c = ClientConfig.{ roots: x509.CertPool.init(), verify: true, alpn: no_protocols() };
        _ = c.roots.load_system() catch |e| {
            c.roots.deinit();
            return e;
        };
        return c;
    }

    /// Trusts the certificates of a PEM text only.
    pub fn with_roots_pem(pem: []u8) !ClientConfig {
        let mut c = ClientConfig.{ roots: x509.CertPool.init(), verify: true, alpn: no_protocols() };
        _ = c.roots.add_pem(pem) catch |e| {
            c.roots.deinit();
            return e;
        };
        return c;
    }

    /// Accepts any server certificate: for testing only, as it allows
    /// anyone in the middle to read and change the traffic.
    pub fn insecure() ClientConfig {
        return ClientConfig.{ roots: x509.CertPool.init(), verify: false, alpn: no_protocols() };
    }

    pub fn deinit(self: *mut ClientConfig) void {
        self.roots.deinit();
    }
}

fn no_protocols() [][]u8 {
    return @slice_from_ptr(@ptr_from_int(*[]u8, 8), 0);
}

/// Settings for servers: the certificate chain (leaf first) and its key.
pub struct ServerConfig {
    chain: vec.Vec.[[]mut u8],  // DER certificates
    key: x509.PrivateKey,
    key_kind: u8,  // x509.KEY_*
    alpn: [][]u8,  // protocols accepted, in order of preference

    /// From PEM texts: the certificates (leaf first) and the private key.
    pub fn from_pem(cert_pem: []u8, key_pem: []u8) !ServerConfig {
        let mut chain = vec.Vec.[[]mut u8].init();
        let mut it = x509.pem_blocks(cert_pem);
        loop {
            let b = it.next() catch break;
            if @mem_eq(b.label, "CERTIFICATE") {
                let d = x509.pem_decode(b) catch |e| {
                    free_chain(&mut chain);
                    return e;
                };
                chain.push(d) catch |e| {
                    mem.free(d);
                    free_chain(&mut chain);
                    return e;
                };
            }
        }
        if chain.len == 0 {
            free_chain(&mut chain);
            return error.NoCertificate;
        }
        let key = x509.parse_private_key_pem(key_pem) catch |e| {
            free_chain(&mut chain);
            return e;
        };
        let mut kind: u8 = 0;
        match key {
            .Rsa(k)     => { kind = x509.KEY_RSA; }
            .P256(k)    => { kind = x509.KEY_P256; }
            .P384(k)    => { kind = x509.KEY_P384; }
            .Ed25519(k) => { kind = x509.KEY_ED25519; }
        }
        return ServerConfig.{ chain: chain, key: key, key_kind: kind, alpn: no_protocols() };
    }

    pub fn deinit(self: *mut ServerConfig) void {
        free_chain(&mut self.chain);
        self.key.deinit();
    }
}

fn free_chain(c: *mut vec.Vec.[[]mut u8]) void {
    for d in c.items() {
        mem.free(d);
    }
    c.deinit();
}

// -----------------------------------------------------------------------------
// Connections
// -----------------------------------------------------------------------------

pub struct Conn {
    stream: net.TcpStream,
    is_client: bool,
    suite: Suite,
    rd: Cipher,
    wr: Cipher,
    // records received: raw bytes in inbuf[..inlen]
    inbuf: [16645]u8,
    inlen: usize,
    // the plaintext of the last record
    plain: [16645]u8,
    plain_start: usize,
    plain_end: usize,
    plain_type: u8,
    // handshake bytes not yet made into messages
    hs: strs.String,
    hs_start: usize,
    // application data received and not yet read
    app_start: usize,
    app_end: usize,
    closed_read: bool,
    closed_write: bool,
    secret_rd: [48]u8,  // the current traffic secrets, for KeyUpdate
    secret_wr: [48]u8,
    alpn_selected: [32]u8,
    alpn_len: usize,
    sni: [256]u8,  // the server name (client: requested; server: received)
    sni_len: usize,
    established: bool,  // the handshake is over
    deadline: i64,      // monotonic ns by which the handshake must be done (0: none)

    /// Reads decrypted application data; 0 means the peer closed the
    /// connection (with close_notify).
    pub fn read(self: *mut Conn, buf: []mut u8) !usize {
        loop {
            if self.app_start < self.app_end {
                let n = @min(buf.len, self.app_end - self.app_start);
                @mem_copy(buf[..n], self.plain[self.app_start..self.app_start + n]);
                self.app_start += n;
                return n;
            }
            if self.closed_read {
                return 0;
            }
            let ty = self.read_record() catch |e| {
                if (e == error.Closed) and self.closed_read {
                    return 0;  // close_notify
                }
                return e;
            };
            if ty == CT_APPLICATION_DATA {
                self.app_start = self.plain_start;
                self.app_end = self.plain_end;
            } else if ty == CT_HANDSHAKE {
                self.post_handshake()!;
            } else {
                return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
            }
        }
    }

    /// Encrypts and sends bytes (in records of at most 16 KiB).
    pub fn write(self: *mut Conn, bytes: []u8) !usize {
        if self.closed_write {
            return error.Closed;
        }
        let n = @min(bytes.len, MAX_PLAINTEXT);
        self.send_record(CT_APPLICATION_DATA, bytes[..n])!;
        return n;
    }

    pub fn write_all(self: *mut Conn, bytes: []u8) !void {
        let mut rest = bytes;
        while rest.len > 0 {
            let n = self.write(rest)!;
            rest = rest[n..];
        }
    }

    pub fn reader(self: *mut Conn) io.Reader {
        return io.Reader.{ ctx: @ptr_cast(*mut void, self), read_fn: conn_read };
    }

    pub fn writer(self: *mut Conn) io.Writer {
        return io.Writer.{ ctx: @ptr_cast(*mut void, self), write_fn: conn_write };
    }

    /// The protocol chosen by ALPN ("" if none).
    pub fn protocol(self: *Conn) []u8 {
        return self.alpn_selected[..self.alpn_len];
    }

    /// The server name: the one the client asked for (SNI).
    pub fn server_name(self: *Conn) []u8 {
        return self.sni[..self.sni_len];
    }

    /// Sends close_notify, closes the socket and frees the connection.
    pub fn close(self: *mut Conn) void {
        if not self.closed_write {
            let a: [2]u8 = [1, ALERT_CLOSE_NOTIFY];
            self.send_record(CT_ALERT, a[..]) catch {};
            self.closed_write = true;
        }
        self.stream.close();
        self.hs.deinit();
        mem.destroy(self);
    }

    // --- records -------------------------------------------------------------

    /// Sends one record, encrypted once keys are set.
    fn send_record(self: *mut Conn, ty: u8, data: []u8) !void {
        let mut rec: [16645]u8 = [0; 16645];
        if not self.wr.active {
            rec[0] = ty;
            rec[1] = 3;
            rec[2] = 3;
            rec[3] = @truncate(u8, data.len >> 8);
            rec[4] = @truncate(u8, data.len);
            @mem_copy(rec[5..5 + data.len], data);
            self.stream.write_all(rec[..5 + data.len])!;
            return;
        }
        // TLSInnerPlaintext: content || type, sealed
        let mut inner: [16385]u8 = [0; 16385];
        @mem_copy(inner[..data.len], data);
        inner[data.len] = ty;
        let clen = (data.len + 1) + 16;
        rec[0] = CT_APPLICATION_DATA;
        rec[1] = 3;
        rec[2] = 3;
        rec[3] = @truncate(u8, clen >> 8);
        rec[4] = @truncate(u8, clen);
        let hdr: [5]u8 = [rec[0], rec[1], rec[2], rec[3], rec[4]];
        self.wr.seal(hdr[..], inner[..data.len + 1], rec[5..5 + clen])!;
        self.stream.write_all(rec[..5 + clen])!;
    }

    /// Reads bytes until inbuf holds at least n.
    fn fill(self: *mut Conn, n: usize) !void {
        while self.inlen < n {
            // (a peer dripping a byte at a time must not hold a handshake open)
            if (self.deadline != 0) and (time.Instant.now().ns > self.deadline) {
                return error.TimedOut;
            }
            let got = self.stream.read(self.inbuf[self.inlen..])!;
            if got == 0 {
                return error.Closed;
            }
            self.inlen += got;
        }
    }

    /// Reads the next record (skipping ChangeCipherSpec) into plain; returns
    /// its content type. Alerts end the connection here.
    fn read_record(self: *mut Conn) !u8 {
        loop {
            self.fill(5)!;
            let ty = self.inbuf[0];
            let len = (@as(usize, self.inbuf[3]) << 8) | @as(usize, self.inbuf[4]);
            // not TLS at all (plain HTTP, say): say so now rather than wait
            // for a body that isn't coming
            if (ty < CT_CHANGE_CIPHER_SPEC) or (ty > CT_APPLICATION_DATA) or (self.inbuf[1] != 3) {
                return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
            }
            if len > MAX_CIPHERTEXT {
                return self.fail(ALERT_DECODE_ERROR, error.RecordOverflow);
            }
            self.fill(5 + len)!;
            let hdr: [5]u8 = [self.inbuf[0], self.inbuf[1], self.inbuf[2], self.inbuf[3], self.inbuf[4]];
            let body = self.inbuf[5..5 + len];
            let mut real_type = ty;
            let mut plen: usize = 0;
            if ty == CT_CHANGE_CIPHER_SPEC {
                // compatibility mode: a single 0x01, dropped, and only during
                // the handshake (RFC 8446 5)
                if self.established or (len != 1) or (body[0] != 1) {
                    return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
                }
                self.consume(5 + len);
                continue;
            }
            if self.rd.active {
                if (ty != CT_APPLICATION_DATA) or (len < 17) {
                    return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
                }
                let n = len - 16;
                let open_result = self.rd.open(hdr[..], body, self.plain[..n]);
                self.consume(5 + len);
                open_result catch |e| {
                    return self.fail(ALERT_BAD_RECORD_MAC, e);
                };
                // strip the zero padding; the last non-zero byte is the type
                let mut k = n;
                while (k > 0) and (self.plain[k - 1] == 0) {
                    k -= 1;
                }
                if k == 0 {
                    return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
                }
                real_type = self.plain[k - 1];
                plen = k - 1;
                if plen > MAX_PLAINTEXT {
                    return self.fail(ALERT_RECORD_OVERFLOW, error.RecordOverflow);
                }
            } else {
                if len > MAX_PLAINTEXT {
                    return self.fail(ALERT_DECODE_ERROR, error.RecordOverflow);
                }
                @mem_copy(self.plain[..len], body);
                plen = len;
                self.consume(5 + len);
            }
            self.plain_start = 0;
            self.plain_end = plen;
            self.plain_type = real_type;
            if real_type == CT_ALERT {
                if plen < 2 {
                    return error.DecodeError;
                }
                if self.plain[1] == ALERT_CLOSE_NOTIFY {
                    self.closed_read = true;
                    return error.Closed;
                }
                self.closed_read = true;
                self.closed_write = true;
                return error.AlertReceived;
            }
            if (plen == 0) and (real_type != CT_APPLICATION_DATA) {
                return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
            }
            return real_type;
        }
    }

    fn consume(self: *mut Conn, n: usize) void {
        let rest = self.inlen - n;
        if rest > 0 {
            @mem_copy(self.inbuf[..rest], self.inbuf[n..self.inlen]);
        }
        self.inlen = rest;
    }

    /// Sends a fatal alert (best effort) and returns e.
    fn fail(self: *mut Conn, alert: u8, e: error) error { if not self.closed_write {
        let a: [2]u8 = [2, alert];
        self.send_record(CT_ALERT, a[..]) catch {};
        self.closed_write = true;
    }
    return e; }

    // --- handshake messages --------------------------------------------------

    /// The next handshake message (header included), reading records as
    /// needed. The slice is valid until the next call.
    fn next_message(self: *mut Conn) ![]u8 {
        loop {
            let avail = self.hs.len - self.hs_start;
            if avail >= 4 {
                let b = self.hs.bytes()[self.hs_start..];
                let len = ((@as(usize, b[1]) << 16) | (@as(usize, b[2]) << 8)) | @as(usize, b[3]);
                if len > 65536 {
                    return self.fail(ALERT_DECODE_ERROR, error.DecodeError);
                }
                if avail >= (4 + len) {
                    let msg = b[..4 + len];
                    self.hs_start += 4 + len;
                    return msg;
                }
            }
            if self.hs_start == self.hs.len {
                self.hs.clear();
                self.hs_start = 0;
            }
            let ty = self.read_record()!;
            if ty != CT_HANDSHAKE {
                return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
            }
            self.hs.push_str(self.plain[self.plain_start..self.plain_end])!;
        }
    }

    /// The next message, which must have the given type.
    fn expect_message(self: *mut Conn, ty: u8) ![]u8 {
        let m = self.next_message()!;
        if m[0] != ty {
            return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
        }
        return m;
    }

    fn send_handshake(self: *mut Conn, msg: []u8) !void {
        let mut rest = msg;
        while rest.len > 0 {
            let n = @min(rest.len, MAX_PLAINTEXT);
            self.send_record(CT_HANDSHAKE, rest[..n])!;
            rest = rest[n..];
        }
    }

    /// Handshake messages after the handshake: tickets and key updates.
    fn post_handshake(self: *mut Conn) !void {
        self.hs.push_str(self.plain[self.plain_start..self.plain_end])!;
        loop {
            let avail = self.hs.len - self.hs_start;
            if avail < 4 {
                break;
            }
            let b = self.hs.bytes()[self.hs_start..];
            let len = ((@as(usize, b[1]) << 16) | (@as(usize, b[2]) << 8)) | @as(usize, b[3]);
            if len > 65536 {
                return self.fail(ALERT_DECODE_ERROR, error.DecodeError);
            }
            if avail < (4 + len) {
                break;
            }
            let ty = b[0];
            let body = b[4..4 + len];
            self.hs_start += 4 + len;
            if (ty == HS_NEW_SESSION_TICKET) and self.is_client {
                continue;  // no resumption (and only servers send tickets)
            }
            if ty == HS_KEY_UPDATE {
                if (len != 1) or (body[0] > 1) {
                    return self.fail(ALERT_DECODE_ERROR, error.DecodeError);
                }
                // the key changes here: nothing more may share its record
                if self.hs_start != self.hs.len {
                    return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
                }
                let requested = body[0] == 1;
                self.update_read_key()!;
                if requested {
                    let msg: [5]u8 = [HS_KEY_UPDATE, 0, 0, 1, 0];
                    self.send_record(CT_HANDSHAKE, msg[..])!;
                    self.update_write_key()!;
                }
                continue;
            }
            return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
        }
        // keep only the unfinished message (the buffer cannot grow without bound)
        let rest = self.hs.len - self.hs_start;
        if (self.hs_start > 0) and (rest > 0) {
            let bytes = self.hs.bytes();
            @mem_copy(bytes[..rest], bytes[self.hs_start..self.hs.len]);
        }
        self.hs.truncate(rest);
        self.hs_start = 0;
    }

    /// Switches to new read keys during the handshake: handshake messages
    /// must not straddle the change (RFC 8446 5.1).
    fn handshake_read_key(self: *mut Conn, secret: []u8) !void {
        if self.hs_start != self.hs.len {
            return self.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
        }
        self.rd.set(self.suite, secret)!;
    }

    fn update_read_key(self: *mut Conn) !void {
        let n = sha2.digest_size(self.suite.hash);
        let mut next: [48]u8 = [0; 48];
        expand_label(self.suite.hash, self.secret_rd[..n], "traffic upd", "", next[..n]);
        self.secret_rd = next;
        self.rd.set(self.suite, self.secret_rd[..n])!;
    }

    fn update_write_key(self: *mut Conn) !void {
        let n = sha2.digest_size(self.suite.hash);
        let mut next: [48]u8 = [0; 48];
        expand_label(self.suite.hash, self.secret_wr[..n], "traffic upd", "", next[..n]);
        self.secret_wr = next;
        self.wr.set(self.suite, self.secret_wr[..n])!;
    }

    /// Asks the peer to update its keys too (and updates ours).
    pub fn key_update(self: *mut Conn) !void {
        let msg: [5]u8 = [HS_KEY_UPDATE, 0, 0, 1, 1];
        self.send_record(CT_HANDSHAKE, msg[..])!;
        self.update_write_key()!;
    }
}

fn conn_read(ctx: *mut void, buf: []mut u8) !usize {
    let c = @ptr_cast(*mut Conn, ctx);
    return c.read(buf);
}

fn conn_write(ctx: *mut void, bytes: []u8) !usize {
    let c = @ptr_cast(*mut Conn, ctx);
    return c.write(bytes);
}

fn new_conn(stream: net.TcpStream, is_client: bool) !*mut Conn {
    let c = mem.alloc.[Conn](1)!;
    let p = &mut c[0];
    // most fields are buffers: start from zeros
    @mem_set(@cast_slice(u8, c), 0);
    p.stream = stream;
    p.is_client = is_client;
    p.suite = Suite.{ id: 0, hash: sha2.SHA256, key_len: 0 };
    p.rd = Cipher.none();
    p.wr = Cipher.none();
    p.hs = strs.String.init();
    return p;
}

// -----------------------------------------------------------------------------
// Key exchange
// -----------------------------------------------------------------------------

/// Our key pairs for the offered groups.
struct Shares {
    x25519: curve25519.X25519Keypair,
    p256: ec.P256,
    p256_pub: [65]u8,
}

fn make_shares() !Shares {
    let mut s = Shares.{ x25519: curve25519.x25519_keypair()!, p256: ec.P256.generate()!, p256_pub: [0; 65] };
    s.p256.public_key(s.p256_pub[..]);
    return s;
}

/// The shared secret with the peer's share in `group`.
fn agree(s: *Shares, group: u16, peer: []u8, out_buf: []mut u8) ![]u8 {
    if group == GROUP_X25519 {
        if peer.len != 32 {
            return error.IllegalParameter;
        }
        curve25519.x25519(s.x25519.secret[..], peer, out_buf[..32]) catch return error.IllegalParameter;
        return out_buf[..32];
    }
    if group == GROUP_P256 {
        s.p256.ecdh(peer, out_buf[..32]) catch return error.IllegalParameter;
        return out_buf[..32];
    }
    return error.IllegalParameter;
}

// -----------------------------------------------------------------------------
// Signatures on the handshake
// -----------------------------------------------------------------------------

/// The content a CertificateVerify signs: 64 spaces, a context string, 0,
/// then the transcript hash.
fn verify_content(is_server: bool, th: []u8, out_buf: []mut u8) []u8 {
    @mem_set(out_buf[..64], ' ');
    let mut ctx: []u8 = "TLS 1.3, client CertificateVerify";
    if is_server {
        ctx = "TLS 1.3, server CertificateVerify";
    }
    @mem_copy(out_buf[64..64 + ctx.len], ctx);
    out_buf[64 + ctx.len] = 0;
    @mem_copy(out_buf[65 + ctx.len..(65 + ctx.len) + th.len], th);
    return out_buf[..(65 + ctx.len) + th.len];
}

/// Checks a CertificateVerify signature with the leaf certificate's key.
fn check_signature(key: *x509.PublicKey, scheme: u16, content: []u8, sig: []u8) bool {
    match scheme {
        0x0807 => { return key.verify(x509.SIG_ED25519, 0, content, sig); }
        0x0403 => { return (key.kind == x509.KEY_P256) and key.verify(x509.SIG_ECDSA, sha2.SHA256, content, sig); }
        0x0503 => { return (key.kind == x509.KEY_P384) and key.verify(x509.SIG_ECDSA, sha2.SHA384, content, sig); }
        0x0804 => { return key.verify(x509.SIG_RSA_PSS, sha2.SHA256, content, sig); }
        0x0805 => { return key.verify(x509.SIG_RSA_PSS, sha2.SHA384, content, sig); }
        0x0806 => { return key.verify(x509.SIG_RSA_PSS, sha2.SHA512, content, sig); }
        else   => { return false; }
    }
}

/// DER encoding of an ECDSA signature from r || s.
fn ecdsa_der(rs: []u8, out_buf: []mut u8) []u8 {
    let half = rs.len / 2;
    let mut o: usize = 2;
    for part in @as(usize, 0)..2 {
        let mut v = rs[part * half..(part + 1) * half];
        while (v.len > 1) and (v[0] == 0) {
            v = v[1..];
        }
        let pad = (v[0] & 0x80) != 0;
        out_buf[o] = 0x02;
        out_buf[o + 1] = @truncate(u8, v.len + @as(usize, if_u8(pad)));
        o += 2;
        if pad {
            out_buf[o] = 0;
            o += 1;
        }
        @mem_copy(out_buf[o..o + v.len], v);
        o += v.len;
    }
    out_buf[0] = 0x30;
    out_buf[1] = @truncate(u8, o - 2);
    return out_buf[..o];
}

fn if_u8(b: bool) u8 {
    if b {
        return 1;
    }
    return 0;
}

/// Signs content with the server key; returns the scheme and signature.
fn sign_content(cfg: *ServerConfig, schemes: []u8, content: []u8, sig: []mut u8, scheme: *mut u16) ![]u8 {
    match cfg.key {
        .Ed25519(k) => {
            if not scheme_offered(schemes, SIG_ED25519) {
                return error.HandshakeFailure;
            }
            k.sign(content, sig[..64]);
            scheme.* = SIG_ED25519;
            return sig[..64];
        }
        .P256(k) => {
            if not scheme_offered(schemes, SIG_ECDSA_P256_SHA256) {
                return error.HandshakeFailure;
            }
            let mut d: [32]u8 = [0; 32];
            sha2.sha256(content, d[..]);
            let mut rs: [64]u8 = [0; 64];
            k.sign(d[..], rs[..])!;
            scheme.* = SIG_ECDSA_P256_SHA256;
            return ecdsa_der(rs[..], sig);
        }
        .P384(k) => {
            if not scheme_offered(schemes, SIG_ECDSA_P384_SHA384) {
                return error.HandshakeFailure;
            }
            let mut d: [48]u8 = [0; 48];
            sha2.sha384(content, d[..]);
            let mut rs: [96]u8 = [0; 96];
            k.sign(d[..], rs[..])!;
            scheme.* = SIG_ECDSA_P384_SHA384;
            return ecdsa_der(rs[..], sig);
        }
        .Rsa(k) => {
            if not scheme_offered(schemes, SIG_RSA_PSS_SHA256) {
                return error.HandshakeFailure;
            }
            let mut d: [32]u8 = [0; 32];
            sha2.sha256(content, d[..]);
            let n = k.public.size;
            k.sign_pss(sha2.SHA256, d[..], sig[..n])!;
            scheme.* = SIG_RSA_PSS_SHA256;
            return sig[..n];
        }
    }
}

fn scheme_offered(list: []u8, id: u16) bool {
    let mut i: usize = 0;
    while (i + 1) < list.len {
        if ((@as(u16, list[i]) << 8) | @as(u16, list[i + 1])) == id {
            return true;
        }
        i += 2;
    }
    return false;
}

// -----------------------------------------------------------------------------
// The client
// -----------------------------------------------------------------------------

/// Connects to hostname:port and performs the handshake.
pub fn connect(hostname: []u8, port: u16, cfg: *ClientConfig) !*mut Conn {
    let s = net.connect(hostname, port)!;
    return client(s, hostname, cfg);
}

/// Runs the client handshake over a connected stream. hostname is sent as
/// SNI (unless it is an IP address) and checked against the certificate.
/// The stream belongs to the Conn afterwards (on failure too).
pub fn client(stream: net.TcpStream, hostname: []u8, cfg: *ClientConfig) !*mut Conn {
    // (longer than any DNS name; and with no name, the certificate could
    // belong to anyone: verification needs one)
    if (hostname.len > 255) or ((hostname.len == 0) and cfg.verify) {
        stream.close();
        return error.InvalidArgument;
    }
    let c = new_conn(stream, true)!;
    let n = hostname.len;
    @mem_copy(c.sni[..n], hostname[..n]);
    c.sni_len = n;
    c.deadline = time.Instant.now().ns + (HANDSHAKE_TIMEOUT_MS * 1000000);
    client_handshake(c, cfg) catch |e| {
        stream.close();
        c.hs.deinit();
        mem.destroy(c);
        return e;
    };
    c.deadline = 0;
    return c;
}

fn is_ip(sni: []u8) bool {
    _ = net.parse_ipv4(sni) catch return false;
    return true;
}

fn client_hello(c: *mut Conn, cfg: *ClientConfig, shares: *Shares, random_bytes: []u8, session_id: []u8,
                only_group: u16, cookie: []u8) !Out {
    let mut o = out();
    o.put8(HS_CLIENT_HELLO)!;
    let body = o.open(3)!;
    o.put16(0x0303)!;
    o.put(random_bytes)!;
    o.put8(32)!;
    o.put(session_id)!;
    let cs = o.open(2)!;
    o.put16(TLS_CHACHA20_POLY1305_SHA256)!;
    o.put16(TLS_AES_128_GCM_SHA256)!;
    o.put16(TLS_AES_256_GCM_SHA384)!;
    o.close(cs, 2);
    o.put8(1)!;
    o.put8(0)!;
    let exts = o.open(2)!;
    let sni = c.sni[..c.sni_len];
    if (sni.len > 0) and not is_ip(sni) {
        o.put16(EXT_SERVER_NAME)!;
        let e = o.open(2)!;
        let l = o.open(2)!;
        o.put8(0)!;
        o.put16(@truncate(u16, sni.len))!;
        o.put(sni)!;
        o.close(l, 2);
        o.close(e, 2);
    }
    o.put16(EXT_SUPPORTED_VERSIONS)!;
    o.put16(3)!;
    o.put8(2)!;
    o.put16(0x0304)!;
    o.put16(EXT_SUPPORTED_GROUPS)!;
    o.put16(6)!;
    o.put16(4)!;
    o.put16(GROUP_X25519)!;
    o.put16(GROUP_P256)!;
    o.put16(EXT_SIGNATURE_ALGORITHMS)!;
    let sa = o.open(2)!;
    let sl = o.open(2)!;
    let schemes: [9]u16 = [SIG_ED25519, SIG_ECDSA_P256_SHA256, SIG_ECDSA_P384_SHA384, SIG_RSA_PSS_SHA256,
                           SIG_RSA_PSS_SHA384, SIG_RSA_PSS_SHA512, SIG_RSA_PKCS1_SHA256, SIG_RSA_PKCS1_SHA384,
                           SIG_RSA_PKCS1_SHA512];
    for sc in schemes {
        o.put16(sc)!;
    }
    o.close(sl, 2);
    o.close(sa, 2);
    o.put16(EXT_KEY_SHARE)!;
    let ks = o.open(2)!;
    let kl = o.open(2)!;
    if (only_group == 0) or (only_group == GROUP_X25519) {
        o.put16(GROUP_X25519)!;
        o.put16(32)!;
        o.put(shares.x25519.public[..])!;
    }
    if (only_group == 0) or (only_group == GROUP_P256) {
        o.put16(GROUP_P256)!;
        o.put16(65)!;
        o.put(shares.p256_pub[..])!;
    }
    o.close(kl, 2);
    o.close(ks, 2);
    if cookie.len > 0 {
        o.put16(EXT_COOKIE)!;
        let ck = o.open(2)!;
        o.put(cookie)!;
        o.close(ck, 2);
    }
    if cfg.alpn.len > 0 {
        o.put16(EXT_ALPN)!;
        let a = o.open(2)!;
        let al = o.open(2)!;
        for p in cfg.alpn {
            o.put8(@truncate(u8, p.len))!;
            o.put(p)!;
        }
        o.close(al, 2);
        o.close(a, 2);
    }
    o.close(exts, 2);
    o.close(body, 3);
    return o;
}

/// The fields of a ServerHello (or HelloRetryRequest).
struct ServerHello {
    hrr: bool,
    suite: u16,
    version: u16,
    group: u16,
    share: []u8,
    cookie: []u8,
    session_id: []u8,
}

fn parse_server_hello(msg: []u8) !ServerHello {
    let mut r = cur(msg[4..]);
    let mut sh = ServerHello.{ hrr: false, suite: 0, version: 0, group: 0, share: "", cookie: "", session_id: "" };
    if r.get16()! != 0x0303 {
        return error.ProtocolVersion;
    }
    let rnd = r.bytes(32)!;
    sh.hrr = @mem_eq(rnd, HRR_RANDOM[..]);
    sh.session_id = r.vec8()!;
    sh.suite = r.get16()!;
    if r.get8()! != 0 {
        return error.IllegalParameter;
    }
    let mut e = cur(r.vec16()!);
    while not e.done() {
        let ty = e.get16()!;
        let mut d = cur(e.vec16()!);
        if ty == EXT_SUPPORTED_VERSIONS {
            sh.version = d.get16()!;
        } else if ty == EXT_KEY_SHARE {
            sh.group = d.get16()!;
            if not sh.hrr {
                sh.share = d.vec16()!;
            }
        } else if ty == EXT_COOKIE {
            sh.cookie = d.vec16()!;
        }
    }
    return sh;
}

fn client_handshake(c: *mut Conn, cfg: *ClientConfig) !void {
    let mut shares = make_shares()!;
    let mut rnd: [32]u8 = [0; 32];
    let mut sid: [32]u8 = [0; 32];
    random.fill(rnd[..])!;
    random.fill(sid[..])!;
    let mut tr = Transcript.init();
    defer tr.deinit();
    let mut ch = client_hello(c, cfg, &shares, rnd[..], sid[..], 0, "")!;
    tr.add(ch.bytes())!;
    c.send_handshake(ch.bytes())!;
    ch.s.deinit();
    let mut msg = c.expect_message(HS_SERVER_HELLO)!;
    let mut sh = parse_server_hello(msg)!;
    if not @mem_eq(sh.session_id, sid[..]) {
        return c.fail(ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);  // must echo ours
    }
    if sh.hrr {
        // HelloRetryRequest: the transcript restarts from a hash of the
        // first ClientHello, and a second ClientHello carries the one share
        // the server asked for
        if (sh.version != 0x0304) or ((sh.group != GROUP_X25519) and (sh.group != GROUP_P256)) {
            return c.fail(ALERT_ILLEGAL_PARAMETER, error.HandshakeFailure);
        }
        let s = suite_of(sh.suite) catch return c.fail(ALERT_HANDSHAKE_FAILURE, error.HandshakeFailure);
        let mut h1: [48]u8 = [0; 48];
        sha2.hash(s.hash, tr.pending.bytes(), h1[..]);
        let hl = sha2.digest_size(s.hash);
        tr.pending.clear();
        let hdr: [4]u8 = [HS_MESSAGE_HASH, 0, 0, @truncate(u8, hl)];
        tr.add(hdr[..])!;
        tr.add(h1[..hl])!;
        tr.add(msg)!;
        let mut cookie = strs.String.init();
        defer cookie.deinit();
        cookie.push_str(sh.cookie)!;
        let mut ch2 = client_hello(c, cfg, &shares, rnd[..], sid[..], sh.group, cookie.bytes())!;
        tr.add(ch2.bytes())!;
        c.send_handshake(ch2.bytes())!;
        ch2.s.deinit();
        msg = c.expect_message(HS_SERVER_HELLO)!;
        let hrr_suite = sh.suite;
        let hrr_group = sh.group;
        sh = parse_server_hello(msg)!;
        if sh.hrr {
            return c.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
        }
        // the ServerHello must keep what the retry chose
        if (sh.suite != hrr_suite) or (sh.group != hrr_group) or not @mem_eq(sh.session_id, sid[..]) {
            return c.fail(ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
        }
    }
    if sh.version != 0x0304 {
        return c.fail(ALERT_PROTOCOL_VERSION, error.ProtocolVersion);
    }
    c.suite = suite_of(sh.suite) catch return c.fail(ALERT_ILLEGAL_PARAMETER, error.HandshakeFailure);
    tr.add(msg)!;
    tr.start(c.suite.hash);
    let hash = c.suite.hash;
    let hl = sha2.digest_size(hash);
    // the handshake secrets
    let mut shared_buf: [48]u8 = [0; 48];
    let shared = agree(&shares, sh.group, sh.share, shared_buf[..]) catch return c.fail(
        ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
    let mut ks = KeySchedule.init(hash);
    let mut th: [48]u8 = [0; 48];
    ks.handshake(shared, tr.current(th[..]));
    c.handshake_read_key(ks.server_hs[..hl])!;
    // encrypted handshake messages
    let ee = c.expect_message(HS_ENCRYPTED_EXTENSIONS)!;
    tr.add(ee)!;
    read_encrypted_extensions(c, ee, cfg.alpn)!;
    let mut next = c.next_message()!;
    let mut cert_requested = false;
    if next[0] == HS_CERTIFICATE_REQUEST {
        cert_requested = true;
        tr.add(next)!;
        next = c.next_message()!;
    }
    if next[0] != HS_CERTIFICATE {
        return c.fail(ALERT_UNEXPECTED_MESSAGE, error.UnexpectedMessage);
    }
    tr.add(next)!;
    let mut chain = parse_certificates(c, next)!;
    defer free_certs(&mut chain);
    if chain.len == 0 {
        return c.fail(ALERT_DECODE_ERROR, error.BadCertificate);
    }
    if cfg.verify {
        let mut roots = cfg.roots;
        x509.verify_chain(chain.items(), &roots, c.sni[..c.sni_len], time.now()) catch |e| {
            return c.fail(ALERT_BAD_CERTIFICATE, e);
        };
    }
    let cv = c.expect_message(HS_CERTIFICATE_VERIFY)!;
    let mut cvr = cur(cv[4..]);
    let scheme = cvr.get16()!;
    let sig = cvr.vec16()!;
    let mut content_buf: [200]u8 = [0; 200];
    let content = verify_content(true, tr.current(th[..]), content_buf[..]);
    if not check_signature(&chain.get_ptr(0).key, scheme, content, sig) {
        return c.fail(ALERT_DECRYPT_ERROR, error.BadCertificate);
    }
    tr.add(cv)!;
    let fin = c.expect_message(HS_FINISHED)!;
    let mut want: [48]u8 = [0; 48];
    finished_mac(hash, ks.server_hs[..hl], tr.current(th[..]), want[..hl]);
    if (fin.len != (4 + hl)) or not subtle.equal(fin[4..], want[..hl]) {
        return c.fail(ALERT_DECRYPT_ERROR, error.HandshakeFailure);
    }
    tr.add(fin)!;
    // application secrets from the transcript through the server Finished
    ks.application(tr.current(th[..]));
    // compatibility: a ChangeCipherSpec before our encrypted messages
    let ccs: [1]u8 = [1];
    c.send_record(CT_CHANGE_CIPHER_SPEC, ccs[..])!;
    c.wr.set(c.suite, ks.client_hs[..hl])!;
    if cert_requested {
        // an empty Certificate (with an empty request context)
        let empty: [8]u8 = [HS_CERTIFICATE, 0, 0, 4, 0, 0, 0, 0];
        tr.add(empty[..])!;
        c.send_handshake(empty[..])!;
    }
    let mut mine: [52]u8 = [0; 52];
    mine[0] = HS_FINISHED;
    mine[3] = @truncate(u8, hl);
    finished_mac(hash, ks.client_hs[..hl], tr.current(th[..]), mine[4..4 + hl]);
    c.send_handshake(mine[..4 + hl])!;
    c.handshake_read_key(ks.server_ap[..hl])!;
    c.wr.set(c.suite, ks.client_ap[..hl])!;
    c.established = true;
    @mem_copy(c.secret_rd[..hl], ks.server_ap[..hl]);
    @mem_copy(c.secret_wr[..hl], ks.client_ap[..hl]);
}

fn read_encrypted_extensions(c: *mut Conn, ee: []u8, offered: [][]u8) !void {
    let mut r = cur(ee[4..]);
    let mut e = cur(r.vec16()!);
    while not e.done() {
        let ty = e.get16()!;
        let data = e.vec16()!;
        if ty == EXT_ALPN {
            let mut a = cur(data);
            let mut l = cur(a.vec16()!);
            let p = l.vec8()!;
            if (p.len == 0) or (p.len > 32) {
                return c.fail(ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
            }
            // the server must pick one of ours
            let mut ok = false;
            for o in offered {
                if @mem_eq(o, p) {
                    ok = true;
                }
            }
            if not ok {
                return c.fail(ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
            }
            @mem_copy(c.alpn_selected[..p.len], p);
            c.alpn_len = p.len;
        }
    }
}

fn parse_certificates(c: *mut Conn, msg: []u8) !vec.Vec.[x509.Certificate] {
    let mut list = vec.Vec.[x509.Certificate].init();
    let mut r = cur(msg[4..]);
    _ = r.vec8()!;  // certificate_request_context
    let mut entries = cur(r.vec24()!);
    while not entries.done() {
        let d = entries.vec24() catch |e| {
            free_certs(&mut list);
            return e;
        };
        _ = entries.vec16() catch |e| {
            free_certs(&mut list);
            return e;
        };
        let mut cert = x509.Certificate.parse(d) catch |e| {
            free_certs(&mut list);
            return c.fail(ALERT_BAD_CERTIFICATE, e);
        };
        list.push(cert) catch |e| {
            cert.deinit();
            free_certs(&mut list);
            return e;
        };
    }
    return list;
}

fn free_certs(list: *mut vec.Vec.[x509.Certificate]) void {
    for i in 0..list.len {
        list.get_ptr(i).deinit();
    }
    list.deinit();
}

/// HMAC(finished_key, transcript hash).
fn finished_mac(hash: u8, base: []u8, th: []u8, out_buf: []mut u8) void {
    let hl = sha2.digest_size(hash);
    let mut fk: [48]u8 = [0; 48];
    expand_label(hash, base, "finished", "", fk[..hl]);
    hmac.hmac(hash, fk[..hl], th, out_buf);
}

/// The TLS 1.3 key schedule without pre-shared keys.
struct KeySchedule {
    hash: u8,
    hl: usize,
    handshake_secret: [48]u8,
    client_hs: [48]u8,
    server_hs: [48]u8,
    client_ap: [48]u8,
    server_ap: [48]u8,

    fn init(hash: u8) KeySchedule {
        return KeySchedule.{ hash: hash, hl: sha2.digest_size(hash), handshake_secret: [0; 48], client_hs: [0; 48],
                             server_hs: [0; 48], client_ap: [0; 48], server_ap: [0; 48] };
    }

    /// Derive-Secret(secret, label, transcript hash).
    fn derive(self: *KeySchedule, secret: []u8, label: []u8, th: []u8, out_buf: []mut u8) void {
        expand_label(self.hash, secret, label, th, out_buf[..self.hl]);
    }

    fn empty_hash(self: *KeySchedule, out_buf: []mut u8) []u8 {
        sha2.hash(self.hash, "", out_buf);
        return out_buf[..self.hl];
    }

    fn handshake(self: *mut KeySchedule, shared: []u8, th: []u8) void {
        let hl = self.hl;
        let zeros: [48]u8 = [0; 48];
        let mut early: [48]u8 = [0; 48];
        hmac.extract(self.hash, "", zeros[..hl], early[..hl]);
        let mut eh: [48]u8 = [0; 48];
        let empty = self.empty_hash(eh[..]);
        let mut derived: [48]u8 = [0; 48];
        self.derive(early[..hl], "derived", empty, derived[..]);
        hmac.extract(self.hash, derived[..hl], shared, self.handshake_secret[..hl]);
        let hs = self.handshake_secret;
        self.derive(hs[..hl], "c hs traffic", th, self.client_hs[..]);
        self.derive(hs[..hl], "s hs traffic", th, self.server_hs[..]);
    }

    fn application(self: *mut KeySchedule, th: []u8) void {
        let hl = self.hl;
        let zeros: [48]u8 = [0; 48];
        let mut eh: [48]u8 = [0; 48];
        let empty = self.empty_hash(eh[..]);
        let mut derived: [48]u8 = [0; 48];
        let hs = self.handshake_secret;
        self.derive(hs[..hl], "derived", empty, derived[..]);
        let mut master: [48]u8 = [0; 48];
        hmac.extract(self.hash, derived[..hl], zeros[..hl], master[..hl]);
        self.derive(master[..hl], "c ap traffic", th, self.client_ap[..]);
        self.derive(master[..hl], "s ap traffic", th, self.server_ap[..]);
    }
}

// -----------------------------------------------------------------------------
// The server
// -----------------------------------------------------------------------------

/// Runs the server handshake on an accepted connection. The stream
/// belongs to the Conn afterwards (and is closed if the handshake fails).
pub fn server(stream: net.TcpStream, cfg: *ServerConfig) !*mut Conn {
    let c = new_conn(stream, false)!;
    c.deadline = time.Instant.now().ns + (HANDSHAKE_TIMEOUT_MS * 1000000);
    server_handshake(c, cfg) catch |e| {
        stream.close();
        c.hs.deinit();
        mem.destroy(c);
        return e;
    };
    c.deadline = 0;
    return c;
}

/// A handshake must finish within this long (whatever the socket's own
/// timeout, which limits each read).
pub const HANDSHAKE_TIMEOUT_MS: i64 = 20000;

/// What the server takes from a ClientHello.
struct Hello {
    session_id: []u8,
    suite: u16,
    tls13: bool,
    group: u16,  // the group of a usable share (0: none)
    share: []u8,
    wants_x25519: bool,  // listed in supported_groups
    wants_p256: bool,
    schemes: []u8,
    alpn: []u8,  // the protocol_name_list
    cookie: []u8,
}

fn parse_client_hello(c: *mut Conn, msg: []u8) !Hello {
    let mut h = Hello.{ session_id: "", suite: 0, tls13: false, group: 0, share: "", wants_x25519: false,
                        wants_p256: false, schemes: "", alpn: "", cookie: "" };
    let mut r = cur(msg[4..]);
    _ = r.get16()!;
    _ = r.bytes(32)!;
    h.session_id = r.vec8()!;
    let mut suites = cur(r.vec16()!);
    // our preference: ChaCha20 (fastest in software), then AES-128, AES-256
    let mut best: u32 = 99;
    while not suites.done() {
        let s = suites.get16()!;
        let mut rank: u32 = 99;
        if s == TLS_CHACHA20_POLY1305_SHA256 {
            rank = 0;
        } else if s == TLS_AES_128_GCM_SHA256 {
            rank = 1;
        } else if s == TLS_AES_256_GCM_SHA384 {
            rank = 2;
        }
        if rank < best {
            best = rank;
            h.suite = s;
        }
    }
    _ = r.vec8()!;
    if r.done() {
        return h;  // no extensions: not TLS 1.3
    }
    let mut e = cur(r.vec16()!);
    while not e.done() {
        let ty = e.get16()!;
        let data = e.vec16()!;
        let mut d = cur(data);
        if ty == EXT_SUPPORTED_VERSIONS {
            let mut vs = cur(d.vec8()!);
            while not vs.done() {
                if vs.get16()! == 0x0304 {
                    h.tls13 = true;
                }
            }
        } else if ty == EXT_SUPPORTED_GROUPS {
            let mut gs = cur(d.vec16()!);
            while not gs.done() {
                let g = gs.get16()!;
                if g == GROUP_X25519 {
                    h.wants_x25519 = true;
                } else if g == GROUP_P256 {
                    h.wants_p256 = true;
                }
            }
        } else if ty == EXT_KEY_SHARE {
            let mut ks = cur(d.vec16()!);
            while not ks.done() {
                let g = ks.get16()!;
                let k = ks.vec16()!;
                if (g == GROUP_X25519) and (h.group != GROUP_X25519) {
                    h.group = g;
                    h.share = k;
                } else if (g == GROUP_P256) and (h.group == 0) {
                    h.group = g;
                    h.share = k;
                }
            }
        } else if ty == EXT_SIGNATURE_ALGORITHMS {
            h.schemes = d.vec16()!;
        } else if ty == EXT_ALPN {
            h.alpn = d.vec16()!;
        } else if ty == EXT_COOKIE {
            h.cookie = d.vec16()!;
        } else if ty == EXT_SERVER_NAME {
            let mut l = cur(d.vec16()!);
            if l.get8()! == 0 {
                let name = l.vec16()!;
                let n = @min(name.len, 255);
                @mem_copy(c.sni[..n], name[..n]);
                c.sni_len = n;
            }
        }
    }
    return h;
}

fn server_hello(sid: []u8, suite: u16, hrr: bool, group: u16, share: []u8) !Out {
    let mut o = out();
    o.put8(HS_SERVER_HELLO)!;
    let body = o.open(3)!;
    o.put16(0x0303)!;
    if hrr {
        o.put(HRR_RANDOM[..])!;
    } else {
        let mut rnd: [32]u8 = [0; 32];
        random.fill(rnd[..])!;
        o.put(rnd[..])!;
    }
    o.put8(@truncate(u8, sid.len))!;
    o.put(sid)!;
    o.put16(suite)!;
    o.put8(0)!;
    let exts = o.open(2)!;
    o.put16(EXT_SUPPORTED_VERSIONS)!;
    o.put16(2)!;
    o.put16(0x0304)!;
    o.put16(EXT_KEY_SHARE)!;
    if hrr {
        o.put16(2)!;
        o.put16(group)!;
    } else {
        let k = o.open(2)!;
        o.put16(group)!;
        o.put16(@truncate(u16, share.len))!;
        o.put(share)!;
        o.close(k, 2);
    }
    o.close(exts, 2);
    o.close(body, 3);
    return o;
}

fn server_handshake(c: *mut Conn, cfg: *ServerConfig) !void {
    let mut tr = Transcript.init();
    defer tr.deinit();
    let mut msg = c.expect_message(HS_CLIENT_HELLO)!;
    let mut hello = parse_client_hello(c, msg)!;
    if not hello.tls13 {
        return c.fail(ALERT_PROTOCOL_VERSION, error.ProtocolVersion);
    }
    if (hello.suite == 0) or (hello.schemes.len == 0) {
        return c.fail(ALERT_HANDSHAKE_FAILURE, error.HandshakeFailure);
    }
    c.suite = suite_of(hello.suite)!;
    let hash = c.suite.hash;
    let hl = sha2.digest_size(hash);
    tr.add(msg)!;
    let mut sid_copy: [32]u8 = [0; 32];
    let sid_len = @min(hello.session_id.len, 32);
    @mem_copy(sid_copy[..sid_len], hello.session_id[..sid_len]);
    let mut sent_ccs = false;
    if hello.group == 0 {
        // no usable share: ask for one (HelloRetryRequest)
        let mut g: u16 = 0;
        if hello.wants_x25519 {
            g = GROUP_X25519;
        } else if hello.wants_p256 {
            g = GROUP_P256;
        } else {
            return c.fail(ALERT_HANDSHAKE_FAILURE, error.HandshakeFailure);
        }
        let mut h1: [48]u8 = [0; 48];
        sha2.hash(hash, tr.pending.bytes(), h1[..]);
        tr.pending.clear();
        let hdr: [4]u8 = [HS_MESSAGE_HASH, 0, 0, @truncate(u8, hl)];
        tr.add(hdr[..])!;
        tr.add(h1[..hl])!;
        let mut hrr = server_hello(sid_copy[..sid_len], hello.suite, true, g, "")!;
        tr.add(hrr.bytes())!;
        c.send_handshake(hrr.bytes())!;
        hrr.s.deinit();
        if sid_len > 0 {
            let ccs: [1]u8 = [1];
            c.send_record(CT_CHANGE_CIPHER_SPEC, ccs[..])!;
            sent_ccs = true;
        }
        msg = c.expect_message(HS_CLIENT_HELLO)!;
        hello = parse_client_hello(c, msg)!;
        if (hello.group != g) or (hello.suite != c.suite.id) {
            return c.fail(ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
        }
        tr.add(msg)!;
    }
    tr.start(hash);
    // our share and the shared secret
    let mut shares = make_shares()!;
    let mut shared_buf: [48]u8 = [0; 48];
    let shared = agree(&shares, hello.group, hello.share, shared_buf[..]) catch return c.fail(
        ALERT_ILLEGAL_PARAMETER, error.IllegalParameter);
    let mut ours: []u8 = shares.x25519.public[..];
    if hello.group == GROUP_P256 {
        ours = shares.p256_pub[..];
    }
    let mut sh = server_hello(sid_copy[..sid_len], hello.suite, false, hello.group, ours)!;
    tr.add(sh.bytes())!;
    c.send_handshake(sh.bytes())!;
    sh.s.deinit();
    if (sid_len > 0) and not sent_ccs {
        let ccs: [1]u8 = [1];
        c.send_record(CT_CHANGE_CIPHER_SPEC, ccs[..])!;
    }
    let mut ks = KeySchedule.init(hash);
    let mut th: [48]u8 = [0; 48];
    ks.handshake(shared, tr.current(th[..]));
    c.wr.set(c.suite, ks.server_hs[..hl])!;
    c.handshake_read_key(ks.client_hs[..hl])!;
    // EncryptedExtensions, with the ALPN choice
    let mut ee = out();
    defer ee.s.deinit();
    ee.put8(HS_ENCRYPTED_EXTENSIONS)!;
    let eb = ee.open(3)!;
    let el = ee.open(2)!;
    let chosen = choose_alpn(cfg.alpn, hello.alpn);
    if chosen.len > 0 {
        ee.put16(EXT_ALPN)!;
        let a = ee.open(2)!;
        let al = ee.open(2)!;
        ee.put8(@truncate(u8, chosen.len))!;
        ee.put(chosen)!;
        ee.close(al, 2);
        ee.close(a, 2);
        @mem_copy(c.alpn_selected[..chosen.len], chosen);
        c.alpn_len = chosen.len;
    } else if (hello.alpn.len > 0) and (cfg.alpn.len > 0) {
        return c.fail(ALERT_NO_APPLICATION_PROTOCOL, error.HandshakeFailure);
    }
    ee.close(el, 2);
    ee.close(eb, 3);
    tr.add(ee.bytes())!;
    c.send_handshake(ee.bytes())!;
    // Certificate
    let mut cm = out();
    defer cm.s.deinit();
    cm.put8(HS_CERTIFICATE)!;
    let cb = cm.open(3)!;
    cm.put8(0)!;
    let cl = cm.open(3)!;
    for d in cfg.chain.items() {
        cm.put24(d.len)!;
        cm.put(d)!;
        cm.put16(0)!;
    }
    cm.close(cl, 3);
    cm.close(cb, 3);
    tr.add(cm.bytes())!;
    c.send_handshake(cm.bytes())!;
    // CertificateVerify
    let mut content_buf: [200]u8 = [0; 200];
    let content = verify_content(true, tr.current(th[..]), content_buf[..]);
    let mut sig_buf: [1024]u8 = [0; 1024];
    let mut scheme: u16 = 0;
    let sig = sign_content(cfg, hello.schemes, content, sig_buf[..], &mut scheme) catch |e| {
        return c.fail(ALERT_HANDSHAKE_FAILURE, e);
    };
    let mut cv = out();
    defer cv.s.deinit();
    cv.put8(HS_CERTIFICATE_VERIFY)!;
    let vb = cv.open(3)!;
    cv.put16(scheme)!;
    cv.put16(@truncate(u16, sig.len))!;
    cv.put(sig)!;
    cv.close(vb, 3);
    tr.add(cv.bytes())!;
    c.send_handshake(cv.bytes())!;
    // Finished
    let mut fin: [52]u8 = [0; 52];
    fin[0] = HS_FINISHED;
    fin[3] = @truncate(u8, hl);
    finished_mac(hash, ks.server_hs[..hl], tr.current(th[..]), fin[4..4 + hl]);
    tr.add(fin[..4 + hl])!;
    c.send_handshake(fin[..4 + hl])!;
    ks.application(tr.current(th[..]));
    c.wr.set(c.suite, ks.server_ap[..hl])!;
    // the client's Finished
    let cf = c.expect_message(HS_FINISHED)!;
    let mut want: [48]u8 = [0; 48];
    finished_mac(hash, ks.client_hs[..hl], tr.current(th[..]), want[..hl]);
    if (cf.len != (4 + hl)) or not subtle.equal(cf[4..], want[..hl]) {
        return c.fail(ALERT_DECRYPT_ERROR, error.HandshakeFailure);
    }
    c.handshake_read_key(ks.client_ap[..hl])!;
    c.established = true;
    @mem_copy(c.secret_rd[..hl], ks.client_ap[..hl]);
    @mem_copy(c.secret_wr[..hl], ks.server_ap[..hl]);
}

/// The first of our protocols that the client offers ("" if none).
fn choose_alpn(ours: [][]u8, offered: []u8) []u8 {
    for p in ours {
        let mut l = cur(offered);
        while not l.done() {
            let q = l.vec8() catch return "";
            if @mem_eq(p, q) {
                return p;
            }
        }
    }
    return "";
}

// -----------------------------------------------------------------------------
// Tests: a client and a server in one process, over loopback. The server
// has a self-signed P-256 certificate for localhost and 127.0.0.1.
// -----------------------------------------------------------------------------

let TEST_CERT: [11][]u8 = [
    "-----BEGIN CERTIFICATE-----\n",
    "MIIBmjCCAUGgAwIBAgIUTGKoTgwLCyiQ8r9NVkXLVdhYYd0wCgYIKoZIzj0EAwIw\n",
    "FDESMBAGA1UEAwwJbG9jYWxob3N0MCAXDTI2MDkyODIzMTA1NloYDzIxMjYwOTA0\n",
    "MjMxMDU2WjAUMRIwEAYDVQQDDAlsb2NhbGhvc3QwWTATBgcqhkjOPQIBBggqhkjO\n",
    "PQMBBwNCAASaXwl2y2wrvsRBwKcBCfVYRP6n+UoDEUmfRQEyVk2p9N6ndI8C/y3W\n",
    "CDL4HVY6cvUGuiHGz6JE9+EFQnQ71rUko28wbTAdBgNVHQ4EFgQUUVQuA75uDa3E\n",
    "2wzrMq/3FPC91PMwHwYDVR0jBBgwFoAUUVQuA75uDa3E2wzrMq/3FPC91PMwDwYD\n",
    "VR0TAQH/BAUwAwEB/zAaBgNVHREEEzARgglsb2NhbGhvc3SHBH8AAAEwCgYIKoZI\n",
    "zj0EAwIDRwAwRAIgK1dyc1W73OQJn/0NmyNXLBzbpck2UpqI3gYtW8nBa9gCIBLK\n",
    "ioXsQVpLpg05LzG+kqwafLSlgwMS0FA0VSfW6O3d\n",
    "-----END CERTIFICATE-----\n",
];

let TEST_KEY: [5][]u8 = [
    "-----BEGIN PRIVATE KEY-----\n",
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgnXxOrb06o0h/7Rns\n",
    "/1p67T6EuZ5xqfjbQRRdFxQAZB2hRANCAASaXwl2y2wrvsRBwKcBCfVYRP6n+UoD\n",
    "EUmfRQEyVk2p9N6ndI8C/y3WCDL4HVY6cvUGuiHGz6JE9+EFQnQ71rUk\n",
    "-----END PRIVATE KEY-----\n",
];

fn join(lines: [][]u8) ![]mut u8 {
    let mut s = strs.String.init();
    for l in lines {
        s.push_str(l)!;
    }
    return s.take();
}

/// A self-signed P-256 certificate for localhost and 127.0.0.1, as PEM (free
/// it with mem.free). Its private key (test_key_pem) is published here, so
/// it is for tests only.
pub fn test_certificate_pem() ![]mut u8 {
    return join(TEST_CERT[..]);
}

/// The private key of test_certificate_pem, as PEM (free it with mem.free).
pub fn test_key_pem() ![]mut u8 {
    return join(TEST_KEY[..]);
}

struct TestServer {
    listener: net.TcpListener,
    cfg: *ServerConfig,
    failures: u32,
}

/// Serves connections: echoes what it reads until the client closes.
fn test_serve(t: *mut TestServer) void {
    loop {
        let s = t.listener.accept() catch return;
        let c = server(s, t.cfg) catch {
            t.failures += 1;
            continue;
        };
        let mut buf: [4096]u8 = [0; 4096];
        loop {
            let n = c.read(buf[..]) catch break;
            if n == 0 {
                break;
            }
            c.write_all(buf[..n]) catch break;
        }
        c.close();
    }
}

fn fails[T](r: !T) bool {
    _ = r catch return true;
    return false;
}

test "client and server" {
    let cert = join(TEST_CERT[..])!;
    defer mem.free(cert);
    let key = join(TEST_KEY[..])!;
    defer mem.free(key);
    let mut scfg = ServerConfig.from_pem(cert, key)!;
    defer scfg.deinit();
    let protos: [2][]u8 = ["h2", "http/1.1"];
    scfg.alpn = protos[1..];
    let mut ts = TestServer.{ listener: net.listen("127.0.0.1", 0)!, cfg: &scfg, failures: 0 };
    let port = ts.listener.port();
    let th = thread.spawn(test_serve, &mut ts)!;

    let mut ccfg = ClientConfig.with_roots_pem(cert)!;
    defer ccfg.deinit();
    ccfg.alpn = protos[..];
    let c = connect("localhost", port, &ccfg)!;
    @expect_eq(c.protocol(), "http/1.1");
    c.write_all("hello")!;
    let mut buf: [8]u8 = [0; 8];
    c.reader().read_exact(buf[..5])!;
    @expect_eq(buf[..5], "hello");
    // more than one record each way, and a key update in the middle
    let big = mem.alloc.[u8](100000)!;
    defer mem.free(big);
    for i in 0..big.len {
        big[i] = @truncate(u8, i * 7);
    }
    c.write_all(big[..50000])!;
    c.key_update()!;
    c.write_all(big[50000..])!;
    let back = mem.alloc.[u8](100000)!;
    defer mem.free(back);
    c.reader().read_exact(back)!;
    @expect(@mem_eq(back, big));
    c.close();

    // by IP address too
    let c2 = connect("127.0.0.1", port, &ccfg)!;
    c2.close();

    // an untrusted certificate and a wrong name are refused
    let mut empty = ClientConfig.with_roots_pem("")!;
    defer empty.deinit();
    @expect(fails(connect("localhost", port, &empty)));
    let s3 = net.connect("127.0.0.1", port)!;
    @expect(fails(client(s3, "other.test", &ccfg)));
    // unless verification is off
    let mut insecure = ClientConfig.insecure();
    defer insecure.deinit();
    let c4 = connect("127.0.0.1", port, &insecure)!;
    c4.write_all("x")!;
    c4.reader().read_exact(buf[..1])!;
    c4.close();

    ts.listener.close();
    th.join();
    @expect_eq(ts.failures, 2);
}
