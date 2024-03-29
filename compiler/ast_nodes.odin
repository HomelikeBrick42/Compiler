package compiler

AstNode :: struct {
	kind: union {
		^AstFile,
		^AstStatement,
		^AstExpression,
	},
}

AstNode_Create :: proc($T: typeid) -> ^T {
	ast     := new(T)
	ast.kind = ast
	return ast
}

AstFile :: struct {
	using ast: AstNode,
	path: string,
	statements: [dynamic]^AstStatement,
	end_of_file_token: Token,
}

AstStatement :: struct {
	using ast: AstNode,
	statement_kind: union {
		^AstScope,
		^AstDeclaration,
		^AstAssignment,
		^AstStatementExpression,
		^AstIf,
		^AstWhile,
		^AstPrint,
	},
}

AstStatement_Create :: proc($T: typeid) -> ^T {
	statement               := new(T)
	statement.kind           = cast(^AstStatement) statement
	statement.statement_kind = statement
	return statement
}

AstScope :: struct {
	using statement: AstStatement,
	statements: [dynamic]^AstStatement,
}

AstDeclaration :: struct {
	using statement: AstStatement,
	name: ^AstName,
	colon_token: Token,
	type: ^AstExpression,
	equals_token: Token,
	value: ^AstExpression,
}

AstAssignment :: struct {
	using assignment: AstStatement,
	operand: ^AstExpression,
	equals_token: Token,
	value: ^AstExpression,
}

AstStatementExpression :: struct {
	using statement: AstStatement,
	expression: ^AstExpression,
}

AstIf :: struct {
	using statement: AstStatement,
	if_token: Token,
	condition: ^AstExpression,
	then_statement: ^AstStatement,
	else_token: Token,
	else_statement: ^AstStatement,
}

AstWhile :: struct {
	using statement: AstStatement,
	while_token: Token,
	condition: ^AstExpression,
	then_statement: ^AstStatement,
}

// This is temporary
AstPrint :: struct {
	using statement: AstStatement,
	print_token: Token,
	expression: ^AstExpression,
}

AstExpression :: struct {
	using ast: AstNode,
	expression_kind: union {
		^AstName,
		^AstInteger,
		^AstArray,
		^AstPointer,
		^AstAddress,
		^AstDeref,
		^AstArrayIndex,
		^AstSizeOf,
		^AstCast,
		^AstTransmute,
		^AstTrue,
		^AstFalse,
		^AstUnary,
		^AstBinary,
	},
}

AstExpression_Create :: proc($T: typeid) -> ^T {
	expression                := new(T)
	expression.kind            = cast(^AstExpression) expression
	expression.expression_kind = expression
	return expression
}

AstName :: struct {
	using expression: AstExpression,
	name_token: Token,
}

AstInteger :: struct {
	using expression: AstExpression,
	integer_token: Token,
}

AstArray :: struct {
	using expression: AstExpression,
	open_bracket_token: Token,
	close_bracket_token: Token,
	count: uint,
	type: ^AstExpression,
}

AstPointer :: struct {
	using expression: AstExpression,
	caret_token: Token,
	type: ^AstExpression,
}

AstAddress :: struct {
	using expression: AstExpression,
	asterisk_token: Token,
	operand: ^AstExpression,
}

AstDeref :: struct {
	using expression: AstExpression,
	operand: ^AstExpression,
	caret_token: Token,
}

AstArrayIndex :: struct {
	using expression: AstExpression,
	operand: ^AstExpression,
	open_bracket_token: Token,
	index: ^AstExpression,
	close_bracket_token: Token,
}

AstSizeOf :: struct {
	using expression: AstExpression,
	sizeof_token: Token,
	operand: ^AstExpression,
}

AstCast :: struct {
	using expression: AstExpression,
	cast_token: Token,
	type: ^AstExpression,
	operand: ^AstExpression,
}

AstTransmute :: struct {
	using expression: AstExpression,
	transmute_token: Token,
	type: ^AstExpression,
	operand: ^AstExpression,
}

AstTrue :: struct {
	using expression: AstExpression,
	true_token: Token,
}

AstFalse :: struct {
	using expression: AstExpression,
	false_token: Token,
}

AstUnary :: struct {
	using expression: AstExpression,
	operator_token: Token,
	operand: ^AstExpression,
}

AstBinary :: struct {
	using expression: AstExpression,
	left: ^AstExpression,
	operator_token: Token,
	right: ^AstExpression,
}
