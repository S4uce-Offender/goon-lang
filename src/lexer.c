#include "lexer.h"

uint64_t hash_table[HASH_TABLE_SIZE] = {0};

uint64_t keyword_type[] = {
    TOK_IF, TOK_ELSE,  TOK_THEN, TOK_TRUE, TOK_FALSE, TOK_AND,
    TOK_OR, TOK_WHILE, TOK_DO,   TOK_FUNC, TOK_NULL,  TOK_END,
};

char* keywords[] = {"if", "else",  "then", "true", "false", "and",
                    "or", "while", "do",   "func", "null",  "end"};

char* tokenTypeToString(enum TokenType type) {
    switch (type) {
        case TOK_IDENTIFIER:
            return "IDENTIFIER";
        case TOK_INT:
            return "INT";
        case TOK_FLOAT:
            return "FLOAT";
        case TOK_STRING:
            return "STRING";
        case TOK_PLUS:
            return "PLUS";
        case TOK_MINUS:
            return "MINUS";
        case TOK_TIMES:
            return "TIMES";
        case TOK_DIV:
            return "DIV";
        case TOK_POW:
            return "POW";
        case TOK_NOT:
            return "NOT";
        case TOK_ASSIGNMENT:
            return "ASSIGNMENT";
        case TOK_GREATER:
            return "GREATER";
        case TOK_GREATER_EQUALS:
            return "GREATER_EQUALS";
        case TOK_LESS:
            return "LESS";
        case TOK_LESS_EQUALS:
            return "LESS_EQUALS";
        case TOK_EQUALS:
            return "EQUALS";
        case TOK_NOT_EQUALS:
            return "NOT_EQUALS";
        case TOK_LBRACE:
            return "LEFT_BRACE";
        case TOK_RBRACE:
            return "RIGHT_BRACE";
        case TOK_IF:
            return "IF";
        case TOK_ELSE:
            return "ELSE";
        case TOK_THEN:
            return "THEN";
        case TOK_TRUE:
            return "TRUE";
        case TOK_FALSE:
            return "FALSE";
        case TOK_AND:
            return "AND";
        case TOK_OR:
            return "OR";
        case TOK_WHILE:
            return "WHILE";
        case TOK_DO:
            return "DO";
        case TOK_FUNC:
            return "FUNC";
        case TOK_NULL:
            return "NULL";
        case TOK_END:
            return "END";
        case TOK_EOF:
            return "EOF";
        case TOK_UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}

void printToken(struct Lexer* lex, struct Token* token) {
    printf("(%s, %.*s, %lu)\n", tokenTypeToString(token->token_type),
           (int)token->len, token->lexeme, token->line);
}

void daAppend(struct TokenDA* arr, struct Token tok) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;

        arr->data = realloc(arr->data, arr->capacity * sizeof(struct Token));
    }

    arr->data[arr->size++] = tok;
}

void daInit(struct TokenDA* arr, uint64_t tok_arr_capacity) {
    if (tok_arr_capacity < 1024) tok_arr_capacity = 1024;

    arr->data = calloc(tok_arr_capacity, sizeof(struct Token));

    arr->size = 0;
    arr->capacity = tok_arr_capacity;
    arr->daAppend = daAppend;
}

char advance(struct Lexer* lex) {
    return lex->source->text[lex->current_char++];
}

char peek(struct Lexer* lex) {
    if (lex->current_char >= lex->source->length) return '\0';

    return lex->source->text[lex->current_char];
}

char lookahead(struct Lexer* lex, uint8_t n) {
    if (lex->current_char + n >= lex->source->length) return '\0';

    return lex->source->text[lex->current_char + n];
}

bool match(struct Lexer* lex, char expected) {
    if (lex->current_char >= lex->source->length) return false;

    if (lex->source->text[lex->current_char] != expected) return false;

    lex->current_char++;
    return true;
}

bool isWhitespace(struct Lexer* lex) {
    char ch = peek(lex);

    return (ch == ' ' || ch == '\t' || ch == '\r');
}

bool isDigit(char ch) { return (ch >= '0' && ch <= '9'); }

