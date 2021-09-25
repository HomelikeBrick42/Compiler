#pragma once

#include "Ast.h"
#include "VM.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct Emitter {
    uint8_t* Code;
    uint64_t CodeSize;
    uint64_t GlobalOffset;
    AstDeclaration** Constants;
    uint64_t ConstantCount;
    struct {
        uint64_t Location;
        AstProcedure* Procedure;
    } * PendingProcedureCallLocation;
    uint64_t PendingProcedureCallLocationCount;
    AstProcedure** PendingProcedureBodies;
    uint64_t PendingProcedureBodyCount;
} Emitter;

bool Emitter_Create(Emitter* emitter);
void Emitter_Destroy(Emitter* emitter);

void Emitter_Emit(Emitter* emitter, Ast* ast);
void Emitter_FindDeclarationOffsets(Emitter* emitter, AstScope* parentScope, Ast* ast, bool global);
void Emitter_EmitAst(Emitter* emitter, Ast* ast, bool constantInitialization);

void Emitter_EmitOp(Emitter* emitter, Op op);
void Emitter_EmitI64(Emitter* emitter, int64_t value);
void Emitter_EmitU64(Emitter* emitter, uint64_t value);
