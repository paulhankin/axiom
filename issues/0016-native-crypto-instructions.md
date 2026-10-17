# 0016: Hardware crypto instructions, and their use in std/crypto

- Type: feature
- Status: fixed (3facaaa)
- Area: language/compiler (intrinsics), std/crypto, std/hash, std/net/tls

## Today

std/crypto is portable Axiom, written to run in constant time without
hardware help:
- **AES and GCM** (`aes.ax`, `aes_gcm.ax`): the S-box is 16 vector-shuffle
  lookups over all 256 entries, and GHASH multiplies with integer
  multiplications masked so that no memory access or branch depends on a
  secret. This is safe, but several times slower than AES-NI or the ARMv8
  AES instructions.
- **Hashes** (`sha1.ax`, `sha2.ax`, `sha3.ax`): scalar.
- **CRC32** (`std/hash/crc.ax`): table-driven.

The compiler has no intrinsics for these instructions. The CPU features it
knows (cmd/axiom/src/target.ax) do not include them.

## Wanted

1. **Intrinsics in both compilers**, gated by target features. Each one
   needs a `--features` name, inclusion in the right `--cpu` presets, and
   encodings in `x64.ax`/`a64.ax` (LLVM intrinsics in the C compiler):
   - x86-64: AES-NI (`aesenc`, `aesenclast`, `aesdec`, `aesdeclast`,
     `aeskeygenassist`, `aesimc`); PCLMULQDQ (carry-less multiply, for
     GHASH and CRC folding); SHA-NI (`sha1rnds4`, `sha1nexte`, `sha1msg1/2`,
     `sha256rnds2`, `sha256msg1/2`); SSE4.2 `crc32`. Later, VAES and
     VPCLMULQDQ on 256- and 512-bit registers (needs 0015), GFNI, and
     SHA512 / SM3 / SM4 (Arrow Lake and later).
   - AArch64 (FEAT_AES, PMULL, SHA1, SHA256, SHA512, SHA3, SM3/SM4, CRC32):
     `aese`, `aesd`, `aesmc`, `aesimc`; `pmull`, `pmull2`; `sha1c/p/m/h`,
     `sha1su0/1`, `sha256h/h2/su0/su1`; `sha512h/h2/su0/su1`; `eor3`,
     `rax1`, `xar`, `bcax` (SHA-3); `crc32*`/`crc32c*`. Apple silicon and
     current Neoverse cores have all of them.
   - wasm: none exist in standard wasm; keep the portable code.
2. **Choosing the implementation.** Each primitive gets a hardware path and
   keeps the portable constant-time path as its fallback. Both are tested on
   every target, and the hardware path is checked against the fallback on
   random inputs. The choice is made:
   - at build time, when the target guarantees the feature (`--cpu=native`,
     `x86-64-v3`+, `apple-m1`, every AArch64 Mac); or
   - at run time, from CPU feature detection with a dispatch on first use
     (issue 0015's detection and multi-versioning; until that exists, a
     small per-module check through a host ABI query such as
     `ax_cpu_features()`).
3. **Use in std:**
   - `crypto/aes`, `crypto/aes_gcm`: AES rounds and key expansion with
     AES-NI or ARMv8 AES. GHASH with PCLMULQDQ/PMULL, using the usual
     aggregated reduction over 4 or 8 blocks. Interleave several blocks per
     loop (CTR mode has no dependency between blocks).
   - `crypto/sha1`, `crypto/sha2`: SHA-NI and the ARMv8 SHA1/SHA256
     instructions. SHA-512 with the ARMv8.2 SHA512 instructions, or AVX2
     message scheduling where they are missing.
   - `crypto/sha3`: the ARMv8.2 SHA3 instructions (`eor3`, `rax1`, `xar`,
     `bcax`) for Keccak-f. AVX2/AVX-512 (`vpternlogq`) once 0015 is done.
   - `crypto/chacha20poly1305`, `crypto/blake2`, `crypto/argon2`: no
     dedicated instructions, but SIMD across 4 to 16 blocks or lanes (128-bit
     vectors today, wider with 0015). Poly1305 with 64×64→128 multiplies.
   - `crypto/curve25519`, `crypto/ec`, `crypto/rsa`: `mulx`/`adcx`/`adox`
     (BMI2/ADX) for the field and bignum arithmetic, keeping it constant
     time.
   - `hash/crc`: the `crc32` instructions for CRC-32C, and CLMUL/PMULL
     folding for CRC-32 (zip, gzip, png).
   - `crypto/random`: unchanged. The OS generator stays the source; RDRAND
     and RDSEED are not to be trusted alone.
   - `net/tls`: picks up the faster AES-GCM. Cipher-suite order could prefer
     AES-GCM when the hardware has AES, and ChaCha20-Poly1305 when it does
     not.
