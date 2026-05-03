#ifndef LEXER_H
#define LEXER_H

typedef enum {
    /* Literálok */
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENT,

    /* Operátorok */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUALS,
    TOKEN_EQEQ,

    /* Kulcsszavak */
    TOKEN_VAR,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FUNCTION,
    TOKEN_RETURN,

    /* Egyéb */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
    int line;
} Token;

typedef struct {
    const char* src;
    int pos;
    int line;
} Lexer;

void lexer_init(Lexer* l, const char* src);
Token lexer_next(Lexer* l);

#endif