#include "errors.h"

#include "lexer.h"

static uint64_t printRemainingLine(char* start) {
    uint64_t chars_printed = 0;
    while (*start != '\n') {
        putchar(*start++);
        chars_printed++;
    }

    return chars_printed;
}

static uint64_t printRemainingLineAsErr(char* start) {
    printf("%s", BRED);
    uint64_t chars_printed = printRemainingLine(start);
    printf("%s", COLOR_RESET);

    return chars_printed;
}
static void printSpaces(uint32_t num) {
    while (num--) putchar(' ');
}

static void printTildes(uint32_t num) {
    while (num--) putchar('~');
}

void lexerThrowError(enum LexerErrorType error_type, char* error_message,
                     struct SourceFile* source, uint64_t line,
                     uint64_t where_firstchar, uint64_t offset) {
    printf("LEXING ERROR: %s\n%ld |\t", error_message, line);
    uint64_t where_symbol = where_firstchar + offset;
    REPL_HAS_ERROR = true;

    switch (error_type) {
        case LEX_ERR_UNKNOWN_SYMBOL:
            // Print everything before symbol
            printf("%.*s", (int)offset, &source->text[where_firstchar]);
            printf("%s%c%s", BRED, source->text[where_symbol], COLOR_RESET);
            printRemainingLine(&source->text[where_symbol + 1]);
            printf("\n  |\t");
            printSpaces(offset);
            printf("%s^%s", BRED, COLOR_RESET);
            break;

        case LEX_ERR_UNTERMINATED_STR:
            printRemainingLine(&source->text[where_firstchar]);
            printf("%s\"%s", BRED, COLOR_RESET);
            break;
    }

    printf("\n");

    if (!REPL_MODE) exit(1);
}

void parserThrowError(enum ParserErrorType error_type, char* error_message,
                      struct SourceFile* source, uint64_t line,
                      uint64_t where_firstchar, uint64_t offset) {
    printf("PARSING ERROR: %s\n%ld |\t", error_message, line);
    uint64_t where_symbol = where_firstchar + offset;
    uint64_t chars_printed;
    REPL_HAS_ERROR = true;

    switch (error_type) {
        case PRSR_ERR_MISSING_LBRACE:
            printf("%.*s", (int)offset, &source->text[where_firstchar]);
            printf("%s%c%s", BRED, source->text[where_symbol], COLOR_RESET);
            printf("\n  |\t");
            printSpaces(offset);
            printf("%s^%s", BRED, COLOR_RESET);
            break;

        case PRSR_ERR_MISSING_RBRACE:
            printf("%.*s", (int)offset, &source->text[where_firstchar]);
            chars_printed =
                printRemainingLineAsErr(&source->text[where_symbol]);
            printf("\n  |\t");
            printSpaces(offset);
            printf("%s^", BRED);
            printTildes(chars_printed - 1);
            printf("%s", COLOR_RESET);
            break;
        case PRSR_ERR_UNEXPECTED_TOKEN:
            printf("%.*s", (int)offset, &source->text[where_firstchar]);
            chars_printed =
                printRemainingLineAsErr(&source->text[where_symbol]);
            printf("\n  |\t");
            printSpaces(offset);
            printf("%s^", BRED);
            printTildes(chars_printed - 1);
            printf("%s", COLOR_RESET);
    }

    printf("\n");

    if (!REPL_MODE) exit(1);
}

void interpreterThrowError(enum InterpreterErrorType error_type,
                           char* error_message, struct SourceFile* source,
                           struct Token* tok1, struct Token* tok2) {
    printf("INTERPRETING ERROR: %s\n%ld |\t", error_message, tok1->line);
    REPL_HAS_ERROR = true;

    if (tok1->token_type == TOK_STRING) {
        tok1->lexeme--;
        tok1->len += 2;
    }

    switch (error_type) {
        case INTPTR_ERR_UNSUPPORTED_UN_OP:
            printf("%.*s", (int)tok1->offset, &source->text[tok1->line_start]);
            printf("%s%.*s%s", BRED, (int)tok1->len, tok1->lexeme, COLOR_RESET);
            printRemainingLine(
                &source->text[tok1->line_start + tok1->offset + tok1->len]);
            printf("\n  |\t");
            printSpaces(tok1->offset);
            printf("%s^", BRED);
            printTildes(tok1->len - 1);
            printf("%s", COLOR_RESET);
            break;
        case INTPTR_ERR_UNSUPPORTED_BIN_OP:
            // Spaghetti slop (human slop) code
            if (tok2->token_type == TOK_STRING) {
                tok2->lexeme--;
                tok2->len += 2;
            }

            uint8_t space_between_toks =
                tok2->offset - (tok1->offset + tok1->len);
            printf("%.*s", (int)tok1->offset, &source->text[tok1->line_start]);
            printf("%s%.*s%s", BBLU, (int)tok1->len, tok1->lexeme, COLOR_RESET);
            printf("%s%.*s%s", BRED, (int)space_between_toks,
                   tok1->lexeme + tok1->len, COLOR_RESET);
            printf("%s%.*s%s", BBLU, (int)tok2->len, tok2->lexeme, COLOR_RESET);
            printRemainingLine(
                &source->text[tok2->line_start + tok2->offset + tok2->len]);
            printf("\n  |\t");
            printSpaces(tok1->offset);
            printf("%s^", BBLU);
            printTildes(tok1->len - 1);
            printSpaces(space_between_toks);
            printf("^");
            printTildes(tok2->len - 1);
            printf("%s", COLOR_RESET);
    }

    printf("\n");

    if (!REPL_MODE) exit(1);
}