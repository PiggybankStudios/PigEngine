(module $assembFuncs
	(memory (import "env" "memory") 1)
	(func (export "abort") unreachable)
	(func (export "memset")
		(param $ptr i32) (param $value i32) (param $num i32)
		(result i32)
		
		local.get $ptr
		local.get $value
		local.get $num
		memory.fill
		
		local.get $ptr
	)
	(func (export "memcpy")
		(param $dst i32) (param $src i32) (param $count i32)
		(result i32)

		local.get $dst
		local.get $src
		local.get $count
		memory.copy

		local.get $dst
	)
)