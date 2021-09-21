#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

bool Lexer_Create(Lexer* lexer, const char* path) {
    if (!lexer) {
        return false;
    }

    *lexer = (Lexer){};

    lexer->FilePath = malloc(4096);

#if defined(_WIN32) || defined(_WIN64)
    GetFullPathNameA(path, 4096, lexer->FilePath, NULL);
#else
    lexer->FilePath = strdup(path);
#endif

    FILE* file = fopen(lexer->FilePath, "rb");
    if (!file) {
        fflush(stdout);
        fprintf(stderr, "Unable to open file '%s'\n", lexer->FilePath);
        return false;
    }

    fseek(file, 0, SEEK_END);
    uint64_t sourceLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(sourceLength + 1);
    if (!source) {
        fflush(stdout);
        fprintf(stderr, "Unable to allocate space for file '%s'\n", lexer->FilePath);
        fclose(file);
        return false;
    }

    if (fread(source, sizeof(char), sourceLength, file) != sourceLength) {
        fflush(stdout);
        fprintf(stderr, "Unable to read file '%s'\n", lexer->FilePath);
        free(source);
        fclose(file);
        return false;
    }

    source[sourceLength] = '\0';

    lexer->Source       = source;
    lexer->SourceLength = sourceLength;

    lexer->Position = 0;
    lexer->Line     = 1;
    lexer->Column   = 1;
    lexer->Current  = lexer->Position < lexer->SourceLength ? lexer->Source[lexer->Position] : '\0';
    lexer->WasError = false;

    return true;
}

void Lexer_Destroy(Lexer* lexer) {
    free(lexer->FilePath);
    free(lexer->Source);
}

Token Lexer_NextToken(Lexer* lexer) {
#define TOKEN(kind)                                  \
    (Token){                                         \
        .Kind     = (kind),                          \
        .FilePath = lexer->FilePath,                 \
        .Source   = lexer->Source,                   \
        .Position = startPosition,                   \
        .Line     = startLine,                       \
        .Column   = startColumn,                     \
        .Length   = lexer->Position - startPosition, \
    };
Start:
    uint64_t startPosition = lexer->Position;
    uint64_t startLine     = lexer->Line;
    uint64_t startColumn   = lexer->Column;

    if (lexer->Current == '\0') {
        return TOKEN(TokenKind_EndOfFile);
    } else if (lexer->Current >= '0' && lexer->Current <= '9') {
        while (lexer->Current >= '0' && lexer->Current <= '9') {
            Lexer_NextChar(lexer);
        }

        if (lexer->Current == '.') {
            Lexer_ExpectChar(lexer, '.');

            while (lexer->Current >= '0' && lexer->Current <= '9') {
                Lexer_NextChar(lexer);
            }

            return TOKEN(TokenKind_Float);
        }

        return TOKEN(TokenKind_Integer);
    } else if ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
               lexer->Current == '_') {
        while ((lexer->Current >= 'a' && lexer->Current <= 'z') || (lexer->Current >= 'A' && lexer->Current <= 'Z') ||
               (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
            Lexer_NextChar(lexer);
        }

        Token identifier = TOKEN(TokenKind_Identifier);

        for (uint64_t i = TokenKind_KeywordBegin + 1; i < TokenKind_KeywordEnd; i++) {
            if (identifier.Length == strlen(TokenKind_Names[i])) {
                if (memcmp(&lexer->Source[startPosition], TokenKind_Names[i], identifier.Length) == 0) {
                    identifier.Kind = i;
                    break;
                }
            }
        }

        return identifier;
    } else {
        switch (lexer->Current) {
            case ' ':
            case '\t':
            case '\n':
            case '\r': {
                Lexer_NextChar(lexer);
                goto Start;
            } break;

#define MATCH1(kind, chr)             \
    case chr: {                       \
        Lexer_ExpectChar(lexer, chr); \
        return TOKEN(kind);           \
    } break

                MATCH1(TokenKind_Semicolon, ';');
                MATCH1(TokenKind_Colon, ':');
                MATCH1(TokenKind_Comma, ',');
                MATCH1(TokenKind_OpenParenthesis, '(');
                MATCH1(TokenKind_CloseParenthesis, ')');
                MATCH1(TokenKind_OpenBrace, '{');
                MATCH1(TokenKind_CloseBrace, '}');

                MATCH1(TokenKind_Plus, '+');
                MATCH1(TokenKind_Asterisk, '*');

#undef MATCH1

            case '/': {
                Lexer_ExpectChar(lexer, '/');
                if (lexer->Current == '/') {
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
                if (lexer->Current == '>') {
                    Lexer_ExpectChar(lexer, '>');
                    return TOKEN(TokenKind_RightArrow);
                }
                return TOKEN(TokenKind_Minus);
            } break;

            case '=': {
                Lexer_ExpectChar(lexer, '=');
                if (lexer->Current == '=') {
                    Lexer_ExpectChar(lexer, '=');
                    return TOKEN(TokenKind_EqualsEquals);
                }
                return TOKEN(TokenKind_Equals);
            } break;

            case '!': {
                Lexer_ExpectChar(lexer, '!');
                if (lexer->Current == '=') {
                    Lexer_ExpectChar(lexer, '=');
                    return TOKEN(TokenKind_BangEquals);
                }
                return TOKEN(TokenKind_Bang);
            } break;

            default: {
                fflush(stdout);
                fprintf(stderr, "%s:%llu:%llu Unexpected '%c'\n", lexer->FilePath, lexer->Line, lexer->Column, lexer->Current);
                Lexer_NextChar(lexer);
                lexer->WasError = true;
                goto Start;
            } break;
        }
    }
#undef TOKEN
}

char Lexer_NextChar(Lexer* lexer) {
    char current = lexer->Current;
    lexer->Position++;
    lexer->Column++;
    if (current == '\n') {
        lexer->Line++;
        lexer->Column = 1;
    }
    lexer->Current = lexer->Position < lexer->SourceLength ? lexer->Source[lexer->Position] : '\0';
    return current;
}

bool Lexer_ExpectChar(Lexer* lexer, char expected) {
    if (lexer->Current != expected) {
        fflush(stdout);
        fprintf(stderr,
                "%s:%llu:%llu Expected '%c', got '%c'\n",
                lexer->FilePath,
                lexer->Line,
                lexer->Column,
                expected,
                lexer->Current);
        Lexer_NextChar(lexer);
        lexer->WasError = true;
        return false;
    } else {
        Lexer_NextChar(lexer);
        return true;
    }
}
