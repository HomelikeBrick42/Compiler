#include "Resolver.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

ARRAY_IMPL(UnaryOperator, UnaryOperator);
ARRAY_IMPL(BinaryOperator, BinaryOperator);
ARRAY_IMPL(Cast, Cast);

static bool Types_Equal(AstType* a, AstType* b) {
    if (!a || !b) {
        return false;
    }

    if (a == b) {
        return true;
    }

    if (a->Kind != b->Kind) {
        return false;
    }

    if (a->Type.Size != b->Type.Size) {
        return false;
    }

    switch (a->Kind) {
        case AstKind_TypeInteger: {
            return a->TypeInteger.Signed == b->TypeInteger.Signed;
        } break;

        case AstKind_TypeFloat:
        case AstKind_TypeBool:
        case AstKind_TypeVoid:
        case AstKind_TypeString: {
            return true;
        } break;

        case AstKind_TypeProcedure: {
            if (a->TypeProcedure.Parameters.Length != b->TypeProcedure.Parameters.Length) {
                return false;
            }

            for (uint64_t j = 0; j < a->TypeProcedure.Parameters.Length; j++) {
                if (!Types_Equal(a->TypeProcedure.Parameters.Data[j]->Declaration.ResolvedType,
                                 b->TypeProcedure.Parameters.Data[j])) {
                    return false;
                }
            }

            if (!Types_Equal(a->TypeProcedure.ReturnType, b->TypeProcedure.ReturnType)) {
                return false;
            }

            return true;
        } break;

        default: {
            assert(false);
            return false;
        } break;
    }
}

static bool Returns(Ast* ast) {
    switch (ast->Kind) {
        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.Statements.Length; i++) {
                if (Returns(ast->Scope.Statements.Data[i])) {
                    return true;
                }
            }
            return false;
        } break;

        case AstKind_If: {
            return Returns(ast->If.ThenStatement) && ast->If.ElseStatement && Returns(ast->If.ElseStatement);
        } break;

        case AstKind_While: {
            return Returns(ast->While.Statement);
        } break;

        case AstKind_Return: {
            return true;
        } break;

        default: {
            return false;
        } break;
    }
}

static bool IsConstant(Ast* ast) {
    switch (ast->Kind) {
        case AstKind_Declaration: {
            return ast->Declaration.Constant;
        } break;

        case AstKind_Unary: {
            return IsConstant(ast->Unary.Operand);
        } break;

        case AstKind_Binary: {
            return IsConstant(ast->Binary.Left) && IsConstant(ast->Binary.Right);
        } break;

        case AstKind_Name: {
            return IsConstant(ast->Name.ResolvedDeclaration);
        } break;

        case AstKind_Call: {
            if (!IsConstant(ast->Call.Operand)) {
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.Arguments.Length; i++) {
                if (!IsConstant(ast->Call.Arguments.Data[i])) {
                    return false;
                }
            }

            return true;
        } break;

        default: {
            return true;
        } break;
    }
}

Resolver Resolver_Create(AstScope* globalScope) {
    Resolver resolver = {
        .GlobalScope       = globalScope,
        .PendingProcedures = AstProcedureArray_Create(),
        .UnaryOperators    = UnaryOperatorArray_Create(),
        .BinaryOperators   = BinaryOperatorArray_Create(),
    };

#define UNARY_OPERATOR_SIMPLE(kind, type)             \
    UnaryOperatorArray_Push(&resolver.UnaryOperators, \
                            (UnaryOperator){          \
                                .Operator = (kind),   \
                                .Operand  = (type),   \
                                .Result   = (type),   \
                            })

    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 1, false));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 2, false));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 4, false));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 8, false));

    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 1, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 2, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 4, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeInteger(&resolver, 8, true));

    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeFloat(&resolver, 4));
    UNARY_OPERATOR_SIMPLE(TokenKind_Plus, Resolver_CreateTypeFloat(&resolver, 8));

    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeInteger(&resolver, 1, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeInteger(&resolver, 2, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeInteger(&resolver, 4, true));
    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeInteger(&resolver, 8, true));

    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeFloat(&resolver, 4));
    UNARY_OPERATOR_SIMPLE(TokenKind_Minus, Resolver_CreateTypeFloat(&resolver, 8));

    UNARY_OPERATOR_SIMPLE(TokenKind_Bang, Resolver_CreateTypeBool(&resolver));