4. **Constant time stays a requirement.** The AES and CLMUL instructions
   are constant time on current CPUs (Intel's DOIT list, ARM's DIT). On
   AArch64 the runtime could set PSTATE.DIT around secret-dependent code
   where the CPU supports it. Hardware paths must not add table lookups or
   branches on secrets.
5. **Benchmarks.** Add the hardware paths to docs/performance.md, against
   OpenSSL/BoringSSL. These primitives are where std is furthest behind C
   libraries today.

## Notes

- Testing: qemu emulates all of these (`-cpu max`), so the suite can run
  both paths on any host. Use the NIST/RFC test vectors that the modules
  already check.
- Related: 0015 (wide vectors, conditional compilation, runtime CPU
  detection and dispatch). The dispatch mechanism should be shared, not
  invented twice.

## Resolution

- **Builtins in both compilers** (spec §13.5): `@x86_aesenc`,
  `@x86_aesenclast`, `@x86_aesdec`, `@x86_aesdeclast`, `@x86_aesimc`,
  `@x86_aeskeygenassist`, `@x86_clmul` (also on 256- and 512-bit vectors,
  one VAES/VPCLMULQDQ instruction where the build has them), the SHA-NI
  rounds and schedules, `@x86_crc32c`; on AArch64 `@arm_aese`/`aesd`/
  `aesmc`/`aesimc`, `@arm_pmull`/`pmull2`, the SHA1, SHA256 and SHA512
  instructions, `@arm_eor3`/`bcax`/`rax1`/`xar` and `@arm_crc32`/`crc32c`.
  Each needs its architecture and extension (E0828), checked like the
  other target builtins of 0015. The self-hosted backends encode them
  (`x64enc.ax`, `a64.ax`); the C compiler calls LLVM's intrinsics.
  `tests/run/insn.ax` (written by `tools/gen_insn_tests.py` from a Python
  model) checks each against known results.
- **std**, each path chosen at run time with `@multiversion` and checked
  against the portable code on random inputs in its tests:
  - `crypto/aes_gcm` (and `crypto/aes`, which uses it): AES-NI or ARMv8
    AES rounds, eight CTR blocks at a time; GHASH by PCLMULQDQ or PMULL
    (Karatsuba, four blocks per reduction).
  - `crypto/sha1`, `crypto/sha2`: SHA-NI on x86-64; the SHA1, SHA256 and
    SHA512 instructions on AArch64.
  - `crypto/sha3`: Keccak-f with EOR3, RAX1, XAR and BCAX on AArch64.
  - `hash/crc`: new `crc32` and `crc32c` functions (used by `Crc` for those
    two presets): SSE4.2 `crc32` or ARM CRC32C for CRC-32C; PCLMULQDQ
    folding or ARM CRC32 for CRC-32, which `compress/deflate.crc32` (gzip,
    zip, PNG) now uses. Without hardware, slicing by 8 for both.
- **Benchmarks** in docs/performance.md, "Hardware instructions": on the
  test machine AES-128-GCM went from 155 to 1773 MB/s, SHA-256 from 382 to
  2859 MB/s, SHA-1 from 442 to 3121 MB/s, CRC-32 from 2.3 to 22 GB/s and
  CRC-32C from 0.74 to 16 GB/s.

The hardware paths are constant time as the portable ones are: no table
lookups or branches on secrets.

Not done, now issue 0019: GCM with VAES/VPCLMULQDQ over 256/512 bits
(OpenSSL is still several times faster), SHA-512 with x86 SHA512 or AVX2
scheduling, SHA-3 with AVX-512 `vpternlogq`, ChaCha20/BLAKE2/Argon2 over
wider vectors, MULX/ADX in the bignum and curve arithmetic, PSTATE.DIT on
AArch64, and TLS cipher-suite order by hardware.
