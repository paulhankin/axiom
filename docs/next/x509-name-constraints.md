## Standard library

- `std/crypto/x509`: a CA's name constraints now apply to the intermediate certificates
  below it as well as to the leaf (RFC 5280 6.1.4). A chain through a sub-CA whose
  names the constraints exclude is refused with `error.NameConstraint`.
