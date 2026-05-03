#include "lexer.h"

/* Minimális segédfüggvények (mivel nincs stdlib.h) */
static int is_digit(char c) { return c >= '0' && c <= '9'; }
static int is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static int is_alnum(char c) { return is_alpha(c) || is_digit(c); }
static int is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a++ != *b++) return 0;
    }
    return *a == *b;
}

static void str_copy(char* dst, const char* src, int len) {
    for (int i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    dst[len] = '\0';
}

/* Lexer inicializálása */
void lexer_init(Lexer* l, const char* src) {
    l->src = src;
    l->pos = 0;
    l->line = 1;
}

/* A következő token lekérése */
Token lexer_next(Lexer* l) {
    Token t;
    
    /* Whitespace és Kommentek átugrása */
    while (l->src[l->pos]) {
        // Whitespace (szóköz, tab, új sor)
        if (is_space(l->src[l->pos])) {
            if (l->src[l->pos] == '\n') l->line++;
            l->pos++;
            continue;
        }
        
        // Egysoros kommentek kezelése (//)
        if (l->src[l->pos] == '/' && l->src[l->pos + 1] == '/') {
            while (l->src[l->pos] && l->src[l->pos] != '\n') {
                l->pos++;
            }
            continue;
        }
        
        break; // Megtaláltuk a következő valódi karaktert
    }

    t.line = l->line;
    char c = l->src[l->pos];

    /* Fájl vége (EOF) */
    if (!c) {
        t.type = TOKEN_EOF;
        t.value[0] = '\0';
        return t;
    }

    /* SZÁMOK (pl: 42) */
    if (is_digit(c)) {
        int start = l->pos;
        while (is_digit(l->src[l->pos])) l->pos++;
        str_copy(t.value, l->src + start, l->pos - start);
        t.type = TOKEN_NUMBER;
        return t;
    }

    /* STRINGEK (pl: "hello" vagy 'hello') */
    if (c == '"' || c == '\'') {
        char quote = c;
        l->pos++; // Nyitó idézőjel átugrása
        int start = l->pos;
        while (l->src[l->pos] && l->src[l->pos] != quote) {
            l->pos++;
        }
        str_copy(t.value, l->src + start, l->pos - start);
        if (l->src[l->pos] == quote) l->pos++; // Záró idézőjel átugrása
        t.type = TOKEN_STRING;
        return t;
    }

    /* IDENTIFIERS (nevek) és KULCSSZAVAK (let, if, stb.) */
    if (is_alpha(c)) {
        int start = l->pos;
        while (is_alnum(l->src[l->pos])) l->pos++;
        str_copy(t.value, l->src + start, l->pos - start);

        if (str_eq(t.value, "var"))           t.type = TOKEN_VAR;
        else if (str_eq(t.value, "let"))      t.type = TOKEN_LET;
        else if (str_eq(t.value, "const"))    t.type = TOKEN_CONST;
        else if (str_eq(t.value, "if"))       t.type = TOKEN_IF;
        else if (str_eq(t.value, "else"))     t.type = TOKEN_ELSE;
        else if (str_eq(t.value, "while"))    t.type = TOKEN_WHILE;
        else if (str_eq(t.value, "function")) t.type = TOKEN_FUNCTION;
        else if (str_eq(t.value, "return"))   t.type = TOKEN_RETURN;
        else                                  t.type = TOKEN_IDENT;
        return t;
    }

    /* OPERÁTOROK és írásjelek */
    l->pos++;
    t.value[0] = c;
    t.value[1] = '\0';

    switch (c) {
        case '+': t.type = TOKEN_PLUS;      break;
        case '-': t.type = TOKEN_MINUS;     break;
        case '*': t.type = TOKEN_STAR;      break;
        case '/': t.type = TOKEN_SLASH;     break;
        case '(': t.type = TOKEN_LPAREN;    break;
        case ')': t.type = TOKEN_RPAREN;    break;
        case '{': t.type = TOKEN_LBRACE;    break;
        case '}': t.type = TOKEN_RBRACE;    break;
        case ';': t.type = TOKEN_SEMICOLON; break;
        case ',': t.type = TOKEN_COMMA;     break;
        case '=':
            // Megnézzük, hogy "==" vagy csak "="
            if (l->src[l->pos] == '=') {
                l->pos++;
                t.type = TOKEN_EQEQ;
                t.value[1] = '=';
                t.value[2] = '\0';
            } else {
                t.type = TOKEN_EQUALS;
            }
            break;
        case '>': t.type = TOKEN_UNKNOWN; break; // Itt bővíthető majd a '>' kezelése
        case '<': t.type = TOKEN_UNKNOWN; break; // Itt bővíthető majd a '<' kezelése
        default:  t.type = TOKEN_UNKNOWN;
    }

    return t;
}