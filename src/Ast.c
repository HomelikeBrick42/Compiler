#include "Ast.h"

#include <stdlib.h>
#include <stdio.h>

ARRAY_IMPL(AstStatement*, AstStatement);
ARRAY_IMPL(AstDeclaration*, AstDeclaration);
ARRAY_IMPL(AstExpression*, AstExpression);
ARRAY_IMPL(AstType*, AstType);
ARRAY_IMPL(AstProcedure*, AstProcedure);

String AstKind_Names[AstKind_Count] = {
#define AST_KIND(name, data)       [AstKind_##name] = String_FromLiteral(#name),
#define AST_KIND_BEGIN(name, data) [AstKind_Begin_##name] = String_FromLiteral(#name "Begin"),
#define AST_KIND_END(name)         [AstKind_End_##name] = String_FromLiteral(#name "End"),
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
};

#define AST_KIND(name, data)
#define AST_KIND_BEGIN(name, data)                                                        \
    bool Ast_Is##name(Ast* ast) {                                                         \
        return ast && ast->Kind > AstKind_Begin_##name && ast->Kind < AstKind_End_##name; \
    }
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

static uint64_t CurrentID = 1;

#define AST_KIND(name, data)                           \
    Ast##name* Ast##name##_Create() {                  \
        Ast##name* ast = calloc(1, sizeof(Ast##name)); \
        ast->Kind      = AstKind_##name;               \
        ast->ID        = CurrentID++;                  \
        return ast;                                    \
    }
#define AST_KIND_BEGIN(name, data)
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

static void PrintIndent(uint64_t indent) {
    for (uint64_t i = 0; i < indent; i++) {
        printf("   ");
    }
}