#define BINARY_OPERATOR_SIMPLE(kind, type)              \
    BinaryOperatorArray_Push(&resolver.BinaryOperators, \
                             (BinaryOperator){          \
                                 .Operator = (kind),    \
                                 .Left     = (type),    \
                                 .Right    = (type),    \
                                 .Result   = (type),    \
                             })

#define BINARY_OPERATOR_COMP(kind, type)                                           \
    BinaryOperatorArray_Push(&resolver.BinaryOperators,                            \
                             (BinaryOperator){                                     \
                                 .Operator = (kind),                               \
                                 .Left     = (type),                               \
                                 .Right    = (type),                               \
                                 .Result   = (Resolver_CreateTypeBool(&resolver)), \
                             })

#define BINARY_OPERATOR_ARITHMETIC(type)                       \
    BINARY_OPERATOR_SIMPLE(TokenKind_Plus, (type));            \
    BINARY_OPERATOR_SIMPLE(TokenKind_Minus, (type));           \
    BINARY_OPERATOR_SIMPLE(TokenKind_Asterisk, (type));        \
    BINARY_OPERATOR_SIMPLE(TokenKind_Slash, (type));           \
    BINARY_OPERATOR_SIMPLE(TokenKind_Percent, (type));         \
    BINARY_OPERATOR_COMP(TokenKind_LessThan, (type));          \
    BINARY_OPERATOR_COMP(TokenKind_LessThanEquals, (type));    \
    BINARY_OPERATOR_COMP(TokenKind_GreaterThan, (type));       \
    BINARY_OPERATOR_COMP(TokenKind_GreaterThanEquals, (type)); \
    BINARY_OPERATOR_COMP(TokenKind_EqualsEquals, (type));      \
    BINARY_OPERATOR_COMP(TokenKind_BangEquals, (type))

    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 1, false));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 2, false));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 4, false));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 8, false));

    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 1, true));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 2, true));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 4, true));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeInteger(&resolver, 8, true));

    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeFloat(&resolver, 4));
    BINARY_OPERATOR_ARITHMETIC(Resolver_CreateTypeFloat(&resolver, 8));

#define CAST_SIMPLE(from, to, sym)     \
    CastArray_Push(&resolver.Casts,    \
                   (Cast){             \
                       .From = (from), \
                       .To   = (to),   \
                   })

    return resolver;
}

void Resolver_Destroy(Resolver* resolver) {
    AstProcedureArray_Destroy(&resolver->PendingProcedures);
}

bool Resolver_Resolve(Resolver* resolver) {
    for (uint64_t i = 0; i < resolver->GlobalScope->Scope.Statements.Length; i++) {
        if (!Resolver_ResolveAst(resolver, resolver->GlobalScope->Scope.Statements.Data[i], NULL)) {
            return false;
        }
    }

    for (uint64_t i = 0; i < resolver->GlobalScope->Scope.Statements.Length; i++) {
        if (resolver->GlobalScope->Scope.Statements.Data[i]->Kind == AstKind_Declaration) {
            for (uint64_t j = 0; j < resolver->GlobalScope->Scope.Statements.Length; j++) {
                if (resolver->GlobalScope->Scope.Statements.Data[j]->Kind == AstKind_Declaration &&
                    resolver->GlobalScope->Scope.Statements.Data[i] != resolver->GlobalScope->Scope.Statements.Data[j]) {
                    if (String_Equal(resolver->GlobalScope->Scope.Statements.Data[i]->Declaration.Name.StringValue,
                                     resolver->GlobalScope->Scope.Statements.Data[j]->Declaration.Name.StringValue)) {
                        Token_Error(resolver->GlobalScope->Scope.Statements.Data[j]->Declaration.Name,
                                    "Name defined twice in the same scope");
                        return false;
                    }
                }
            }
        }
    }

    while (resolver->PendingProcedures.Length > 0) {
        AstProcedure* procedure = AstProcedureArray_Pop(&resolver->PendingProcedures);
        if (!Resolver_ResolveAst(resolver, procedure->Procedure.Body, NULL)) {
            return false;
        }
    }

    return true;
}

