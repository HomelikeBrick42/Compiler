package compiler

import "core:fmt"

Parser :: struct {
	path: string,
	lexer: Lexer,
	previous: Token,
	current: Token,
}

Parser_Create :: proc(source: string, path: string) -> (parser: Parser, error: Maybe(Error)) {
	parser.path    = path
	parser.lexer   = Lexer_Create(source, path)
	parser.current = Lexer_NextToken(&parser.lexer) or_return
	return parser, nil
}

@(private="file")
Parser_NextToken :: proc(parser: ^Parser) -> (token: Token, error: Maybe(Error)) {
	parser.previous = parser.current
	parser.current = Lexer_NextToken(&parser.lexer) or_return
	return parser.previous, nil
}

@(private="file")
Parser_ExpectToken :: proc(parser: ^Parser, kind: TokenKind) -> (token: Token, error: Maybe(Error)) {
	if parser.current.kind == kind {
		return Parser_NextToken(parser)
	}

	return {}, Error{
		loc     = parser.current.loc,
		message = fmt.tprintf("Unexpected token '{}', expected '{}'", parser.current.kind, kind),
	}
}

Parser_ParseFile :: proc(parser: ^Parser) -> (file: ^AstFile, error: Maybe(Error)) {
	file = AstNode_Create(AstFile)

	for parser.current.kind != .EndOfFile {
		statement := Parser_ParseStatement(parser) or_return
		if parser.previous.kind != .CloseBrace || parser.current.kind == .Semicolon {
			Parser_ExpectToken(parser, .Semicolon) or_return
		}
		append(&file.statements, statement)
	}

	file.end_of_file_token = Parser_ExpectToken(parser, .EndOfFile) or_return
	return file, nil
}

Parser_ParseScope :: proc(parser: ^Parser) -> (scope: ^AstScope, error: Maybe(Error)) {
	Parser_ExpectToken(parser, .OpenBrace) or_return
	scope = AstStatement_Create(AstScope)
	for parser.current.kind != .CloseBrace && parser.current.kind != .EndOfFile {
		statement := Parser_ParseStatement(parser) or_return
		if parser.previous.kind != .CloseBrace || parser.current.kind == .Semicolon {
			Parser_ExpectToken(parser, .Semicolon) or_return
		}
		append(&scope.statements, statement)
	}
	Parser_ExpectToken(parser, .CloseBrace) or_return
	return scope, nil
}

Parser_ParseStatement :: proc(parser: ^Parser) -> (statement: ^AstStatement, error: Maybe(Error)) {
	#partial switch parser.current.kind {
		case .OpenBrace: {
			return Parser_ParseScope(parser)
		}

		case .IfKeyword: {
			iff := AstStatement_Create(AstIf)
			iff.if_token  = Parser_ExpectToken(parser, .IfKeyword) or_return
			iff.condition = Parser_ParseExpression(parser) or_return
			if parser.current.kind == .DoKeyword {
				Parser_ExpectToken(parser, .DoKeyword) or_return
				iff.then_statement = Parser_ParseStatement(parser) or_return
			} else {
				iff.then_statement = Parser_ParseScope(parser) or_return
			}
			if parser.current.kind == .ElseKeyword {
				iff.else_token = Parser_ExpectToken(parser, .ElseKeyword) or_return
				iff.else_statement = Parser_ParseStatement(parser) or_return
			}
			return iff, nil
		}

		case .WhileKeyword: {
			whilee := AstStatement_Create(AstWhile)
			whilee.while_token = Parser_ExpectToken(parser, .WhileKeyword) or_return
			whilee.condition = Parser_ParseExpression(parser) or_return
			if parser.current.kind == .DoKeyword {
				Parser_ExpectToken(parser, .DoKeyword) or_return
				whilee.then_statement = Parser_ParseStatement(parser) or_return
			} else {
				whilee.then_statement = Parser_ParseScope(parser) or_return
			}
			return whilee, nil
		}

		// This is temporary
		case .PrintKeyword: {
			print := AstStatement_Create(AstPrint)
			print.print_token = Parser_ExpectToken(parser, .PrintKeyword) or_return
			print.expression  = Parser_ParseExpression(parser) or_return
			return print, nil
		}

		case: {
			expression := Parser_ParseExpression(parser) or_return
			#partial switch parser.current.kind {
				case .Colon: {
					declaration := AstStatement_Create(AstDeclaration)
					declaration.colon_token = Parser_ExpectToken(parser, .Colon) or_return

					name, ok := expression.expression_kind.(^AstName)
					if !ok {
						return {}, Error{
							loc     = declaration.colon_token.loc,
							message = fmt.tprintf("Expected name before ':' in declaration"),
						}
					}

					declaration.name = name

					if parser.current.kind != .Equals {
						declaration.type = Parser_ParseExpression(parser) or_return
					}

					if parser.current.kind == .Equals {
						declaration.equals_token = Parser_ExpectToken(parser, .Equals) or_return
						declaration.value = Parser_ParseExpression(parser) or_return
					}

					return declaration, nil
				}

				case .Equals, .PlusEquals, .MinusEquals, .AsteriskEquals, .SlashEquals, .PercentEquals: {
					assignment := AstStatement_Create(AstAssignment)
					assignment.operand = expression
					assignment.equals_token = Parser_NextToken(parser) or_return
					assignment.value = Parser_ParseExpression(parser) or_return
					return assignment, nil
				}

				case: {
					statement_expression := AstStatement_Create(AstStatementExpression)
					statement_expression.expression = expression
					return statement_expression, nil
				}
			}
		}
	}
}

