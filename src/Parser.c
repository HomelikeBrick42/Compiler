#include "Parser.h"

#include <stdlib.h>
#include <assert.h>

static void Parser_SetExpressionParentStatement(AstExpression* expression, AstStatement* statement) {
    if (!expression) {
        return;
    }

    assert(Ast_IsExpression(expression));
    assert(Ast_IsStatement(statement));

    expression->Expression.ParentStatement = statement;

    switch (expression->Kind) {
        case AstKind_Unary: {
            Parser_SetExpressionParentStatement(expression->Unary.Operand, statement);
        } break;

        case AstKind_Binary: {
            Parser_SetExpressionParentStatement(expression->Binary.Left, statement);
            Parser_SetExpressionParentStatement(expression->Binary.Right, statement);
        } break;

        case AstKind_Cast: {
            Parser_SetExpressionParentStatement(expression->Cast.Expression, statement);
        } break;

        case AstKind_Transmute: {
            Parser_SetExpressionParentStatement(expression->Transmute.Expression, statement);
        } break;

        case AstKind_TypeOf: {
            Parser_SetExpressionParentStatement(expression->TypeOf.Expression, statement);
        } break;

        case AstKind_SizeOf: {
            Parser_SetExpressionParentStatement(expression->SizeOf.Expression, statement);
        } break;

        case AstKind_Call: {
            Parser_SetExpressionParentStatement(expression->Call.Operand, statement);
            for (uint64_t i = 0; i < expression->Call.Arguments.Length; i++) {
                Parser_SetExpressionParentStatement(expression->Call.Arguments.Data[i], statement);
            }
        } break;

        case AstKind_InvalidExpression:
        case AstKind_Integer:
        case AstKind_Float:
        case AstKind_String:
        case AstKind_Name:
        case AstKind_Procedure: {
        } break;

        default: {
            assert(false);
        } break;
    }
}

Parser Parser_Create(String filePath, String source) {
    Parser parser        = {};
    parser.Lexer         = Lexer_Create(filePath, source);
    parser.PreviousToken = (Token){
        .Kind     = TokenKind_EndOfFile,
        .FilePath = parser.Lexer.FilePath,
        .Source   = parser.Lexer.Source,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = 0,
    };
    parser.Current     = Lexer_NextToken(&parser.Lexer);
    parser.ParentScope = NULL;
    return parser;
}

Token Parser_NextToken(Parser* parser) {
    parser->PreviousToken = parser->Current;
    parser->Current       = Lexer_NextToken(&parser->Lexer);
    return parser->PreviousToken;
}

Token Parser_ExpectToken(Parser* parser, TokenKind kind) {
    if (parser->Current.Kind == kind) {
        return Parser_NextToken(parser);
    }

    Token_Error(parser->Current,
                "Expected '%.*s', got '%.*s'",
                String_Fmt(TokenKind_Names[kind]),
                String_Fmt(TokenKind_Names[parser->Current.Kind]));
    parser->Error = true;

    Token token = {
        .Kind     = kind,
        .FilePath = parser->Current.FilePath,
        .Source   = parser->Current.Source,
        .Position = parser->Current.Position,
        .Line     = parser->Current.Line,
        .Column   = parser->Current.Column,
        .Length   = parser->Current.Length,
    };

    Parser_NextToken(parser);

    return token;
}

AstScope* Parser_Parse(Parser* parser) {
    AstScope* scope = AstScope_Create();

    scope->Statement.ParentScope = parser->ParentScope;
    parser->ParentScope          = scope;

    scope->Scope.Global     = true;
    scope->Scope.Statements = AstStatementArray_Create();

    while (parser->Current.Kind != TokenKind_EndOfFile) {
        AstStatementArray_Push(&scope->Scope.Statements, Parser_ParseStatement(parser));
        if (parser->PreviousToken.Kind != TokenKind_CloseBrace) {
            Parser_ExpectToken(parser, TokenKind_Semicolon);
        }
    }

    parser->ParentScope = scope->Statement.ParentScope;

    return scope;
}

