#include "Resolver.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char* Type_TypeName             = "type";
static Type Type_Type                        = {};
static AstTypeExpression Type_TypeExpression = {};
static AstDeclaration Type_TypeDeclaration   = {};

static const char* Type_IntegerSignedName             = "int";
static Type Type_IntegerSigned                        = {};
static AstTypeExpression Type_IntegerSignedExpression = {};
static AstDeclaration Type_IntegerSignedDeclaration   = {};

static const char* Type_BoolName             = "bool";
static Type Type_Bool                        = {};
static AstTypeExpression Type_BoolExpression = {};
static AstDeclaration Type_BoolDeclaration   = {};

static struct {
    TokenKind Operator;
    Type* Operand;
    Type* ResultType;
} UnaryOperators[] = {};

static struct {
    TokenKind Operator;
    Type* Left;
    Type* Right;
    Type* ResultType;
} BinaryOperators[] = { {
                            .Operator   = TokenKind_EqualsEquals,
                            .Left       = &Type_IntegerSigned,
                            .Right      = &Type_IntegerSigned,
                            .ResultType = &Type_Bool,
                        },
                        {
                            .Operator   = TokenKind_Minus,
                            .Left       = &Type_IntegerSigned,
                            .Right      = &Type_IntegerSigned,
                            .ResultType = &Type_IntegerSigned,
                        },
                        {
                            .Operator   = TokenKind_Asterisk,
                            .Left       = &Type_IntegerSigned,
                            .Right      = &Type_IntegerSigned,
                            .ResultType = &Type_IntegerSigned,
                        } };

void InitTypes() {
    Type_Type.Kind = TypeKind_Type;
    Type_Type.Size = 8;

    Type_TypeExpression.Kind                = AstKind_TypeExpression;
    Type_TypeExpression.Resolution          = Resolution_Resolved;
    Type_TypeExpression.ResolvedType        = &Type_Type;
    Type_TypeExpression.TypeExpression.Type = &Type_Type;

    Type_TypeDeclaration.Kind             = AstKind_Declaration;
    Type_TypeDeclaration.Resolution       = Resolution_Resolved;
    Type_TypeDeclaration.Declaration.Name = (Token){
        .Kind     = TokenKind_Identifier,
        .FilePath = "Builtin",
        .Source   = Type_TypeName,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = strlen(Type_TypeName),
    };
    Type_TypeDeclaration.Declaration.ResolvedType = &Type_Type;
    Type_TypeDeclaration.Declaration.Value        = &Type_TypeExpression;

    Type_IntegerSigned.Kind           = TypeKind_Integer;
    Type_IntegerSigned.Size           = 8;
    Type_IntegerSigned.Integer.Signed = true;

    Type_IntegerSignedExpression.Kind                = AstKind_TypeExpression;
    Type_IntegerSignedExpression.Resolution          = Resolution_Resolved;
    Type_IntegerSignedExpression.ResolvedType        = &Type_Type;
    Type_IntegerSignedExpression.TypeExpression.Type = &Type_IntegerSigned;

    Type_IntegerSignedDeclaration.Kind             = AstKind_Declaration;
    Type_IntegerSignedDeclaration.Resolution       = Resolution_Resolved;
    Type_IntegerSignedDeclaration.Declaration.Name = (Token){
        .Kind     = TokenKind_Identifier,
        .FilePath = "Builtin",
        .Source   = Type_IntegerSignedName,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = strlen(Type_IntegerSignedName),
    };
    Type_IntegerSignedDeclaration.Declaration.ResolvedType = &Type_Type;
    Type_IntegerSignedDeclaration.Declaration.Value        = &Type_IntegerSignedExpression;

    Type_Bool.Kind           = TypeKind_Bool;
    Type_Bool.Size           = 1;
    Type_Bool.Integer.Signed = true;

    Type_BoolExpression.Kind                = AstKind_TypeExpression;
    Type_BoolExpression.Resolution          = Resolution_Resolved;
    Type_BoolExpression.ResolvedType        = &Type_Type;
    Type_BoolExpression.TypeExpression.Type = &Type_Bool;

    Type_BoolDeclaration.Kind             = AstKind_Declaration;
    Type_BoolDeclaration.Resolution       = Resolution_Resolved;
    Type_BoolDeclaration.Declaration.Name = (Token){
        .Kind     = TokenKind_Identifier,
        .FilePath = "Builtin",
        .Source   = Type_BoolName,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = strlen(Type_BoolName),
    };
    Type_BoolDeclaration.Declaration.ResolvedType = &Type_Type;
    Type_BoolDeclaration.Declaration.Value        = &Type_BoolExpression;
}

