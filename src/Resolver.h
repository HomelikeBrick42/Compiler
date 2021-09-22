#pragma once

#include "Ast.h"
#include "Type.h"

#include <stdint.h>
#include <stdbool.h>

void InitTypes();
bool ResolveAst(Ast* ast, AstScope* parentScope);
