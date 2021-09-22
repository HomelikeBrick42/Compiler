#include "Resolver.h"

#include <stdio.h>

bool ResolveAst(Ast* ast) {
    switch (ast->Resolution) {
        case Resolution_Resolved: {
            return true;
        } break;

        case Resolution_Resolving: {
            fflush(stdout);
            // TODO: Better error message
            fprintf(stderr, "Recursive dependency found!\n");
            return false;
        } break;

        case Resolution_Unresolved: {
            ast->Resolution = Resolution_Resolving;
        } break;
    }

    switch (ast->Kind) {
        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                if (!ResolveAst(ast->Scope.Statements[i])) {
                    return false;
                }
            }
        } break;

        case AstKind_Declaration: {
            // TODO: Register declared name?

            if (ast->Declaration.Type) {
                if (!ResolveAst(ast->Declaration.Type)) {
                    return false;
                }
            }

            if (ast->Declaration.Value) {
                if (!ResolveAst(ast->Declaration.Value)) {
                    return false;
                }
            }

            // TODO: Check compatible types
        } break;

        case AstKind_Assignment: {
            if (!ResolveAst(ast->Assignment.Operand)) {
                return false;
            }

            if (!ResolveAst(ast->Assignment.Value)) {
                return false;
            }

            // TODO: Check compatible types
        } break;

        case AstKind_If: {
            if (!ResolveAst(ast->If.Condition)) {
                return false;
            }

            // TODO: Check condition is boolean

            if (!ResolveAst(ast->If.ThenScope)) {
                return false;
            }

            if (ast->If.ElseScope) {
                if (!ResolveAst(ast->If.ElseScope)) {
                    return false;
                }
            }
        } break;

        case AstKind_Return: {
            if (!ResolveAst(ast->Return.Value)) {
                return false;
            }

            // TODO: Check compatible with procedure return type
        } break;

        case AstKind_Print: {
            if (!ResolveAst(ast->Return.Value)) {
                return false;
            }

            // TODO: Check printable
        } break;

        case AstKind_Unary: {
            if (!ResolveAst(ast->Unary.Operand)) {
                return false;
            }

            // TODO: Check compatible operator
        } break;

        case AstKind_Binary: {
            if (!ResolveAst(ast->Binary.Left)) {
                return false;
            }

            if (!ResolveAst(ast->Binary.Right)) {
                return false;
            }

            // TODO: Check compatible operator
        } break;

        case AstKind_Integer: {
            // TODO: Do we need to do anything?
        } break;

        case AstKind_Name: {
            // TODO: Find declaration
        } break;

        case AstKind_Procedure: {
            for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                if (!ResolveAst(ast->Procedure.Parameters[i])) {
                    return false;
                }
            }

            if (ast->Procedure.ReturnType) {
                if (!ResolveAst(ast->Procedure.ReturnType)) {
                    return false;
                }
            }

            if (ast->Procedure.Body) {
                if (!ResolveAst(ast->Procedure.Body)) {
                    return false;
                }
            }
        } break;

        case AstKind_Call: {
            if (!ResolveAst(ast->Call.Operand)) {
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                if (!ResolveAst(ast->Call.Arguments[i])) {
                    return false;
                }

                // TODO: Check compatible with argument type
            }
        } break;
    }

    ast->Resolution = Resolution_Resolved;
    return true;
}