static Type* ExpressionToType(AstExpression* expression) {
    if (expression->Resolution != Resolution_Resolved) {
        fflush(stdout);
        fprintf(stderr, "Internal Error: ExpressionToType expression is not resolved\n");
        return NULL;
    }

    switch (expression->Kind) {
        case AstKind_Procedure: {
            AstProcedureData* procedure = &expression->Procedure;
            if (procedure->Body) {
                fflush(stdout);
                fprintf(stderr, "Procedure type cannot have body\n");
                return NULL;
            }

            TypeProcedure* type = calloc(1, sizeof(TypeProcedure));
            type->Kind          = TypeKind_Procedure;
            type->Size          = 64;

            if (procedure->ParameterCount > 0) {
                type->Procedure.ParameterTypeCount = procedure->ParameterCount;
                type->Procedure.ParameterTypes     = calloc(1, type->Procedure.ParameterTypeCount * sizeof(Type));
                for (uint64_t i = 0; i < procedure->ParameterCount; i++) {
                    type->Procedure.ParameterTypes[i] = procedure->Parameters[i]->Declaration.ResolvedType;
                }
            }

            if (procedure->ResolvedReturnType) {
                type->Procedure.ReturnType = procedure->ResolvedReturnType;
            }

            return type;
        } break;

        case AstKind_Name: {
            if (expression->Name.ResolvedDeclaration->Declaration.ResolvedType->Kind == TypeKind_Type) {
                return ExpressionToType(expression->Name.ResolvedDeclaration->Declaration.Value);
            } else {
                Token nameToken = expression->Name.Token;
                fflush(stdout);
                fprintf(stderr, "Name '%.*s' is not a type\n", (uint32_t)nameToken.Length, &nameToken.Source[nameToken.Position]);
                return NULL;
            }
        } break;

        case AstKind_TypeExpression: {
            return expression->TypeExpression.Type;
        } break;

        default: {
            fflush(stdout);
            fprintf(stderr, "Expression '%s' is not convertable a type\n", AstKind_Names[expression->Kind]);
            return NULL;
        } break;
    }
}

static bool TypesEqual(Type* a, Type* b) {
    if (a == b) {
        return true;
    }

    if (a->Kind != b->Kind) {
        return false;
    }

    switch (a->Kind) {
        case TypeKind_Type:
        case TypeKind_Float:
        case TypeKind_Bool:
            return true;

        case TypeKind_Integer:
            return a->Integer.Signed == b->Integer.Signed;

        case TypeKind_Procedure: {
            if (!TypesEqual(a->Procedure.ReturnType, b->Procedure.ReturnType)) {
                return false;
            }

            if (a->Procedure.ParameterTypeCount != b->Procedure.ParameterTypeCount) {
                return false;
            }

            for (uint64_t i = 0; i < a->Procedure.ParameterTypeCount; i++) {
                if (!TypesEqual(a->Procedure.ParameterTypes[i], b->Procedure.ParameterTypes[i])) {
                    return false;
                }
            }

            return true;
        } break;
    }
}

static bool AssertTypesEqual(Type* a, Type* b) {
    if (!TypesEqual(a, b)) {
        fflush(stdout);
        // TODO: Improve message
        fprintf(stderr, "Types '%s' and '%s' are not compatible\n", TypeKind_Names[a->Kind], TypeKind_Names[b->Kind]);
        return false;
    }
    return true;
}

static AstScope** PendingProcedureBodyScopes    = NULL;
static uint64_t PendingProcedureBodyScopeCount = 0;

