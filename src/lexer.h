#ifndef LEXER_H
#define LEXER_H

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "source.h"

#define HASH_TABLE_SIZE 257
#define HASH 54

extern uint64_t hash_table[HASH_TABLE_SIZE];
extern uint64_t keyword_type[];
extern char* keywords[];

enum TokenType {
    TOK_IDENTIFIER = 1,
    TOK_INT,
    TOK_FLOAT,
    TOK_STRING,
    TOK_PLUS,
    TOK_MINUS,
    TOK_TIMES,
    TOK_DIV,
    TOK_POW,
    TOK_BOOL_NOT,
    TOK_BOOL_AND,
    TOK_BOOL_OR,
    TOK_BIT_NOT,
    TOK_BIT_AND,
    TOK_BIT_OR,
    TOK_BIT_XOR,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_ASSIGNMENT,
    TOK_GREATER,
    TOK_GREATER_EQUALS,
    TOK_LESS,
    TOK_LESS_EQUALS,
    TOK_EQUALS,
    TOK_NOT_EQUALS,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_IF,
    TOK_ELSE,
    TOK_THEN,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
    TOK_WHILE,
    TOK_DO,
    TOK_FUNC,
    TOK_NULL,
    TOK_END,
    TOK_EOF,
    TOK_UNKNOWN
};

struct Token {
    char* lexeme;
    uint64_t line_start;
    uint64_t offset;
    uint64_t line;
    uint8_t len;
    enum TokenType token_type;
};

struct TokenDA {
    struct Token* data;
    uint64_t size;
    uint64_t capacity;

    void (*daAppend)(struct TokenDA*, struct Token);
};

struct Lexer {
    struct TokenDA tokens;
    struct SourceFile* source;

    uint64_t line;
    uint64_t start_char;
    uint64_t current_char;
    uint64_t where_firstchar;

    char (*advance)(struct Lexer*);
    char (*peek)(struct Lexer*);
    char (*lookahead)(struct Lexer*, uint8_t);
    bool (*match)(struct Lexer*, char);
    bool (*isWhitespace)(struct Lexer*);
    bool (*isDigit)(char);
    bool (*isAlpha)(char);
    bool (*isAlnum)(char);

    void (*skipWhitespace)(struct Lexer*);
    void (*consumeNum)(struct Lexer*);
    void (*tokenize)(struct Lexer*);
};

char* tokenTypeToString(enum TokenType type);
void printToken(struct Lexer* lex, struct Token* token);

void daInit(struct TokenDA* arr, uint64_t tok_arr_size);
void daAppend(struct TokenDA* arr, struct Token tok);

static inline char advance(struct Lexer* lex);
static inline char peek(struct Lexer* lex);
static inline char lookahead(struct Lexer* lex, uint8_t n);
bool match(struct Lexer* lex, char expected);
static inline bool isWhitespace(struct Lexer* lex);
static inline bool isDigit(char ch);
static inline bool isAlpha(char ch);
static inline bool isAlnum(char ch);
static inline void skipWhitespace(struct Lexer* lex);

void consumeNum(struct Lexer* lex);
void consumeString(struct Lexer* lex);
void consumeIdentifier(struct Lexer* lex);

uint64_t djb2Hash(char* string);
uint64_t murmurOOATHash(char* string, uint32_t h);
void hashKeywords();
uint64_t getKeywordType(char* string);

char* createLexeme(struct Lexer* lex, uint8_t lexeme_size);
void createToken(struct Lexer* lex, uint8_t len, enum TokenType token_type);
void tokenize(struct Lexer* lex);

void initLexerDefault(struct Lexer* lex, struct SourceFile* source);
void initLexerManual(struct Lexer* lex, struct SourceFile* source,
                     uint64_t tok_arr_capacity);

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME

#define initLexer(...) \
    GET_MACRO(__VA_ARGS__, initLexerManual, initLexerDefault)(__VA_ARGS__)

void freeLexer(struct Lexer* lex);

#endif