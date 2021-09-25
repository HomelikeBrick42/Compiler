#include "Emitter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

bool Emitter_Create(Emitter* emitter) {
    if (!emitter) {
        return false;
    }

    *emitter = (Emitter){};

    return true;
}

void Emitter_Destroy(Emitter* emitter) {
    free(emitter->Code);
    free(emitter->Constants);
    free(emitter->PendingProcedureCallLocation);
    free(emitter->PendingProcedureBodies);
}

void Emitter_FindDeclarationOffsets(Emitter* emitter, AstScope* parentScope, Ast* ast, bool global) {
    switch (ast->Kind) {
        case AstKind_Scope: {
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                Emitter_FindDeclarationOffsets(emitter, ast, ast->Scope.Statements[i], ast->Scope.Global);
            }
        } break;

        case AstKind_Transmute: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Transmute.Expression, global);
        } break;

        case AstKind_Declaration: {
            if (global || ast->Declaration.Constant) {
                ast->Declaration.GlobalOffset = true;
                ast->Declaration.Offset       = emitter->GlobalOffset;
                emitter->GlobalOffset += ast->Declaration.ResolvedType->Size;
                emitter->Constants = realloc(emitter->Constants, (emitter->ConstantCount + 1) * sizeof(AstDeclaration*));
                emitter->Constants[emitter->ConstantCount] = ast;
                emitter->ConstantCount++;
            } else {
                ast->Declaration.GlobalOffset = false;
                AstScope* scope               = parentScope;
                while (scope->Scope.Nested) {
                    scope = scope->Scope.ParentScope;
                }
                assert(scope);
                assert(!scope->Scope.Global);
                ast->Declaration.Offset = scope->Scope.DeclarationOffset;
                scope->Scope.DeclarationOffset += ast->Declaration.ResolvedType->Size;
            }

            if (ast->Declaration.Value) {
                Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Declaration.Value, global);
            }
        } break;

        case AstKind_If: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->If.ThenScope, global);
            if (ast->If.ElseScope) {
                Emitter_FindDeclarationOffsets(emitter, parentScope, ast->If.ElseScope, global);
            }
        } break;

        case AstKind_Unary: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Unary.Operand, global);
        } break;

        case AstKind_Binary: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Binary.Left, global);
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Binary.Right, global);
        } break;

        case AstKind_Procedure: {
            for (uint64_t i = 0; i < ast->Procedure.ParameterCount; i++) {
                Emitter_FindDeclarationOffsets(emitter, ast->Procedure.Body, ast->Procedure.Parameters[i], false);
            }

            if (ast->Procedure.Body) {
                Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Procedure.Body, false);
            }
        } break;

        case AstKind_Assignment: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Assignment.Operand, global);
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Assignment.Value, global);
        } break;

        case AstKind_Return: {
            if (ast->Return.Value) {
                Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Return.Value, global);
            }
        } break;

        case AstKind_Print: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Call.Operand, global);
        } break;

        case AstKind_Call: {
            Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Call.Operand, global);
            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                Emitter_FindDeclarationOffsets(emitter, parentScope, ast->Call.Arguments[i], global);
            }
        } break;

        case AstKind_Name:
        case AstKind_Integer:
        case AstKind_TypeExpression: {
        } break;
    }
}