Parser_ParseExpression :: proc(parser: ^Parser) -> (expression: ^AstExpression, error: Maybe(Error)) {
	return Parser_ParseBinaryExpression(parser, 0)
}

Parser_ParsePrimaryExpression :: proc(parser: ^Parser) -> (expression: ^AstExpression, error: Maybe(Error)) {
	#partial switch parser.current.kind {
		case .Name: {
			name := AstExpression_Create(AstName)
			name.name_token = Parser_ExpectToken(parser, .Name) or_return
			return name, nil
		}

		case .Integer: {
			integer := AstExpression_Create(AstInteger)
			integer.integer_token = Parser_ExpectToken(parser, .Integer) or_return
			return integer, nil
		}

		case .SizeOfKeyword: {
			sizeof := AstExpression_Create(AstSizeOf)
			sizeof.sizeof_token = Parser_ExpectToken(parser, .SizeOfKeyword) or_return
			Parser_ExpectToken(parser, .OpenParenthesis) or_return
			sizeof.operand = Parser_ParseExpression(parser) or_return
			Parser_ExpectToken(parser, .CloseParenthesis) or_return
			return sizeof, nil
		}

		case .TrueKeyword: {
			truee := AstExpression_Create(AstTrue)
			truee.true_token = Parser_ExpectToken(parser, .TrueKeyword) or_return
			return truee, nil
		}

		case .FalseKeyword: {
			falsee := AstExpression_Create(AstFalse)
			falsee.false_token = Parser_ExpectToken(parser, .FalseKeyword) or_return
			return falsee, nil
		}

		case .OpenParenthesis: {
			Parser_ExpectToken(parser, .OpenParenthesis) or_return
			expression := Parser_ParseExpression(parser) or_return
			Parser_ExpectToken(parser, .CloseParenthesis) or_return
			return expression, nil
		}

		case: {
			return {}, Error{
				loc     = parser.current.loc,
				message = fmt.tprintf("Unexpected token '{}' in expression", parser.current.kind),
			}
		}
	}
}

@(private="file")
GetUnaryOperatorPrecedence :: proc(kind: TokenKind) -> uint {
	#partial switch kind {
		case .Plus, .Minus, .ExclamationMark, .OpenBracket: {
			return 6
		}

		case: {
			return 0
		}
	}
}

@(private="file")
GetBinaryOperatorPrecedence :: proc(kind: TokenKind) -> uint {
	#partial switch kind {
		case .Asterisk, .Slash, .Percent: {
			return 5
		}

		case .Plus, .Minus: {
			return 4
		}

		case .EqualsEquals, .ExclamationMarkEquals, .GreaterThan, .GreaterThanEquals, .LessThan, .LessThanEquals: {
			return 3
		}

		case .AmpersandAmpersand: {
			return 2
		}

		case .PipePipe: {
			return 1
		}

		case: {
			return 0
		}
	}
}

Parser_ParseBinaryExpression :: proc(parser: ^Parser, parent_precedence: uint) -> (expression: ^AstExpression, error: Maybe(Error)) {
	if unary_precedence := GetUnaryOperatorPrecedence(parser.current.kind); unary_precedence > 0 {
		token := Parser_NextToken(parser) or_return
		if token.kind == .OpenBracket {
			array := AstExpression_Create(AstArray)
			array.open_bracket_token = token
			integer := Parser_ExpectToken(parser, .Integer) or_return
			array.count = cast(uint) integer.data.(u64)
			array.close_bracket_token = Parser_ExpectToken(parser, .CloseBracket) or_return
			array.type = Parser_ParseBinaryExpression(parser, unary_precedence) or_return
			expression = array
		} else {
			unary := AstExpression_Create(AstUnary)
			unary.operator_token = token
			unary.operand = Parser_ParseBinaryExpression(parser, unary_precedence) or_return
			expression = unary
		}
	} else {
		expression = Parser_ParsePrimaryExpression(parser) or_return
	}

	for {
		if parser.current.kind == .OpenBracket {
			array_index := AstExpression_Create(AstArrayIndex)
			array_index.operand = expression
			array_index.open_bracket_token = Parser_ExpectToken(parser, .OpenBracket) or_return
			array_index.index = Parser_ParseExpression(parser) or_return
			array_index.close_bracket_token = Parser_ExpectToken(parser, .CloseBracket) or_return
			expression = array_index
		} else {
			binary_precedence := GetBinaryOperatorPrecedence(parser.current.kind)
			if binary_precedence <= parent_precedence {
				break
			}

			binary := AstExpression_Create(AstBinary)
			binary.left = expression
			binary.operator_token = Parser_NextToken(parser) or_return
			binary.right = Parser_ParseBinaryExpression(parser, binary_precedence) or_return
			expression = binary
		}
	}

	return
}