bool Resolver_ResolveAst(Resolver* resolver, Ast* ast, AstType* expectedType) {
    assert(ast);

    if (expectedType) {
        assert(Ast_IsExpression(ast));
    }

    switch (ast->Kind) {
        case AstKind_Invalid:
        case AstKind_InvalidStatement: {
        } break;

        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.Statements.Length; i++) {
                if (!Resolver_ResolveAst(resolver, ast->Scope.Statements.Data[i], NULL)) {
                    return false;
                }
            }

            for (uint64_t i = 0; i < ast->Scope.Statements.Length; i++) {
                if (ast->Scope.Statements.Data[i]->Kind == AstKind_Declaration) {
                    for (uint64_t j = 0; j < ast->Scope.Statements.Length; j++) {
                        if (ast->Scope.Statements.Data[j]->Kind == AstKind_Declaration &&
                            ast->Scope.Statements.Data[i] != ast->Scope.Statements.Data[j]) {
                            if (String_Equal(ast->Scope.Statements.Data[i]->Declaration.Name.StringValue,
                                             ast->Scope.Statements.Data[j]->Declaration.Name.StringValue)) {
                                Token_Error(ast->Scope.Statements.Data[j]->Declaration.Name,
                                            "Name defined twice in the same scope");
                                return false;
                            }
                        }
                    }
                }
            }

            if (ast->Scope.ParentProcedure) {
                assert(ast->Scope.ParentProcedure->Expression.ResolvedType->Kind == AstKind_TypeProcedure);
                if (!Types_Equal(ast->Scope.ParentProcedure->Expression.ResolvedType->TypeProcedure.ReturnType,
                                 Resolver_CreateTypeVoid(resolver))) {
                    if (!Returns(ast->Scope.ParentProcedure->Procedure.Body)) {
                        Token_Error(ast->Scope.ParentProcedure->Procedure.Body->Scope.CloseBrace, "Procedure does not return");
                        return false;
                    }
                }
            }
        } break;

        case AstKind_Declaration: {
            if (ast->Declaration.Type) {
                if (!Resolver_ResolveAst(resolver, ast->Declaration.Type, Resolver_CreateTypeType(resolver))) {
                    return false;
                }

                ast->Declaration.ResolvedType = Resolver_ExpressionToType(resolver, ast->Declaration.Type);
                if (!ast->Declaration.ResolvedType) {
                    return false;
                }
            }

            if (ast->Declaration.Value) {
                if (!Resolver_ResolveAst(resolver, ast->Declaration.Value, ast->Declaration.ResolvedType)) {
                    return false;
                }
            }

            if (!ast->Declaration.ResolvedType) {
                assert(ast->Declaration.Value);
                ast->Declaration.ResolvedType = ast->Declaration.Value->Expression.ResolvedType;
            }

            if (ast->Declaration.Value) {
                if (!Types_Equal(ast->Declaration.ResolvedType, ast->Declaration.Value->Expression.ResolvedType)) {
                    Token_Error(ast->Declaration.EqualsOrColonToken,
                                "Value of type '%.*s' does not match type '%.*s'",
                                String_Fmt(AstKind_Names[ast->Declaration.Value->Expression.ResolvedType->Kind]),
                                String_Fmt(AstKind_Names[ast->Declaration.ResolvedType->Kind]));
                    return false;
                }
            }
        } break;

        case AstKind_Assignment: {
            if (!Resolver_ResolveAst(resolver, ast->Assignment.Operand, NULL)) {
                return false;
            }

            if (IsConstant(ast->Assignment.Operand)) {
                Token_Error(ast->Assignment.EqualsToken, "Cannot assign to a constant");
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->Assignment.Value, ast->Assignment.Operand->Expression.ResolvedType)) {
                return false;
            }

            if (ast->Assignment.EqualsToken.Kind != TokenKind_Equals) {
                TokenKind operator;
                switch (ast->Assignment.EqualsToken.Kind) {
                    case TokenKind_PlusEquals: {
                        operator= TokenKind_Plus;
                    } break;

                    case TokenKind_MinusEquals: {
                        operator= TokenKind_Minus;
                    } break;

                    case TokenKind_AsteriskEquals: {
                        operator= TokenKind_Asterisk;
                    } break;

                    case TokenKind_SlashEquals: {
                        operator= TokenKind_Slash;
                    } break;

                    case TokenKind_PercentEquals: {
                        operator= TokenKind_Percent;
                    } break;

                    default: {
                        Token_Error(ast->Assignment.EqualsToken, "Internal Error: Unknown equals operator");
                        return false;
                    } break;
                }

                BinaryOperator* binaryOperator = NULL;
                for (uint64_t i = 0; i < resolver->BinaryOperators.Length; i++) {
                    if (resolver->BinaryOperators.Data[i].Operator == operator&&
                            Types_Equal(ast->Assignment.Operand->Expression.ResolvedType,
                                        resolver->BinaryOperators.Data[i].Left) &&
                        Types_Equal(ast->Assignment.Value->Expression.ResolvedType, resolver->BinaryOperators.Data[i].Right)) {
                        binaryOperator = &resolver->BinaryOperators.Data[i];
                        break;
                    }
                }

                if (!binaryOperator) {
                    Token_Error(ast->Assignment.EqualsToken,
                                "Cannot find binary operator '%.*s' for types '%.*s' and '%.*s'",
                                String_Fmt(TokenKind_Names[ast->Assignment.EqualsToken.Kind]),
                                String_Fmt(AstKind_Names[ast->Assignment.Operand->Expression.ResolvedType->Kind]),
                                String_Fmt(AstKind_Names[ast->Assignment.Value->Expression.ResolvedType->Kind]));
                    return false;
                }
            }
        } break;

        case AstKind_If: {
            if (!Resolver_ResolveAst(resolver, ast->If.Condition, Resolver_CreateTypeBool(resolver))) {
                return false;
            }

            if (ast->If.Condition->Expression.ResolvedType->Kind != AstKind_TypeBool) {
                Token_Error(ast->If.IfToken,
                            "Condition must be of type bool. Type was '%.*s'",
                            String_Fmt(AstKind_Names[ast->If.Condition->Expression.ResolvedType->Kind]));
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->If.ThenStatement, NULL)) {
                return false;
            }

            if (ast->If.ElseStatement) {
                if (!Resolver_ResolveAst(resolver, ast->If.ElseStatement, NULL)) {
                    return false;
                }
            }
        } break;

        case AstKind_While: {
            if (!Resolver_ResolveAst(resolver, ast->While.Condition, Resolver_CreateTypeBool(resolver))) {
                return false;
            }

            if (ast->While.Condition->Expression.ResolvedType->Kind != AstKind_TypeBool) {
                Token_Error(ast->If.IfToken,
                            "Condition must be of type bool. Type was '%.*s'",
                            String_Fmt(AstKind_Names[ast->While.Condition->Expression.ResolvedType->Kind]));
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->While.Statement, NULL)) {
                return false;
            }
        } break;

        case AstKind_Return: {
            AstScope* scope = ast->Statement.ParentScope;
            while (scope && !scope->Scope.ParentProcedure) {
                scope = scope->Statement.ParentScope;
            }

            if (!scope) {
                Token_Error(ast->Return.ReturnToken, "Cannot have return in global scope");
                return false;
            }

            assert(scope->Scope.ParentProcedure);
            assert(scope->Scope.ParentProcedure->Expression.ResolvedType->Kind == AstKind_TypeProcedure);

            if (ast->Return.Value) {
                if (!Resolver_ResolveAst(resolver,
                                         ast->Return.Value,
                                         scope->Scope.ParentProcedure->Expression.ResolvedType->TypeProcedure.ReturnType)) {
                    return false;
                }

                if (!Types_Equal(ast->Return.Value->Expression.ResolvedType,
                                 scope->Scope.ParentProcedure->Expression.ResolvedType->TypeProcedure.ReturnType)) {
                    Token_Error(ast->Return.ReturnToken,
                                "Return type '%.*s' does not match procedure return type '%.*s'",
                                String_Fmt(AstKind_Names[ast->Return.Value->Expression.ResolvedType->Kind]),
                                String_Fmt(AstKind_Names[scope->Scope.ParentProcedure->Expression.ResolvedType->TypeProcedure
                                                             .ReturnType->Kind]));
                    return false;
                }
            }
        } break;

        case AstKind_StatementExpression: {
            if (!Resolver_ResolveAst(resolver, ast->StatementExpression.Expression, NULL)) {
                return false;
            }
        } break;

        case AstKind_Semicolon:
        case AstKind_InvalidExpression: {
        } break;

        case AstKind_Unary: {
            if (!Resolver_ResolveAst(resolver, ast->Unary.Operand, expectedType)) {
                return false;
            }

            for (uint64_t i = 0; i < resolver->UnaryOperators.Length; i++) {
                if (resolver->UnaryOperators.Data[i].Operator == ast->Unary.Operator.Kind &&
                    Types_Equal(ast->Unary.Operand->Expression.ResolvedType, resolver->UnaryOperators.Data[i].Operand)) {
                    ast->Expression.ResolvedType = resolver->UnaryOperators.Data[i].Result;
                    break;
                }
            }

            if (!ast->Expression.ResolvedType) {
                Token_Error(ast->Unary.Operator,
                            "Cannot find unary operator '%.*s' for type '%.*s'",
                            String_Fmt(TokenKind_Names[ast->Unary.Operator.Kind]),
                            String_Fmt(AstKind_Names[ast->Unary.Operand->Expression.ResolvedType->Kind]));
                return false;
            }
        } break;

        case AstKind_Binary: {
            if (!Resolver_ResolveAst(resolver, ast->Binary.Left, expectedType)) {
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->Binary.Right, expectedType)) {
                return false;
            }

            for (uint64_t i = 0; i < resolver->BinaryOperators.Length; i++) {
                if (resolver->BinaryOperators.Data[i].Operator == ast->Binary.Operator.Kind &&
                    Types_Equal(ast->Binary.Left->Expression.ResolvedType, resolver->BinaryOperators.Data[i].Left) &&
                    Types_Equal(ast->Binary.Right->Expression.ResolvedType, resolver->BinaryOperators.Data[i].Right)) {
                    ast->Expression.ResolvedType = resolver->BinaryOperators.Data[i].Result;
                    break;
                }
            }

            if (!ast->Expression.ResolvedType) {
                Token_Error(ast->Binary.Operator,
                            "Cannot find binary operator '%.*s' for types '%.*s' and '%.*s'",
                            String_Fmt(TokenKind_Names[ast->Binary.Operator.Kind]),
                            String_Fmt(AstKind_Names[ast->Binary.Left->Expression.ResolvedType->Kind]),
                            String_Fmt(AstKind_Names[ast->Binary.Right->Expression.ResolvedType->Kind]));
                return false;
            }
        } break;

        case AstKind_Cast: {
            if (!Resolver_ResolveAst(resolver, ast->Cast.Type, Resolver_CreateTypeType(resolver))) {
                return false;
            }

            ast->Expression.ResolvedType = Resolver_ExpressionToType(resolver, ast->Cast.Type);
            if (!ast->Expression.ResolvedType) {
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->Cast.Expression, ast->Expression.ResolvedType)) {
                return false;
            }

            if (!Types_Equal(ast->Cast.Expression->Expression.ResolvedType, ast->Expression.ResolvedType)) {
                bool found = false;
                for (uint64_t i = 0; i < resolver->Casts.Length; i++) {
                    if (Types_Equal(ast->Expression.ResolvedType, resolver->Casts.Data[i].To) &&
                        Types_Equal(ast->Cast.Expression->Expression.ResolvedType, resolver->Casts.Data[i].From)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    Token_Error(ast->Cast.CastToken,
                                "Cannot cast type '%.*s' to type '%.*s'",
                                String_Fmt(AstKind_Names[ast->Cast.Expression->Expression.ResolvedType->Kind]),
                                String_Fmt(AstKind_Names[ast->Expression.ResolvedType->Kind]));
                    return false;
                }
            }
        } break;

        case AstKind_Transmute: {
            if (!Resolver_ResolveAst(resolver, ast->Transmute.Expression, NULL)) {
                return false;
            }

            if (!Resolver_ResolveAst(resolver, ast->Transmute.Type, Resolver_CreateTypeType(resolver))) {
                return false;
            }

            ast->Expression.ResolvedType = Resolver_ExpressionToType(resolver, ast->Cast.Type);
            if (!ast->Expression.ResolvedType) {
                return false;
            }

            if (ast->Expression.ResolvedType->Type.Size != ast->Transmute.Expression->Expression.ResolvedType->Type.Size) {
                Token_Error(ast->Transmute.TransmuteToken,
                            "Cannot transmute type '%.*s' to type '%.*s' of different sizes",
                            String_Fmt(AstKind_Names[ast->Transmute.Expression->Expression.ResolvedType->Kind]),
                            String_Fmt(AstKind_Names[ast->Expression.ResolvedType->Kind]));
                return false;
            }
        } break;

        case AstKind_TypeOf: {
            if (!Resolver_ResolveAst(resolver, ast->TypeOf.Expression, NULL)) {
                return false;
            }

            ast->Expression.ResolvedType = Resolver_CreateTypeType(resolver);
        } break;

        case AstKind_SizeOf: {
            if (!Resolver_ResolveAst(resolver, ast->SizeOf.Expression, NULL)) {
                return false;
            }

            ast->Expression.ResolvedType = Resolver_CreateTypeInteger(resolver, 8, false);
        } break;

        case AstKind_Integer: {
            if (expectedType && expectedType->Kind == AstKind_TypeInteger) {
                ast->Expression.ResolvedType = expectedType;
            } else {
                ast->Expression.ResolvedType = Resolver_CreateTypeInteger(resolver, 8, true);
            }
        } break;

        case AstKind_Float: {
            if (expectedType && expectedType->Kind == AstKind_TypeFloat) {
                ast->Expression.ResolvedType = expectedType;
            } else {
                ast->Expression.ResolvedType = Resolver_CreateTypeFloat(resolver, 8);
            }
        } break;

        case AstKind_String: {
            if (expectedType && expectedType->Kind == AstKind_TypeString) {
                ast->Expression.ResolvedType = expectedType;
            } else {
                ast->Expression.ResolvedType = Resolver_CreateTypeString(resolver);
            }
        } break;

        case AstKind_Name: {
            AstStatement* statement = ast->Expression.ParentStatement;
            AstScope* scope         = ast->Expression.ParentStatement->Statement.ParentScope;
            bool inFunction         = scope && !scope->Scope.Global;
            bool after              = false;
            while (scope && !ast->Name.ResolvedDeclaration) {
                if (scope->Scope.ParentProcedure) {
                    for (uint64_t i = 0; i < scope->Scope.ParentProcedure->Procedure.Parameters.Length; i++) {
                        if (String_Equal(scope->Scope.ParentProcedure->Procedure.Parameters.Data[i]->Declaration.Name.StringValue,
                                         ast->Name.Token.StringValue)) {
                            ast->Name.ResolvedDeclaration = scope->Scope.ParentProcedure->Procedure.Parameters.Data[i];
                            break;
                        }
                    }
                }
                for (uint64_t i = 0; i < scope->Scope.Statements.Length; i++) {
                    if (scope->Scope.Statements.Data[i] == statement) {
                        statement = statement->Statement.ParentScope;
                        after     = true;
                    }

                    if (scope->Scope.Statements.Data[i]->Kind == AstKind_Declaration) {
                        AstDeclaration* declaration = scope->Scope.Statements.Data[i];
                        if (((!inFunction && !scope->Scope.Global) || after) && !declaration->Declaration.Constant) {
                            continue;
                        }

                        if (String_Equal(scope->Scope.Statements.Data[i]->Declaration.Name.StringValue,
                                         ast->Name.Token.StringValue)) {
                            ast->Name.ResolvedDeclaration = scope->Scope.Statements.Data[i];
                            break;
                        }
                    }
                }
                if (scope->Scope.ParentProcedure) {
                    inFunction = false;
                }
                after = false;
                scope = scope->Statement.ParentScope;
            }

            if (!ast->Name.ResolvedDeclaration) {
                Token_Error(ast->Name.Token, "Unable to find name '%.*s'", String_Fmt(ast->Name.Token.StringValue));
                return false;
            }

            if (!ast->Name.ResolvedDeclaration->Declaration.ResolvedType) {
                if (!Resolver_ResolveAst(resolver, ast->Name.ResolvedDeclaration, NULL)) {
                    return false;
                }
            }

            ast->Expression.ResolvedType = ast->Name.ResolvedDeclaration->Declaration.ResolvedType;
        } break;

        case AstKind_Procedure: {
            for (uint64_t i = 0; i < ast->Procedure.Parameters.Length; i++) {
                if (!Resolver_ResolveAst(resolver, ast->Procedure.Parameters.Data[i], NULL)) {
                    return false;
                }

                if (ast->Procedure.Parameters.Data[i]->Declaration.Value) {
                    Token_Error(ast->Procedure.Parameters.Data[i]->Declaration.EqualsOrColonToken,
                                "Procedure parameter cannot have default value");
                    return false;
                }
            }

            if (!Resolver_ResolveAst(resolver, ast->Procedure.ReturnType, Resolver_CreateTypeType(resolver))) {
                return false;
            }

            AstType* resolvedReturnType = Resolver_ExpressionToType(resolver, ast->Procedure.ReturnType);
            if (!resolvedReturnType) {
                return false;
            }

            ast->Expression.ResolvedType = Resolver_CreateTypeProcedure(resolver, ast->Procedure.Parameters, resolvedReturnType);
            if (!ast->Expression.ResolvedType) {
                return false;
            }

            if (ast->Procedure.Body) {
                AstProcedureArray_Push(&resolver->PendingProcedures, ast);
            }
        } break;

        case AstKind_Call: {
            if (!Resolver_ResolveAst(resolver, ast->Call.Operand, NULL)) {
                return false;
            }

            if (ast->Call.Operand->Expression.ResolvedType->Kind != AstKind_TypeProcedure) {
                Token_Error(ast->Call.OpenParenthesis,
                            "Cannot call type '%.*s'",
                            String_Fmt(AstKind_Names[ast->Call.Operand->Expression.ResolvedType->Kind]));
                return false;
            }

            AstTypeProcedure* procedureType = ast->Call.Operand->Expression.ResolvedType;
            if (procedureType->TypeProcedure.Parameters.Length != ast->Call.Arguments.Length) {
                Token_Error(ast->Call.CloseParenthesis,
                            "Wrong number of arguments for call. Expected %llu, got %llu",
                            procedureType->TypeProcedure.Parameters.Length,
                            ast->Call.Arguments.Length);
                return false;
            }

            for (uint64_t i = 0; i < ast->Call.Arguments.Length; i++) {
                if (!Resolver_ResolveAst(resolver, ast->Call.Arguments.Data[i], NULL)) {
                    return false;
                }

                if (!Types_Equal(procedureType->TypeProcedure.Parameters.Data[i],
                                 ast->Call.Arguments.Data[i]->Expression.ResolvedType)) {
                    Token_Error(ast->Call.OpenParenthesis,
                                "Argument %llu with type '%.*s' does not match type '%.*s'",
                                i,
                                String_Fmt(AstKind_Names[ast->Call.Arguments.Data[i]->Expression.ResolvedType->Kind]),
                                String_Fmt(AstKind_Names[procedureType->TypeProcedure.Parameters.Data[i]->Kind]));
                    return false;
                }
            }

            ast->Expression.ResolvedType = procedureType->TypeProcedure.ReturnType;
        } break;

        case AstKind_BuitinType: {
            ast->Expression.ResolvedType = Resolver_CreateTypeType(resolver);
        } break;

        case AstKind_InvalidType:
        case AstKind_TypeType:
        case AstKind_TypeInteger:
        case AstKind_TypeFloat:
        case AstKind_TypeString:
        case AstKind_TypeBool:
        case AstKind_TypeVoid:
        case AstKind_TypeProcedure: {
        } break;

