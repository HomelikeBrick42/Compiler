#include "Resolver.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct {
    const char* Name;
    AstDeclaration* Declaration;
} BuiltinNames[] = {
    {
        .Name        = "type",
        .Declaration = &Type_TypeDeclaration,
    },
    {
        .Name        = "int",
        .Declaration = &Type_IntegerSignedDeclaration,
    },
    {
        .Name        = "uint",
        .Declaration = &Type_IntegerUnsignedDeclaration,
    },
    {
        .Name        = "bool",
        .Declaration = &Type_BoolDeclaration,
    },
    {
        .Name        = "void",
        .Declaration = &Type_VoidDeclaration,
    },
};

static struct {
    TokenKind Operator;
    Type* Operand;
    Type* ResultType;
} UnaryOperators[] = {
    {
        .Operator   = TokenKind_Plus,
        .Operand    = &Type_IntegerSigned,
        .ResultType = &Type_IntegerSigned,
    },
    {
        .Operator   = TokenKind_Minus,
        .Operand    = &Type_IntegerSigned,
        .ResultType = &Type_IntegerSigned,
    },
    {
        .Operator   = TokenKind_Bang,
        .Operand    = &Type_Bool,
        .ResultType = &Type_Bool,
    },
};

static struct {
    TokenKind Operator;
    Type* Left;
    Type* Right;
    Type* ResultType;
} BinaryOperators[] = {
    {
        .Operator   = TokenKind_Plus,
        .Left       = &Type_IntegerSigned,
        .Right      = &Type_IntegerSigned,
        .ResultType = &Type_IntegerSigned,
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
    },
    {
        .Operator   = TokenKind_Slash,
        .Left       = &Type_IntegerSigned,
        .Right      = &Type_IntegerSigned,
        .ResultType = &Type_IntegerSigned,
    },
    {
        .Operator   = TokenKind_Plus,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_IntegerUnsigned,
    },
    {
        .Operator   = TokenKind_Minus,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_IntegerUnsigned,
    },
    {
        .Operator   = TokenKind_Asterisk,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_IntegerUnsigned,
    },
    {
        .Operator   = TokenKind_Slash,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_IntegerUnsigned,
    },
    {
        .Operator   = TokenKind_BangEquals,
        .Left       = &Type_Type,
        .Right      = &Type_Type,
        .ResultType = &Type_Type,
    },
    {
        .Operator   = TokenKind_EqualsEquals,
        .Left       = &Type_Type,
        .Right      = &Type_Type,
        .ResultType = &Type_Type,
    },
    {
        .Operator   = TokenKind_BangEquals,
        .Left       = &Type_IntegerSigned,
        .Right      = &Type_IntegerSigned,
        .ResultType = &Type_Bool,
    },
    {
        .Operator   = TokenKind_EqualsEquals,
        .Left       = &Type_IntegerSigned,
        .Right      = &Type_IntegerSigned,
        .ResultType = &Type_Bool,
    },
    {
        .Operator   = TokenKind_BangEquals,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_Bool,
    },
    {
        .Operator   = TokenKind_EqualsEquals,
        .Left       = &Type_IntegerUnsigned,
        .Right      = &Type_IntegerUnsigned,
        .ResultType = &Type_Bool,
    },
    {
        .Operator   = TokenKind_BangEquals,
        .Left       = &Type_Bool,
        .Right      = &Type_Bool,
        .ResultType = &Type_Bool,
    },
    {
        .Operator   = TokenKind_EqualsEquals,
        .Left       = &Type_Bool,
        .Right      = &Type_Bool,
        .ResultType = &Type_Bool,
    },
};

struct {
    Type* From;
    Type* To;
} CastableTypes[] = {
    {
        .From = &Type_IntegerSigned,
        .To   = &Type_IntegerUnsigned,
    },
    {
        .From = &Type_IntegerUnsigned,
        .To   = &Type_IntegerSigned,
    },
};

