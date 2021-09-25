#pragma once

#include "Ast.h"
#include "Type.h"

#include <stdint.h>
#include <stdbool.h>

bool ResolveAst(Ast* ast, Type* expectedType, AstScope* parentScope, AstProcedure* parentProcedure);
