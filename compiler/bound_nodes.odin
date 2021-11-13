package compiler

import "core:fmt"
import "core:reflect"

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
		^BoundIf,
		^BoundPrint,
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
	global: bool,
}

BoundDeclaration :: struct {
	using statement: BoundStatement,
	name: string,
	type: ^BoundType,
	value: ^BoundExpression,
	stack_location: uint,
	global: bool,
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

BoundIf :: struct {
	using statement: BoundStatement,
	condition: ^BoundExpression,
	then_statement: ^BoundStatement,
	else_statement: ^BoundStatement,
}

// This is temporary
BoundPrint :: struct {
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
		^BoundTrue,
		^BoundFalse,
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

BoundTrue :: struct {
	using expression: BoundExpression,
}

BoundFalse :: struct {
	using expression: BoundExpression,
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
	id: uint,
	size: uint,
	type_kind: union {
		^BoundIntegerType,
		^BoundBoolType,
	},
}

BoundType_Create :: proc($T: typeid, size: uint, id: uint) -> ^T {
	node          := new(T)
	node.kind      = cast(^BoundType) node
	node.type_kind = node
	node.id        = id
	node.size      = size
	return node
}

BoundIntegerType :: struct {
	using type: BoundType,
	signed: bool,
}

BoundBoolType :: struct {
	using type: BoundType,
}

BoundNode_Print :: proc(bound_node: ^BoundNode, indent: uint) {
	PrintIndent :: proc(indent: uint) {
		for _ in 0..cast(int)indent - 1 {
			fmt.print("    ")
		}
	}

	PrintUnaryOperator :: proc(kind: TokenKind) {
		#partial switch kind {
			case .Plus: {
				fmt.print("+")
			}

			case .Minus: {
				fmt.print("-")
			}

			case .ExclamationMark: {
				fmt.print("!")
			}

			case: {
				assert(false, "unreachable UnaryOperator kind in PrintUnaryOperator")
			}
		}
	}

	PrintBinaryOperator :: proc(kind: TokenKind) {
		#partial switch kind {
			case .Plus: {
				fmt.print("+")
			}

			case .Minus: {
				fmt.print("-")
			}

			case .Asterisk: {
				fmt.print("*")
			}

			case .Slash: {
				fmt.print("/")
			}

			case .Percent: {
				fmt.print("%")
			}

			case .EqualsEquals: {
				fmt.print("==")
			}

			case .ExclamationMarkEquals: {
				fmt.print("!=")
			}

			case .LessThan: {
				fmt.print("<")
			}

			case .LessThanEquals: {
				fmt.print("<=")
			}

			case .GreaterThan: {
				fmt.print(">")
			}

			case .GreaterThanEquals: {
				fmt.print(">=")
			}

			case .AmpersandAmpersand: {
				fmt.print("&&")
			}

			case .PipePipe: {
				fmt.print("||")
			}

			case: {
				assert(false, "unreachable assignment BiaryOperator kind in PrintBinaryOperator")
			}
		}
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
						PrintIndent(indent + 1)
						BoundNode_Print(statement, indent + 1)
						fmt.println(";")
					}
					PrintIndent(indent)
					fmt.print("}")
				}

				case ^BoundDeclaration: {
					declaration := s
					fmt.printf("{}: ", declaration.name)
					BoundNode_Print(declaration.type, indent)
					if declaration.value != nil {
						fmt.print(" = ")
						BoundNode_Print(declaration.value, indent)
					}
				}

				case ^BoundAssignment: {
					assignment := s
					BoundNode_Print(assignment.operand, indent)
					fmt.print(" ")
					if assignment.binary_operator != nil {
						PrintBinaryOperator(assignment.binary_operator.operator_kind)
					}
					fmt.print("= ")
					BoundNode_Print(assignment.value, indent)
				}

				case ^BoundStatementExpression: {
					statement_expression := s
					BoundNode_Print(statement_expression.expression, indent)
				}

				case ^BoundIf: {
					iff := s
					fmt.print("if ")
					BoundNode_Print(iff.condition, indent)
					fmt.print(" ")
					if reflect.union_variant_typeid(iff.then_statement.statement_kind) != ^BoundScope {
						fmt.print("do ")
					}
					BoundNode_Print(iff.then_statement, indent)
					if iff.else_statement != nil {
						fmt.print(" else ")
						BoundNode_Print(iff.else_statement, indent)
					}
				}

				case ^BoundPrint: {
					print := s
					fmt.print("print ")
					BoundNode_Print(print.expression, indent)
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
					fmt.print(name.name)
				}

				case ^BoundInteger: {
					integer := e
					fmt.print(integer.value)
				}

				case ^BoundTrue: {
					truee := e
					fmt.print("true")
				}

				case ^BoundFalse: {
					falsee := e
					fmt.print("false")
				}

				case ^BoundUnary: {
					unary := e
					fmt.print("(")
					PrintUnaryOperator(unary.unary_operator.operator_kind)
					BoundNode_Print(unary.operand, indent)
					fmt.print(")")
				}

				case ^BoundBinary: {
					binary := e
					fmt.print("(")
					BoundNode_Print(binary.left, indent)
					fmt.print(" ")
					PrintBinaryOperator(binary.binary_operator.operator_kind)
					fmt.print(" ")
					BoundNode_Print(binary.right, indent)
					fmt.print(")")
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
					fmt.printf("{}{}", integer_type.signed ? "s" : "u", integer_type.size * 8)
				}

				case ^BoundBoolType: {
					bool_type := t
					fmt.print("bool")
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
