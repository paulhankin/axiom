// target.h - architectures, CPU presets and processor extensions, with the
// names and bits of std/os/cpu.ax (selfhost/src/target.ax is the same).
#ifndef AX_TARGET_H
#define AX_TARGET_H

#include <stdbool.h>
#include <stdint.h>

enum { ARCH_WASM32 = 1, ARCH_WASM64 = 2, ARCH_X64 = 3, ARCH_A64 = 4 };
enum { OS_LINUX = 0, OS_DARWIN = 1, OS_WINDOWS = 2 };

// x86-64 extensions (std/os/cpu.ax's X_* bits)
#define X_AVX ((uint64_t)0x100)
#define X_FMA ((uint64_t)0x400)

extern int g_arch;
extern int g_os;
extern uint64_t g_features;  // the target's extensions (--cpu, --features)
extern const char *g_cpu;    // the --cpu name

uint64_t feature_bit(const char *name);  // on g_arch; 0 if it has no such extension
bool feature_known(const char *name);    // an extension of any architecture
const char *feature_names(void);         // g_arch's, for messages
const char *features_string(uint64_t bits);  // "+a,+b"
uint64_t feature_closure(uint64_t bits);     // with everything they imply
bool cpu_preset(int arch, const char *name, uint64_t *out);
const char *apply_features(const char *spec);  // "+a,-b": NULL, or the first unknown name
const char *llvm_features(uint64_t bits);      // LLVM's target-features string
uint64_t parse_feature_set(const char *spec, bool *foreign);  // "a,b" (for @features/@multiversion)
uint64_t features_from_llvm(const char *llvm_features);       // "+avx2,+fma,-avx512f,..."

#endif