#include "AstBeginEndSwitchCases.h"
    }
    return true;
}

// TODO: This is just the evaluation of constants make this work with any expression
AstType* Resolver_ExpressionToType(Resolver* resolver, AstExpression* expression) {
    switch (expression->Kind) {
        case AstKind_Name: {
            if (expression->Expression.ResolvedType->Kind != AstKind_TypeType) {
                Token_Error(expression->Name.Token, "'%.*s' is not a type", String_Fmt(expression->Name.Token.StringValue));
                return NULL;
            }
            if (!expression->Name.ResolvedDeclaration->Declaration.Constant) {
                Token_Error(expression->Name.Token, "Type is not a constant");
                return false;
            }
            return Resolver_ExpressionToType(resolver, expression->Name.ResolvedDeclaration->Declaration.Value);
        } break;

        case AstKind_TypeOf: {
            return expression->TypeOf.Expression->Expression.ResolvedType;
        } break;

        case AstKind_BuitinType: {
            switch (expression->BuitinType.BuiltinToken.Kind) {
                case TokenKind_BuiltinU8: {
                    return Resolver_CreateTypeInteger(resolver, 1, false);
                } break;

                case TokenKind_BuiltinU16: {
                    return Resolver_CreateTypeInteger(resolver, 2, false);
                } break;

                case TokenKind_BuiltinU32: {
                    return Resolver_CreateTypeInteger(resolver, 4, false);
                } break;

                case TokenKind_BuiltinU64: {
                    return Resolver_CreateTypeInteger(resolver, 8, false);
                } break;

                case TokenKind_BuiltinS8: {
                    return Resolver_CreateTypeInteger(resolver, 1, true);
                } break;

                case TokenKind_BuiltinS16: {
                    return Resolver_CreateTypeInteger(resolver, 2, true);
                } break;

                case TokenKind_BuiltinS32: {
                    return Resolver_CreateTypeInteger(resolver, 4, true);
                } break;

                case TokenKind_BuiltinS64: {
                    return Resolver_CreateTypeInteger(resolver, 8, true);
                } break;

                case TokenKind_BuiltinF32: {
                    return Resolver_CreateTypeFloat(resolver, 4);
                } break;

                case TokenKind_BuiltinF64: {
                    return Resolver_CreateTypeFloat(resolver, 8);
                } break;

                case TokenKind_BuiltinBool: {
                    return Resolver_CreateTypeBool(resolver);
                } break;

                case TokenKind_BuiltinVoid: {
                    return Resolver_CreateTypeVoid(resolver);
                } break;

                case TokenKind_BuiltinType: {
                    return Resolver_CreateTypeType(resolver);
                } break;

                default: {
                    assert(false);
                    return NULL;
                } break;
            }
        } break;

        default: {
            assert(false);
            return NULL;
        } break;
    }
}

