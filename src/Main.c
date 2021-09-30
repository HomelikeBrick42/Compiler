#include "Array.h"
#include "Strings.h"
#include "Lexer.h"
#include "Parser.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

int main(int argc, char** argv) {
    if (argc != 2) {
#if defined(_WIN32) || defined(_WIN64)
        HANDLE console                         = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
        GetConsoleScreenBufferInfo(console, &consoleInfo);
        SetConsoleTextAttribute(console, FOREGROUND_RED);
#endif
        fflush(stdout);
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
#if defined(_WIN32) || defined(_WIN64)
        SetConsoleTextAttribute(console, consoleInfo.wAttributes);
#endif
        return EXIT_FAILURE;
    }

    String filePath = String_GetFullPath(String_FromCString(argv[1]));
    String source;
    if (!String_LoadFile(&source, filePath)) {
#if defined(_WIN32) || defined(_WIN64)
        HANDLE console                         = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
        GetConsoleScreenBufferInfo(console, &consoleInfo);
        SetConsoleTextAttribute(console, FOREGROUND_RED);
#endif
        fflush(stdout);
        fprintf(stderr, "Unable to open file '%.*s'\n", String_Fmt(filePath));
        free(filePath.Data);
#if defined(_WIN32) || defined(_WIN64)
        SetConsoleTextAttribute(console, consoleInfo.wAttributes);
#endif
        return EXIT_FAILURE;
    }

#if 0
    Lexer lexer = Lexer_Create(filePath, source);

    while (true) {
        Token token = Lexer_NextToken(&lexer);

        String name = TokenKind_Names[token.Kind];
        printf("TokenKind: '%.*s'\n", String_Fmt(name));

        if (token.Kind == TokenKind_EndOfFile) {
            break;
        }
    }
#endif

    Parser parser         = Parser_Create(filePath, source);
    AstScope* globalScope = Parser_Parse(&parser);

    if (Parser_WasError(&parser)) {
        return EXIT_FAILURE;
    }

    Ast_Print(globalScope, 0);
    printf("\n");

    free(source.Data);
    free(filePath.Data);
    return EXIT_SUCCESS;
}
