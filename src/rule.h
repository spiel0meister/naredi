#ifndef RULE_H
#define RULE_H

#include "common.h"
#include "lexer.h"

typedef struct {
    Naredi_Small_String out;
    Naredi_Small_Strings in;
    Naredi_Small_Strings deps;
    Naredi_Small_Strings cmd;
}Naredi_Rule;

typedef struct {
    Naredi_Rule* items;
    size_t count, capacity;
}Naredi_Rules;

void naredi_rule_destroy(Naredi_Rule* rule);
bool naredi_parse_rule(Naredi_Lexer* lexer, Naredi_Rule* out_rule);

pid_t naredi_rule_start(Naredi_Rule rule);
bool naredi_rule_wait(pid_t pid);

#endif // RULE_H