bool isAlpha(char ch) {
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

bool isAlnum(char ch) { return isAlpha(ch) || isDigit(ch); }

void skipWhitespace(struct Lexer* lex) {
    while (isWhitespace(lex)) lex->current_char++;
}

void consumeNum(struct Lexer* lex) {
    uint8_t lex_len = 1;

    while (isDigit(peek(lex))) {
        advance(lex);
        lex_len++;
    }

    enum TokenType type = TOK_INT;

    if (peek(lex) == '.' && isDigit(lookahead(lex, 1))) {
        type = TOK_FLOAT;

        advance(lex);
        lex_len++;

        while (isDigit(peek(lex))) {
            advance(lex);
            lex_len++;
        }
    }

    lex->tokens.daAppend(
        &lex->tokens,
        (struct Token){
            .lexeme = lex->source->text + lex->start_char,

            .line_start = lex->where_firstchar,
            .offset = (lex->current_char - lex->where_firstchar - 1),

            .line = lex->line,
            .len = lex_len,
            .token_type = type,
        });
}

void consumeString(struct Lexer* lex) {
    uint8_t lex_len = 0;

    while (peek(lex) != '"' && lex->current_char < lex->source->length) {
        if (peek(lex) == '\n') lex->line++;

        advance(lex);
        lex_len++;
    }

    if (lex->current_char >= lex->source->length) {
        lexerThrowError(LEX_ERR_UNTERMINATED_STR, "Unterminated string",
                        lex->source, lex->line, lex->where_firstchar,
                        (lex->current_char - lex->where_firstchar - 1));
        return;
    }

    advance(lex);

    lex->tokens.daAppend(
        &lex->tokens,
        (struct Token){
            .lexeme = lex->source->text + lex->start_char + 1,

            .line_start = lex->where_firstchar,
            .offset = (lex->current_char - lex->where_firstchar - 1),

            .line = lex->line,
            .len = lex_len,
            .token_type = TOK_STRING,
        });
}

uint64_t djb2Hash(char* string) {
    uint64_t hash = 5381;

    int c;

    while ((c = *string++)) hash = ((hash << 5) + hash) + c;

    return hash;
}

uint64_t murmurOOATHash(char* string, uint32_t h) {
    for (; *string; ++string) {
        h ^= *string;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }

    return h;
}

void hashKeywords() {
    uint64_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);

    for (uint64_t i = 0; i < keyword_count; i++) {
        uint64_t index = murmurOOATHash(keywords[i], HASH) % HASH_TABLE_SIZE;

        hash_table[index] = keyword_type[i];
    }
}

uint64_t getKeywordType(char* string) {
    uint64_t index = murmurOOATHash(string, HASH) % HASH_TABLE_SIZE;

    return hash_table[index];
}

char* createLexeme(struct Lexer* lex, uint8_t lexeme_size) {
    char* lexeme = malloc(lexeme_size + 1);

    strncpy(lexeme, lex->source->text + lex->start_char, lexeme_size);

    lexeme[lexeme_size] = '\0';

    return lexeme;
}

void consumeIdentifier(struct Lexer* lex) {
    uint8_t lex_len = 1;

    while (isAlnum(peek(lex)) || peek(lex) == '_') {
        advance(lex);
        lex_len++;
    }

    char* lexeme = createLexeme(lex, lex_len);

    uint64_t keyword = getKeywordType(lexeme);

    lex->tokens.daAppend(
        &lex->tokens,
        (struct Token){
            .lexeme = lex->source->text + lex->start_char,

            .line_start = lex->where_firstchar,
            .offset = (lex->current_char - lex->where_firstchar - 1),

            .line = lex->line,
            .len = lex_len,

            .token_type = (keyword > 0) ? keyword : TOK_IDENTIFIER,
        });

    free(lexeme);
}

