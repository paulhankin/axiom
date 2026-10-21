## Security fixes

- `std/crypto/x509` refuses any certificate path through the eight eMudhra intermediate
  CA keys revoked for key compromise in October 2026 (EM DV, OV and EV TLS CA and EM OV
  CS CA, each G3B and G3C, including their cross-signed copies), with the new error
  `Distrusted`, even while the system's CA bundle still holds the roots that signed
  them. `std/net/tls` clients (and servers checking client certificates) and
  `std/encoding/jwt` get the check through `verify_chain`. The keys are listed by the
  SHA-256 of their SubjectPublicKeyInfo in `x509.DISTRUSTED`; `x509.distrusted(cert)`
  tells whether a certificate's key is on it. Error sets that hold `X509Error`
  (`TlsError`, `HttpError`, `JwtError`) gain `Distrusted`.
