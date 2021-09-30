#pragma once

#include <stdint.h>

#define ARRAY_DECL(type, name)                      \
    typedef struct name##Array {                    \
        type* Data;                                 \
        uint64_t Length;                            \
        uint64_t Capacity;                          \
    } name##Array;                                  \
                                                    \
    name##Array name##Array##_Create();             \
    void name##Array##_Destroy(name##Array* array); \
                                                    \
    void name##Array##_Push(name##Array* array, type value)

#define ARRAY_IMPL(type, name)                                                                \
    name##Array name##Array##_Create() {                                                      \
        return (name##Array){                                                                 \
            .Data     = NULL,                                                                 \
            .Length   = 0,                                                                    \
            .Capacity = 0,                                                                    \
        };                                                                                    \
    }                                                                                         \
                                                                                              \
    void name##Array##_Destroy(name##Array* array) {                                          \
        free(array->Data);                                                                    \
                                                                                              \
        *array = (name##Array){                                                               \
            .Data     = NULL,                                                                 \
            .Length   = 0,                                                                    \
            .Capacity = 0,                                                                    \
        };                                                                                    \
    }                                                                                         \
                                                                                              \
    void name##Array##_Push(name##Array* array, type value) {                                 \
        if (array->Length >= array->Capacity) {                                               \
            array->Capacity = array->Capacity == 0 ? 1 : array->Capacity * 2;                 \
            array->Data     = realloc(array->Data, array->Capacity * sizeof(array->Data[0])); \
        }                                                                                     \
                                                                                              \
        array->Data[array->Length] = value;                                                   \
        array->Length++;                                                                      \
    }
