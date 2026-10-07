# 0019: Faster cryptography with wide vectors and the remaining instructions

- Type: feature
- Status: open
- Area: std/crypto, std/net/tls

## Today

Issue 0016 gave std/crypto hardware paths for AES-GCM, SHA-1, SHA-256,
SHA-512 (AArch64), SHA-3 (AArch64) and CRC (docs/performance.md, "Hardware
instructions"). Still behind C libraries:

- **AES-GCM**: about 1.8 GB/s against OpenSSL's 17.7 GB/s on the same
  machine. OpenSSL interleaves AES and GHASH, works on 256- or 512-bit
  registers (VAES, VPCLMULQDQ; the builtins accept `u8x32`/`u8x64` and
  `u64x4`/`u64x8` already) and reduces once per eight blocks.
- **SHA-512 on x86-64**: portable. The SHA512 instructions (Arrow Lake,
  Lunar Lake) or AVX2 message scheduling would help.
- **SHA-3 on x86-64**: portable; AVX-512 `vpternlogq` and `vprolq` would
  help (needs AVX-512VL forms, issue 0018). Under the self-hosted compiler
  the 25-word state lives in memory, which makes SHA-3 several times
  slower than under the C compiler.
- **ChaCha20-Poly1305, BLAKE2, Argon2**: 128-bit vectors; 256/512-bit
  versions over several blocks with `@multiversion`.
- **curve25519, ec, rsa**: MULX/ADCX/ADOX (BMI2, ADX) for the limb
  arithmetic, keeping it constant time.
- **PSTATE.DIT** on AArch64 around secret-dependent code.
- **TLS**: prefer AES-GCM when the processor has AES instructions and
  ChaCha20-Poly1305 when it does not.
