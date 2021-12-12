package x64emit

Register :: enum u8 {
	RAX = 0,
	RCX = 1,
	RDX = 2,
	RBX = 3,
	RSP = 4,
	RBP = 5,
	RSI = 6,
	RDI = 7,
	R8  = 8,
	R9  = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15,
}

Scale :: enum u8 {
	X1 = 0,
	X2 = 1,
	X4 = 2,
	X8 = 3,
}

Mode :: enum u8 {
	Indirect              = 0,
	ByteDisplacedIndirect = 1,
	DisplacedIndirect     = 2,
	Direct                = 3,
}

Emitter :: struct {
	code: [dynamic]byte,
}

Emitter_Create :: proc() -> Emitter {
	return {
		code = make([dynamic]byte),
	}
}

Emitter_Destroy :: proc(emitter: ^Emitter) {
	delete(emitter.code)
}

EmitByte :: proc(emitter: ^Emitter, value: byte) {
	append(&emitter.code, value)
}

EmitREX :: proc(emitter: ^Emitter, rx: Register, base: Register) {
	EmitByte(emitter, 0x48 | ((cast(u8) base >> 3) << 0) | ((cast(u8) rx >> 3) << 2))
}

EmitREXIndexed :: proc(emitter: ^Emitter, rx: Register, base: Register, index: Register) {
	EmitByte(emitter, 0x48 | ((cast(u8) base >> 3) << 0) | ((cast(u8) index >> 3) << 1) | ((cast(u8) rx >> 3) << 2))
}

EmitU32 :: proc(emitter: ^Emitter, value: u32) {
	EmitByte(emitter, byte((value >> 0) & 0xFF))
	EmitByte(emitter, byte((value >> 8) & 0xFF))
	EmitByte(emitter, byte((value >> 16) & 0xFF))
	EmitByte(emitter, byte((value >> 24) & 0xFF))
}

EmitModRxRm :: proc(emitter: ^Emitter, mod: u8, rx: u8, rm: u8) {
	assert(mod < 4)
	assert(rx < 16)
	assert(rm < 16)
	EmitByte(emitter, (rm & 7) | ((rx & 7) << 3) | (mod << 6))
}

// add rax, [rcx]
// rx = RAX, operand = RCX
EmitDirect :: proc(emitter: ^Emitter, rx: u8, operand: Register) {
	EmitModRxRm(emitter, auto_cast Mode.Direct, rx, auto_cast operand)
}

// add rax, [rcx]
// rx = RAX, base = RCX
EmitIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register) {
	assert(base != .RSP)
	assert(base != .RBP)
	EmitModRxRm(emitter, auto_cast Mode.Indirect, rx, auto_cast base)
}

// add rax, [rip + 0x12345678]
// rx = RAX, displacement = 0x12345678
EmitDisplacedIndirectRIP :: proc(emitter: ^Emitter, rx: u8, displacement: u32) {
	EmitModRxRm(emitter, auto_cast Mode.Indirect, rx, auto_cast Register.RBP)
	EmitU32(emitter, displacement)
}

// add rax, [rcx + 0x12]
// rx = RAX, base = RCX, displacement = 0x12
EmitByteDisplacedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, displacement: u8) {
	assert(base != .RSP)
	EmitModRxRm(emitter, auto_cast Mode.ByteDisplacedIndirect, rx, auto_cast base)
	EmitByte(emitter, displacement)
}

// add rax, [rcx + 0x12345678]
// rx = RAX, base = RCX, displacement = 0x12345678
EmitDisplacedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, displacement: u32) {
	assert(base != .RSP)
	EmitModRxRm(emitter, auto_cast Mode.DisplacedIndirect, rx, auto_cast base)
	EmitU32(emitter, displacement)
}

// add rax, [rcx + 4*rdx]
// rx = RAX, base = RCX, index = RDX, scale = X4
EmitIndexedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, index: Register, scale: Scale) {
	EmitModRxRm(emitter, auto_cast Mode.Indirect, rx, auto_cast Register.RSP)
	EmitModRxRm(emitter, auto_cast scale, auto_cast index, auto_cast base)
}

// add rax, [rcx + 4*rdx + 0x12]
// rx = RAX, base = RCX, index = RDX, scale = X4, displacement = 0x12
EmitByteDisplacedIndexedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, index: Register, scale: Scale, displacement: u8) {
	EmitModRxRm(emitter, auto_cast Mode.ByteDisplacedIndirect, rx, auto_cast Register.RSP)
	EmitModRxRm(emitter, auto_cast scale, auto_cast index, auto_cast base)
	EmitByte(emitter, displacement)
}

// add rax, [rcx + 4*rdx + 0x12345678]
// rx = RAX, base = RCX, index = RDX, scale = X4, displacement = 0x12345678
EmitDisplacedIndexedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, index: Register, scale: Scale, displacement: u32) {
	EmitModRxRm(emitter, auto_cast Mode.DisplacedIndirect, rx, auto_cast Register.RSP)
	EmitModRxRm(emitter, auto_cast scale, auto_cast index, auto_cast base)
	EmitU32(emitter, displacement)
}

// add rax, [0x12345678]
// rx = RAX, displacement = 0x12345678
EmitDisplaced :: proc(emitter: ^Emitter, rx: u8, displacement: u32) {
	EmitModRxRm(emitter, auto_cast Mode.Indirect, rx, auto_cast Register.RSP)
	EmitModRxRm(emitter, auto_cast Scale.X1, auto_cast Register.RSP, auto_cast Register.RBP)
	EmitU32(emitter, displacement)
}

EmitAddToRegister :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x03)
}

EmitAddToMemory :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x01)
}
