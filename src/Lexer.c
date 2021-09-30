#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>

Lexer Lexer_Create(String filePath, String source) {
    return (Lexer){
        .FilePath = filePath,
        .Source   = source,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Current  = source.Length > 0 ? source.Data[0] : '\0',
    };
}

uint8_t Lexer_NextChar(Lexer* lexer) {
    uint8_t current = lexer->Current;

    lexer->Position++;
    lexer->Column++;
    if (current == '\n') {
        lexer->Line++;
        lexer->Column = 1;
    }

    lexer->Current = lexer->Position < lexer->Source.Length ? lexer->Source.Data[lexer->Position] : '\0';

    return current;
}

uint8_t Lexer_ExpectChar(Lexer* lexer, uint8_t chr) {
    if (lexer->Current == chr) {
        return Lexer_NextChar(lexer);
    }

    Token token = {
        .Kind     = TokenKind_Error,
        .FilePath = lexer->FilePath,
        .Source   = lexer->Source,
        .Position = lexer->Position,
        .Line     = lexer->Line,
        .Column   = lexer->Column,
        .Length   = 1,
    };

    Token_Error(token, "Expected '%c', got '%c'", chr, lexer->Current);
    lexer->Error = true;

    return chr;
}

Token Lexer_NextToken(Lexer* lexer) {
#define TOKEN(kind)                                                                                                         \
    (Token) {                                                                                                               \
        .Kind = (kind), .FilePath = lexer->FilePath, .Source = lexer->Source, .Position = startPosition, .Line = startLine, \
        .Column = startColumn, .Length = lexer->Position - startPosition,                                                   \
    }

#define TOKEN_INT(kind, value)                                                                                              \
    (Token) {                                                                                                               \
        .Kind = (kind), .FilePath = lexer->FilePath, .Source = lexer->Source, .Position = startPosition, .Line = startLine, \
        .Column = startColumn, .Length = lexer->Position - startPosition, .IntValue = (value),                              \
    }

#define TOKEN_FLOAT(kind, value)                                                                                            \
    (Token) {                                                                                                               \
        .Kind = (kind), .FilePath = lexer->FilePath, .Source = lexer->Source, .Position = startPosition, .Line = startLine, \
        .Column = startColumn, .Length = lexer->Position - startPosition, .FloatValue = (value),                            \
    }

#define TOKEN_STRING(kind, value)                       \
    (Token){                                            \
        .Kind        = (kind),                          \
        .FilePath    = lexer->FilePath,                 \
        .Source      = lexer->Source,                   \
        .Position    = startPosition,                   \
        .Line        = startLine,                       \
        .Column      = startColumn,                     \
        .Length      = lexer->Position - startPosition, \
        .StringValue = (value),                         \
    }

Start:
    uint64_t startPosition = lexer->Position;
    uint64_t startLine     = lexer->Line;
    uint64_t startColumn   = lexer->Column;

    if (lexer->Current == '\0') {
        return TOKEN(TokenKind_EndOfFile);
    } else if (lexer->Current >= '0' && lexer->Current <= '9') {
        uint64_t base     = 10;
        uint64_t intValue = 0;

        if (lexer->Current == '0') {
            Lexer_ExpectChar(lexer, '0');

            while (lexer->Current == '_') {
                Lexer_ExpectChar(lexer, '_');
            }

            switch (lexer->Current) {
                case 'b': {
                    Lexer_ExpectChar(lexer, 'b');
                    base = 2;
                } break;

                case 'o': {
                    Lexer_ExpectChar(lexer, 'o');
                    base = 8;
                } break;

                case 'd': {
                    Lexer_ExpectChar(lexer, 'd');
                    base = 10;
                } break;

                case 'x': {
                    Lexer_ExpectChar(lexer, 'x');
                    base = 16;
                } break;

                default: {
                    base = 10;
                } break;
            }
        }

        while ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
               (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
            if (lexer->Current == '_') {
                Lexer_ExpectChar(lexer, '_');
                continue;
            }

            uint64_t value;
            if (lexer->Current >= '0' && lexer->Current <= '9') {
                value = lexer->Current - '0';
            } else if (lexer->Current >= 'a' && lexer->Current <= 'z') {
                value = lexer->Current - 'a' + 10;
            } else if (lexer->Current >= 'A' && lexer->Current <= 'Z') {
                value = lexer->Current - 'A' + 10;
            } else {
                Token token = (Token){
                    .Kind     = TokenKind_Error,
                    .FilePath = lexer->FilePath,
                    .Source   = lexer->Source,
                    .Position = lexer->Position,
                    .Line     = lexer->Line,
                    .Column   = lexer->Column,
                    .Length   = 1,
                };
                Token_Error(token, "Internal Error: Unreachable character to int conversion in lexer\n");
                lexer->Error = true;
                exit(EXIT_FAILURE);
            }

            if (value >= base) {
                Token token = (Token){
                    .Kind     = TokenKind_Error,
                    .FilePath = lexer->FilePath,
                    .Source   = lexer->Source,
                    .Position = lexer->Position,
                    .Line     = lexer->Line,
                    .Column   = lexer->Column,
                    .Length   = 1,
                };
                Token_Error(token, "Unexpected digit for base %llu", base);
                lexer->Error = true;
            }

            intValue *= 10;
            intValue += value;

            Lexer_NextChar(lexer);
        }

        if (lexer->Current == '.') {
            Token token = (Token){
                .Kind     = TokenKind_Error,
                .FilePath = lexer->FilePath,
                .Source   = lexer->Source,
                .Position = lexer->Position,
                .Line     = lexer->Line,
                .Column   = lexer->Column,
                .Length   = 1,
            };
            Token_Error(token, "Internal Error: Float literal not implemented in lexer");
            lexer->Error = true;
            exit(EXIT_FAILURE);
        }

        return TOKEN_INT(TokenKind_Integer, intValue);
    } else if ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
               lexer->Current == '_') {
        String name = String_Create(&lexer->Source.Data[lexer->Position], 0);

        while ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
               (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
            Lexer_NextChar(lexer);
            name.Length++;
        }

        for (TokenKind i = TokenKind_Begin_Keyword + 1; i < TokenKind_End_Keyword; i++) {
            if (String_Equal(TokenKind_Names[i], name)) {
                return TOKEN(i);
            }
        }

        return TOKEN_STRING(TokenKind_Identifier, name);
    } else {
        switch (lexer->Current) {
            case ' ':
            case '\t':
            case '\n':
            case '\r': {
                Lexer_NextChar(lexer);
                goto Start;
            } break;

            case '#': {
                String name = String_Create(&lexer->Source.Data[lexer->Position], 0);
                Lexer_ExpectChar(lexer, '#');
                name.Length++;
                while ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
                       (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
                    Lexer_NextChar(lexer);
                    name.Length++;
                }

                for (TokenKind i = TokenKind_Begin_HashDirective; i < TokenKind_End_HashDirective; i++) {
                    if (String_Equal(TokenKind_Names[i], name)) {
                        return TOKEN(i);
                    }
                }

                Token token = TOKEN(TokenKind_Error);
                Token_Error(token, "Unable to find hash directive '%.*s'", String_Fmt(name));
                lexer->Error = true;
                return token;
            } break;

            case '\"': {
                Lexer_ExpectChar(lexer, '\"');

                String string = String_Create(&lexer->Source.Data[lexer->Position], 0);
                while (lexer->Current != '\"' && lexer->Current != '\0') {
                    Lexer_NextChar(lexer);
                    string.Length++;
                }

                Lexer_ExpectChar(lexer, '\"');

                return TOKEN_STRING(TokenKind_String, string);
            } break;

            case '/': {
                Lexer_ExpectChar(lexer, '/');
                if (lexer->Current == '=') {
                    Lexer_ExpectChar(lexer, '=');
                    return TOKEN(TokenKind_SlashEquals);
                } else if (lexer->Current == '/') {
                    Lexer_ExpectChar(lexer, '/');
                    while (lexer->Current != '\n' && lexer->Current != '\0') {
                        Lexer_NextChar(lexer);
                    }
                    goto Start;
                } else if (lexer->Current == '*') {
                    Lexer_ExpectChar(lexer, '*');

                    uint64_t depth = 1;

                    while (depth > 0 && lexer->Current != '\0') {
                        if (lexer->Current == '/') {
                            Lexer_ExpectChar(lexer, '/');
                            if (lexer->Current == '*') {
                                Lexer_ExpectChar(lexer, '*');
                                depth++;
                            }
                        } else if (lexer->Current == '*') {
                            Lexer_ExpectChar(lexer, '*');
                            if (lexer->Current == '/') {
                                Lexer_ExpectChar(lexer, '/');
                                depth--;
                            }
                        } else {
                            Lexer_NextChar(lexer);
                        }
                    }

                    goto Start;
                }
                return TOKEN(TokenKind_Slash);
            } break;

            case '-': {
                Lexer_ExpectChar(lexer, '-');
                if (lexer->Current == '=') {
                    Lexer_ExpectChar(lexer, '=');
                    return TOKEN(TokenKind_MinusEquals);
                } else if (lexer->Current == '>') {
                    Lexer_ExpectChar(lexer, '>');
                    return TOKEN(TokenKind_RightArrow);
                }
                return TOKEN(TokenKind_Minus);
            } break;

#define SINGLE_CHAR(chr, kind)        \
    case chr: {                       \
        Lexer_ExpectChar(lexer, chr); \
        return TOKEN(kind);           \
    } break

                SINGLE_CHAR(';', TokenKind_Semicolon);
                SINGLE_CHAR(':', TokenKind_Colon);
                SINGLE_CHAR(',', TokenKind_Comma);
                SINGLE_CHAR('.', TokenKind_Period);
                SINGLE_CHAR('(', TokenKind_OpenParenthesis);
                SINGLE_CHAR(')', TokenKind_CloseParenthesis);
                SINGLE_CHAR('{', TokenKind_OpenBrace);
                SINGLE_CHAR('}', TokenKind_CloseBrace);

#undef SINGLE_CHAR

#define DOUBLE_CHAR(chr, kind, chr2, kind2) \
    case chr: {                             \
        Lexer_ExpectChar(lexer, chr);       \
        if (lexer->Current == chr2) {       \
            Lexer_ExpectChar(lexer, chr2);  \
            return TOKEN(kind2);            \
        }                                   \
        return TOKEN(kind);                 \
    } break

                DOUBLE_CHAR('+', TokenKind_Plus, '=', TokenKind_PlusEquals);
                DOUBLE_CHAR('*', TokenKind_Asterisk, '=', TokenKind_AsteriskEquals);
                DOUBLE_CHAR('%', TokenKind_Percent, '=', TokenKind_PercentEquals);
                DOUBLE_CHAR('<', TokenKind_LessThan, '=', TokenKind_LessThanEquals);
                DOUBLE_CHAR('>', TokenKind_GreaterThan, '=', TokenKind_GreaterThanEquals);
                DOUBLE_CHAR('=', TokenKind_Equals, '=', TokenKind_EqualsEquals);
                DOUBLE_CHAR('!', TokenKind_Bang, '=', TokenKind_BangEquals);

#undef DOUBLE_CHAR

            default: {
                Lexer_NextChar(lexer);
                Token token = TOKEN(TokenKind_Error);
                Token_Error(token, "Unexpected '%c'", lexer->Source.Data[token.Position]);
                lexer->Error = true;
                return token;
            } break;
        }
    }

#undef TOKEN
#undef TOKEN_INT
#undef TOKEN_FLOAT
#undef TOKEN_STRING
}
