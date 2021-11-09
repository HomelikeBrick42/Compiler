package compiler

import "core:fmt"

Binder :: struct {
	types: [dynamic]^BoundType,
	unary_operators: [dynamic]^UnaryOperator,
	binary_operators: [dynamic]^BinaryOperator,
}

Binder_Create :: proc() -> Binder {
	binder: Binder

	append(&binder.unary_operators, new_clone(UnaryOperator{
		operator_kind = .Plus,
		operand_type  = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.unary_operators, new_clone(UnaryOperator{
		operator_kind = .Minus,
		operand_type  = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.binary_operators, new_clone(BinaryOperator{
		operator_kind = .Plus,
		left_type     = Binder_GetIntegerType(&binder, 64, true),
		right_type    = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.binary_operators, new_clone(BinaryOperator{
		operator_kind = .Minus,
		left_type     = Binder_GetIntegerType(&binder, 64, true),
		right_type    = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.binary_operators, new_clone(BinaryOperator{
		operator_kind = .Asterisk,
		left_type     = Binder_GetIntegerType(&binder, 64, true),
		right_type    = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.binary_operators, new_clone(BinaryOperator{
		operator_kind = .Slash,
		left_type     = Binder_GetIntegerType(&binder, 64, true),
		right_type    = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

	append(&binder.binary_operators, new_clone(BinaryOperator{
		operator_kind = .Percent,
		left_type     = Binder_GetIntegerType(&binder, 64, true),
		right_type    = Binder_GetIntegerType(&binder, 64, true),
		result_type   = Binder_GetIntegerType(&binder, 64, true),
	}))

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

	integer_type := BoundType_Create(BoundIntegerType, size)
	integer_type.signed = signed
	append(&binder.types, integer_type)
	return integer_type
}

Binder_BindFile :: proc(binder: ^Binder, file: ^AstFile) -> (bound_file: ^BoundFile, error: Maybe(Error)) {
	bound_file = BoundNode_Create(BoundFile)
	bound_file.scope = BoundStatement_Create(BoundScope, bound_file, nil)

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
			integer_type := Binder_GetIntegerType(binder, 64, true)
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