void Emitter_Emit(Emitter* emitter, Ast* ast) {
    assert(ast->Kind == AstKind_Scope);
    assert(ast->Scope.Global);

    Emitter_FindDeclarationOffsets(emitter, ast, ast, true);

    Emitter_EmitOp(emitter, Op_AllocStack);
    Emitter_EmitU64(emitter, emitter->GlobalOffset);

    AstDeclaration* mainProc = NULL;

    for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
        if (ast->Scope.Statements[i]->Kind == AstKind_Declaration) {
            AstDeclaration* declaration = ast->Scope.Statements[i];
            Token nameToken             = declaration->Declaration.Name;
            if (declaration->Declaration.Constant && nameToken.Length == 4 &&
                strncmp("main", &nameToken.Source[nameToken.Position], 4) == 0) {
                assert(declaration->Declaration.GlobalOffset);
                // TODO: Typechecking
                if (!mainProc) {
                    mainProc = declaration;
                } else {
                    fflush(stdout);
                    fprintf(stderr, "Multiple definitions of main\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    if (!mainProc) {
        fflush(stdout);
        fprintf(stderr, "main not defined\n");
        exit(EXIT_FAILURE);
    }

    for (uint64_t i = 0; i < emitter->ConstantCount; i++) {
        // TODO: Sort these so constants get initialized in the right order
        Emitter_EmitAst(emitter, emitter->Constants[i], true);
    }

    Emitter_EmitOp(emitter, Op_LoadAbsolute);
    Emitter_EmitU64(emitter, mainProc->Declaration.Offset);
    Emitter_EmitU64(emitter, mainProc->Declaration.ResolvedType->Size);

    Emitter_EmitOp(emitter, Op_Call);
    Emitter_EmitU64(emitter, 0);

    Emitter_EmitOp(emitter, Op_Exit);

    while (emitter->PendingProcedureBodyCount) {
        emitter->PendingProcedureBodyCount--;
        AstProcedure* procedure = emitter->PendingProcedureBodies[emitter->PendingProcedureBodyCount];
        for (uint64_t i = 0; i < emitter->PendingProcedureCallLocationCount; i++) {
            if (emitter->PendingProcedureCallLocation[i].Procedure == procedure) {
                *(uint64_t*)&emitter->Code[emitter->PendingProcedureCallLocation[i].Location] = emitter->CodeSize;
            }
        }
        Emitter_EmitAst(emitter, procedure->Procedure.Body, false);
    }
}

void Emitter_EmitAst(Emitter* emitter, Ast* ast, bool constantInitialization) {
    switch (ast->Kind) {
        case AstKind_Scope: {
            if (!ast->Scope.Global && !ast->Scope.Nested) {
                Emitter_EmitOp(emitter, Op_AllocStack);
                Emitter_EmitU64(emitter, ast->Scope.DeclarationOffset);
            }
            for (uint64_t i = 0; i < ast->Scope.StatementCount; i++) {
                Emitter_EmitAst(emitter, ast->Scope.Statements[i], constantInitialization);
            }
            if (!ast->Scope.Global && !ast->Scope.Nested) {
                Emitter_EmitOp(emitter, Op_Return);
                Emitter_EmitU64(emitter, 0);
            }
        } break;

        case AstKind_Transmute: {
            Emitter_EmitAst(emitter, ast->Transmute.Expression, constantInitialization);
        } break;

        case AstKind_Declaration: {
            if ((ast->Declaration.Constant && constantInitialization) || !ast->Declaration.Constant) {
                Emitter_EmitAst(emitter, ast->Declaration.Value, constantInitialization);
                if (ast->Declaration.GlobalOffset) {
                    Emitter_EmitOp(emitter, Op_StoreAbsolute);
                } else {
                    Emitter_EmitOp(emitter, Op_StoreRelative);
                }
                Emitter_EmitU64(emitter, ast->Declaration.Offset);
                Emitter_EmitU64(emitter, ast->Declaration.ResolvedType->Size);
            }
        } break;

        case AstKind_Assignment: {
            Emitter_EmitAst(emitter, ast->Assignment.Value, constantInitialization);
            // TODO: Support other types of assignment
            assert(ast->Assignment.Operand->Kind == AstKind_Name);
            if (ast->Assignment.Operand->Name.ResolvedDeclaration->Declaration.GlobalOffset) {
                Emitter_EmitOp(emitter, Op_StoreAbsolute);
            } else {
                Emitter_EmitOp(emitter, Op_StoreRelative);
            }
            Emitter_EmitU64(emitter, ast->Assignment.Operand->Name.ResolvedDeclaration->Declaration.Offset);
            Emitter_EmitU64(emitter, ast->Assignment.Operand->Name.ResolvedDeclaration->Declaration.ResolvedType->Size);
        } break;

        case AstKind_If: {
            Emitter_EmitAst(emitter, ast->If.Condition, constantInitialization);
            Emitter_EmitOp(emitter, Op_JumpZero);
            uint64_t jumpFalseLocation = emitter->CodeSize;
            Emitter_EmitU64(emitter, 0);
            Emitter_EmitU64(emitter, 1);
            Emitter_EmitAst(emitter, ast->If.ThenScope, constantInitialization);
            Emitter_EmitOp(emitter, Op_Jump);
            uint64_t jumpEndOfThen = emitter->CodeSize;
            Emitter_EmitU64(emitter, 0);
            *(uint64_t*)&emitter->Code[jumpFalseLocation] = emitter->CodeSize;
            if (ast->If.ElseScope) {
                Emitter_EmitAst(emitter, ast->If.ElseScope, constantInitialization);
            }
            *(uint64_t*)&emitter->Code[jumpEndOfThen] = emitter->CodeSize;
        } break;

        case AstKind_Return: {
            if (ast->Return.Value) {
                Emitter_EmitAst(emitter, ast->Return.Value, constantInitialization);
                Emitter_EmitOp(emitter, Op_Return);
                Emitter_EmitU64(emitter, ast->Return.Value->ResolvedType->Size);
            } else {
                Emitter_EmitOp(emitter, Op_Return);
                Emitter_EmitU64(emitter, 0);
            }
        } break;

        case AstKind_Print: {
            assert((ast->Print.Value->ResolvedType->Kind == TypeKind_Integer) && (ast->Print.Value->ResolvedType->Size == 8));
            Emitter_EmitAst(emitter, ast->Print.Value, constantInitialization);
            if (ast->Print.Value->ResolvedType->Integer.Signed) {
                Emitter_EmitOp(emitter, Op_PrintI64);
            } else {
                Emitter_EmitOp(emitter, Op_PrintU64);
            }
        } break;

        case AstKind_Unary: {
            Emitter_EmitAst(emitter, ast->Unary.Operand, constantInitialization);
            switch (ast->Unary.Operator.Kind) {
                case TokenKind_Plus: {
                    assert((ast->Unary.Operand->ResolvedType->Kind == TypeKind_Integer) && (ast->Unary.Operand->ResolvedType->Size == 8));
                } break;

                case TokenKind_Minus: {
                    assert((ast->Unary.Operand->ResolvedType->Kind == TypeKind_Integer) && (ast->Unary.Operand->ResolvedType->Size == 8));
                    if (ast->Unary.Operand->ResolvedType->Integer.Signed) {
                        Emitter_EmitOp(emitter, Op_NegateI64);
                    } else {
                        Emitter_EmitOp(emitter, Op_NegateU64);
                    }
                } break;

                default: {
                    assert(false);
                } break;
            }
        } break;

        case AstKind_Binary: {
            Emitter_EmitAst(emitter, ast->Binary.Left, constantInitialization);
            Emitter_EmitAst(emitter, ast->Binary.Right, constantInitialization);
            switch (ast->Binary.Operator.Kind) {
                case TokenKind_EqualsEquals: {
                    Emitter_EmitOp(emitter, Op_Equal);
                    Emitter_EmitU64(emitter, ast->Binary.Left->ResolvedType->Size);
                } break;

                case TokenKind_Plus: {
                    assert((ast->Binary.Left->ResolvedType->Kind == TypeKind_Integer) && (ast->Binary.Left->ResolvedType->Size == 8));
                    if (ast->Binary.Left->ResolvedType->Integer.Signed) {
                        Emitter_EmitOp(emitter, Op_AddI64);
                    } else {
                        Emitter_EmitOp(emitter, Op_AddU64);
                    }
                } break;

                case TokenKind_Minus: {
                    assert((ast->Binary.Left->ResolvedType->Kind == TypeKind_Integer) && (ast->Binary.Left->ResolvedType->Size == 8));
                    if (ast->Binary.Left->ResolvedType->Integer.Signed) {
                        Emitter_EmitOp(emitter, Op_SubI64);
                    } else {
                        Emitter_EmitOp(emitter, Op_SubU64);
                    }
                } break;

                case TokenKind_Asterisk: {
                    assert((ast->Binary.Left->ResolvedType->Kind == TypeKind_Integer) && (ast->Binary.Left->ResolvedType->Size == 8));
                    if (ast->Binary.Left->ResolvedType->Integer.Signed) {
                        Emitter_EmitOp(emitter, Op_MulI64);
                    } else {
                        Emitter_EmitOp(emitter, Op_MulU64);
                    }
                } break;

                case TokenKind_Slash: {
                    assert((ast->Binary.Left->ResolvedType->Kind == TypeKind_Integer) && (ast->Binary.Left->ResolvedType->Size == 8));
                    if (ast->Binary.Left->ResolvedType->Integer.Signed) {
                        Emitter_EmitOp(emitter, Op_DivI64);
                    } else {
                        Emitter_EmitOp(emitter, Op_DivU64);
                    }
                } break;

                default: {
                    assert(false);
                } break;
            }
        } break;

        case AstKind_Integer: {
            Emitter_EmitOp(emitter, Op_Push);
            Emitter_EmitU64(emitter, 8); // TODO: Different sized integers
            char buffer[64] = {};
            memcpy(buffer, &ast->Integer.Token.Source[ast->Integer.Token.Position], ast->Integer.Token.Length);
            buffer[ast->Integer.Token.Length] = '\0';
            Emitter_EmitU64(emitter, strtoll(buffer, NULL, 10));
        } break;

        case AstKind_TypeExpression: {
            assert(false);
        } break;

        case AstKind_Name: {
            if (ast->Name.ResolvedDeclaration->Declaration.GlobalOffset) {
                Emitter_EmitOp(emitter, Op_LoadAbsolute);
            } else {
                Emitter_EmitOp(emitter, Op_LoadRelative);
            }
            Emitter_EmitU64(emitter, ast->Name.ResolvedDeclaration->Declaration.Offset);
            Emitter_EmitU64(emitter, ast->Name.ResolvedDeclaration->Declaration.ResolvedType->Size);
        } break;

        case AstKind_Procedure: {
            Emitter_EmitOp(emitter, Op_Push);
            Emitter_EmitU64(emitter, ast->ResolvedType->Size);
            emitter->PendingProcedureCallLocation =
                realloc(emitter->PendingProcedureCallLocation,
                        (emitter->PendingProcedureCallLocationCount + 1) * sizeof(emitter->PendingProcedureCallLocation[0]));
            emitter->PendingProcedureCallLocation[emitter->PendingProcedureCallLocationCount] =
                (__typeof__(emitter->PendingProcedureCallLocation[0])){
                    .Location  = emitter->CodeSize,
                    .Procedure = ast,
                };
            emitter->PendingProcedureCallLocationCount++;
            Emitter_EmitU64(emitter, 0);
            emitter->PendingProcedureBodies =
                realloc(emitter->PendingProcedureBodies, (emitter->PendingProcedureBodyCount + 1) * sizeof(AstProcedure*));
            emitter->PendingProcedureBodies[emitter->PendingProcedureBodyCount] = ast;
            emitter->PendingProcedureBodyCount++;
        } break;

        case AstKind_Call: {
            Emitter_EmitAst(emitter, ast->Call.Operand, constantInitialization);
            uint64_t argSize = 0;
            for (uint64_t i = 0; i < ast->Call.ArgumentCount; i++) {
                Emitter_EmitAst(emitter, ast->Call.Arguments[i], constantInitialization);
                argSize += ast->Call.Arguments[i]->ResolvedType->Size;
            }
            Emitter_EmitOp(emitter, Op_Call);
            Emitter_EmitU64(emitter, argSize);
        } break;
    }
}

void Emitter_EmitOp(Emitter* emitter, Op op) {
    emitter->Code                    = realloc(emitter->Code, emitter->CodeSize + 1);
    emitter->Code[emitter->CodeSize] = op;
    emitter->CodeSize++;
}

void Emitter_EmitI64(Emitter* emitter, int64_t value) {
    emitter->Code                                = realloc(emitter->Code, emitter->CodeSize + sizeof(int64_t));
    *(int64_t*)&emitter->Code[emitter->CodeSize] = value;
    emitter->CodeSize += sizeof(int64_t);
}

void Emitter_EmitU64(Emitter* emitter, uint64_t value) {
    emitter->Code                                 = realloc(emitter->Code, emitter->CodeSize + sizeof(uint64_t));
    *(uint64_t*)&emitter->Code[emitter->CodeSize] = value;
    emitter->CodeSize += sizeof(uint64_t);
}
