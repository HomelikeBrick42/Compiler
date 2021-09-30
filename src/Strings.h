#pragma once

#include "Array.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct String {
    uint8_t* Data;
    uint64_t Length;
} String;

String String_Create(uint8_t* data, uint64_t length);

#define String_Fmt(s) (uint32_t)(s).Length, (s).Data

#define String_FromLiteral(s)                           \
    (String) {                                          \
        .Data = (uint8_t*)(s), .Length = sizeof(s) - 1, \
    }

String String_FromCString(const char* string);
char* String_ToCString(String string);

String String_GetFullPath(String relPath);
bool String_LoadFile(String* outString, String path);

bool String_Equal(String a, String b);

ARRAY_DECL(String, String);

StringArray String_GetLines(String string);
