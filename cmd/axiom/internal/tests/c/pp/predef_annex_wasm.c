/* Annexes F and G are not claimed on WebAssembly, which has no floating-point
   environment (no rounding modes, no exception flags); the decimal types'
   (in software) and Annex H, by them (H.1p2), are. */
/* flags: --c-std=c23 --target=wasm32 */
__STDC_IEC_559__ __STDC_IEC_60559_BFP__ __STDC_IEC_559_COMPLEX__ __STDC_IEC_60559_COMPLEX__ __STDC_IEC_60559_DFP__ __STDC_IEC_60559_TYPES__