AstStatement* Parser_ParseStatement(Parser* parser) {
    switch (parser->Current.Kind) {
        case TokenKind_Semicolon: {
            AstSemicolon* semicolon          = AstSemicolon_Create();
            semicolon->Statement.ParentScope = parser->ParentScope;
            Parser_ExpectToken(parser, TokenKind_Semicolon);
            return semicolon;
        } break;

        case TokenKind_OpenBrace: {
            return Parser_ParseScope(parser, NULL);
        } break;

        case TokenKind_If: {
            AstIf* iff        = AstIf_Create();
            iff->If.IfToken   = Parser_ExpectToken(parser, TokenKind_If);
            iff->If.Condition = Parser_ParseExpression(parser);
            Parser_SetExpressionParentStatement(iff->If.Condition, iff);
            if (parser->Current.Kind == TokenKind_Do) {
                Parser_ExpectToken(parser, TokenKind_Do);
                iff->If.ThenStatement = Parser_ParseStatement(parser);
            } else {
                iff->If.ThenStatement = Parser_ParseScope(parser, NULL);
                if (parser->Current.Kind == TokenKind_Else) {
                    iff->If.ElseToken     = Parser_ExpectToken(parser, TokenKind_Else);
                    iff->If.ElseStatement = Parser_ParseScope(parser, NULL);
                }
            }
            return iff;
        } break;

        case TokenKind_While: {
            AstWhile* whilee         = AstWhile_Create();
            whilee->While.WhileToken = Parser_ExpectToken(parser, TokenKind_While);
            whilee->While.Condition  = Parser_ParseExpression(parser);
            Parser_SetExpressionParentStatement(whilee->While.Condition, whilee);
            if (parser->Current.Kind == TokenKind_Do) {
                Parser_ExpectToken(parser, TokenKind_Do);
                whilee->While.Statement = Parser_ParseStatement(parser);
            } else {
                whilee->While.Statement = Parser_ParseScope(parser, NULL);
            }
            return whilee;
        } break;

        case TokenKind_Return: {
            AstReturn* returnn          = AstReturn_Create();
            returnn->Return.ReturnToken = Parser_ExpectToken(parser, TokenKind_Return);
            if (parser->Current.Kind != TokenKind_Semicolon) {
                returnn->Return.Value = Parser_ParseExpression(parser);
                Parser_SetExpressionParentStatement(returnn->Return.Value, returnn);
            }
            return returnn;
        } break;

        default: {
            AstExpression* expression = Parser_ParseExpression(parser);
            if (expression && expression->Kind == AstKind_Name && parser->Current.Kind == TokenKind_Colon) {
                AstDeclaration* declaration = Parser_ParseDeclaration(parser, expression->Name.Token, NULL);
                free(expression);
                return declaration;
            } else if (Token_IsAssignment(parser->Current)) {
                AstAssignment* assignment      = AstAssignment_Create();
                assignment->Assignment.Operand = expression;
                Parser_SetExpressionParentStatement(assignment->Assignment.Operand, assignment);
                assignment->Assignment.EqualsToken = Parser_NextToken(parser);
                assignment->Assignment.Value       = Parser_ParseExpression(parser);
                Parser_SetExpressionParentStatement(assignment->Assignment.Value, assignment);
                return assignment;
            } else {
                AstStatementExpression* statementExpression = AstStatementExpression_Create();
                statementExpression->Statement.ParentScope  = parser->ParentScope;
                Parser_SetExpressionParentStatement(expression, statementExpression);
                statementExpression->StatementExpression.Expression = expression;
                return statementExpression;
            }
        } break;
    }
}

AstStatement* Parser_ParseScope(Parser* parser, AstProcedure* parentProcedure) {
    AstScope* scope = AstScope_Create();

    scope->Statement.ParentScope = parser->ParentScope;

    parser->ParentScope = scope;

    scope->Scope.ParentProcedure = parentProcedure;
    scope->Scope.OpenBrace       = Parser_ExpectToken(parser, TokenKind_OpenBrace);
    scope->Scope.Statements      = AstStatementArray_Create();

    while (parser->Current.Kind != TokenKind_CloseBrace && parser->Current.Kind != TokenKind_EndOfFile) {
        AstStatementArray_Push(&scope->Scope.Statements, Parser_ParseStatement(parser));
        if (parser->PreviousToken.Kind != TokenKind_CloseBrace) {
            Parser_ExpectToken(parser, TokenKind_Semicolon);
        }
    }

    scope->Scope.CloseBrace = Parser_ExpectToken(parser, TokenKind_CloseBrace);
    parser->ParentScope     = scope->Statement.ParentScope;

    return scope;
}

