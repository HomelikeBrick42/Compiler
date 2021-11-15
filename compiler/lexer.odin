package compiler

import "core:fmt"
import "core:strings"
import "core:unicode"
import "core:strconv"

@(private="file")
lexer_single_tokens := map[rune]TokenKind{
	'(' = .OpenParenthesis,
	')' = .CloseParenthesis,
	'{' = .OpenBrace,
	'}' = .CloseBrace,
	'[' = .OpenBracket,
	']' = .CloseBracket,
	':' = .Colon,
	';' = .Semicolon,
	',' = .Comma,

	'+' = .Plus,
	'-' = .Minus,
	'*' = .Asterisk,
	'/' = .Slash,
	'%' = .Percent,
	'=' = .Equals,
	'!' = .ExclamationMark,
	'<' = .LessThan,
	'>' = .GreaterThan,
}

@(private="file")
lexer_double_tokens := map[rune][]struct{
	second: rune,
	kind: TokenKind,
} {
	'+' = { { '=', .PlusEquals            } },
	'-' = { { '=', .MinusEquals           }, { '>', .RightArrow } },
	'*' = { { '=', .AsteriskEquals        } },
	'/' = { { '=', .SlashEquals           } },
	'%' = { { '=', .PercentEquals         } },
	'=' = { { '=', .EqualsEquals          } },
	'!' = { { '=', .ExclamationMarkEquals } },
	'<' = { { '=', .LessThanEquals        } },
	'>' = { { '=', .GreaterThanEquals     } },
	'&' = { { '&', .AmpersandAmpersand    } },
	'|' = { { '|', .PipePipe              } },
}

@(private="file")
lexer_keywords := map[string]TokenKind{
	"print"  = .PrintKeyword, // This is temporary
	"if"     = .IfKeyword,
	"else"   = .ElseKeyword,
	"do"     = .DoKeyword,
	"true"   = .TrueKeyword,
	"false"  = .FalseKeyword,
	"while"  = .WhileKeyword,
	"sizeof" = .SizeOfKeyword,
	"cast"   = .CastKeyword,
	"return" = .ReturnKeyword,
}

Lexer :: struct {
	using loc: SourceLoc,
	reader:    strings.Reader,
	current:   rune,
}

Lexer_Create :: proc(source: string, path: string) -> Lexer {
	lexer         := Lexer{}
	lexer.path     = path
	lexer.source   = source
	lexer.position = 0
	lexer.line     = 1
	lexer.column   = 1

	strings.reader_init(&lexer.reader, lexer.source)

	chr, _, err := strings.reader_read_rune(&lexer.reader)
	if err == nil {
		lexer.current = chr
	} else {
		assert(err == .EOF)
		lexer.current = 0
	}

	return lexer
}

@(private="file")
Lexer_NextRune :: proc(lexer: ^Lexer) -> rune {
	current := lexer.current
	
	lexer.position += 1
	lexer.column   += 1

	if current == '\n' {
		lexer.line  += 1
		lexer.column = 1
	}

	chr, _, err := strings.reader_read_rune(&lexer.reader)
	if err == nil {
		lexer.current = chr
	} else {
		assert(err == .EOF)
		lexer.current = 0
	}

	return current
}

Lexer_NextToken :: proc(lexer: ^Lexer) -> (token: Token, error: Maybe(Error)) {
	for {
		start_loc := lexer.loc

		if lexer.current == 0 {
			return Token{
				kind   = .EndOfFile,
				loc    = start_loc,
				length = lexer.position - start_loc.position,
			}, nil
		} else if unicode.is_space(lexer.current) {
			Lexer_NextRune(lexer)
			continue
		} else if unicode.is_digit(lexer.current) {
			base := 10

			if lexer.current == '0' {
				Lexer_NextRune(lexer)
				switch lexer.current {
					case 'x': {
						Lexer_NextRune(lexer)
						base = 16
					}

					case 'd': {
						Lexer_NextRune(lexer)
						base = 10
					}

					case 'o': {
						Lexer_NextRune(lexer)
						base = 8
					}

					case 'b': {
						Lexer_NextRune(lexer)
						base = 2
					}

					case: {
						base = 10
					}
				}
			}

			for unicode.is_digit(lexer.current) || lexer.current == '_' {
				Lexer_NextRune(lexer)
			}

			// TODO: Parse integers without this function so we can have proper error locations
			value, ok := strconv.parse_u64_of_base(lexer.source[start_loc.position:lexer.position], base)
			if !ok {
				return {}, Error{
					loc = start_loc,
					message = fmt.tprintf("Invalid number literal for base {}", base),
				}
			}

			return Token{
				kind   = .Integer,
				loc    = start_loc,
				length = lexer.position - start_loc.position,
				data   = value,
			}, nil
		} else if unicode.is_letter(lexer.current) || lexer.current == '_' {
			for unicode.is_letter(lexer.current) || unicode.is_digit(lexer.current) || lexer.current == '_' {
				Lexer_NextRune(lexer)
			}

			name := lexer.source[start_loc.position:lexer.position]

			if kind, ok := lexer_keywords[name]; ok {
				return Token{
					kind   = kind,
					loc    = start_loc,
					length = lexer.position - start_loc.position,
				}, nil
			}

			return Token{
				kind   = .Name,
				loc    = start_loc,
				length = lexer.position - start_loc.position,
				data   = name,
			}, nil
		} else {
			last := Lexer_NextRune(lexer)

			if last == '/' && lexer.current == '*' {
				depth := 1
				for depth > 0 && lexer.current != 0 {
					chr := Lexer_NextRune(lexer)
					if chr == '/' && lexer.current == '*' {
						Lexer_NextRune(lexer)
						depth += 1
						continue
					} else if chr == '*' && lexer.current == '/' {
						Lexer_NextRune(lexer)
						depth -= 1
						continue
					}
				}
				continue
			} else if last == '/' && lexer.current == '/' {
				for lexer.current != '\n' && lexer.current != 0 {
					Lexer_NextRune(lexer)
				}
				continue
			}

			if info, ok := lexer_double_tokens[last]; ok {
				for info in info do if lexer.current == info.second {
					Lexer_NextRune(lexer)
					return {
						kind   = info.kind,
						loc    = start_loc,
						length = lexer.position - start_loc.position,
					}, nil
				}
			}

			if kind, ok := lexer_single_tokens[last]; ok {
				return {
					kind   = kind,
					loc    = start_loc,
					length = lexer.position - start_loc.position,
				}, nil
			}

			return {}, Error{
				loc = start_loc,
				message = fmt.tprintf("Unexpected character: '{}'", last),
			}
		}

		message := "unreachable end of loop in Lexer_NextToken"
		assert(false, message)
		return {}, Error{
			loc = start_loc,
			message = message,
		}
	}
}
