package compiler

import "core:fmt"
import "core:strings"
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
		^BoundWhile,
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
	stack_offset: uint,
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

BoundWhile :: struct {
	using statement: BoundStatement,
	condition: ^BoundExpression,
	then_statement: ^BoundStatement,
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
		^BoundArrayIndex,
		^BoundCast,
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

BoundArrayIndex :: struct {
	using expression: BoundExpression,
	operand: ^BoundExpression,
	index: ^BoundExpression,
}

BoundCast :: struct {
	using expression: BoundExpression,
	castt: ^Cast,
	operand: ^BoundExpression,
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
		^BoundArrayType,
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

BoundArrayType :: struct {
	using type: BoundType,
	count: uint,
	inner_type: ^BoundType,
}

BoundNode_ToString :: proc(bound_node: ^BoundNode, allocator := context.allocator) -> string {
	builder: strings.Builder
	strings.init_builder(&builder, allocator)
	BoundNode_Print(bound_node, 0, &builder)
	return strings.to_string(builder)
}

BoundNode_Print :: proc(bound_node: ^BoundNode, indent: uint, builder: ^strings.Builder) {
	PrintIndent :: proc(indent: uint, builder: ^strings.Builder) {
		for _ in 0..cast(int)indent - 1 {
			strings.write_string(builder, "    ")
		}
	}

	switch n in bound_node.kind {
		case ^BoundFile: {
			file := n
			for statement in file.scope.statements {
				BoundNode_Print(statement, indent, builder)
				strings.write_string(builder, ";\n")
			}
		}

		case ^BoundStatement: {
			statement := n
			switch s in statement.statement_kind {
				case ^BoundScope: {
					scope := s
					strings.write_string(builder, "{\n")
					for statement in scope.statements {
						PrintIndent(indent + 1, builder)
						BoundNode_Print(statement, indent + 1, builder)
						strings.write_string(builder, ";\n")
					}
					PrintIndent(indent, builder)
					strings.write_string(builder, "}")
				}

				case ^BoundDeclaration: {
					declaration := s
					strings.write_string(builder, fmt.tprintf("{}: ", declaration.name))
					BoundNode_Print(declaration.type, indent, builder)
					if declaration.value != nil {
						strings.write_string(builder, " = ")
						BoundNode_Print(declaration.value, indent, builder)
					}
				}

				case ^BoundAssignment: {
					assignment := s
					BoundNode_Print(assignment.operand, indent, builder)
					strings.write_string(builder, " ")
					if assignment.binary_operator != nil {
						BinaryOperator_Print(assignment.binary_operator.operator_kind, builder)
					}
					strings.write_string(builder, "= ")
					BoundNode_Print(assignment.value, indent, builder)
				}

				case ^BoundStatementExpression: {
					statement_expression := s
					BoundNode_Print(statement_expression.expression, indent, builder)
				}

				case ^BoundIf: {
					iff := s
					strings.write_string(builder, "if ")
					BoundNode_Print(iff.condition, indent, builder)
					strings.write_string(builder, " ")
					if reflect.union_variant_typeid(iff.then_statement.statement_kind) != ^BoundScope {
						strings.write_string(builder, "do ")
					}
					BoundNode_Print(iff.then_statement, indent, builder)
					if iff.else_statement != nil {
						strings.write_string(builder, " else ")
						BoundNode_Print(iff.else_statement, indent, builder)
					}
				}

				case ^BoundWhile: {
					whilee := s
					strings.write_string(builder, "while ")
					BoundNode_Print(whilee.condition, indent, builder)
					strings.write_string(builder, " ")
					if reflect.union_variant_typeid(whilee.then_statement.statement_kind) != ^BoundScope {
						strings.write_string(builder, "do ")
					}
					BoundNode_Print(whilee.then_statement, indent, builder)
				}

				case ^BoundPrint: {
					print := s
					strings.write_string(builder, "print ")
					BoundNode_Print(print.expression, indent, builder)
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
					strings.write_string(builder, fmt.tprint(name.name))
				}

				case ^BoundInteger: {
					integer := e
					strings.write_string(builder, fmt.tprint(integer.value))
				}

				case ^BoundArrayIndex: {
					array_index := e
					strings.write_string(builder, "(")
					BoundNode_Print(array_index.operand, indent, builder)
					strings.write_string(builder, "[")
					BoundNode_Print(array_index.index, indent, builder)
					strings.write_string(builder, "]")
					strings.write_string(builder, ")")
				}

				case ^BoundCast: {
					castt := e
					strings.write_string(builder, "cast(")
					BoundNode_Print(castt.castt.to, indent, builder)
					strings.write_string(builder, ") ")
					BoundNode_Print(castt.operand, indent, builder)
				}

				case ^BoundTrue: {
					truee := e
					strings.write_string(builder, "true")
				}

				case ^BoundFalse: {
					falsee := e
					strings.write_string(builder, "false")
				}

				case ^BoundUnary: {
					unary := e
					strings.write_string(builder, "(")
					UnaryOperator_Print(unary.unary_operator.operator_kind, builder)
					BoundNode_Print(unary.operand, indent, builder)
					strings.write_string(builder, ")")
				}

				case ^BoundBinary: {
					binary := e
					strings.write_string(builder, "(")
					BoundNode_Print(binary.left, indent, builder)
					strings.write_string(builder, " ")
					BinaryOperator_Print(binary.binary_operator.operator_kind, builder)
					strings.write_string(builder, " ")
					BoundNode_Print(binary.right, indent, builder)
					strings.write_string(builder, ")")
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
					strings.write_string(builder, fmt.tprintf("{}{}", integer_type.signed ? "s" : "u", integer_type.size * 8))
				}

				case ^BoundBoolType: {
					bool_type := t
					strings.write_string(builder, "bool")
				}

				case ^BoundArrayType: {
					array_type := t
					strings.write_string(builder, fmt.tprintf("[{}]", array_type.count))
					BoundNode_Print(array_type.inner_type, indent, builder)
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
