package compiler

import "core:fmt"
import "core:mem"

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

			case InstLoadGlobalPtr: {
				VM_Push(vm, cast(uintptr) vm.sb - cast(uintptr) inst.offset)
			}

			case InstLoadLocalPtr: {
				VM_Push(vm, cast(uintptr) vm.bp - cast(uintptr) inst.offset)
			}

			case InstStorePtr: {
				ptr := VM_Pop(vm, rawptr)
				mem.copy(ptr, vm.sp, cast(int) inst.size)
				(cast(^uintptr) &vm.sp)^ += cast(uintptr) inst.size
			}

			case InstLoadPtr: {
				ptr := VM_Pop(vm, rawptr)
				(cast(^uintptr) &vm.sp)^ -= cast(uintptr) inst.size
				mem.copy(vm.sp, ptr, cast(int) inst.size)
			}

			case InstPushU8: {
				VM_Push(vm, inst.value)
			}

			case InstAddU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a + b)
			}

			case InstSubU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a - b)
			}

			case InstMulU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a * b)
			}

			case InstDivU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a / b)
			}

			case InstModU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a % b)
			}

			case InstEqualU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a == b)
			}

			case InstNotEqualU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a != b)
			}

			case InstLessThanU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a < b)
			}

			case InstLessThanEqualU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a <= b)
			}

			case InstGreaterThanU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a > b)
			}

			case InstGreaterThanEqualU8: {
				b := VM_Pop(vm, u8)
				a := VM_Pop(vm, u8)
				VM_Push(vm, a >= b)
			}

			case InstPrintU8: {
				value := VM_Pop(vm, u8)
				fmt.println(value)
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

			case InstPushPtr: {
				VM_Push(vm, inst.value)
			}

			case InstAddPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a + b)
			}

			case InstSubPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a - b)
			}

			case InstMulPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a * b)
			}

			case InstEqualPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a == b)
			}

			case InstNotEqualPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a != b)
			}

			case InstLessThanPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a < b)
			}

			case InstLessThanEqualPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a <= b)
			}

			case InstGreaterThanPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a > b)
			}

			case InstGreaterThanEqualPtr: {
				b := VM_Pop(vm, uintptr)
				a := VM_Pop(vm, uintptr)
				VM_Push(vm, a >= b)
			}

			case InstPrintPtr: {
				value := VM_Pop(vm, rawptr)
				fmt.println(value)
			}

			case InstS64ToU8: {
				value := VM_Pop(vm, i64)
				VM_Push(vm, cast(u8) value)
			}

			case InstU8ToS64: {
				value := VM_Pop(vm, u8)
				VM_Push(vm, cast(i64) value)
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
