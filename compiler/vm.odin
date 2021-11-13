package compiler

import "core:fmt"

VM :: struct {
	program: []Instruction,
	ip: uint,
	stack: []u8,
	sb: rawptr,
	sp: rawptr,
	bp: rawptr,
}

VM_Create :: proc(program: []Instruction, stack_size: uint) -> VM {
	vm: VM
	vm.program = program
	vm.ip      = 0
	vm.stack   = make([]u8, stack_size)
	vm.sb      = cast(rawptr) (cast(uintptr) &vm.stack[0] + cast(uintptr) stack_size)
	vm.sp      = vm.sb
	vm.bp      = vm.sp
	return vm
}

VM_Destroy :: proc(vm: ^VM) {
	delete(vm.stack)
}

VM_Push :: proc(vm: ^VM, value: $T) {
	(cast(^uintptr) &vm.sp)^ -= cast(uintptr) size_of(T)
	(cast(^T) vm.sp)^ = value
}

VM_Pop :: proc(vm: ^VM, $T: typeid) -> T {
	value := (cast(^T) vm.sp)^
	(cast(^uintptr) &vm.sp)^ += cast(uintptr) size_of(T)
	return value
}

VM_Run :: proc(vm: ^VM) -> Maybe(string) {
	for {
		// fmt.println(vm.program[vm.ip])
		switch inst in vm.program[vm.ip] {
			case InstExit: {
				return nil
			}

			case InstNoOp: {
			}

			case InstJump: {
				vm.ip = inst.location
				continue
			}

			case InstJumpTrue: {
				value := VM_Pop(vm, bool)
				if value {
					vm.ip = inst.location
				} else {
					vm.ip += 1
				}
				continue
			}

			case InstJumpFalse: {
				value := VM_Pop(vm, bool)
				if !value {
					vm.ip = inst.location
				} else {
					vm.ip += 1
				}
				continue
			}

			case InstAllocStack: {
				(cast(^uintptr) &vm.sp)^ -= cast(uintptr) inst.size
			}

			case InstPop: {
				(cast(^uintptr) &vm.sp)^ += cast(uintptr) inst.size
			}

			case InstLoadGlobal: {
				(cast(^uintptr) &vm.sp)^ -= cast(uintptr) inst.size
				copy((cast([^]u8) (cast(uintptr) vm.sp))[:inst.size], (cast([^]u8) (cast(uintptr) vm.sb - cast(uintptr) inst.offset))[:inst.size])
			}

			case InstStoreGlobal: {
				copy((cast([^]u8) (cast(uintptr) vm.sb - cast(uintptr) inst.offset))[:inst.size], (cast([^]u8) (cast(uintptr) vm.sp))[:inst.size])
				(cast(^uintptr) &vm.sp)^ += cast(uintptr) inst.size
			}

			case InstLoadLocal: {
				(cast(^uintptr) &vm.sp)^ -= cast(uintptr) inst.size
				copy((cast([^]u8) (cast(uintptr) vm.sp))[:inst.size], (cast([^]u8) (cast(uintptr) vm.bp - cast(uintptr) inst.offset))[:inst.size])
			}

			case InstStoreLocal: {
				copy((cast([^]u8) (cast(uintptr) vm.bp - cast(uintptr) inst.offset))[:inst.size], (cast([^]u8) (cast(uintptr) vm.sp))[:inst.size])
				(cast(^uintptr) &vm.sp)^ += cast(uintptr) inst.size
			}

			case InstPushS64: {
				VM_Push(vm, inst.value)
			}

			case InstNegateS64: {
				value := VM_Pop(vm, i64)
				VM_Push(vm, -value)
			}

			case InstAddS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a + b)
			}

			case InstSubS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a - b)
			}

			case InstMulS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a * b)
			}

			case InstDivS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a / b)
			}

			case InstModS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a % b)
			}

			case InstEqualS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a == b)
			}

			case InstNotEqualS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a != b)
			}

			case InstLessThanS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a < b)
			}

			case InstLessThanEqualS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a <= b)
			}

			case InstGreaterThanS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a > b)
			}

			case InstGreaterThanEqualS64: {
				b := VM_Pop(vm, i64)
				a := VM_Pop(vm, i64)
				VM_Push(vm, a >= b)
			}

			case InstPrintS64: {
				value := VM_Pop(vm, i64)
				fmt.println(value)
			}

			case InstPushBool: {
				VM_Push(vm, inst.value)
			}

			case InstNegateBool: {
				value := VM_Pop(vm, bool)
				VM_Push(vm, !value)
			}

			case InstAndBool: {
				b := VM_Pop(vm, bool)
				a := VM_Pop(vm, bool)
				VM_Push(vm, a && b)
			}

			case InstOrBool: {
				b := VM_Pop(vm, bool)
				a := VM_Pop(vm, bool)
				VM_Push(vm, a || b)
			}

			case InstEqualBool: {
				b := VM_Pop(vm, bool)
				a := VM_Pop(vm, bool)
				VM_Push(vm, a == b)
			}

			case InstNotEqualBool: {
				b := VM_Pop(vm, bool)
				a := VM_Pop(vm, bool)
				VM_Push(vm, a != b)
			}

			case InstPrintBool: {
				value := VM_Pop(vm, bool)
				fmt.println(value)
			}

			case: {
				message := "unreachable Instruction default case in VM_Run"
				assert(false, message)
				return message
			}
		}
		vm.ip += 1
	}
}
