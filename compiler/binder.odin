package compiler

import "core:fmt"
import "core:reflect"

Cast :: struct {
	from: ^BoundType,
	to: ^BoundType,
	operation: Instruction,
}

Binder :: struct {
	types: [dynamic]^BoundType,
	unary_operators: [dynamic]^UnaryOperator,
	binary_operators: [dynamic]^BinaryOperator,
	casts: [dynamic]^Cast,
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

	AddUnary(
		&binder,
		.ExclamationMark,
		InstNegateBool{},
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
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

	AddBinary(
		&binder,
		.Plus,
		InstAddU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.Minus,
		InstSubU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.Asterisk,
		InstMulU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.Slash,
		InstDivU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.Percent,
		InstModU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.Percent,
		InstModU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddBinary(
		&binder,
		.EqualsEquals,
		InstEqualU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.ExclamationMarkEquals,
		InstNotEqualU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.LessThan,
		InstLessThanU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.LessThanEquals,
		InstLessThanEqualU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.GreaterThan,
		InstGreaterThanU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.GreaterThanEquals,
		InstGreaterThanEqualU8{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.EqualsEquals,
		InstEqualBool{},
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.ExclamationMarkEquals,
		InstNotEqualBool{},
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.AmpersandAmpersand,
		InstAndBool{},
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
	)

	AddBinary(
		&binder,
		.PipePipe,
		InstOrBool{},
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
		Binder_GetBoolType(&binder),
	)

	AddCast :: proc(binder: ^Binder, operation: Instruction, from: ^BoundType, to: ^BoundType) {
		castt := new(Cast)
		castt^ = Cast{
			from      = from,
			to        = to,
			operation = operation,
		}
		append(&binder.casts, castt)
	}

	AddCast(
		&binder,
		InstS64ToU8{},
		Binder_GetIntegerType(&binder, 8, true),
		Binder_GetIntegerType(&binder, 1, false),
	)

	AddCast(
		&binder,
		InstU8ToS64{},
		Binder_GetIntegerType(&binder, 1, false),
		Binder_GetIntegerType(&binder, 8, true),
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

Binder_GetArrayType :: proc(binder: ^Binder, inner_type: ^BoundType, count: uint) -> ^BoundArrayType {
	// Speed this up somehow
	for type in binder.types {
		if array_type, ok := type.type_kind.(^BoundArrayType); ok {
			if array_type.inner_type == inner_type && array_type.count == count {
				return array_type
			}
		}
	}

	array_type := BoundType_Create(BoundArrayType, inner_type.size * count, len(binder.types))
	array_type.inner_type = inner_type
	array_type.count = count
	append(&binder.types, array_type)
	return array_type
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

Binder_IsAssignable :: proc(binder: ^Binder, expression: ^BoundExpression) -> bool {
	switch e in expression.expression_kind {
		case ^BoundName: {
			return true
		}

		case ^BoundInteger: {
			return false
		}

		case ^BoundArrayIndex: {
			return true
		}

		case ^BoundCast: {
			return false
		}

		case ^BoundTrue: {
			return false
		}

		case ^BoundFalse: {
			return false
		}

		case ^BoundUnary: {
			return false
		}

		case ^BoundBinary: {
			return false
		}

		case: {
			assert(false, "unreachable default case in Binder_IsAssignable")
			return false
		}
	}
}

Binder_BindAsType :: proc(binder: ^Binder, expression: ^AstExpression, parent_scope: ^BoundScope) -> (type: ^BoundType, error: Maybe(Error)) {
	switch e in expression.expression_kind {
		case ^AstName: {
			name := e
			name_string := name.name_token.data.(string)
			switch name_string {
				case "u8": {
					return Binder_GetIntegerType(binder, 1, false), nil
				}

				case "s64": {
					return Binder_GetIntegerType(binder, 8, true), nil
				}

				case "bool": {
					return Binder_GetBoolType(binder), nil
				}

				case: {
					return nil, Error{
						loc     = name.name_token.loc,
						message = fmt.tprintf("'{}' does not name a type", name_string),
					}
				}
			}
		}

		case ^AstInteger: {
			integer := e
			return nil, Error{
				loc     = integer.integer_token.loc,
				message = "Cannot convert integer to type",
			}
		}

		case ^AstArray: {
			array := e
			inner_type := Binder_BindAsType(binder, array.type, parent_scope) or_return
			return Binder_GetArrayType(binder, inner_type, array.count), nil
		}

		case ^AstArrayIndex: {
			array_index := e
			return nil, Error{
				loc     = array_index.open_bracket_token.loc,
				message = "Cannot convert array index to type",
			}
		}

		case ^AstSizeOf: {
			sizeof := e
			return nil, Error{
				loc     = sizeof.sizeof_token.loc,
				message = "Cannot convert sizeof to type",
			}
		}

		case ^AstCast: {
			castt := e
			return nil, Error{
				loc     = castt.cast_token.loc,
				message = "Cannot convert cast to type",
			}
		}

		case ^AstTrue: {
			truee := e
			return nil, Error{
				loc     = truee.true_token.loc,
				message = "Cannot convert boolean literal to type",
			}
		}

		case ^AstFalse: {
			falsee := e
			return nil, Error{
				loc     = falsee.false_token.loc,
				message = "Cannot convert boolean literal to type",
			}
		}

		case ^AstUnary: {
			unary := e
			return nil, Error{
				loc     = unary.operator_token.loc,
				message = "Cannot convert unary expression to type",
			}
		}

		case ^AstBinary: {
			binary := e
			return nil, Error{
				loc     = binary.operator_token.loc,
				message = "Cannot convert binary expression to type",
			}
		}

		case: {
			message := "unreachable default case in Binder_BindAsType"
			assert(false, message)
			return nil, Error{
				message = message,
			}
		}
	}
}

Binder_BindStatement :: proc(binder: ^Binder, statement: ^AstStatement, parent_file: ^BoundFile, parent_scope: ^BoundScope) -> (bound_statement: ^BoundStatement, error: Maybe(Error)) {
	switch s in statement.statement_kind {
		case ^AstScope: {
			scope := s
			bound_scope := BoundStatement_Create(BoundScope, parent_file, parent_scope)
			bound_scope.stack_offset = parent_scope.stack_size

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
				bound_declaration.type = Binder_BindAsType(binder, declaration.type, parent_scope) or_return
			}

			if declaration.value != nil {
				bound_declaration.value = Binder_BindExpression(binder, declaration.value, bound_declaration.type, bound_declaration) or_return
			}

			if bound_declaration.type == nil {
				bound_declaration.type = bound_declaration.value.type
			} else if bound_declaration.value != nil && bound_declaration.type != bound_declaration.value.type {
				return nil, Error{
					loc     = declaration.equals_token,
					message = fmt.tprintf(
						"Cannot assign type '{}' to type '{}'",
						BoundNode_ToString(bound_declaration.value.type, context.temp_allocator),
						BoundNode_ToString(bound_declaration.type, context.temp_allocator),
					),
				}
			}

			parent_scope.stack_size += bound_declaration.type.size
			bound_declaration.stack_location = parent_scope.stack_offset + parent_scope.stack_size

			parent_scope.declarations[bound_declaration.name] = bound_declaration
			return bound_declaration, nil
		}

		case ^AstAssignment: {
			assignment := s
			bound_assignment := BoundStatement_Create(BoundAssignment, parent_file, parent_scope)
			bound_assignment.operand = Binder_BindExpression(binder, assignment.operand, nil, bound_assignment) or_return
			bound_assignment.value   = Binder_BindExpression(binder, assignment.value, bound_assignment.operand.type, bound_assignment) or_return
			
			if !Binder_IsAssignable(binder, bound_assignment.operand) {
				return nil, Error{
					loc     = assignment.equals_token.loc,
					message = "Operand is not assignable",
				}
			}

			if assignment.equals_token.kind == .Equals {
				if bound_assignment.operand.type != bound_assignment.value.type {
					return nil, Error{
						loc     = assignment.equals_token.loc,
						message = fmt.tprintf(
							"Cannot assign type '{}' to type '{}'",
							BoundNode_ToString(bound_assignment.value.type, context.temp_allocator),
							BoundNode_ToString(bound_assignment.operand.type, context.temp_allocator),
						),
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
					message = fmt.tprintf(
						"Cannot find binary operator '{}' for types '{}' and '{}'",
						BinaryOperator_ToString(operator_kind, context.temp_allocator),
						BoundNode_ToString(bound_assignment.operand.type, context.temp_allocator),
						BoundNode_ToString(bound_assignment.value.type, context.temp_allocator),
					),
				}
			}

			return bound_assignment, nil
		}

		case ^AstStatementExpression: {
			statement_expression := s
			bound_statement_expression := BoundStatement_Create(BoundStatementExpression, parent_file, parent_scope)
			bound_statement_expression.expression = Binder_BindExpression(binder, statement_expression.expression, nil, bound_statement_expression) or_return
			return bound_statement_expression, nil
		}

		case ^AstIf: {
			iff := s
			bound_if := BoundStatement_Create(BoundIf, parent_file, parent_scope)
			bound_if.condition = Binder_BindExpression(binder, iff.condition, Binder_GetBoolType(binder), bound_if) or_return
			if bound_if.condition.type != Binder_GetBoolType(binder) {
				return {}, Error{
					loc     = iff.if_token.loc,
					message = fmt.tprintf(
						"Cannot use type '{}' in if condition",
						BoundNode_ToString(bound_if.condition.type, context.temp_allocator),
					),
				}
			}
			bound_if.then_statement = Binder_BindStatement(binder, iff.then_statement, parent_file, parent_scope) or_return
			if iff.else_statement != nil {
				bound_if.else_statement = Binder_BindStatement(binder, iff.else_statement, parent_file, parent_scope) or_return
			}
			return bound_if, nil
		}

		case ^AstWhile: {
			whilee := s
			bound_while := BoundStatement_Create(BoundWhile, parent_file, parent_scope)
			bound_while.condition = Binder_BindExpression(binder, whilee.condition, Binder_GetBoolType(binder), bound_while) or_return
			if bound_while.condition.type != Binder_GetBoolType(binder) {
				return {}, Error{
					loc     = whilee.while_token.loc,
					message = fmt.tprintf(
						"Cannot use type '{}' in while condition",
						BoundNode_ToString(bound_while.condition.type, context.temp_allocator),
					),
				}
			}
			bound_while.then_statement = Binder_BindStatement(binder, whilee.then_statement, parent_file, parent_scope) or_return
			return bound_while, nil
		}

		// This is temporary
		case ^AstPrint: {
			print := s
			bound_print := BoundStatement_Create(BoundPrint, parent_file, parent_scope)
			bound_print.expression = Binder_BindExpression(binder, print.expression, nil, bound_print) or_return
			if bound_print.expression.type != Binder_GetIntegerType(binder, 8, true) && bound_print.expression.type != Binder_GetIntegerType(binder, 1, false) {
				return {}, Error{
					loc     = print.print_token,
					message = fmt.tprintf(
						"Cannot print type '{}'",
						BoundNode_ToString(bound_print.expression.type, context.temp_allocator),
					),
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

Binder_BindExpression :: proc(binder: ^Binder, expression: ^AstExpression, suggested_type: ^BoundType, parent_statement: ^BoundStatement) -> (bound_expression: ^BoundExpression, error: Maybe(Error)) {
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
			integer_type := cast(^BoundType) Binder_GetIntegerType(binder, 8, true)
			if suggested_type != nil {
				if _, ok := suggested_type.type_kind.(^BoundIntegerType); ok {
					integer_type = suggested_type
				}
			}
			bound_integer := BoundExpression_Create(BoundInteger, integer_type, parent_statement)
			bound_integer.value = integer.integer_token.data.(u64)
			return bound_integer, nil
		}

		case ^AstArray: {
			array := e
			return nil, Error{
				loc     = array.open_bracket_token.loc,
				message = "Cannot declare array type outside of type context (for now)",
			}
		}

		case ^AstArrayIndex: {
			array_index := e
			bound_operand := Binder_BindExpression(binder, array_index.operand, nil, parent_statement) or_return
			if array_type, ok := bound_operand.type.type_kind.(^BoundArrayType); !ok {
				return nil, Error{
					loc     = array_index.open_bracket_token.loc,
					message = fmt.tprintf(
						"Cannot index a '{}'",
						BoundNode_ToString(bound_operand.type, context.temp_allocator),
					),
				}
			}
			bound_index := Binder_BindExpression(binder, array_index.index, nil, parent_statement) or_return
			if bound_index.type != Binder_GetIntegerType(binder, 8, true) {
				return nil, Error{
					loc     = array_index.open_bracket_token.loc,
					message = fmt.tprintf(
						"Cannot index array with type '{}'",
						BoundNode_ToString(bound_index.type, context.temp_allocator),
					),
				}
			}
			bound_array_index := BoundExpression_Create(BoundArrayIndex, bound_operand.type.type_kind.(^BoundArrayType).inner_type, parent_statement)
			bound_array_index.operand = bound_operand	
			bound_array_index.index = bound_index
			return bound_array_index, nil
		}

		case ^AstSizeOf: {
			sizeof := e
			bound_operand := Binder_BindExpression(binder, sizeof.operand, nil, parent_statement) or_return
			integer_type  := cast(^BoundType) Binder_GetIntegerType(binder, 8, true)
			if suggested_type != nil {
				if _, ok := suggested_type.type_kind.(^BoundIntegerType); ok {
					integer_type = suggested_type
				}
			}
			bound_integer := BoundExpression_Create(BoundInteger, integer_type, parent_statement)
			bound_integer.value = cast(u64) bound_operand.type.size
			return bound_integer, nil
		}

		case ^AstCast: {
			castt := e
			type := Binder_BindAsType(binder, castt.type, parent_statement.parent_scope) or_return
			bound_operand := Binder_BindExpression(binder, castt.operand, type, parent_statement) or_return
			if bound_operand.type == type {
				return bound_operand, nil
			}
			cast_type: ^Cast = nil
			for castt in binder.casts {
				if castt.from == bound_operand.type && castt.to == type {
					cast_type = castt
				}
			}
			bound_cast := BoundExpression_Create(BoundCast, type, parent_statement)
			bound_cast.castt = cast_type
			bound_cast.operand = bound_operand
			return bound_cast, nil
		}

		case ^AstTrue: {
			truee := e
			bool_type := Binder_GetBoolType(binder)
			bound_true := BoundExpression_Create(BoundTrue, bool_type, parent_statement)
			return bound_true, nil
		}

		case ^AstFalse: {
			falsee := e
			bool_type := Binder_GetBoolType(binder)
			bound_false := BoundExpression_Create(BoundFalse, bool_type, parent_statement)
			return bound_false, nil
		}

		case ^AstUnary: {
			unary := e
			bound_operand := Binder_BindExpression(binder, unary.operand, suggested_type, parent_statement) or_return

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
					message = fmt.tprintf(
						"Cannot find unary operator '{}' for type '{}'",
						BinaryOperator_ToString(unary.operator_token.kind, context.temp_allocator),
						BoundNode_ToString(bound_operand.type, context.temp_allocator),
					),
				}
			}

			bound_unary := BoundExpression_Create(BoundUnary, unary_operator.result_type, parent_statement)
			bound_unary.unary_operator = unary_operator
			bound_unary.operand = bound_operand
			return bound_unary, nil
		}

		case ^AstBinary: {
			binary := e
			bound_left := Binder_BindExpression(binder, binary.left, suggested_type, parent_statement) or_return
			bound_right := Binder_BindExpression(binder, binary.right, bound_left.type, parent_statement) or_return

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
					message = fmt.tprintf(
						"Cannot find binary operator '{}' for types '{}' and '{}'",
						BinaryOperator_ToString(binary.operator_token.kind, context.temp_allocator),
						BoundNode_ToString(bound_left.type, context.temp_allocator),
						BoundNode_ToString(bound_right.type, context.temp_allocator),
					),
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