AstTypeType* Resolver_CreateTypeType(Resolver* resolver) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeType) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeType* type = AstTypeType_Create();
    type->Type.Size   = 8;
    return type;
}

AstTypeBool* Resolver_CreateTypeBool(Resolver* resolver) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeBool) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeBool* type = AstTypeBool_Create();
    type->Type.Size   = 1;
    return type;
}

AstTypeInteger* Resolver_CreateTypeInteger(Resolver* resolver, uint64_t size, bool signedd) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeInteger && resolver->InternedTypes.Data[i]->Type.Size == size &&
            resolver->InternedTypes.Data[i]->TypeInteger.Signed == signedd) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeInteger* type     = AstTypeInteger_Create();
    type->Type.Size          = size;
    type->TypeInteger.Signed = signedd;
    return type;
}

AstTypeFloat* Resolver_CreateTypeFloat(Resolver* resolver, uint64_t size) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeString && resolver->InternedTypes.Data[i]->Type.Size == size) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeFloat* type = AstTypeFloat_Create();
    type->Type.Size    = size;
    return type;
}

AstTypeString* Resolver_CreateTypeString(Resolver* resolver) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeString) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeString* type = AstTypeString_Create();
    type->Type.Size     = 16;
    return type;
}

AstTypeString* Resolver_CreateTypeVoid(Resolver* resolver) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeVoid) {
            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeVoid* type = AstTypeVoid_Create();
    type->Type.Size   = 0;
    return type;
}

