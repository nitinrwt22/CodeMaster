#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_keyword(const char* str) {
    const char* keywords[] = {
        "int", "float", "char", "void", "return",
        "if", "else", "for", "while", "do"
    };
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < num_keywords; ++i) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

static void add_token(Token** tokens, int* count, int* capacity, TokenType type, const char* value, int line) {
    if (*count >= *capacity) {
        *capacity *= 2;
        *tokens = (Token*)realloc(*tokens, sizeof(Token) * (*capacity));
    }
    (*tokens)[*count].type = type;
    strncpy((*tokens)[*count].value, value, 127);
    (*tokens)[*count].value[127] = '\0';
    (*tokens)[*count].line = line;
    (*count)++;
}

Token* tokenize_file(const char* filename, int* token_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        *token_count = 0;
        return NULL;
    }

    int capacity = 256;
    Token* tokens = (Token*)malloc(sizeof(Token) * capacity);
    *token_count = 0;

    int line = 1;
    int c = fgetc(file);

    while (c != EOF) {
        // Skip whitespace
        if (isspace(c)) {
            if (c == '\n') line++;
            c = fgetc(file);
            continue;
        }

        // Skip comments and handle division operator
        if (c == '/') {
            int next = fgetc(file);
            if (next == '/') {
                // Single-line comment
                while ((c = fgetc(file)) != EOF && c != '\n');
                if (c == '\n') line++;
                c = fgetc(file);
                continue;
            } else if (next == '*') {
                // Multi-line comment
                int prev = 0;
                while ((c = fgetc(file)) != EOF) {
                    if (c == '\n') line++;
                    if (prev == '*' && c == '/') {
                        c = fgetc(file);
                        break;
                    }
                    prev = c;
                }
                continue;
            } else {
                // Not a comment, it's a division-related operator
                char buf[3] = {'/', '\0', '\0'};
                if (next == '=') {
                    buf[1] = '=';
                    c = fgetc(file);
                } else {
                    c = next;
                }
                add_token(&tokens, token_count, &capacity, TOKEN_OPERATOR, buf, line);
                continue;
            }
        }

        // Identifiers and keywords
        if (isalpha(c) || c == '_') {
            char buf[128];
            int len = 0;
            while ((isalnum(c) || c == '_') && len < 127) {
                buf[len++] = c;
                c = fgetc(file);
            }
            buf[len] = '\0';
            if (is_keyword(buf)) {
                add_token(&tokens, token_count, &capacity, TOKEN_KEYWORD, buf, line);
            } else {
                add_token(&tokens, token_count, &capacity, TOKEN_IDENTIFIER, buf, line);
            }
            continue;
        }

        // Numeric literals
        if (isdigit(c)) {
            char buf[128];
            int len = 0;
            while ((isdigit(c) || c == '.') && len < 127) {
                buf[len++] = c;
                c = fgetc(file);
            }
            buf[len] = '\0';
            add_token(&tokens, token_count, &capacity, TOKEN_NUMBER, buf, line);
            continue;
        }

        // String literals
        if (c == '"') {
            char buf[128];
            int len = 0;
            buf[len++] = c; // include opening quote
            c = fgetc(file);
            while (c != EOF && c != '"' && len < 126) {
                if (c == '\n') line++;
                buf[len++] = c;
                if (c == '\\') {
                    c = fgetc(file);
                    if (c != EOF && len < 126) {
                        buf[len++] = c;
                    } else {
                        break;
                    }
                }
                c = fgetc(file);
            }
            if (c == '"') {
                buf[len++] = c; // include closing quote
                c = fgetc(file);
            }
            buf[len] = '\0';
            add_token(&tokens, token_count, &capacity, TOKEN_STRING, buf, line);
            continue;
        }

        // Other operators: + - * % = == != < > <= >= && || !
        if (strchr("+-*%=!<>|&", c)) {
            char buf[3] = {c, '\0', '\0'};
            int next = fgetc(file);
            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=') ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {
                buf[1] = next;
                c = fgetc(file);
            } else {
                c = next;
            }
            add_token(&tokens, token_count, &capacity, TOKEN_OPERATOR, buf, line);
            continue;
        }

        // Punctuation: ; , ( ) { } [ ]
        if (strchr(";,(){}[]", c)) {
            char buf[2] = {c, '\0'};
            add_token(&tokens, token_count, &capacity, TOKEN_PUNCTUATION, buf, line);
            c = fgetc(file);
            continue;
        }

        char unk[2] = {c, '\0'};
        add_token(&tokens, token_count, &capacity, TOKEN_UNKNOWN, unk, line);
        c = fgetc(file);
    }

    add_token(&tokens, token_count, &capacity, TOKEN_EOF, "EOF", line);
    fclose(file);
    return tokens;
}

void print_tokens(Token* tokens, int count) {
    if (!tokens) return;
    const char* typeNames[] = {
        "KEYWORD", "IDENTIFIER", "NUMBER", "STRING", 
        "OPERATOR", "PUNCTUATION", "EOF", "UNKNOWN"
    };
    for (int i = 0; i < count; i++) {
        printf("Line %d: %-12s '%s'\n", tokens[i].line, typeNames[tokens[i].type], tokens[i].value);
    }
}
