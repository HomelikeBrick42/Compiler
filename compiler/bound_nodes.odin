package compiler

import "core:fmt"

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
	stack_size: uint,
}

BoundDeclaration :: struct {
	using statement: BoundStatement,
	name: string,
	type: ^BoundType,
	value: ^BoundExpression,
	stack_location: uint,
}

BoundAssignment :: struct {
	using statement: BoundStatement,
	operand: ^BoundExpression,
	binary_operator: ^BinaryOperator, // this will be nil if = is used
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
	name: string,
	declaration: ^BoundDeclaration,
}

BoundInteger :: struct {
	using expression: BoundExpression,
	value: u64,
}

BoundUnary :: struct {
	using expression: BoundExpression,
	unary_operator: ^UnaryOperator,
	operand: ^BoundExpression,
}

BoundBinary :: struct {
	using expression: BoundExpression,
	left: ^BoundExpression,
	binary_operator: ^BinaryOperator,
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

BoundNode_Print :: proc(bound_node: ^BoundNode, indent: uint) {
	for _ in 0..indent {
		fmt.print("    ")
	}

	switch n in bound_node.kind {
		case ^BoundFile: {
			file := n
			for statement in file.scope.statements {
				BoundNode_Print(statement, indent)
				fmt.println(";")
			}
		}

		case ^BoundStatement: {
			statement := n
			switch s in statement.statement_kind {
				case ^BoundScope: {
					scope := s
					fmt.println("{")
					for statement in scope.statements {
						BoundNode_Print(statement, indent + 1)
						fmt.println(";")
					}
					fmt.println("}")
				}

				case ^BoundDeclaration: {
					declaration := s
				}

				case ^BoundAssignment: {
					assignment := s
				}

				case ^BoundStatementExpression: {
					statement_expression := s
				}

				case: {
					assert(false, "unreachable BoundStatement default case in BoundNode_Print")
				}
			}
		}

		case ^BoundExpression: {
			expression := n
			switch e in expression.expression_kind {
				case ^BoundName: {
					name := e
				}

				case ^BoundInteger: {
					integer := e
				}

				case ^BoundUnary: {
					unary := e
				}

				case ^BoundBinary: {
					binary := e
				}

				case: {
					assert(false, "unreachable BoundExpression default case in BoundNode_Print")
				}
			}
		}

		case ^BoundType: {
			type := n
			switch t in type.type_kind {
				case ^BoundIntegerType: {
					integer_type := t
				}

				case: {
					assert(false, "unreachable BoundType default case in BoundNode_Print")
				}
			}
		}

		case: {
			assert(false, "unreachable BoundNode default case in BoundNode_Print")
		}
	}
}
