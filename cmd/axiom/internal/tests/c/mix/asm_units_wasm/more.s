;; a plain assembly source: module fields without (module ...)
(import "env" "fma3" (func $f3 (param i32 i32 i32) (result i32)))
(func $via_s (param i32) (result i32)
  (call $f3 (local.get 0) (i32.const 3) (i32.const 2)))