AstTypeProcedure* Resolver_CreateTypeProcedure(Resolver* resolver, AstDeclarationArray parameters, AstType* returnType) {
    for (uint64_t i = 0; i < resolver->InternedTypes.Length; i++) {
        if (resolver->InternedTypes.Data[i]->Kind == AstKind_TypeProcedure) {
            if (parameters.Length != resolver->InternedTypes.Data[i]->TypeProcedure.Parameters.Length) {
                continue;
            }

            bool equal = true;
            for (uint64_t j = 0; j < parameters.Length; j++) {
                if (!Types_Equal(parameters.Data[j]->Declaration.ResolvedType,
                                 resolver->InternedTypes.Data[i]->TypeProcedure.Parameters.Data[j])) {
                    equal = false;
                    break;
                }
            }
            if (!equal) {
                continue;
            }

            if (!Types_Equal(returnType, resolver->InternedTypes.Data[i]->TypeProcedure.ReturnType)) {
                continue;
            }

            return resolver->InternedTypes.Data[i];
        }
    }

    AstTypeProcedure* type         = AstTypeProcedure_Create();
    type->Type.Size                = 8;
    type->TypeProcedure.Parameters = AstTypeArray_Create();
    for (uint64_t i = 0; i < parameters.Length; i++) {
        AstTypeArray_Push(&type->TypeProcedure.Parameters, parameters.Data[i]->Declaration.ResolvedType);
    }
    type->TypeProcedure.ReturnType = returnType;
    return type;
}
