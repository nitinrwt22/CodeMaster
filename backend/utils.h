// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME 128
#define MAX_TYPE 64
#define MAX_CATEGORY 32
#define MAX_SCOPE 64

typedef enum {
    SYM_FUNCTION,
    SYM_VARIABLE,
    SYM_LOOP,
    SYM_CONDITION,
    SYM_OTHER
} SymbolCategory;

typedef struct {
    char name[MAX_NAME];
    char type[MAX_TYPE];
    char category[MAX_CATEGORY];  // Can be used as scope
    int line;
} SymbolInfo;

#endif // UTILS_H