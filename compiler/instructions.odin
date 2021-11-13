package compiler

Instruction :: union {
	InstExit,
	InstNoOp,
	InstJump,
	InstJumpTrue,
	InstJumpFalse,

	InstAllocStack,
	InstPop,

	InstLoadGlobalPtr,
	InstLoadLocalPtr,
	InstLoadPtr,
	InstStorePtr,

	InstPushS64,
	InstNegateS64,
	InstAddS64,
	InstSubS64,
	InstMulS64,
	InstDivS64,
	InstModS64,
	InstEqualS64,
	InstNotEqualS64,
	InstLessThanS64,
	InstLessThanEqualS64,
	InstGreaterThanS64,
	InstGreaterThanEqualS64,
	InstPrintS64,

	InstPushBool,
	InstNegateBool,
	InstAndBool,
	InstOrBool,
	InstEqualBool,
	InstNotEqualBool,
	InstPrintBool,

	InstPushPtr,
	InstAddPtr,
	InstSubPtr,
	InstPrintPtr,
}

InstExit      :: struct {}
InstNoOp      :: struct {}
InstJump      :: struct { location: uint }
InstJumpTrue  :: struct { location: uint }
InstJumpFalse :: struct { location: uint }

InstAllocStack :: struct { size: uint }
InstPop        :: struct { size: uint }

InstLoadGlobalPtr  :: struct { offset: uint }
InstLoadLocalPtr   :: struct { offset: uint }
InstLoadPtr        :: struct { size: uint }
InstStorePtr       :: struct { size: uint }

InstPushS64             :: struct { value: i64 }
InstNegateS64           :: struct {}
InstAddS64              :: struct {}
InstSubS64              :: struct {}
InstMulS64              :: struct {}
InstDivS64              :: struct {}
InstModS64              :: struct {}
InstEqualS64            :: struct {}
InstNotEqualS64         :: struct {}
InstLessThanS64         :: struct {}
InstLessThanEqualS64    :: struct {}
InstGreaterThanS64      :: struct {}
InstGreaterThanEqualS64 :: struct {}
InstPrintS64            :: struct {}

InstPushBool     :: struct { value: bool }
InstNegateBool   :: struct {}
InstAndBool      :: struct {}
InstOrBool       :: struct {}
InstEqualBool    :: struct {}
InstNotEqualBool :: struct {}
InstPrintBool    :: struct {}

InstPushPtr      :: struct { value: uintptr }
InstAddPtr       :: struct {}
InstSubPtr       :: struct {}
InstPrintPtr     :: struct {}
