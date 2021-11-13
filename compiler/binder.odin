package compiler

import "core:fmt"
import "core:reflect"

Binder :: struct {
	types: [dynamic]^BoundType,
	unary_operators: [dynamic]^UnaryOperator,
	binary_operators: [dynamic]^BinaryOperator,
}

Binder_Create :: proc() -> Binder {
	binder: Binder

	AddUnary :: proc(binder: ^Binder, kind: TokenKind, operation: Instruction, operand_type: ^BoundType, result_type: ^BoundType) {
		operator := new(UnaryOperator)
		operator^ = UnaryOperator{
			operator_kind = kind,
			operation     = operation,
			operand_type  = operand_type,
			result_type   = result_type,
		}
		append(&binder.unary_operators, operator)
	}

	AddUnary(
		&binder,
		.Plus,
		InstNoOp{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddUnary(
		&binder,
		.Minus,
		InstNegateS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary :: proc(binder: ^Binder, kind: TokenKind, operation: Instruction, left_type: ^BoundType, right_type: ^BoundType, result_type: ^BoundType) {
		operator := new(BinaryOperator)
		operator^ = BinaryOperator{
			operator_kind = kind,
			operation     = operation,
			left_type     = left_type,
			right_type    = right_type,
			result_type   = result_type,
		}
		append(&binder.binary_operators, operator)
	}

	AddBinary(
		&binder,
		.Plus,
		InstAddS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.Minus,
		InstSubS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.Asterisk,
		InstMulS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.Slash,
		InstDivS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.Percent,
		InstModS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.Percent,
		InstModS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
	)

	AddBinary(
		&binder,
		.EqualsEquals,
		InstEqualS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.ExclamationMarkEquals,
		InstNotEqualS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.LessThan,
		InstLessThanS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.LessThanEquals,
		InstLessThanEqualS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.GreaterThan,
		InstGreaterThanS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.GreaterThanEquals,
		InstGreaterThanEqualS64{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetBoolType(&binder),
	)

	return binder
}

Binder_Destroy :: proc(binder: ^Binder) {
	delete(binder.types)
	delete(binder.unary_operators)
	delete(binder.binary_operators)
}

Binder_GetIntegerType :: proc(binder: ^Binder, size: uint, signed: bool) -> ^BoundIntegerType {
	// Speed this up somehow
	for type in binder.types {
		if integer_type, ok := type.type_kind.(^BoundIntegerType); ok {
			if integer_type.size == size && integer_type.signed == signed {
				return integer_type
			}
		}
	}

	integer_type := BoundType_Create(BoundIntegerType, size, len(binder.types))
	integer_type.signed = signed
	append(&binder.types, integer_type)
	return integer_type
}

Binder_GetBoolType :: proc(binder: ^Binder) -> ^BoundBoolType {
	// Speed this up somehow
	for type in binder.types {
		if bool_type, ok := type.type_kind.(^BoundBoolType); ok {
			if bool_type.size == 1 {
				return bool_type
			}
		}
	}

	bool_type := BoundType_Create(BoundBoolType, 1, len(binder.types))
	append(&binder.types, bool_type)
	return bool_type
}

Binder_BindFile :: proc(binder: ^Binder, file: ^AstFile) -> (bound_file: ^BoundFile, error: Maybe(Error)) {
	bound_file = BoundNode_Create(BoundFile)
	bound_file.scope = BoundStatement_Create(BoundScope, bound_file, nil)
	bound_file.scope.global = true

	for statement in file.statements {
		append(&bound_file.scope.statements, Binder_BindStatement(binder, statement, bound_file, bound_file.scope) or_return)
	}

	return bound_file, nil
}

Binder_BindStatement :: proc(binder: ^Binder, statement: ^AstStatement, parent_file: ^BoundFile, parent_scope: ^BoundScope) -> (bound_statement: ^BoundStatement, error: Maybe(Error)) {
	switch s in statement.statement_kind {
		case ^AstScope: {
			scope := s
			bound_scope := BoundStatement_Create(BoundScope, parent_file, parent_scope)

			for statement in scope.statements {
				append(&bound_scope.statements, Binder_BindStatement(binder, statement, parent_file, bound_scope) or_return)
			}

			return bound_scope, nil
		}

		case ^AstDeclaration: {
			declaration := s
			bound_declaration := BoundStatement_Create(BoundDeclaration, parent_file, parent_scope)
			bound_declaration.global = parent_scope.global

			bound_declaration.name = declaration.name.name_token.data.(string)

			if decl, found := parent_scope.declarations[bound_declaration.name]; found {
				return nil, Error{
					loc     = declaration.name.name_token.loc,
					message = fmt.tprintf("Redeclaration of name '{}'", bound_declaration.name),
				}
			}

			if declaration.type != nil {
				return nil, Error{
					loc     = declaration.colon_token,
					message = "unimplemented type handling for declaration",
				}
			}

			if declaration.value != nil {
				bound_declaration.value = Binder_BindExpression(binder, declaration.value, bound_declaration) or_return
			}

			if bound_declaration.type == nil {
				bound_declaration.type = bound_declaration.value.type
			} else if bound_declaration.type != bound_declaration.value.type {
				return nil, Error{
					loc     = declaration.equals_token,
					message = "Type for declaration and the value type do not match",
				}
			}

			bound_declaration.stack_location = parent_scope.stack_size
			parent_scope.stack_size += bound_declaration.type.size

			parent_scope.declarations[bound_declaration.name] = bound_declaration
			return bound_declaration, nil
		}

		case ^AstAssignment: {
			assignment := s
			bound_assignment := BoundStatement_Create(BoundAssignment, parent_file, parent_scope)
			bound_assignment.operand = Binder_BindExpression(binder, assignment.operand, bound_assignment) or_return
			bound_assignment.value   = Binder_BindExpression(binder, assignment.value, bound_assignment) or_return
			
			if _, ok := bound_assignment.operand.expression_kind.(^BoundName); !ok {
				return nil, Error{
					loc     = assignment.equals_token.loc,
					message = "Cannot assign non names (for now)",
				}
			}

			if assignment.equals_token.kind == .Equals {
				if bound_assignment.operand.type != bound_assignment.value.type {
					return nil, Error{
						loc     = assignment.equals_token.loc,
						message = "Types for assignment do not match",
					}
				}
			} 

			operator_kind, found := equals_to_operator[assignment.equals_token.kind]
			assert(found)

			for operator in binder.binary_operators {
				if operator.operator_kind == operator_kind &&
					operator.left_type == bound_assignment.operand.type &&
					operator.right_type == bound_assignment.value.type {
					bound_assignment.binary_operator = operator
					break
				}
			}

			if operator_kind != .Equals && bound_assignment.binary_operator == nil {
				return nil, Error{
					loc     = assignment.equals_token.loc,
					message = "Unable to find binary operator for types in assignment",
				}
			}

			return bound_assignment, nil
		}

		case ^AstStatementExpression: {
			statement_expression := s
			bound_statement_expression := BoundStatement_Create(BoundStatementExpression, parent_file, parent_scope)
			bound_statement_expression.expression = Binder_BindExpression(binder, statement_expression.expression, bound_statement_expression) or_return
			return bound_statement_expression, nil
		}

		case ^AstIf: {
			iff := s
			bound_if := BoundStatement_Create(BoundIf, parent_file, parent_scope)
			bound_if.condition = Binder_BindExpression(binder, iff.condition, bound_if) or_return
			if bound_if.condition.type != Binder_GetBoolType(binder) {
				return {}, Error{
					loc     = iff.if_token.loc,
					message = "if condition needs a bool type",
				}
			}
			bound_if.then_statement = Binder_BindStatement(binder, iff.then_statement, parent_file, parent_scope) or_return
			if iff.else_statement != nil {
				bound_if.else_statement = Binder_BindStatement(binder, iff.else_statement, parent_file, parent_scope) or_return
			}
			return bound_if, nil
		}

		// This is temporary
		case ^AstPrint: {
			print := s
			bound_print := BoundStatement_Create(BoundPrint, parent_file, parent_scope)
			bound_print.expression = Binder_BindExpression(binder, print.expression, bound_print) or_return
			if bound_print.expression.type != Binder_GetIntegerType(binder, 8, true) {
				return {}, Error{
					loc     = print.print_token,
					message = "Print can only print s64 for now",
				}
			}
			return bound_print, nil
		}

		case: {
			message := "unreachable default case in Binder_BindStatement"
			assert(false, message)
			return nil, Error{
				message = message,
			}
		}
	}
}

Binder_BindExpression :: proc(binder: ^Binder, expression: ^AstExpression, parent_statement: ^BoundStatement) -> (bound_expression: ^BoundExpression, error: Maybe(Error)) {
	switch e in expression.expression_kind {
		case ^AstName: {
			name := e
			scope := parent_statement.parent_scope

			name_string := name.name_token.data.(string)

			for scope != nil {
				if declaration, found := scope.declarations[name_string]; found {
					bound_name := BoundExpression_Create(BoundName, declaration.type, parent_statement)
					bound_name.name = name_string
					bound_name.declaration = declaration
					return bound_name, nil
				}
				scope = scope.parent_scope
			}

			return nil, Error{
				loc     = name.name_token.loc,
				message = fmt.tprintf("Unable to find name '{}'", name_string),
			}
		}

		case ^AstInteger: {
			integer := e
			integer_type := Binder_GetIntegerType(binder, 8, true)
			bound_integer := BoundExpression_Create(BoundInteger, integer_type, parent_statement)
			bound_integer.value = integer.integer_token.data.(u64)
			return bound_integer, nil
		}

		case ^AstUnary: {
			unary := e
			bound_operand := Binder_BindExpression(binder, unary.operand, parent_statement) or_return

			unary_operator: ^UnaryOperator = nil
			for operator in binder.unary_operators {
				if operator.operator_kind == unary.operator_token.kind &&
					operator.operand_type == bound_operand.type {
					unary_operator = operator
					break
				}
			}

			if unary_operator == nil {
				return nil, Error{
					loc     = unary.operator_token.loc,
					message = "Unable to find unary operator",
				}
			}

			bound_unary := BoundExpression_Create(BoundUnary, unary_operator.result_type, parent_statement)
			bound_unary.unary_operator = unary_operator
			bound_unary.operand = bound_operand
			return bound_unary, nil
		}

		case ^AstBinary: {
			binary := e
			bound_left := Binder_BindExpression(binder, binary.left, parent_statement) or_return
			bound_right := Binder_BindExpression(binder, binary.right, parent_statement) or_return

			binary_operator: ^BinaryOperator = nil
			for operator in binder.binary_operators {
				if operator.operator_kind == binary.operator_token.kind &&
					operator.left_type == bound_left.type &&
					operator.right_type == bound_right.type {
					binary_operator = operator
					break
				}
			}

			if binary_operator == nil {
				return nil, Error{
					loc     = binary.operator_token.loc,
					message = "Unable to find binary operator",
				}
			}

			bound_binary := BoundExpression_Create(BoundBinary, binary_operator.result_type, parent_statement)
			bound_binary.left = bound_left
			bound_binary.binary_operator = binary_operator
			bound_binary.right = bound_right
			return bound_binary, nil
		}

		case: {
			message := "unreachable default case in Binder_BindExpression"
			assert(false, message)
			return nil, Error{
				message = message,
			}
		}
	}
}
