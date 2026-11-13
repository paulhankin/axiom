;; WebAssembly functions in the text format (a preprocessed source)
(module
  (import "env" "c_helper" (func $helper (param i32) (result i32)))
  (func $fma3 (param i32 i32 i32) (result i32)
    local.get 0 local.get 1 i32.mul local.get 2 i32.add)
#ifdef __AXIOM_ASM_WASM__
  (func $twice_via_c (param $x i32) (result i32)
    (call $helper (i32.shl (local.get $x) (i32.const 1))))
#endif
  ;; a loop as tail calls: return_call does not grow the stack
  (func $count_down (param $n i32) (param $acc i32) (result i32)
    (if (result i32) (i32.eqz (local.get $n))
      (then (local.get $acc))
      (else (return_call $count_down (i32.sub (local.get $n) (i32.const 1))
                                     (i32.add (local.get $acc) (i32.const 2))))))
  ;; 1 if calling f throws (longjmp does), else 0
  (func $caught (param $f i32) (result i32)
    (block $h
      (try_table (catch_all $h) (call_indirect (param) (result) (local.get $f)))
      (return (i32.const 0)))
    i32.const 1)
  ;; f's exception caught with its reference, and thrown again
  (func $rethrown (param $f i32) (result i32)
    (throw_ref
      (block $h (result exnref)
        (try_table (catch_all_ref $h) (call_indirect (param) (result) (local.get $f)))
        (return (i32.const 0)))))
)