bool ResolveAst(Ast* ast, AstScope* parentScope) {
    switch (ast->Resolution) {
        case Resolution_Resolved: {
            return true;
        } break;

        case Resolution_Resolving: {
            fflush(stdout);
            // TODO: Better error message
            fprintf(stderr, "Recursive dependency found\n");
            exit(EXIT_FAILURE);
            return false;
        } break;

        case Resolution_Unresolved: {
            ast->Resolution = Resolution_Resolving;
        } break;
    }

    switch (ast->Kind) {
        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                if (!ResolveAst(ast->Scope.Statements[i], ast)) {
                    return false;
                }
            }

            for (int64_t i = (int64_t)PendingProcedureBodyScopeCount - 1; i >= 0; i--) {
                PendingProcedureBodyScopeCount--;
                if (!ResolveAst(PendingProcedureBodyScopes[i], ast)) {
                    return false;
                }
            }
        } break;

        case AstKind_Declaration: {
            if (ast->Declaration.Type) {
                if (!ResolveAst(ast->Declaration.Type, parentScope)) {
                    return false;
                }

                ast->Declaration.ResolvedType = ExpressionToType(ast->Declaration.Type);
                if (!ast->Declaration.ResolvedType) {
                    return false;
                }
            }

            if (ast->Declaration.Value) {
                if (!ResolveAst(ast->Declaration.Value, parentScope)) {
                    return false;
                }
            }

            if (!ast->Declaration.ResolvedType) {
                ast->Declaration.ResolvedType = ast->Declaration.Value->ResolvedType;
            } else if (ast->Declaration.Value) {
                if (!AssertTypesEqual(ast->Declaration.ResolvedType, ast->Declaration.Value->ResolvedType)) {
                    return false;
                }
            }

            if (!ast->Declaration.IsProcedureParam) {
                parentScope->Scope.Declarations =
                    realloc(parentScope->Scope.Declarations, (parentScope->Scope.DeclarationCount + 1) * sizeof(AstDeclaration*));
                parentScope->Scope.Declarations[parentScope->Scope.DeclarationCount] = ast;
                parentScope->Scope.DeclarationCount++;
            }
        } break;

        case AstKind_Assignment: {
            if (!ResolveAst(ast->Assignment.Operand, parentScope)) {
                return false;
            }

            if (!ResolveAst(ast->Assignment.Value, parentScope)) {
                return false;
            }

            if (!AssertTypesEqual(ast->Assignment.Operand->ResolvedType, ast->Assignment.Value->ResolvedType)) {
                return false;
            }
        } break;

        case AstKind_If: {
            if (!ResolveAst(ast->If.Condition, parentScope)) {
                return false;
            }

            if (ast->If.Condition->ResolvedType->Kind != TypeKind_Bool) {
                fflush(stdout);
                fprintf(stderr, "If statement condition must be a boolean\n");
                return false;
            }

            if (!ResolveAst(ast->If.ThenScope, parentScope)) {
                return false;
            }

            if (ast->If.ElseScope) {
                if (!ResolveAst(ast->If.ElseScope, parentScope)) {
                    return false;
                }
            }
        } break;

        case AstKind_Return: {
            if (!ResolveAst(ast->Return.Value, parentScope)) {
                return false;
            }

            // TODO: Check compatible with procedure return type
        } break;

        case AstKind_Print: {
            if (!ResolveAst(ast->Return.Value, parentScope)) {
                return false;
            }

            if (ast->Return.Value->ResolvedType->Kind != TypeKind_Integer) {
                fflush(stdout);
                fprintf(stderr, "Type '%s' is not printable\n", TypeKind_Names[ast->Return.Value->ResolvedType->Kind]);
                return false;
            }
        } break;

        case AstKind_Unary: {
            if (!ResolveAst(ast->Unary.Operand, parentScope)) {
                return false;
            }

            for (uint64_t i = 0; i < sizeof(UnaryOperators) / sizeof(UnaryOperators[0]); i++) {
                if (UnaryOperators[i].Operator != ast->Unary.Operator.Kind)
                    continue;

                if (!TypesEqual(UnaryOperators[i].Operand, ast->Unary.Operand->ResolvedType))
                    continue;

                ast->ResolvedType = UnaryOperators[i].ResultType;
                break;
            }

            if (!ast->ResolvedType) {
                fflush(stdout);
                fprintf(stderr,
                        "Unable to find unary operator '%s' for type '%s'\n",
                        TokenKind_Names[ast->Unary.Operator.Kind],
                        TypeKind_Names[ast->Unary.Operand->ResolvedType->Kind]);
                return false;
            }
        } break;

        case AstKind_Binary: {
            if (!ResolveAst(ast->Binary.Left, parentScope)) {
                return false;
            }

            if (!ResolveAst(ast->Binary.Right, parentScope)) {
                return false;
            }

            for (uint64_t i = 0; i < sizeof(BinaryOperators) / sizeof(BinaryOperators[0]); i++) {
                if (BinaryOperators[i].Operator != ast->Binary.Operator.Kind)
                    continue;

                if (!TypesEqual(BinaryOperators[i].Left, ast->Binary.Left->ResolvedType))
                    continue;

                if (!TypesEqual(BinaryOperators[i].Right, ast->Binary.Right->ResolvedType))
                    continue;

                ast->ResolvedType = BinaryOperators[i].ResultType;
                break;
            }

            if (!ast->ResolvedType) {
                fflush(stdout);
                fprintf(stderr,
                        "Unable to find binary operator '%s' for types '%s' and '%s'\n",
                        TokenKind_Names[ast->Binary.Operator.Kind],
                        TypeKind_Names[ast->Binary.Left->ResolvedType->Kind],
                        TypeKind_Names[ast->Binary.Right->ResolvedType->Kind]);
                return false;
            }
        } break;

        case AstKind_Integer: {
            ast->ResolvedType = &Type_IntegerSigned;
        } break;

        case AstKind_Name: {
            Token nameToken = ast->Name.Token;

            AstScope* scope = parentScope;
            while (scope) {
                for (int64_t i = (int64_t)scope->Scope.DeclarationCount - 1; i >= 0; i--) {
                    AstDeclaration* declaration = scope->Scope.Declarations[i];
                    if (nameToken.Length == declaration->Declaration.Name.Length) {
                        if (memcmp(&nameToken.Source[nameToken.Position],
                                   &declaration->Declaration.Name.Source[declaration->Declaration.Name.Position],
                                   nameToken.Length) == 0) {
                            ast->Name.ResolvedDeclaration = declaration;
                            goto Exit;
                        }
                    }
                }

                for (uint64_t i = 0; i < scope->Scope.StatementCount; i++) {
                    AstStatement* statement = scope->Scope.Statements[i];
                    if (statement->Kind != AstKind_Declaration)
                        continue;

                    AstDeclaration* declaration = statement;

                    if (!declaration->Declaration.Constant)
                        continue;

                    if (nameToken.Length == declaration->Declaration.Name.Length) {
                        if (memcmp(&nameToken.Source[nameToken.Position],
                                   &declaration->Declaration.Name.Source[declaration->Declaration.Name.Position],
                                   nameToken.Length) == 0) {
                            if (!ast->Name.ResolvedDeclaration) {
                                ResolveAst(declaration, scope);
                                ast->Name.ResolvedDeclaration = declaration;
                            } else {
                                fflush(stdout);
                                fprintf(stderr,
                                        "Ambiguous name because there are multiple constant declarations in the same scope\n");
                                return false;
                            }
                        }
                    }
                }

                scope = scope->Scope.ParentScope;
            }
Exit:
            if (!ast->Name.ResolvedDeclaration) {
                if (nameToken.Length == strlen(Type_IntegerSignedName) &&
                    strncmp(&nameToken.Source[nameToken.Position], Type_IntegerSignedName, strlen(Type_IntegerSignedName)) == 0) {
                    ast->Name.ResolvedDeclaration = &Type_IntegerSignedDeclaration;
                }
            }

            if (!ast->Name.ResolvedDeclaration) {
                fflush(stdout);
                fprintf(stderr,
                        "%s:%llu:%llu Unable to find name '%.*s'\n",
                        nameToken.FilePath,
                        nameToken.Line,
                        nameToken.Column,
                        (uint32_t)nameToken.Length,
                        &nameToken.Source[nameToken.Position]);
                return false;
            }

            ast->ResolvedType = ast->Name.ResolvedDeclaration->Declaration.ResolvedType;
        } break;

        case AstKind_Procedure: {
            for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                if (!ResolveAst(ast->Procedure.Parameters[i], parentScope)) {
                    return false;
                }

                if (ast->Procedure.Body) {
                    assert(ast->Procedure.Parameters[i]->Declaration.IsProcedureParam);
                    ast->Procedure.Body->Scope.Declarations =
                        realloc(ast->Procedure.Body->Scope.Declarations,
                                (ast->Procedure.Body->Scope.DeclarationCount + 1) * sizeof(AstDeclaration*));
                    ast->Procedure.Body->Scope.Declarations[ast->Procedure.Body->Scope.DeclarationCount] =
                        ast->Procedure.Parameters[i];
                    ast->Procedure.Body->Scope.DeclarationCount++;
                }
            }

            if (ast->Procedure.ReturnType) {
                if (!ResolveAst(ast->Procedure.ReturnType, parentScope)) {
                    return false;
                }

                ast->Procedure.ResolvedReturnType = ExpressionToType(ast->Procedure.ReturnType);
                if (!ast->Procedure.ResolvedReturnType) {
                    return false;
                }
            }

            if (ast->Procedure.Body) {
                PendingProcedureBodyScopes = realloc(PendingProcedureBodyScopes, (PendingProcedureBodyScopeCount + 1) * sizeof(AstScope*));
                PendingProcedureBodyScopes[PendingProcedureBodyScopeCount] = ast->Procedure.Body;
                PendingProcedureBodyScopeCount++;

                TypeProcedure* type = calloc(1, sizeof(TypeProcedure));
                type->Kind          = TypeKind_Procedure;
                type->Size          = 64;

                if (ast->Procedure.ParameterCount > 0) {
                    type->Procedure.ParameterTypeCount = ast->Procedure.ParameterCount;
                    type->Procedure.ParameterTypes     = calloc(1, type->Procedure.ParameterTypeCount * sizeof(Type));
                    for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                        type->Procedure.ParameterTypes[i] = ast->Procedure.Parameters[i]->Declaration.ResolvedType;
                    }
                }

                if (ast->Procedure.ResolvedReturnType) {
                    type->Procedure.ReturnType = ast->Procedure.ResolvedReturnType;
                }

                ast->ResolvedType = type;
            } else {
                ast->Resolution     = Resolution_Resolved;
                TypeProcedure* type = ExpressionToType(ast);
                if (!type) {
                    return false;
                }

                memset(ast, 0, sizeof(Ast));
                ast->Kind                = AstKind_TypeExpression;
                ast->ResolvedType        = &Type_Type;
                ast->TypeExpression.Type = type;
            }
        } break;

        case AstKind_Call: {
            if (!ResolveAst(ast->Call.Operand, parentScope)) {
                return false;
            }

            if (ast->Call.Operand->ResolvedType->Kind != TypeKind_Procedure) {
                fflush(stdout);
                fprintf(stderr, "Cannot call a '%s'\n", TypeKind_Names[ast->Call.Operand->ResolvedType->Kind]);
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                if (!ResolveAst(ast->Call.Arguments[i], parentScope)) {
                    return false;
                }
            }

            TypeProcedure* type = ast->Call.Operand->ResolvedType;

            if (ast->Call.ArgumentCount != type->Procedure.ParameterTypeCount) {
                fflush(stdout);
                fprintf(stderr, "Wrong number of arguments in procedure call\n");
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                if (!AssertTypesEqual(ast->Call.Arguments[i]->ResolvedType, type->Procedure.ParameterTypes[i])) {
                    return false;
                }
            }

            ast->ResolvedType = type->Procedure.ReturnType;
        } break;

        case AstKind_TypeExpression: {
        } break;
    }

    ast->Resolution = Resolution_Resolved;
    return true;
}
