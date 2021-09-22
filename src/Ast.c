#include "Ast.h"

#include <stdio.h>

const char* AstKind_Names[AstKind_Count] = {
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) [AstKind_##name] = #name,
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN
};

static void Indent(uint64_t indent) {
    for (uint64_t j = 0; j < indent; j++) {
        putchar(' ');
        putchar(' ');
    }
}

void PrintAst(Ast* ast, uint64_t indent) {
    printf("%s", AstKind_Names[ast->Kind]);
    switch (ast->Kind) {
        case AstKind_Scope: {
            putchar('\n');
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                Indent(indent + 1);
                PrintAst(ast->Scope.Statements[i], indent + 1);
                if (i < ast->Scope.StatementCount - 1) {
                    putchar('\n');
                }
            }
        } break;

        case AstKind_Declaration: {
            putchar('\n');
            Indent(indent + 1);
            printf("Constant: %s\n", ast->Declaration.Constant ? "true" : "false");
            Token nameToken = ast->Declaration.Name;
            Indent(indent + 1);
            printf("Name: '%.*s'", (uint32_t)nameToken.Length, &nameToken.Source[nameToken.Position]);

            if (ast->Declaration.Type) {
                putchar('\n');
                Indent(indent + 1);
                printf("Type: ");
                PrintAst(ast->Declaration.Type, indent + 1);
            }

            if (ast->Declaration.Value) {
                putchar('\n');
                Indent(indent + 1);
                printf("Value: ");
                PrintAst(ast->Declaration.Value, indent + 1);
            }
        } break;

        case AstKind_Assignment: {
            putchar('\n');
            Indent(indent + 1);
            printf("Operand: ");
            PrintAst(ast->Assignment.Operand, indent + 1);
            putchar('\n');
            Indent(indent + 1);
            printf("Value: ");
            PrintAst(ast->Assignment.Value, indent + 1);
        } break;

        case AstKind_If: {
            putchar('\n');
            Indent(indent + 1);
            printf("Condition: ");
            PrintAst(ast->If.Condition, indent + 1);
            putchar('\n');
            Indent(indent + 1);
            printf("Then: ");
            PrintAst(ast->If.ThenScope, indent + 1);
            if (ast->If.ThenScope) {
                putchar('\n');
                Indent(indent + 1);
                printf("Else: ");
                PrintAst(ast->If.ElseScope, indent + 1);
            }
        } break;

        case AstKind_Return: {
            putchar('\n');
            Indent(indent + 1);
            printf("Value: ");
            PrintAst(ast->Return.Value, indent + 1);
        } break;

        case AstKind_Print: {
            putchar('\n');
            Indent(indent + 1);
            printf("Value: ");
            PrintAst(ast->Print.Value, indent + 1);
        } break;

        case AstKind_Unary: {
            putchar('\n');
            Indent(indent + 1);
            printf("Operator: '%s'\n", TokenKind_Names[ast->Unary.Operator.Kind]);
            Indent(indent + 1);
            printf("Operand: ");
            PrintAst(ast->Unary.Operand, indent + 1);
        } break;

        case AstKind_Binary: {
            putchar('\n');
            Indent(indent + 1);
            printf("Left: ");
            PrintAst(ast->Binary.Left, indent + 1);
            putchar('\n');
            Indent(indent + 1);
            printf("Operator: '%s'\n", TokenKind_Names[ast->Binary.Operator.Kind]);
            Indent(indent + 1);
            printf("Right: ");
            PrintAst(ast->Binary.Right, indent + 1);
        } break;

        case AstKind_Name: {
            Token nameToken = ast->Name.Token;
            printf(": '%.*s'", (uint32_t)nameToken.Length, &nameToken.Source[nameToken.Position]);
        } break;

        case AstKind_Integer: {
            Token integerToken = ast->Integer.Token;
            printf(": %.*s", (uint32_t)integerToken.Length, &integerToken.Source[integerToken.Position]);
        } break;

        case AstKind_Procedure: {
            if (ast->Procedure.ParameterCount > 0) {
                putchar('\n');
                Indent(indent + 1);
                printf("Parameters:");
                for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                    putchar('\n');
                    Indent(indent + 2);
                    PrintAst(ast->Procedure.Parameters[i], indent + 2);
                }
            }

            if (ast->Procedure.ReturnType) {
                putchar('\n');
                Indent(indent + 1);
                printf("Return Type: ");
                PrintAst(ast->Procedure.ReturnType, indent + 1);
            }

            if (ast->Procedure.Body) {
                putchar('\n');
                Indent(indent + 1);
                printf("Body: ");
                PrintAst(ast->Procedure.Body, indent + 1);
            }
        } break;

        case AstKind_Call: {
            if (ast->Call.ArgumentCount > 0) {
                putchar('\n');
                Indent(indent + 1);
                printf("Arguments:");
                for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                    putchar('\n');
                    Indent(indent + 2);
                    PrintAst(ast->Call.Arguments[i], indent + 2);
                }
            }
        } break;

        case AstKind_TypeExpression:
            break;
    }
}
