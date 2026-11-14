;; a mutable global, which main.c's asm declares immutable
(global $count (mut i32) (i32.const 0))
