#ifndef LEXER_H
#define LEXER_H

#include "common.h"

#include <stdbool.h>
#include <ctype.h>

typedef enum {
    TK_LITERAL, // text
    TK_VAR, // $variable
    TK_COLON, // :
    TK_NEWLINE, // new line

    TK_COUNT_
}Naredi_Token_Type;

char* naredi_token_type_to_str(Naredi_Token_Type type);

typedef struct {
    Naredi_Token_Type type;
    Naredi_Small_String value;
    // TODO: position
}Naredi_Token;

typedef struct {
    const char* start;
    int len, pointer;
}Naredi_Lexer;

#define naredi_lexer_current(lexer) (lexer)->start[(lexer)->pointer]

void naredi_lexer_trim_left(Naredi_Lexer* lexer);

bool naredi_lexer_next(Naredi_Lexer* lexer, Naredi_Token* out_token);
bool naredi_lexer_expect(Naredi_Lexer* lexer, Naredi_Token_Type type, Naredi_Token* out_token);

#endif // LEXER_H
