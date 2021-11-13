package compiler

Instruction :: union {
	InstExit,
	InstNoOp,

	InstAllocStack,
	InstPushS64,
	InstPop,

	InstLoadGlobal,
	InstStoreGlobal,
	InstLoadLocal,
	InstStoreLocal,

	InstNegateS64,
	InstAddS64,
	InstSubS64,
	InstMulS64,
	InstDivS64,
	InstModS64,

	InstPrintS64,
}

InstExit :: struct {}
InstNoOp :: struct {}

InstAllocStack :: struct { size: uint }
InstPushS64    :: struct { value: i64 }
InstPop        :: struct { size: uint }

InstLoadGlobal  :: struct { offset: uint, size: uint }
InstStoreGlobal :: struct { offset: uint, size: uint }
InstLoadLocal   :: struct { offset: uint, size: uint }
InstStoreLocal  :: struct { offset: uint, size: uint }

InstNegateS64 :: struct {}
InstAddS64    :: struct {}
InstSubS64    :: struct {}
InstMulS64    :: struct {}
InstDivS64    :: struct {}
InstModS64    :: struct {}

InstPrintS64 :: struct {}
