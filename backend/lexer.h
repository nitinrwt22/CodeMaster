#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[128];
    int line;
} Token;

Token* tokenize_file(const char* filename, int* token_count);
void print_tokens(Token* tokens, int count);

#endif // LEXER_H
