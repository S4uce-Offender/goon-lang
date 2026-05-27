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
            return "ASSIGNMENT OP";
        case TOK_GREATER:
            return "GREATER";
        case TOK_GREATER_EQUALS:
            return "GREATER EQUALS";
        case TOK_LESS:
            return "LESS";
        case TOK_LESS_EQUALS:
            return "LESS EQUALS";
        case TOK_EQUALS:
            return "EQUALS";
        case TOK_NOT_EQUALS:
            return "NOT EQUALS";
        case TOK_LBRACE:
            return "LEFT BRACE";
        case TOK_RBRACE:
            return "RIGHT BRACE";
        case TOK_IF:
            return "TOK_IF";
        case TOK_ELSE:
            return "TOK_ELSE";
        case TOK_THEN:
            return "TOK_THEN";
        case TOK_TRUE:
            return "TOK_TRUE";
        case TOK_FALSE:
            return "TOK_FALSE";
        case TOK_AND:
            return "TOK_AND";
        case TOK_OR:
            return "TOK_OR";
        case TOK_WHILE:
            return "TOK_WHILE";
        case TOK_DO:
            return "TOK_DO";
        case TOK_FUNC:
            return "TOK_FUNC";
        case TOK_NULL:
            return "TOK_NULL";
        case TOK_END:
            return "TOK_END";
        case TOK_UNKNOWN:
            return "UNKNOWN";
    }
}
void printToken(struct Lexer* lex, struct Token* token) {
    printf("(%s, %.*s, %ld)\n", tokenTypeToString(token->token_type),
           (int)token->len, token->lexeme, token->line);
}

void daAppend(struct TokenDA* arr, struct Token tok) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(struct Token));
    }

    arr->data[arr->size++] = tok;
}

void daInit(struct TokenDA* arr, uint64_t tok_arr_capacity) {
    // Min array size must be 1024
    if (tok_arr_capacity < 1024) tok_arr_capacity = 1024;

    arr->data = calloc(tok_arr_capacity, sizeof(struct Token));
    arr->daAppend = daAppend;
    arr->size = 0;
    arr->capacity = tok_arr_capacity;
}

char advance(struct Lexer* lex) {
    char ch = lex->source[lex->current_char++];
    return ch;
}

char peek(struct Lexer* lex) {
    if (lex->current_char >= lex->source_len) return '\0';

    return lex->source[lex->current_char];
}

char lookahead(struct Lexer* lex, uint8_t n) {
    if (lex->current_char + n >= lex->source_len) return '\0';
    return lex->source[lex->current_char + n];
}

bool match(struct Lexer* lex, char expected) {
    if (lex->current_char >= lex->source_len) return false;
    if (lex->source[lex->current_char] != expected) return false;

    lex->current_char++;
    return true;
}

bool isWhitespace(struct Lexer* lex) {
    if (lex->source[lex->current_char] == ' ')
        return true;
    else
        return false;
}

void skipWhitespace(struct Lexer* lex) {
    while (isWhitespace(lex)) {
        lex->current_char++;
    }
}

bool isDigit(char ch) { return (ch >= '0' && ch <= '9'); }

bool isAlpha(char ch) {
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}

bool isAlnum(char ch) { return (isAlpha(ch) || isDigit(ch)); }

void consumeNum(struct Lexer* lex) {
    uint8_t lex_len = 1;
    while (isDigit(lex->peek(lex))) {
        lex->advance(lex);
        lex_len++;
    }

    if (lex->peek(lex) == '.' && isDigit(lex->lookahead(lex, 1))) {
        lex->advance(lex);
        lex_len++;
        while (isDigit(lex->peek(lex))) {
            lex->advance(lex);
            lex_len++;
        }
        lex->tokens.daAppend(
            &lex->tokens,
            (struct Token){.lexeme = lex->source + lex->start_char,
                           .len = lex_len,
                           .token_type = TOK_FLOAT,
                           .line = lex->line});
    } else {
        lex->tokens.daAppend(
            &lex->tokens,
            (struct Token){.lexeme = lex->source + lex->start_char,
                           .len = lex_len,
                           .token_type = TOK_INT,
                           .line = lex->line});
    }
}

void consumeString(struct Lexer* lex) {
    uint8_t lex_len = 0;
    while (lex->peek(lex) != '"' && !(lex->current_char >= lex->source_len)) {
        lex->advance(lex);
        lex_len++;
    }

    if (lex->current_char >= lex->source_len) {
        perror("Unterminated string");
        freeLexer(lex);
        raise(SIGINT);
    }

    lex->advance(lex);
    lex->tokens.daAppend(&lex->tokens,
                         (struct Token){.lexeme = lex->source + lex->start_char,
                                        .len = lex_len,
                                        .token_type = TOK_STRING,
                                        .line = lex->line});
}

uint64_t djb2Hash(char* string) {
    uint64_t hash = 5381;
    int c;

    while ((c = *string++)) {
        hash = ((hash << 5) + hash) + c;
    }

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
    uint8_t keyword_size = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < keyword_size; i++) {
        uint64_t index = murmurOOATHash(keywords[i], HASH) % HASH_TABLE_SIZE;
        if (hash_table[index] > 0) {
            printf("Collision in hash table!");
        }

        hash_table[index] = keyword_type[i];
    }
}

uint64_t getKeywordType(char* string) {
    uint64_t index = murmurOOATHash(string, HASH) % HASH_TABLE_SIZE;

    return hash_table[index];
}

