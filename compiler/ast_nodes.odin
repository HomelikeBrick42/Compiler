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