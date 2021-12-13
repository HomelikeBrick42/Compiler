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
	assert((base & cast(Register) 7) != .RSP)
	assert((base & cast(Register) 7) != .RBP)
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
	assert((base & cast(Register) 7) != .RSP)
	EmitModRxRm(emitter, auto_cast Mode.ByteDisplacedIndirect, rx, auto_cast base)
	EmitByte(emitter, displacement)
}

// add rax, [rcx + 0x12345678]
// rx = RAX, base = RCX, displacement = 0x12345678
EmitDisplacedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, displacement: u32) {
	assert((base & cast(Register) 7) != .RSP)
	EmitModRxRm(emitter, auto_cast Mode.DisplacedIndirect, rx, auto_cast base)
	EmitU32(emitter, displacement)
}

// add rax, [rcx + 4*rdx]
// rx = RAX, base = RCX, index = RDX, scale = X4
EmitIndexedIndirect :: proc(emitter: ^Emitter, rx: u8, base: Register, index: Register, scale: Scale) {
	assert((base & cast(Register) 7) != .RBP)
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

// TODO: Remove these functions
EmitAddRegister :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x03)
}

EmitAddMemory :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x01)
}

EmitAddImmediate :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x81)
}

EmitSubRegister :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x2B)
}

EmitSubMemory :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x29)
}

EmitSubImmediate :: proc(emitter: ^Emitter) {
	EmitByte(emitter, 0x81)
}

Operation :: enum {
	Add,
	Sub,
}

RegisterOperations := [Operation]Maybe(u8){
	.Add = 0x03,
	.Sub = 0x2B,
}

MemoryOperations := [Operation]Maybe(u8){
	.Add = 0x01,
	.Sub = 0x29,
}

ImmediateOperation :: struct { opcode: u8, extention: u8 }
ImmediateOperations := [Operation]Maybe(ImmediateOperation){
	.Add = ImmediateOperation{ 0x81, 0x0 },
	.Sub = ImmediateOperation{ 0x81, 0x5 },
}

EmitOperationRegisterRegister :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source: Register) {
	EmitREX(emitter, destination, source)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitDirect(emitter, auto_cast destination, source)
}

EmitOperationRegisterMemory :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source: Register) {
	EmitREX(emitter, destination, source)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitIndirect(emitter, auto_cast destination, source)
}

EmitOperationRegisterMemoryByteDisplaced :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source: Register, source_displacement: u8) {
	EmitREX(emitter, destination, source)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitByteDisplacedIndirect(emitter, auto_cast destination, source, source_displacement)
}

EmitOperationRegisterMemoryDisplaced :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source: Register, source_displacement: u32) {
	EmitREX(emitter, destination, source)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitDisplacedIndirect(emitter, auto_cast destination, source, source_displacement)
}

EmitOperationRegisterSIB :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source_base: Register, source_scale: Scale, source_index: Register) {
	EmitREXIndexed(emitter, destination, source_base, source_index)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitIndexedIndirect(emitter, auto_cast destination, source_base, source_index, source_scale)
}

EmitOperationRegisterSIBByteDisplaced :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source_base: Register, source_scale: Scale, source_index: Register, source_displacement: u8) {
	EmitREXIndexed(emitter, destination, source_base, source_index)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitByteDisplacedIndexedIndirect(emitter, auto_cast destination, source_base, source_index, source_scale, source_displacement)
}

EmitOperationRegisterSIBDisplaced :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source_base: Register, source_scale: Scale, source_index: Register, source_displacement: u32) {
	EmitREXIndexed(emitter, destination, source_base, source_index)
	EmitByte(emitter, RegisterOperations[operation].?)
	EmitDisplacedIndexedIndirect(emitter, auto_cast destination, source_base, source_index, source_scale, source_displacement)
}

EmitOperationMemoryRegister :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source: Register) {
	EmitREX(emitter, source, destination)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitIndirect(emitter, auto_cast source, destination)
}

