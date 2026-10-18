## Standard library

- `std/net/dns.parse` parses a reply message from bytes, `std/net/tls.parse_hello` a
  ServerHello or ClientHello message, and `std/net/websocket.from_bytes` makes a
  connection that receives given frames: for tests and the fuzzer, which now covers
  TLS hello messages, HTTP/2 frames, HPACK blocks, WebSocket frames and DNS replies.
- `std/thread` has tests of its own API.