void Ast_Print(Ast* ast, uint64_t indent) {
    if (!ast) {
        printf("null");
        return;
    }

    printf("(%.*s %llu", String_Fmt(AstKind_Names[ast->Kind]), ast->ID);

    if (Ast_IsStatement(ast)) {
        printf("\n");
        PrintIndent(indent + 1);
        printf("(ParentScope: %llu)", ast->Statement.ParentScope ? ast->Statement.ParentScope->ID : 0);
    } else if (Ast_IsExpression(ast)) {
        printf("\n");
        PrintIndent(indent + 1);
        printf("(ParentStatement: %llu),\n", ast->Expression.ParentStatement ? ast->Expression.ParentStatement->ID : 0);
        PrintIndent(indent + 1);
        printf("(ResolvedType: ");
        Ast_Print(ast->Expression.ResolvedType, indent + 1);
        printf(")");
    } else if (Ast_IsType(ast)) {
        printf("\n");
        PrintIndent(indent + 1);
        printf("(Size: %llu)", ast->Type.Size);
    }

    switch (ast->Kind) {
        case AstKind_Invalid:
        case AstKind_InvalidStatement: {
        } break;

        case AstKind_Scope: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Global: %s),\n", ast->Scope.Global ? "true" : "false");
            PrintIndent(indent + 1);
            printf("(Statements");
            if (ast->Scope.Statements.Length > 0) {
                printf(":\n");
                for (uint64_t i = 0; i < ast->Scope.Statements.Length; i++) {
                    if (i != 0) {
                        printf(",\n");
                    }
                    PrintIndent(indent + 2);
                    Ast_Print(ast->Scope.Statements.Data[i], indent + 2);
                }
            }
            printf(")");
        } break;

        case AstKind_Declaration: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Name: '%.*s'),\n", String_Fmt(ast->Declaration.Name.StringValue));
            PrintIndent(indent + 1);
            printf("(Constant: %s),\n", ast->Declaration.Constant ? "true" : "false");
            PrintIndent(indent + 1);
            printf("(ProcedureParam: %s),\n", ast->Declaration.ParentProcedure ? "true" : "false");
            PrintIndent(indent + 1);
            printf("(Type: ");
            Ast_Print(ast->Declaration.Type, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(ResolvedType: ");
            Ast_Print(ast->Declaration.ResolvedType, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Value: ");
            Ast_Print(ast->Declaration.Value, indent + 1);
            printf(")");
        } break;

        case AstKind_Assignment: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Operand: ");
            Ast_Print(ast->Assignment.Operand, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Value: ");
            Ast_Print(ast->Assignment.Value, indent + 1);
            printf(")");
        } break;

        case AstKind_If: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Condition: ");
            Ast_Print(ast->If.Condition, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(ThenStatement: ");
            Ast_Print(ast->If.ThenStatement, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(ElseStatement: ");
            Ast_Print(ast->If.ElseStatement, indent + 1);
            printf(")");
        } break;

        case AstKind_While: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Condition: ");
            Ast_Print(ast->While.Condition, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Statement: ");
            Ast_Print(ast->While.Statement, indent + 1);
            printf(")");
        } break;

        case AstKind_Return: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Value: ");
            Ast_Print(ast->Return.Value, indent + 1);
            printf(")");
        } break;

        case AstKind_StatementExpression: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Expression:\n");
            PrintIndent(indent + 2);
            Ast_Print(ast->StatementExpression.Expression, indent + 2);
            printf(")");
        } break;

        case AstKind_Semicolon:
        case AstKind_InvalidExpression: {
        } break;

        case AstKind_Unary: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Operator: '%.*s'),\n", String_Fmt(TokenKind_Names[ast->Unary.Operator.Kind]));
            PrintIndent(indent + 1);
            printf("(Operand: ");
            Ast_Print(ast->Unary.Operand, indent + 1);
            printf(")");
        } break;

        case AstKind_Binary: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Operator: '%.*s'),\n", String_Fmt(TokenKind_Names[ast->Binary.Operator.Kind]));
            PrintIndent(indent + 1);
            printf("(Left: ");
            Ast_Print(ast->Binary.Left, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Right: ");
            Ast_Print(ast->Binary.Right, indent + 1);
            printf(")");
        } break;

        case AstKind_Cast: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Type: ");
            Ast_Print(ast->Cast.Type, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Expression: ");
            Ast_Print(ast->Cast.Expression, indent + 1);
            printf(")");
        } break;

        case AstKind_Transmute: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Type: ");
            Ast_Print(ast->Transmute.Type, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Expression: ");
            Ast_Print(ast->Transmute.Expression, indent + 1);
            printf(")");
        } break;

        case AstKind_TypeOf: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Expression: ");
            Ast_Print(ast->TypeOf.Expression, indent + 1);
            printf(")");
        } break;

        case AstKind_SizeOf: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Expression: ");
            Ast_Print(ast->SizeOf.Expression, indent + 1);
            printf(")");
        } break;

        case AstKind_Integer: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Value: %llu)", ast->Integer.Token.IntValue);
        } break;

        case AstKind_Float: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Value: %f)", ast->Float.Token.FloatValue);
        } break;

        case AstKind_String: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Value: \"%.*s\")", String_Fmt(ast->String.Token.StringValue));
        } break;

        case AstKind_Name: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Identifier: '%.*s'),\n", String_Fmt(ast->Name.Token.StringValue));
            PrintIndent(indent + 1);
            printf("(ResolvedDeclaration: %llu)", ast->Name.ResolvedDeclaration ? ast->Name.ResolvedDeclaration->ID : 0);
        } break;

        case AstKind_Procedure: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(CompilerProc: %s),\n", ast->Procedure.CompilerProc ? "true" : "false");
            if (ast->Procedure.CompilerProc) {
                PrintIndent(indent + 1);
                printf("(CompilerProcName: '%.*s'),\n", String_Fmt(ast->Procedure.Data.CompilerProcString.StringValue));
            }
            PrintIndent(indent + 1);
            printf("(Parameters");
            if (ast->Procedure.Parameters.Length > 0) {
                printf(":\n");
                for (uint64_t i = 0; i < ast->Procedure.Parameters.Length; i++) {
                    PrintIndent(indent + 2);
                    Ast_Print(ast->Procedure.Parameters.Data[i], indent + 2);
                    if (i < ast->Procedure.Parameters.Length - 1) {
                        printf(",\n");
                    }
                }
            }
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(ReturnType: ");
            Ast_Print(ast->Procedure.ReturnType, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Body: ");
            Ast_Print(ast->Procedure.Body, indent + 1);
            printf(")");
        } break;

        case AstKind_Call: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Operand: ");
            Ast_Print(ast->Call.Operand, indent + 1);
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(Arguments");
            if (ast->Call.Arguments.Length > 0) {
                printf(":\n");
                for (uint64_t i = 0; i < ast->Call.Arguments.Length; i++) {
                    PrintIndent(indent + 2);
                    Ast_Print(ast->Call.Arguments.Data[i], indent + 2);
                    if (i < ast->Call.Arguments.Length - 1) {
                        printf(",\n");
                    }
                }
            }
            printf(")");
        } break;

        case AstKind_BuitinType: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Name: '%.*s')", String_Fmt(TokenKind_Names[ast->BuitinType.BuiltinToken.Kind]));
        } break;

        case AstKind_InvalidType:
        case AstKind_TypeType: {
        } break;

        case AstKind_TypeInteger: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Signed: %s)", ast->TypeInteger.Signed ? "true" : "false");
        } break;

        case AstKind_TypeFloat:
        case AstKind_TypeString:
        case AstKind_TypeBool:
        case AstKind_TypeVoid: {
        } break;

        case AstKind_TypeProcedure: {
            printf(",\n");
            PrintIndent(indent + 1);
            printf("(Parameters");
            if (ast->TypeProcedure.Parameters.Length > 0) {
                printf(":\n");
                for (uint64_t i = 0; i < ast->TypeProcedure.Parameters.Length; i++) {
                    PrintIndent(indent + 2);
                    Ast_Print(ast->TypeProcedure.Parameters.Data[i], indent + 2);
                    if (i < ast->TypeProcedure.Parameters.Length - 1) {
                        printf(",\n");
                    }
                }
            }
            printf("),\n");
            PrintIndent(indent + 1);
            printf("(ReturnType: ");
            Ast_Print(ast->TypeProcedure.ReturnType, indent + 1);
            printf(")");
        } break;

#include "AstBeginEndSwitchCases.h"
    }

    printf(")");
}
