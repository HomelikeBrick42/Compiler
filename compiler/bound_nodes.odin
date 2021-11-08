package compiler

BoundNode :: struct {
	kind: union {
		^BoundFile,
		^BoundStatement,
		^BoundExpression,
		^BoundType,
	},
}

BoundNode_Create :: proc($T: typeid) -> ^T {
	node := new(T)
	node.kind = node
	return node
}

BoundFile :: struct {
	using node: BoundNode,
	scope: ^BoundScope,
}

BoundStatement :: struct {
	using node: BoundNode,
	parent_file: ^BoundFile,
	parent_scope: ^BoundScope,
	statement_kind: union {
		^BoundScope,
		^BoundDeclaration,
		^BoundAssignment,
		^BoundStatementExpression,
	},
}

BoundStatement_Create :: proc($T: typeid, parent_file: ^BoundFile, parent_scope: ^BoundScope) -> ^T {
	node               := new(T)
	node.kind           = cast(^BoundStatement) node
	node.statement_kind = node
	node.parent_file    = parent_file
	node.parent_scope   = parent_scope
	return node
}

BoundScope :: struct {
	using statement: BoundStatement,
	statements: [dynamic]^BoundStatement,
	declarations: map[string]^BoundDeclaration,
}

BoundDeclaration :: struct {
	using statement: BoundStatement,
	name: string,
	type: ^BoundType,
	value: ^BoundExpression,
}

BoundAssignment :: struct {
	using statement: BoundStatement,
	operand: ^BoundExpression,
	value: ^BoundExpression,
}

BoundStatementExpression :: struct {
	using statement: BoundStatement,
	expression: ^BoundExpression,
}

BoundExpression :: struct {
	using node: BoundNode,
	parent_statement: ^BoundStatement,
	type: ^BoundType,
	expression_kind: union {
		^BoundName,
		^BoundInteger,
		^BoundUnary,
		^BoundBinary,
	},
}

BoundExpression_Create :: proc($T: typeid, type: ^BoundType, parent_statement: ^BoundStatement) -> ^T {
	node                 := new(T)
	node.kind             = cast(^BoundExpression) node
	node.expression_kind  = node
	node.type             = type
	node.parent_statement = parent_statement
	return node
}

BoundName :: struct {
	using expression: BoundExpression,
	declaration: ^BoundDeclaration,
}

BoundInteger :: struct {
	using expression: BoundExpression,
	value: u64,
}

BoundUnary :: struct {
	using expression: BoundExpression,
	operand: ^BoundExpression,
}

BoundBinary :: struct {
	using expression: BoundExpression,
	left: ^BoundExpression,
	right: ^BoundExpression,
}

BoundType :: struct {
	using node: BoundNode,
	size: uint,
	type_kind: union {
		^BoundIntegerType,
	},
}

BoundType_Create :: proc($T: typeid, size: uint) -> ^T {
	node          := new(T)
	node.kind      = cast(^BoundType) node
	node.type_kind = node
	node.size      = size
	return node
}

BoundIntegerType :: struct {
	using type: BoundType,
	signed: bool,
}
