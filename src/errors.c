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

    exit(1);
}

void parserThrowError(enum ParserErrorType error_type, char* error_message,
                      struct SourceFile* source, uint64_t line,
                      uint64_t where_firstchar, uint64_t offset) {
    printf("PARSING ERROR: %s\n%ld |\t", error_message, line);
    uint64_t where_symbol = where_firstchar + offset;

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
            uint64_t chars_printed =
                printRemainingLineAsErr(&source->text[where_symbol]);
            printf("\n  |\t");
            printSpaces(offset);
            printf("%s^", BRED);
            printTildes(chars_printed);
            printf("%s", COLOR_RESET);
            break;
        case PRSR_ERR_UNEXPECTED_TOKEN:
            break;
    }

    printf("\n");

    exit(1);
}

void interpreterThrowError(enum InterpreterErrorType error_type,
                           char* error_message, struct SourceFile* source,
                           struct Token* tok1, struct Token* tok2) {
    printf("INTERPRETING ERROR: %s\n%ld |\t", error_message, tok1->line);

    switch (error_type) {
        case INTPTR_ERR_UNSUPPORTED_UN_OP:
            break;
        case INTPTR_ERR_UNSUPPORTED_BIN_OP:
            break;
    }

    printf("\n");

    exit(1);
}