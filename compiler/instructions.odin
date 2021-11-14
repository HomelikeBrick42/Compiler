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

	InstPushU8,
	InstAddU8,
	InstSubU8,
	InstMulU8,
	InstDivU8,
	InstModU8,
	InstEqualU8,
	InstNotEqualU8,
	InstLessThanU8,
	InstLessThanEqualU8,
	InstGreaterThanU8,
	InstGreaterThanEqualU8,
	InstPrintU8,

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
	InstMulPtr,
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

InstPushU8             :: struct { value: u8 }
InstAddU8              :: struct {}
InstSubU8              :: struct {}
InstMulU8              :: struct {}
InstDivU8              :: struct {}
InstModU8              :: struct {}
InstEqualU8            :: struct {}
InstNotEqualU8         :: struct {}
InstLessThanU8         :: struct {}
InstLessThanEqualU8    :: struct {}
InstGreaterThanU8      :: struct {}
InstGreaterThanEqualU8 :: struct {}
InstPrintU8            :: struct {}

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
InstMulPtr       :: struct {}
InstPrintPtr     :: struct {}