void tokenize(struct Lexer* lex) {
    while (lex->current_char < lex->source->length) {
        skipWhitespace(lex);

        lex->start_char = lex->current_char;

        char ch = advance(lex);

        uint8_t lex_len = 1;

        if (isDigit(ch)) {
            consumeNum(lex);
            continue;
        }

        if (isAlpha(ch) || ch == '_') {
            consumeIdentifier(lex);
            continue;
        }

        switch (ch) {
            case '"':
                consumeString(lex);
                break;

            case '+':
                lex->tokens.daAppend(
                    &lex->tokens,
                    (struct Token){
                        .lexeme = lex->source->text + lex->start_char,

                        .line_start = lex->where_firstchar,
                        .offset =
                            (lex->current_char - lex->where_firstchar - 1),

                        .line = lex->line,
                        .len = 1,
                        .token_type = TOK_PLUS,
                    });
                break;

            case '-':
                lex->tokens.daAppend(
                    &lex->tokens,
                    (struct Token){
                        .lexeme = lex->source->text + lex->start_char,

                        .line_start = lex->where_firstchar,
                        .offset =
                            (lex->current_char - lex->where_firstchar - 1),

                        .line = lex->line,
                        .len = 1,
                        .token_type = TOK_MINUS,
                    });
                break;

            case '*':
                if (match(lex, '*')) {
                    lex_len = 2;

                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){
                            .lexeme = lex->source->text + lex->start_char,

                            .line_start = lex->where_firstchar,
                            .offset =
                                (lex->current_char - lex->where_firstchar - 1),

                            .line = lex->line,
                            .len = lex_len,
                            .token_type = TOK_POW,
                        });

                } else {
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){
                            .lexeme = lex->source->text + lex->start_char,

                            .line_start = lex->where_firstchar,
                            .offset =
                                (lex->current_char - lex->where_firstchar - 1),

                            .line = lex->line,
                            .len = 1,
                            .token_type = TOK_TIMES,
                        });
                }

                break;

            case '/':
                lex->tokens.daAppend(
                    &lex->tokens,
                    (struct Token){
                        .lexeme = lex->source->text + lex->start_char,

                        .line_start = lex->where_firstchar,
                        .offset =
                            (lex->current_char - lex->where_firstchar - 1),

                        .line = lex->line,
                        .len = 1,
                        .token_type = TOK_DIV,
                    });

                break;

            case '(':
                lex->tokens.daAppend(
                    &lex->tokens,
                    (struct Token){
                        .lexeme = lex->source->text + lex->start_char,

                        .line_start = lex->where_firstchar,
                        .offset =
                            (lex->current_char - lex->where_firstchar - 1),

                        .line = lex->line,
                        .len = 1,
                        .token_type = TOK_LBRACE,
                    });

                break;

            case ')':
                lex->tokens.daAppend(
                    &lex->tokens,
                    (struct Token){
                        .lexeme = lex->source->text + lex->start_char,

                        .line_start = lex->where_firstchar,
                        .offset =
                            (lex->current_char - lex->where_firstchar - 1),

                        .line = lex->line,
                        .len = 1,
                        .token_type = TOK_RBRACE,
                    });

                break;

            case '\n':
                lex->line++;
                lex->where_firstchar = lex->current_char;
                break;

            case '#':
                while (peek(lex) != '\n' && peek(lex) != '\0') {
                    advance(lex);
                }

                break;

            default: {
                char error_msg[64];

                sprintf(error_msg, "Unknown symbol '%c'", ch);

                lexerThrowError(LEX_ERR_UNKNOWN_SYMBOL, error_msg, lex->source,
                                lex->line, lex->where_firstchar,
                                (lex->current_char - lex->where_firstchar - 1));

                break;
            }
        }
    }

    lex->tokens.daAppend(
        &lex->tokens, (struct Token){
                          .lexeme = lex->source->text + lex->current_char,

                          .line_start = lex->where_firstchar,
                          .offset = (lex->current_char - lex->where_firstchar),

                          .line = lex->line,
                          .len = 0,
                          .token_type = TOK_EOF,
                      });
}

void initLexerDefault(struct Lexer* lex, struct SourceFile* source) {
    hashKeywords();

    lex->source = source;

    lex->line = 1;
    lex->start_char = 0;
    lex->current_char = 0;
    lex->where_firstchar = 0;

    daInit(&lex->tokens, 1024);

    lex->advance = advance;
    lex->peek = peek;
    lex->lookahead = lookahead;
    lex->match = match;

    lex->isWhitespace = isWhitespace;
    lex->isDigit = isDigit;
    lex->isAlpha = isAlpha;
    lex->isAlnum = isAlnum;

    lex->skipWhitespace = skipWhitespace;

    lex->consumeNum = consumeNum;

    lex->tokenize = tokenize;
}

void initLexerManual(struct Lexer* lex, struct SourceFile* source,
                     uint64_t tok_arr_capacity) {
    initLexerDefault(lex, source);

    daInit(&lex->tokens, tok_arr_capacity);
}

void freeLexer(struct Lexer* lex) { free(lex->tokens.data); }