EmitOperationMemoryByteDisplacedRegister :: proc(emitter: ^Emitter, operation: Operation, destination: Register, destination_displacement: u8, source: Register) {
	EmitREX(emitter, source, destination)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitByteDisplacedIndirect(emitter, auto_cast source, destination, destination_displacement)
}

EmitOperationMemoryDisplacedRegister :: proc(emitter: ^Emitter, operation: Operation, destination: Register, destination_displacement: u32, source: Register) {
	EmitREX(emitter, source, destination)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitDisplacedIndirect(emitter, auto_cast source, destination, destination_displacement)
}

EmitOperationSIBRegister :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, source: Register) {
	EmitREXIndexed(emitter, source, destination_base, destination_index)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitIndexedIndirect(emitter, auto_cast source, destination_base, destination_index, destination_scale)
}

EmitOperationSIBByteDisplacedRegister :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, destination_displacement: u8, source: Register) {
	EmitREXIndexed(emitter, source, destination_base, destination_index)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitByteDisplacedIndexedIndirect(emitter, auto_cast source, destination_base, destination_index, destination_scale, destination_displacement)
}

EmitOperationSIBDisplacedRegister :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, destination_displacement: u32, source: Register) {
	EmitREXIndexed(emitter, source, destination_base, destination_index)
	EmitByte(emitter, MemoryOperations[operation].?)
	EmitDisplacedIndexedIndirect(emitter, auto_cast source, destination_base, destination_index, destination_scale, destination_displacement)
}

EmitOperationRegisterImmediate :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source_immediate: u32) {
	EmitREX(emitter, auto_cast 0, destination)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitDirect(emitter, ImmediateOperations[operation].?.extention, destination)
	EmitU32(emitter, source_immediate)
}

EmitOperationMemoryImmediate :: proc(emitter: ^Emitter, operation: Operation, destination: Register, source_immediate: u32) {
	EmitREX(emitter, auto_cast 0, destination)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitIndirect(emitter, ImmediateOperations[operation].?.extention, destination)
	EmitU32(emitter, source_immediate)
}

EmitOperationMemoryByteDisplacedImmediate :: proc(emitter: ^Emitter, operation: Operation, destination: Register, destination_displacement: u8, source_immediate: u32) {
	EmitREX(emitter, auto_cast 0, destination)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitByteDisplacedIndirect(emitter, ImmediateOperations[operation].?.extention, destination, destination_displacement)
	EmitU32(emitter, source_immediate)
}

EmitOperationMemoryDisplacedImmediate :: proc(emitter: ^Emitter, operation: Operation, destination: Register, destination_displacement: u32, source_immediate: u32) {
	EmitREX(emitter, auto_cast 0, destination)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitDisplacedIndirect(emitter, ImmediateOperations[operation].?.extention, destination, destination_displacement)
	EmitU32(emitter, source_immediate)
}

EmitOperationSIBImmediate :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, source_immediate: u32) {
	EmitREXIndexed(emitter, auto_cast 0, destination_base, destination_index)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitIndexedIndirect(emitter, ImmediateOperations[operation].?.extention, destination_base, destination_index, destination_scale)
	EmitU32(emitter, source_immediate)
}

EmitOperationSIBByteDisplacedImmediate :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, destination_displacement: u8, source_immediate: u32) {
	EmitREXIndexed(emitter, auto_cast 0, destination_base, destination_index)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitByteDisplacedIndexedIndirect(emitter, ImmediateOperations[operation].?.extention, destination_base, destination_index, destination_scale, destination_displacement)
	EmitU32(emitter, source_immediate)
}

EmitOperationSIBDisplacedImmediate :: proc(emitter: ^Emitter, operation: Operation, destination_base: Register, destination_scale: Scale, destination_index: Register, destination_displacement: u32, source_immediate: u32) {
	EmitREXIndexed(emitter, auto_cast 0, destination_base, destination_index)
	EmitByte(emitter, ImmediateOperations[operation].?.opcode)
	EmitDisplacedIndexedIndirect(emitter, ImmediateOperations[operation].?.extention, destination_base, destination_index, destination_scale, destination_displacement)
	EmitU32(emitter, source_immediate)
}
