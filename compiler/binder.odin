package compiler

import "core:fmt"

Binder :: struct {
	types: [dynamic]^BoundType,
}

Binder_Destroy :: proc(binder: ^Binder) {
	delete(binder.types)
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
				if bound_declaration.type == nil {
					bound_declaration.type = bound_declaration.value.type
				}
			}

			if bound_declaration.type != bound_declaration.value.type {
				return nil, Error{
					loc     = declaration.equals_token,
					message = "Type for declaration and the value type do not match",
				}
			}

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
					loc     = assignment.assignment_token.loc,
					message = "Cannot assign non names (for now)",
				}
			}

			if bound_assignment.operand.type != bound_assignment.value.type {
				return nil, Error{
					loc     = assignment.assignment_token.loc,
					message = "Types for assignment do not match",
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

			// TODO: Proper type checking
			type := bound_operand.type

			bound_unary := BoundExpression_Create(BoundUnary, type, parent_statement)
			bound_unary.operand = bound_operand
			return bound_unary, nil
		}

		case ^AstBinary: {
			binary := e
			bound_left := Binder_BindExpression(binder, binary.left, parent_statement) or_return
			bound_right := Binder_BindExpression(binder, binary.right, parent_statement) or_return

			if bound_left.type != bound_right.type {
				return nil, Error{
					loc     = binary.operator_token.loc,
					message = "Types for binary expression do not match",
				}
			}

			// TODO: Proper type checking
			type := bound_left.type

			bound_binary := BoundExpression_Create(BoundBinary, type, parent_statement)
			bound_binary.left = bound_left
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