static Type* ExpressionToType(AstExpression* expression) {
    if (expression->Resolution != Resolution_Resolved) {
        fflush(stdout);
        fprintf(stderr, "Internal Error: ExpressionToType expression is not resolved\n");
        return NULL;
    }

    switch (expression->Kind) {
        case AstKind_Struct: {
            AstStructData* structt = &expression->Struct;

            TypeStruct* structType = calloc(1, sizeof(TypeStruct));
            structType->Kind       = TypeKind_Struct;

            if (structt->MemberCount > 0) {
                structType->Struct.MemberCount = structt->MemberCount;
                structType->Struct.MemberTypes = calloc(structType->Struct.MemberCount, sizeof(Type*));
                structType->Struct.MemberNames = calloc(structType->Struct.MemberCount, sizeof(Token));
                for (uint64_t i = 0; i < structt->MemberCount; i++) {
                    structType->Struct.MemberTypes[i] = structt->Members[i]->Declaration.ResolvedType;
                    structType->Struct.MemberNames[i] = structt->Members[i]->Declaration.Name;
                    structType->Size += structType->Struct.MemberTypes[i]->Size;
                }
            }

            return structType;
        } break;

        case AstKind_Procedure: {
            AstProcedureData* procedure = &expression->Procedure;
            if (procedure->Body) {
                fflush(stdout);
                fprintf(stderr, "Procedure type cannot have body\n");
                return NULL;
            }

            TypeProcedure* type = calloc(1, sizeof(TypeProcedure));
            type->Kind          = TypeKind_Procedure;
            type->Size          = 8;

            if (procedure->ParameterCount > 0) {
                type->Procedure.ParameterTypeCount = procedure->ParameterCount;
                type->Procedure.ParameterTypes     = calloc(type->Procedure.ParameterTypeCount, sizeof(Type*));
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
                if (expression->Name.ResolvedDeclaration->Declaration.Constant) {
                    return ExpressionToType(expression->Name.ResolvedDeclaration->Declaration.Value);
                } else {
                    Token nameToken = expression->Name.Token;
                    fflush(stdout);
                    fprintf(stderr,
                            "Name '%.*s' is not a constant\n",
                            (uint32_t)nameToken.Length,
                            &nameToken.Source[nameToken.Position]);
                    return NULL;
                }
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

static bool AssertTypesEqual(Type* a, Type* b) {
    if (!TypesEqual(a, b)) {
        fflush(stdout);
        // TODO: Improve message
        fprintf(stderr, "Types '%s' and '%s' are not compatible\n", TypeKind_Names[a->Kind], TypeKind_Names[b->Kind]);
        return false;
    }
    return true;
}

static bool BodyReturns(Ast* ast) {
    switch (ast->Kind) {
        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                if (BodyReturns(ast->Scope.Statements[i])) {
                    return true;
                }
            }
            return false;
        } break;

        case AstKind_If: {
            bool returns = BodyReturns(ast->If.ThenStatement);
            if (returns && ast->If.ElseStatement) {
                returns = BodyReturns(ast->If.ElseStatement);
                return returns;
            }
            return false;
        } break;

        case AstKind_Return: {
            return true;
        } break;

        default: {
            return false;
        } break;
    }
}

static AstProcedure** PendingProcedureBodies = NULL;
static uint64_t PendingProcedureBodyCount    = 0;

bool ResolveAst(Ast* ast, Type* expectedType, AstScope* parentScope, AstProcedure* parentProcedure, bool inLoop) {
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
                if (!ResolveAst(ast->Scope.Statements[i], NULL, ast, parentProcedure, inLoop)) {
                    return false;
                }
            }

            for (int64_t i = (int64_t)PendingProcedureBodyCount - 1; i >= 0; i--) {
                PendingProcedureBodyCount--;
                if (!ResolveAst(PendingProcedureBodies[i]->Procedure.Body, NULL, ast, PendingProcedureBodies[i], false)) {
                    return false;
                }

                if (!TypesEqual(PendingProcedureBodies[i]->ResolvedType->Procedure.ReturnType, &Type_Void)) {
                    if (!BodyReturns(PendingProcedureBodies[i]->Procedure.Body)) {
                        fflush(stdout);
                        fprintf(stderr, "All control paths must return\n");
                        return false;
                    }
                }
            }
        } break;

        case AstKind_Struct: {
            for (uint64_t i = 0; i < ast->Struct.MemberCount; i++) {
                if (!ResolveAst(ast->Struct.Members[i], NULL, parentScope, parentProcedure, false)) {
                    return false;
                }

                if (ast->Struct.Members[i]->Declaration.Value) {
                    fflush(stdout);
                    fprintf(stderr, "Default values are not supported for struct literals\n");
                    return false;
                }
            }

            ast->ResolvedType = &Type_Type;
        } break;

        case AstKind_MemberAccess: {
            if (!ResolveAst(ast->MemberAccess.Operand, NULL, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->MemberAccess.Operand->Kind != AstKind_Name && ast->MemberAccess.Operand->Kind != AstKind_MemberAccess) {
                fflush(stdout);
                fprintf(stderr, "Internal Error: Unable to access member of non-name\n");
                return false;
            }

            if (ast->MemberAccess.Operand->ResolvedType->Kind != TypeKind_Struct) {
                fflush(stdout);
                fprintf(stderr, "Unable to access member of non-struct\n");
                return false;
            }

            ast->MemberAccess.ResolvedStruct = ast->MemberAccess.Operand->ResolvedType;

            Token nameToken = ast->MemberAccess.Name;

            for (uint64_t i = 0; i < ast->MemberAccess.ResolvedStruct->Struct.MemberCount; i++) {
                Token memberName = ast->MemberAccess.ResolvedStruct->Struct.MemberNames[i];
                if (nameToken.Length == memberName.Length) {
                    if (memcmp(&nameToken.Source[nameToken.Position],
                               &memberName.Source[memberName.Position],
                               nameToken.Length) == 0) {
                        ast->ResolvedType = ast->MemberAccess.ResolvedStruct->Struct.MemberTypes[i];
                        break;
                    }
                }
            }

            if (!ast->ResolvedType) {
                fflush(stdout);
                fprintf(stderr,
                        "Unable to find struct name '%.*s'\n",
                        (uint32_t)nameToken.Length,
                        &nameToken.Source[nameToken.Position]);
                return false;
            }
        } break;

        case AstKind_Transmute: {
            if (!ResolveAst(ast->Transmute.Type, &Type_Type, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            ast->ResolvedType = ExpressionToType(ast->Transmute.Type);
            if (!ast->ResolvedType) {
                return false;
            }

            if (!ResolveAst(ast->Transmute.Expression, ast->ResolvedType, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->ResolvedType->Size != ast->Transmute.Expression->ResolvedType->Size) {
                fflush(stdout);
                fprintf(stderr, "Can only transmute to types of the same size\n");
                return false;
            }
        } break;

        case AstKind_Cast: {
            if (!ResolveAst(ast->Cast.Type, &Type_Type, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            ast->ResolvedType = ExpressionToType(ast->Cast.Type);
            if (!ast->ResolvedType) {
                return false;
            }

            if (!ResolveAst(ast->Cast.Expression, ast->ResolvedType, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (!TypesEqual(ast->Cast.Expression->ResolvedType, ast->ResolvedType)) {
                bool found = false;
                for (uint64_t i = 0; i < sizeof(CastableTypes) / sizeof(CastableTypes[0]); i++) {
                    if (TypesEqual(CastableTypes[i].To, ast->ResolvedType) &&
                        TypesEqual(CastableTypes[i].From, ast->Cast.Expression->ResolvedType)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    fflush(stdout);
                    fprintf(stderr,
                            "Cannot cast '%s' to '%s'\n",
                            TypeKind_Names[ast->ResolvedType->Kind],
                            TypeKind_Names[ast->Cast.Expression->ResolvedType->Kind]);
                    return false;
                }
            }
        } break;

        case AstKind_Declaration: {
            if (ast->Declaration.Type) {
                if (!ResolveAst(ast->Declaration.Type, &Type_Type, parentScope, parentProcedure, inLoop)) {
                    return false;
                }

                ast->Declaration.ResolvedType = ExpressionToType(ast->Declaration.Type);
                if (!ast->Declaration.ResolvedType) {
                    return false;
                }
            }

            if (ast->Declaration.Value) {
                if (!ResolveAst(ast->Declaration.Value, ast->Declaration.ResolvedType, parentScope, parentProcedure, inLoop)) {
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
            if (parentScope->Scope.Global) {
                fflush(stdout);
                fprintf(stderr, "Cannot have assignments statements in global scope\n");
                return false;
            }

            if (!ResolveAst(ast->Assignment.Operand, NULL, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->Assignment.Operand->Kind != AstKind_Name && ast->Assignment.Operand->Kind != AstKind_MemberAccess) {
                fflush(stdout);
                fprintf(stderr, "Cannot assign to something that is not a name\n");
                return false;
            }

            // TODO: Check if not constant
            /*
            if (ast->Assignment.Operand->Name.ResolvedDeclaration->Declaration.Constant) {
                fflush(stdout);
                fprintf(stderr, "Cannot assign to a constant\n");
                return false;
            }
            */

            if (!ResolveAst(ast->Assignment.Value, ast->Assignment.Operand->ResolvedType, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (!AssertTypesEqual(ast->Assignment.Operand->ResolvedType, ast->Assignment.Value->ResolvedType)) {
                return false;
            }
        } break;

        case AstKind_If: {
            if (parentScope->Scope.Global) {
                fflush(stdout);
                fprintf(stderr, "Cannot have if statements in global scope\n");
                return false;
            }

            if (!ResolveAst(ast->If.Condition, &Type_Bool, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->If.Condition->ResolvedType->Kind != TypeKind_Bool) {
                fflush(stdout);
                fprintf(stderr, "If statement condition must be a boolean\n");
                return false;
            }

            if (!ResolveAst(ast->If.ThenStatement, NULL, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->If.ElseStatement) {
                if (!ResolveAst(ast->If.ElseStatement, NULL, parentScope, parentProcedure, inLoop)) {
                    return false;
                }
            }
        } break;

        case AstKind_While: {
            if (parentScope->Scope.Global) {
                fflush(stdout);
                fprintf(stderr, "Cannot have while statements in global scope\n");
                return false;
            }

            if (!ResolveAst(ast->While.Condition, &Type_Bool, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->While.Condition->ResolvedType->Kind != TypeKind_Bool) {
                fflush(stdout);
                fprintf(stderr, "If statement condition must be a boolean\n");
                return false;
            }

            if (!ResolveAst(ast->While.Scope, NULL, parentScope, parentProcedure, true)) {
                return false;
            }
        } break;

        case AstKind_Break: {
            if (!inLoop) {
                fflush(stdout);
                fprintf(stderr, "Cannot have break outside of a loop\n");
                return false;
            }
        } break;

        case AstKind_Continue: {
            if (!inLoop) {
                fflush(stdout);
                fprintf(stderr, "Cannot have continue outside of a loop\n");
                return false;
            }
        } break;

        case AstKind_Return: {
            if (parentScope->Scope.Global) {
                fflush(stdout);
                fprintf(stderr, "Cannot have return statements in global scope\n");
                return false;
            }

            if (ast->Return.Value) {
                if (!ResolveAst(
                        ast->Return.Value, parentProcedure->Procedure.ResolvedReturnType, parentScope, parentProcedure, inLoop)) {
                    return false;
                }

                if (!AssertTypesEqual(parentProcedure->Procedure.ResolvedReturnType, ast->Return.Value->ResolvedType)) {
                    return false;
                }
            }
        } break;

        case AstKind_Print: {
            if (parentScope->Scope.Global) {
                fflush(stdout);
                fprintf(stderr, "Cannot have print statements in global scope\n");
                return false;
            }

            if (!ResolveAst(ast->Print.Value, NULL, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->Print.Value->ResolvedType->Kind != TypeKind_Integer &&
                ast->Print.Value->ResolvedType->Kind != TypeKind_Bool) {
                fflush(stdout);
                fprintf(stderr, "Type '%s' is not printable\n", TypeKind_Names[ast->Return.Value->ResolvedType->Kind]);
                return false;
            }
        } break;

        case AstKind_Unary: {
            if (!ResolveAst(ast->Unary.Operand, expectedType, parentScope, parentProcedure, inLoop)) {
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
            if (!ResolveAst(ast->Binary.Left, expectedType, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            // TODO: Is this correct?
            if (ast->Binary.Left->ResolvedType) {
                expectedType = ast->Binary.Left->ResolvedType;
            }

            if (!ResolveAst(ast->Binary.Right, expectedType, parentScope, parentProcedure, inLoop)) {
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
            if (expectedType && expectedType->Kind == TypeKind_Integer) {
                ast->ResolvedType = expectedType;
            } else {
                ast->ResolvedType = &Type_IntegerSigned;
            }
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
                                ResolveAst(declaration, NULL, scope, parentProcedure, false);
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
                for (uint64_t i = 0; i < sizeof(BuiltinNames) / sizeof(BuiltinNames[0]); i++) {
                    if (nameToken.Length == strlen(BuiltinNames[i].Name) &&
                        strncmp(&nameToken.Source[nameToken.Position], BuiltinNames[i].Name, strlen(BuiltinNames[i].Name)) == 0) {
                        ast->Name.ResolvedDeclaration = BuiltinNames[i].Declaration;
                        break;
                    }
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
                if (!ResolveAst(ast->Procedure.Parameters[i], NULL, parentScope, parentProcedure, false)) {
                    return false;
                }

                if (ast->Procedure.Parameters[i]->Declaration.Value) {
                    fflush(stdout);
                    fprintf(stderr, "Default values are not supported for procedure parameters\n");
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
                if (!ResolveAst(ast->Procedure.ReturnType, NULL, parentScope, parentProcedure, false)) {
                    return false;
                }

                ast->Procedure.ResolvedReturnType = ExpressionToType(ast->Procedure.ReturnType);
                if (!ast->Procedure.ResolvedReturnType) {
                    return false;
                }
            } else {
                ast->Procedure.ResolvedReturnType = &Type_Void;
            }

            if (ast->Procedure.Body) {
                PendingProcedureBodies = realloc(PendingProcedureBodies, (PendingProcedureBodyCount + 1) * sizeof(AstScope*));
                PendingProcedureBodies[PendingProcedureBodyCount] = ast;
                PendingProcedureBodyCount++;

                TypeProcedure* type = calloc(1, sizeof(TypeProcedure));
                type->Kind          = TypeKind_Procedure;
                type->Size          = 8;

                if (ast->Procedure.ParameterCount > 0) {
                    type->Procedure.ParameterTypeCount = ast->Procedure.ParameterCount;
                    type->Procedure.ParameterTypes     = calloc(1, type->Procedure.ParameterTypeCount * sizeof(Type));
                    for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                        type->Procedure.ParameterTypes[i] = ast->Procedure.Parameters[i]->Declaration.ResolvedType;
                    }
                }

                type->Procedure.ReturnType = ast->Procedure.ResolvedReturnType;

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
            if (!ResolveAst(ast->Call.Operand, NULL, parentScope, parentProcedure, inLoop)) {
                return false;
            }

            if (ast->Call.Operand->ResolvedType->Kind != TypeKind_Procedure) {
                fflush(stdout);
                fprintf(stderr, "Cannot call a '%s'\n", TypeKind_Names[ast->Call.Operand->ResolvedType->Kind]);
                return false;
            }

            TypeProcedure* type = ast->Call.Operand->ResolvedType;

            if (ast->Call.ArgumentCount != type->Procedure.ParameterTypeCount) {
                fflush(stdout);
                fprintf(stderr, "Wrong number of arguments in procedure call\n");
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                if (!ResolveAst(ast->Call.Arguments[i],
                                ast->Call.Operand->ResolvedType->Procedure.ParameterTypes[i],
                                parentScope,
                                parentProcedure,
                                inLoop)) {
                    return false;
                }
            }

            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                if (!AssertTypesEqual(ast->Call.Arguments[i]->ResolvedType, type->Procedure.ParameterTypes[i])) {
                    return false;
                }
            }

            ast->ResolvedType = type->Procedure.ReturnType;
        } break;

        case AstKind_True:
        case AstKind_False: {
            ast->ResolvedType = &Type_Bool;
        } break;

        case AstKind_TypeExpression: {
        } break;
    }

    ast->Resolution = Resolution_Resolved;
    return true;
}
