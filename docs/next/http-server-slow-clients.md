## Standard library

- The `std/net/http` server no longer lets a few idle or slow clients take it offline
  (issue #54). Its worker threads (`Options.threads`, still 8) now only answer requests:
  `run()`'s own thread waits on every connection with a `net.Poller` and hands one to a
  worker when bytes arrive, and the worker reads without waiting, so a connection
  sending its TLS handshake, request head or body slowly, or idle between keep-alive
  requests, holds no thread. A connection used to hold a worker for as long as it was
  open (up to `idle_timeout_ms` per read), so as many idle sockets as workers stopped the
  server. New `Options` fields, with their defaults:
  - `head_timeout_ms` (5000): a request head must arrive within this long of its first
    byte (a new connection's, of the accept or the TLS handshake), else 408;
  - `handshake_timeout_ms` (5000): the TLS handshake's deadline;
  - `min_body_rate` (500 bytes per second, after the first `head_timeout_ms`; 0 for
    none): a slower request body gets 408;
  - `max_connections` (512): at the limit, the connection idle longest is closed to
    make room (or new connections wait in the listen backlog);
  - `max_connections_per_addr` (64; IPv6 addresses counted by /64, 0 for no limit):
    more connections from one address are closed as they are accepted.

  `run()`'s thread no longer serves connections itself: there are `threads` workers
  besides it. A program that builds `Options` with a struct literal must add the new
  fields; one that starts from `Options.default()` gets them. `std/net/tls` can run a server handshake in steps (`server_start`,
  `Conn.handshake_step` with `Conn.nonblocking`), `net.poll_one` waits for one socket,
  and `net.Poller.clear` empties a poller.
