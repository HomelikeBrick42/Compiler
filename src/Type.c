#include "Type.h"

#include <string.h>

const char* TypeKind_Names[TypeKind_Count] = {
#define TYPE_KIND(name, data) [TypeKind_##name] = #name,
    TYPE_KINDS
#undef TYPE_KIND
};

bool TypesEqual(Type* a, Type* b) {
    if (a == b) {
        return true;
    }

    if (a->Kind != b->Kind) {
        return false;
    }

    if (a->Size != b->Size) {
        return false;
    }

    switch (a->Kind) {
        case TypeKind_Type:
        case TypeKind_Float:
        case TypeKind_Bool:
        case TypeKind_Void:
            return true;

        case TypeKind_Integer:
            return a->Integer.Signed == b->Integer.Signed;

        case TypeKind_Struct: {
            for (uint64_t i = 0; i < a->Struct.MemberCount; i++) {
                if (!TypesEqual(a->Struct.MemberTypes[i], b->Struct.MemberTypes[i])) {
                    return false;
                }
            }

            return true;
        } break;

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

const char* Type_TypeName             = "type";
Type Type_Type                        = {};
AstTypeExpression Type_TypeExpression = {};
AstDeclaration Type_TypeDeclaration   = {};

const char* Type_IntegerSignedName             = "int";
Type Type_IntegerSigned                        = {};
AstTypeExpression Type_IntegerSignedExpression = {};
AstDeclaration Type_IntegerSignedDeclaration   = {};

const char* Type_IntegerUnsignedName             = "uint";
Type Type_IntegerUnsigned                        = {};
AstTypeExpression Type_IntegerUnsignedExpression = {};
AstDeclaration Type_IntegerUnsignedDeclaration   = {};

const char* Type_BoolName             = "bool";
Type Type_Bool                        = {};
AstTypeExpression Type_BoolExpression = {};
AstDeclaration Type_BoolDeclaration   = {};

const char* Type_VoidName             = "void";
Type Type_Void                        = {};
AstTypeExpression Type_VoidExpression = {};
AstDeclaration Type_VoidDeclaration   = {};

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
    Type_TypeDeclaration.Declaration.Constant     = true;

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
    Type_IntegerSignedDeclaration.Declaration.Constant     = true;

    Type_IntegerUnsigned.Kind           = TypeKind_Integer;
    Type_IntegerUnsigned.Size           = 8;
    Type_IntegerUnsigned.Integer.Signed = false;

    Type_IntegerUnsignedExpression.Kind                = AstKind_TypeExpression;
    Type_IntegerUnsignedExpression.Resolution          = Resolution_Resolved;
    Type_IntegerUnsignedExpression.ResolvedType        = &Type_Type;
    Type_IntegerUnsignedExpression.TypeExpression.Type = &Type_IntegerUnsigned;

    Type_IntegerUnsignedDeclaration.Kind             = AstKind_Declaration;
    Type_IntegerUnsignedDeclaration.Resolution       = Resolution_Resolved;
    Type_IntegerUnsignedDeclaration.Declaration.Name = (Token){
        .Kind     = TokenKind_Identifier,
        .FilePath = "Builtin",
        .Source   = Type_IntegerUnsignedName,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = strlen(Type_IntegerUnsignedName),
    };
    Type_IntegerUnsignedDeclaration.Declaration.ResolvedType = &Type_Type;
    Type_IntegerUnsignedDeclaration.Declaration.Value        = &Type_IntegerUnsignedExpression;
    Type_IntegerUnsignedDeclaration.Declaration.Constant     = true;

    Type_Bool.Kind = TypeKind_Bool;
    Type_Bool.Size = 1;

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
    Type_BoolDeclaration.Declaration.Constant     = true;

    Type_Void.Kind = TypeKind_Void;
    Type_Void.Size = 0;

    Type_VoidExpression.Kind                = AstKind_TypeExpression;
    Type_VoidExpression.Resolution          = Resolution_Resolved;
    Type_VoidExpression.ResolvedType        = &Type_Type;
    Type_VoidExpression.TypeExpression.Type = &Type_Void;

    Type_VoidDeclaration.Kind             = AstKind_Declaration;
    Type_VoidDeclaration.Resolution       = Resolution_Resolved;
    Type_VoidDeclaration.Declaration.Name = (Token){
        .Kind     = TokenKind_Identifier,
        .FilePath = "Builtin",
        .Source   = Type_VoidName,
        .Position = 0,
        .Line     = 1,
        .Column   = 1,
        .Length   = strlen(Type_VoidName),
    };
    Type_VoidDeclaration.Declaration.ResolvedType = &Type_Type;
    Type_VoidDeclaration.Declaration.Value        = &Type_VoidExpression;
    Type_VoidDeclaration.Declaration.Constant     = true;
}
