#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define vu8 volatile u8
#define vu16 volatile u16
#define vu32 volatile u32
#define vu64 volatile u64

inline char* strupper(const char* str) {
    char* buffer = (char*)malloc(strlen(str) + 1);
    for (int i = 0; i < strlen(str); ++i)
        buffer[i] = toupper((unsigned)str[i]);
    return buffer;
}

inline char* strlower(const char* str) {
    char* buffer = (char*)malloc(strlen(str) + 1);
    for (int i = 0; i < strlen(str); ++i)
        buffer[i] = tolower((unsigned)str[i]);
    return buffer;
}
