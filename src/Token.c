#include "Token.h"

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

String TokenKind_Names[TokenKind_Count] = {
#define TOKEN_KIND(name, str)  [TokenKind_##name] = String_FromLiteral(str),
#define TOKEN_KIND_BEGIN(name) [TokenKind_Begin_##name] = String_FromLiteral(#name "Begin"),
#define TOKEN_KIND_END(name)   [TokenKind_End_##name] = String_FromLiteral(#name "End"),
    TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END
};

#define TOKEN_KIND(name, str)
#define TOKEN_KIND_BEGIN(name)                                                           \
    bool Token_Is##name(Token token) {                                                   \
        return token.Kind > TokenKind_Begin_##name && token.Kind < TokenKind_End_##name; \
    }
#define TOKEN_KIND_END(name)
TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END

void Token_Error(Token token, const char* message, ...) {
#if defined(_WIN32) || defined(_WIN64)
    fflush(stdout);

    HANDLE console                         = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
    GetConsoleScreenBufferInfo(console, &consoleInfo);

    SetConsoleTextAttribute(console, FOREGROUND_RED | COMMON_LVB_UNDERSCORE);
    fprintf(stderr, "%.*s:%llu:%llu", String_Fmt(token.FilePath), token.Line, token.Column);
    SetConsoleTextAttribute(console, FOREGROUND_RED);

    va_list args1;
    va_start(args1, message);
    uint64_t length = vsnprintf(NULL, 0, message, args1);
    va_end(args1);

    char* buffer = malloc(length + 1);

    va_list args2;
    va_start(args2, message);
    vsprintf(buffer, message, args2);
    va_end(args2);

    fprintf(stderr, ": %s\n", buffer);

    free(buffer);

    if (token.Length == 0) {
        return;
    }

    fprintf(stderr, "\n");

    StringArray lines = String_GetLines(token.Source);

    SetConsoleTextAttribute(console, consoleInfo.wAttributes);
    fprintf(stderr, "> ");
    SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
    if (token.Line > 1) {
        for (uint64_t i = 0; i < lines.Data[token.Line - 2].Length; i++) {
            uint8_t chr = lines.Data[token.Line - 2].Data[i];
            if (chr == '\t') {
                fprintf(stderr, "    ");
            } else {
                fprintf(stderr, "%c", chr);
            }
        }
    }
    SetConsoleTextAttribute(console, consoleInfo.wAttributes);
    fprintf(stderr, "\n");

    fprintf(stderr, "> ");
    SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
    for (uint64_t i = 0; i < token.Column - 1; i++) {
        uint8_t chr = lines.Data[token.Line - 1].Data[i];
        if (chr == '\t') {
            fprintf(stderr, "    ");
        } else {
            fprintf(stderr, "%c", chr);
        }
    }
    SetConsoleTextAttribute(console, FOREGROUND_RED);
    for (uint64_t i = token.Column - 1; i < token.Column + token.Length - 1; i++) {
        uint8_t chr = lines.Data[token.Line - 1].Data[i];
        if (chr == '\t') {
            fprintf(stderr, "    ");
        } else {
            fprintf(stderr, "%c", chr);
        }
    }
    SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
    for (uint64_t i = token.Column + token.Length - 1; i < lines.Data[token.Line - 1].Length; i++) {
        uint8_t chr = lines.Data[token.Line - 1].Data[i];
        if (chr == '\t') {
            fprintf(stderr, "    ");
        } else {
            fprintf(stderr, "%c", chr);
        }
    }
    SetConsoleTextAttribute(console, consoleInfo.wAttributes);
    fprintf(stderr, "\n");

    SetConsoleTextAttribute(console, consoleInfo.wAttributes);
    for (uint64_t i = 0; i < token.Column + token.Length / 2 + token.Length % 2; i++) {
        uint8_t chr = lines.Data[token.Line - 1].Data[i];
        if (chr == '\t') {
            fprintf(stderr, "----");
        } else {
            fprintf(stderr, "-");
        }
    }
    fprintf(stderr, "^\n");
    SetConsoleTextAttribute(console, consoleInfo.wAttributes);

    fprintf(stderr, "> ");
    SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
    if (token.Line < lines.Length) {
        for (uint64_t i = 0; i < lines.Data[token.Line].Length; i++) {
            uint8_t chr = lines.Data[token.Line].Data[i];
            if (chr == '\t') {
                fprintf(stderr, "    ");
            } else {
                fprintf(stderr, "%c", chr);
            }
        }
    }
    SetConsoleTextAttribute(console, consoleInfo.wAttributes);
    fprintf(stderr, "\n\n");

    StringArray_Destroy(&lines);
#else
    #error "Unsupported platform"
#endif
}