AstDeclaration* Parser_ParseDeclaration(Parser* parser, Token name, AstProcedure* parentProcedure) {
    AstDeclaration* declaration = AstDeclaration_Create();

    declaration->Statement.ParentScope = parser->ParentScope;

    declaration->Declaration.Name            = name;
    declaration->Declaration.ParentProcedure = parentProcedure;
    declaration->Declaration.ColonToken      = Parser_ExpectToken(parser, TokenKind_Colon);

    if (parser->Current.Kind != TokenKind_Colon && parser->Current.Kind != TokenKind_Equals) {
        declaration->Declaration.Type = Parser_ParseExpression(parser);
        Parser_SetExpressionParentStatement(declaration->Declaration.Type, declaration);
    }

    if (parser->Current.Kind == TokenKind_Colon) {
        declaration->Declaration.EqualsOrColonToken = Parser_ExpectToken(parser, TokenKind_Colon);
        declaration->Declaration.Value              = Parser_ParseExpression(parser);
        Parser_SetExpressionParentStatement(declaration->Declaration.Value, declaration);
        declaration->Declaration.Constant = true;
    } else if (parser->Current.Kind == TokenKind_Equals) {
        declaration->Declaration.EqualsOrColonToken = Parser_ExpectToken(parser, TokenKind_Equals);
        declaration->Declaration.Value              = Parser_ParseExpression(parser);
        Parser_SetExpressionParentStatement(declaration->Declaration.Value, declaration);
        declaration->Declaration.Constant = false;
    }

    return declaration;
}

AstExpression* Parser_ParseExpression(Parser* parser) {
    return Parser_ParseBinaryExpression(parser, 0);
}

AstExpression* Parser_ParsePrimaryExpression(Parser* parser) {
    switch (parser->Current.Kind) {
        case TokenKind_Identifier: {
            AstName* name    = AstName_Create();
            name->Name.Token = Parser_ExpectToken(parser, TokenKind_Identifier);
            return name;
        } break;

        case TokenKind_Integer: {
            AstInteger* integer    = AstInteger_Create();
            integer->Integer.Token = Parser_ExpectToken(parser, TokenKind_Integer);
            return integer;
        } break;

        case TokenKind_Float: {
            AstFloat* floatt    = AstFloat_Create();
            floatt->Float.Token = Parser_ExpectToken(parser, TokenKind_Integer);
            return floatt;
        } break;

        case TokenKind_String: {
            AstString* string    = AstString_Create();
            string->String.Token = Parser_ExpectToken(parser, TokenKind_Integer);
            return string;
        } break;

        case TokenKind_TypeOf: {
            AstTypeOf* typeoff          = AstTypeOf_Create();
            typeoff->TypeOf.TypeOfToken = Parser_ExpectToken(parser, TokenKind_TypeOf);
            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            typeoff->TypeOf.Expression = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);
            return typeoff;
        } break;

        case TokenKind_SizeOf: {
            AstSizeOf* sizeoff          = AstSizeOf_Create();
            sizeoff->SizeOf.SizeOfToken = Parser_ExpectToken(parser, TokenKind_SizeOf);
            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            sizeoff->SizeOf.Expression = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);
            return sizeoff;
        } break;

        case TokenKind_OpenParenthesis: {
            Token openParenthesis = Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            if (parser->Current.Kind == TokenKind_CloseParenthesis) {
                AstProcedure* procedure = AstProcedure_Create();

                procedure->Procedure.OpenParenthesis  = openParenthesis;
                procedure->Procedure.CloseParenthesis = Parser_ExpectToken(parser, TokenKind_CloseParenthesis);
                procedure->Procedure.Parameters       = AstDeclarationArray_Create();

                Parser_ExpectToken(parser, TokenKind_RightArrow);
                procedure->Procedure.ReturnType = Parser_ParseExpression(parser);

                if (parser->Current.Kind == TokenKind_CompilerProc) {
                    Parser_ExpectToken(parser, TokenKind_CompilerProc);
                    procedure->Procedure.CompilerProc            = true;
                    procedure->Procedure.Data.CompilerProcString = Parser_ExpectToken(parser, TokenKind_String);
                } else if (parser->Current.Kind == TokenKind_OpenBrace) {
                    procedure->Procedure.Body = Parser_ParseScope(parser, procedure);
                }

                return procedure;
            } else {
                AstExpression* expression = Parser_ParseExpression(parser);
                if (expression && expression->Kind == AstKind_Name && parser->Current.Kind == TokenKind_Colon) {
                    Token name = expression->Name.Token;
                    free(expression);

                    AstProcedure* procedure = AstProcedure_Create();

                    procedure->Procedure.OpenParenthesis = openParenthesis;

                    AstDeclarationArray_Push(&procedure->Procedure.Parameters, Parser_ParseDeclaration(parser, name, procedure));

                    while (parser->Current.Kind != TokenKind_CloseParenthesis && parser->Current.Kind != TokenKind_EndOfFile) {
                        Parser_ExpectToken(parser, TokenKind_Comma);
                        if (parser->Current.Kind == TokenKind_CloseParenthesis) {
                            break;
                        }

                        name = Parser_ExpectToken(parser, TokenKind_Identifier);
                        AstDeclarationArray_Push(&procedure->Procedure.Parameters,
                                                 Parser_ParseDeclaration(parser, name, procedure));
                    }

                    procedure->Procedure.CloseParenthesis = Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

                    Parser_ExpectToken(parser, TokenKind_RightArrow);
                    procedure->Procedure.ReturnType = Parser_ParseExpression(parser);

                    if (parser->Current.Kind == TokenKind_CompilerProc) {
                        Parser_ExpectToken(parser, TokenKind_CompilerProc);
                        procedure->Procedure.CompilerProc            = true;
                        procedure->Procedure.Data.CompilerProcString = Parser_ExpectToken(parser, TokenKind_String);
                    } else if (parser->Current.Kind == TokenKind_OpenBrace) {
                        procedure->Procedure.Body = Parser_ParseScope(parser, procedure);
                    }

                    return procedure;
                } else {
                    Parser_ExpectToken(parser, TokenKind_CloseParenthesis);
                    return expression;
                }
            }
        } break;

        default: {
            Token_Error(parser->Current, "Unexpected '%.*s' in expression", String_Fmt(TokenKind_Names[parser->Current.Kind]));
            parser->Error = true;
            return AstInvalidExpression_Create();
        } break;
    }
}

