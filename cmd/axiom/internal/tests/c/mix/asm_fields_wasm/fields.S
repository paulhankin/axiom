;; Module fields of an assembly source (a preprocessed one: the pointers'
;; type is the target's)
#if __AXIOM_PTR_WIDTH__ == 64
#define PTR i64
#else
#define PTR i32
#endif
(module
  (memory PTR 1)
  (table $tab 1 funcref)
  (type $iv (func (param i32)))
  ;; a tag the C unit's asm throws and catches, by name
  (tag $err (export "err") (type $iv))
  ;; a mutable global both units read and write, exported
  (global $counter (export "counter") (mut i32) (i32.const 5))
  ;; an immutable global of the C unit's (its file-scope asm), and one
  ;; initialized from it
  (import "env" "base" (global $base i32))
  (global $derived i32 (global.get $base))
  (global $big i64 (i64.const -1234567890123))
  (global $half f64 (f64.const 0x1.8p1))
  (global $vec v128 (v128.const i32x4 1 2 3 4))
  (global $exn (mut exnref) (ref.null exn))
  ;; a passive data segment and an element segment
  (data $msg "hello" "\00")
  (elem $fs func $two)
  (elem $unused declare func $two)
  (func $two (result i32) i32.const 2)
  ;; the start function: runs when the module is instantiated
  (func $init
    (global.set $counter (i32.add (global.get $counter) (i32.const 10))))
  (start $init)
  (func $bump (param i32) (result i32)
    (global.set $counter (i32.add (global.get $counter) (local.get 0)))
    global.get $counter)
  (func $derived_value (result i32) global.get $derived)
  (func $big_value (result i64) global.get $big)
  (func $half_value (result f64) global.get $half)
  (func $lane2 (result i32) (i32x4.extract_lane 2 (global.get $vec)))
  (func $exn_is_null (result i32) (ref.is_null (global.get $exn)))
  ;; copies the passive segment to dst, then drops it (a copy of more than
  ;; nothing after that traps)
  (func $copy_msg (param $dst PTR) (result i32)
    (memory.init $msg (local.get $dst) (i32.const 0) (i32.const 6))
    data.drop $msg
    i32.const 6)
  ;; puts $two in the table's slot `slot` (C's function pointer's) and drops
  ;; the segment
  (func $patch (param $slot i32)
    (table.init $tab $fs (local.get $slot) (i32.const 0) (i32.const 1))
    elem.drop $fs)
  ;; table.copy of the program's table (to itself: no change)
  (func $copy_slot (param $slot i32)
    (table.copy (local.get $slot) (local.get $slot) (i32.const 1)))
  ;; f() throws $err (the C unit's raise_err): its value + 1000, or -1
  (func $catch_err (param $f PTR) (result i32)
    (block $h (result i32)
      (try_table (catch $err $h)
        (call_indirect (type $iv) (i32.const 3)
#if __AXIOM_PTR_WIDTH__ == 64
          (i32.wrap_i64 (local.get $f))
#else
          (local.get $f)
#endif
          ))
      (return (i32.const -1)))
    i32.const 1000
    i32.add)
)
