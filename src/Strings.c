#include "Strings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ARRAY_IMPL(String, String);

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

String String_Create(uint8_t* data, uint64_t length) {
    return (String){
        .Data   = data,
        .Length = length,
    };
}

String String_FromCString(const char* string) {
    return String_Create((uint8_t*)string, strlen(string));
}

char* String_ToCString(String string) {
    char* cstring = malloc(string.Length + 1);
    memcpy(cstring, string.Data, string.Length);
    cstring[string.Length] = '\0';
    return cstring;
}

String String_GetFullPath(String relPath) {
    char* relPathCString = String_ToCString(relPath);

#if defined(_WIN32) || defined(_WIN64)
    uint64_t length = GetFullPathNameA(relPathCString, 0, NULL, NULL);

    char* buffer = malloc(length);

    GetFullPathNameA(relPathCString, length, buffer, NULL);
#else
    #error "Unsupported platform!"
#endif

    free(relPathCString);

    return String_Create((uint8_t*)buffer, length);
}

bool String_LoadFile(String* outString, String path) {
    if (!outString) {
        return false;
    }

    *outString = (String){};

    char* filePath = String_ToCString(path);
    FILE* file     = fopen(filePath, "rb");
    free(filePath);
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    uint64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* data = malloc(length);

    if (fread(data, 1, length, file) != length) {
        free(data);
        fclose(file);
        return false;
    }

    fclose(file);

    outString->Data   = data;
    outString->Length = length;

    return true;
}

bool String_Equal(String a, String b) {
    if (a.Length != b.Length) {
        return false;
    }

    if (a.Data == b.Data) {
        return true;
    }

    for (uint64_t i = 0; i < a.Length; i++) {
        if (a.Data[i] != b.Data[i]) {
            return false;
        }
    }

    return true;
}

StringArray String_GetLines(String string) {
    StringArray lines = StringArray_Create();

    uint64_t position      = 0;
    uint64_t startPosition = position;
    while (position < string.Length) {
        if (string.Data[position] == '\n') {
            String str = String_Create(&string.Data[startPosition], position - startPosition);
            if (str.Length > 0) {
                str.Length -= 1;
            }
            StringArray_Push(&lines, str);
            position++;
            startPosition = position;
        } else {
            position++;
        }
    }

    if (startPosition < position) {
        String str = String_Create(&string.Data[startPosition], position - startPosition);
        StringArray_Push(&lines, str);
    }

    return lines;
}