uint64_t Parser_GetUnaryOperatorPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind_Cast:
        case TokenKind_Transmute:
            return 6;

        case TokenKind_Plus:
        case TokenKind_Minus:
        case TokenKind_Bang:
            return 5;

        default:
            return 0;
    }
}

uint64_t Parser_GetBinaryOperatorPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind_Asterisk:
        case TokenKind_Slash:
            return 3;

        case TokenKind_Plus:
        case TokenKind_Minus:
            return 2;

        case TokenKind_EqualsEquals:
        case TokenKind_BangEquals:
        case TokenKind_LessThan:
        case TokenKind_GreaterThan:
        case TokenKind_LessThanEquals:
        case TokenKind_GreaterThanEquals:
            return 1;

        default:
            return 0;
    }
}

AstExpression* Parser_ParseBinaryExpression(Parser* parser, uint64_t parentPrecedence) {
    AstExpression* left;

    uint64_t unaryPrecedence = Parser_GetUnaryOperatorPrecedence(parser->Current.Kind);
    if (unaryPrecedence != 0) {
        Token unaryOperator = Parser_NextToken(parser);
        if (unaryOperator.Kind == TokenKind_Cast) {
            AstCast* cast = AstCast_Create();

            cast->Cast.CastToken = unaryOperator;

            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            cast->Cast.Type = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            cast->Cast.Expression = Parser_ParseBinaryExpression(parser, unaryPrecedence);

            left = cast;
        } else if (unaryOperator.Kind == TokenKind_Transmute) {
            AstTransmute* transmute = AstTransmute_Create();

            transmute->Transmute.TransmuteToken = unaryOperator;

            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            transmute->Transmute.Type = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            transmute->Transmute.Expression = Parser_ParseBinaryExpression(parser, unaryPrecedence);

            left = transmute;
        } else {
            AstUnary* unary = AstUnary_Create();

            unary->Unary.Operator = unaryOperator;
            unary->Unary.Operand  = Parser_ParseBinaryExpression(parser, unaryPrecedence);

            left = unary;
        }
    } else {
        left = Parser_ParsePrimaryExpression(parser);
    }

    while (true) {
        while (parser->Current.Kind == TokenKind_OpenParenthesis) {
            AstCall* call = AstCall_Create();

            call->Call.OpenParenthesis = Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            call->Call.Operand         = left;
            call->Call.Arguments       = AstExpressionArray_Create();

            if (parser->Current.Kind != TokenKind_CloseParenthesis) {
                AstExpressionArray_Push(&call->Call.Arguments, Parser_ParseExpression(parser));

                while (parser->Current.Kind != TokenKind_CloseParenthesis && parser->Current.Kind != TokenKind_EndOfFile) {
                    Parser_ExpectToken(parser, TokenKind_Comma);
                    if (parser->Current.Kind == TokenKind_CloseParenthesis) {
                        break;
                    }

                    AstExpressionArray_Push(&call->Call.Arguments, Parser_ParseExpression(parser));
                }
            }

            call->Call.CloseParenthesis = Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            left = call;
        }

        uint64_t binaryPrecedence = Parser_GetBinaryOperatorPrecedence(parser->Current.Kind);
        if (binaryPrecedence <= parentPrecedence) {
            break;
        }

        AstBinary* binary = AstBinary_Create();

        binary->Binary.Left     = left;
        binary->Binary.Operator = Parser_NextToken(parser);

        binary->Binary.Right = Parser_ParseBinaryExpression(parser, binaryPrecedence);

        left = binary;
    }

    return left;
}