void consumeIdentifier(struct Lexer* lex) {
    char* lexeme;
    uint64_t keyword_type;

    uint8_t lex_len = 1;
    while (isAlnum(lex->peek(lex)) || lex->peek(lex) == '_') {
        lex->advance(lex);
        lex_len++;
    }

    lexeme = createLexeme(lex, lex_len);
    keyword_type = getKeywordType(lexeme);

    if (keyword_type > 0) {
        lex->tokens.daAppend(
            &lex->tokens,
            (struct Token){.lexeme = lex->source + lex->start_char,
                           .len = lex_len,
                           .token_type = keyword_type,
                           .line = lex->line});
    } else {
        lex->tokens.daAppend(
            &lex->tokens,
            (struct Token){.lexeme = lex->source + lex->start_char,
                           .len = lex_len,
                           .token_type = TOK_IDENTIFIER,
                           .line = lex->line});
    }

    free(lexeme);
}

char* createLexeme(struct Lexer* lex, uint8_t lexeme_len) {
    char* lexeme = malloc(lexeme_len + 1);
    strncpy(lexeme, lex->source + lex->start_char, lexeme_len);
    lexeme[lexeme_len] = '\0';
    return lexeme;
}

void tokenize(struct Lexer* lex) {
    uint32_t current_col = -1;
    uint32_t current_line_len = 0;

    while (lex->current_char < lex->source_len) {
        skipWhitespace(lex);
        lex->start_char = lex->current_char;
        current_col++;
        current_line_len++;

        char ch = lex->advance(lex);

        uint8_t lex_len = 1;

        if (isDigit(ch)) {
            consumeNum(lex);
        } else if (isAlpha(ch) || ch == '_') {
            consumeIdentifier(lex);
        } else {
            switch (ch) {
                case '"':
                    lex->start_char++;
                    consumeString(lex);
                    break;

                case '+':
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){.lexeme = lex->source + lex->start_char,
                                       .len = lex_len,
                                       .token_type = TOK_PLUS,
                                       .line = lex->line});
                    break;

                case '-':
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){.lexeme = lex->source + lex->start_char,
                                       .len = lex_len,
                                       .token_type = TOK_MINUS,
                                       .line = lex->line});
                    break;

                case '*':
                    if (lex->match(lex, '*')) {
                        lex_len++;
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_POW,
                                .line = lex->line});
                    } else {
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_TIMES,
                                .line = lex->line});
                    }
                    break;

                case '/':
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){.lexeme = lex->source + lex->start_char,
                                       .len = lex_len,
                                       .token_type = TOK_DIV,
                                       .line = lex->line});
                    break;

                case '(':
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){.lexeme = lex->source + lex->start_char,
                                       .len = lex_len,
                                       .token_type = TOK_LBRACE,
                                       .line = lex->line});
                    break;

                case ')':
                    lex->tokens.daAppend(
                        &lex->tokens,
                        (struct Token){.lexeme = lex->source + lex->start_char,
                                       .len = lex_len,
                                       .token_type = TOK_RBRACE,
                                       .line = lex->line});
                    break;

                case '\n':
                    lex->line++;
                    current_col = 0;
                    lex->where_firstchar = lex->current_char;
                    break;

                case '#':
                    while (lex->peek(lex) != '\n') lex->advance(lex);
                    break;

                case '<':
                    if (lex->match(lex, '=')) {
                        lex_len++;
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_LESS_EQUALS,
                                .line = lex->line});
                        lex->advance(lex);
                    } else {
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_LESS,
                                .line = lex->line});
                    }
                    break;

                case '>':
                    if (lex->match(lex, '=')) {
                        lex_len++;
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_GREATER_EQUALS,
                                .line = lex->line});
                        lex->advance(lex);
                    } else {
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_GREATER,
                                .line = lex->line});
                    }
                    break;

                case '~':
                    if (lex->match(lex, '=')) {
                        lex_len++;
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_NOT_EQUALS,
                                .line = lex->line});
                    } else {
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_NOT,
                                .line = lex->line});
                    }
                    break;

                case '=':
                    if (lex->match(lex, '=')) {
                        lex_len++;
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_EQUALS,
                                .line = lex->line});
                    } else {
                        lex->tokens.daAppend(
                            &lex->tokens,
                            (struct Token){
                                .lexeme = lex->source + lex->start_char,
                                .len = lex_len,
                                .token_type = TOK_ASSIGNMENT,
                                .line = lex->line});
                    }
                    break;

                default:
                    char error_msg[20];
                    sprintf(error_msg, "Unknown symbol '%c'", ch);

                    lexerThrowError(LEX_ERR_UNKNOWN_SYMBOL, error_msg, lex);

                    break;
            }
        }
    }
}

void initLexerDefault(struct Lexer* lex, char* text_source,
                      uint64_t source_len) {
    hashKeywords();
    lex->source = text_source;
    lex->source_len = source_len;
    lex->line = 1;
    lex->start_char = 0;
    lex->current_char = 0;
    lex->where_firstchar = 0;

    daInit(&lex->tokens, 1024);

    lex->advance = advance;
    lex->peek = peek;
    lex->lookahead = lookahead;
    lex->match = match;
    lex->tokenize = tokenize;
}

void initLexerManual(struct Lexer* lex, char* text_source, uint64_t source_len,
                     uint64_t tok_arr_capacity) {
    hashKeywords();

    lex->source = text_source;
    lex->source_len = source_len;
    lex->line = 1;
    lex->start_char = 0;
    lex->current_char = 0;
    lex->where_firstchar = 0;

    daInit(&lex->tokens, tok_arr_capacity);

    lex->advance = advance;
    lex->peek = peek;
    lex->lookahead = lookahead;
    lex->match = match;
    lex->tokenize = tokenize;
}

void freeLexer(struct Lexer* lex) { free(lex->tokens.data); }
