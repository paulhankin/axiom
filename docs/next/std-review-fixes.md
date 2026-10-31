## Standard library

- `jwt.verify` checks a token's member names for duplicates in n log n time (a header
  of many members took quadratic time, before its signature was checked).
- `http.request` refuses proxy credentials too long for its header with `BadUrl`; long
  ones made the client trap.
- `kdf.scrypt` refuses parameters whose work, n · r · p, exceeds `SCRYPT_MAX_WORK`
  (2^28) with `TooExpensive`, as it did those needing too much memory: parameters from
  a stored hash cannot make it run for hours.
- The WebSocket client fails (`NetError`, `HandshakeFailed`) if the system cannot give
  it random bytes, instead of masking frames with zeros.
- `dns.Resolver.lookup_ip` returns only the addresses of the name asked about and of
  the names its CNAMEs lead to (at most 8); a reply could add addresses of any other
  name. A query fails with `NetError` if the system cannot give it a random id.
- An HTTP server response with status 1xx, 204 or 304 that a handler flushes is no
  longer sent chunked (with a `0` terminator that a client on the same connection
  took for the start of the next response); its body is dropped.
