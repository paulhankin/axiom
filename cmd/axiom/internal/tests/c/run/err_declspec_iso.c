/* flags: --c-std=c17 --target=x86_64 */
/* expect-error: C2024 */
/* under an ISO standard __declspec is an identifier but on Windows */
__declspec(align(16)) int x;
