package compiler

import "core:fmt"

Parser :: struct {
	path: string,
	lexer: Lexer,
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
	current := parser.current
	parser.current = Lexer_NextToken(&parser.lexer) or_return
	return current, nil
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
		Parser_ExpectToken(parser, .Semicolon) or_return
		append(&file.statements, statement)
	}

	file.end_of_file_token = Parser_ExpectToken(parser, .EndOfFile) or_return
	return file, nil
}

Parser_ParseStatement :: proc(parser: ^Parser) -> (statement: ^AstStatement, error: Maybe(Error)) {
	#partial switch parser.current.kind {
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
					assignment.assignment_token = Parser_NextToken(parser) or_return
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
		case .Plus, .Minus, .ExclamationMark: {
			return 4
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
			return 3
		}

		case .Plus, .Minus: {
			return 2
		}

		case .EqualsEquals, .ExclamationMarkEquals: {
			return 1
		}

		case: {
			return 0
		}
	}
}

Parser_ParseBinaryExpression :: proc(parser: ^Parser, parent_precedence: uint) -> (expression: ^AstExpression, error: Maybe(Error)) {
	if unary_precedence := GetUnaryOperatorPrecedence(parser.current.kind); unary_precedence > 0 {
		unary := AstExpression_Create(AstUnary)
		unary.operator_token = Parser_NextToken(parser) or_return
		unary.operand = Parser_ParseBinaryExpression(parser, unary_precedence) or_return
		expression = unary
	} else {
		expression = Parser_ParsePrimaryExpression(parser) or_return
	}

	for {
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

	return
}
