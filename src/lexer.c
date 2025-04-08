#include "common.h"
#include "lexer.h"

#include <stdbool.h>
#include <ctype.h>

char* naredi_token_type_to_str(Naredi_Token_Type type) {
    switch (type) {
        case TK_LITERAL:
            return "literal";
        case TK_VAR:
            return "variable";
        case TK_COLON:
            return "colon";
        case TK_NEWLINE:
            return "new line";
        case TK_COUNT_:
        default:
            unreachable();
    }
}

void naredi_lexer_trim_left(Naredi_Lexer* lexer) {
    while (lexer->pointer < lexer->len && isspace(naredi_lexer_current(lexer)) && naredi_lexer_current(lexer) != '\n') {
        lexer->pointer++;
    }
}

bool naredi_lexer_next(Naredi_Lexer* lexer, Naredi_Token* out_token) {
    naredi_lexer_trim_left(lexer);

    if (lexer->pointer >= lexer->len) return false;
    
    if (naredi_lexer_current(lexer) == '\n') {
        out_token->type = TK_NEWLINE;
        lexer->pointer++;
        return true;
    }

    if (naredi_lexer_current(lexer) == '$') {
        if (lexer->pointer >= lexer->len || !isalpha(lexer->start[lexer->pointer + 1])) {
            eprintf("Missing variable name\n");
            return false;
        }

        int start = lexer->pointer++;
        while (lexer->pointer < lexer->len && isalnum(lexer->start[lexer->pointer])) {
            lexer->pointer++;
        }

        out_token->type = TK_VAR;
        out_token->value = naredi_small_string_from_sized(&lexer->start[start], lexer->pointer - start);

        return true;
    }

    char current = naredi_lexer_current(lexer);
    switch (current) {
        case ':':
            out_token->type = TK_COLON;
            lexer->pointer++;
            return true;
    }

    int start = lexer->pointer++;
    while (lexer->pointer < lexer->len && !isspace(lexer->start[lexer->pointer])) {
        if (naredi_lexer_current(lexer) == ':') break;
        lexer->pointer++;
    }

    out_token->type = TK_LITERAL;
    out_token->value = naredi_small_string_from_sized(&lexer->start[start], lexer->pointer - start);

    return true;
}

bool naredi_lexer_expect(Naredi_Lexer* lexer, Naredi_Token_Type type, Naredi_Token* out_token) {
    bool ok = naredi_lexer_next(lexer, out_token);
    if (!ok) {
        eprintf("Unexpected EOF\n");
        return false;
    } else if (out_token->type != type) {
        eprintf("Unexpected %s when %s was expected\n", naredi_token_type_to_str(out_token->type), naredi_token_type_to_str(type));
        return false;
    }

    return true;
}

