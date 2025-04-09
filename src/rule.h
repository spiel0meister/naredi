#ifndef RULE_H
#define RULE_H

#include <sys/types.h>

#include "common.h"
#include "lexer.h"

typedef struct {
    pid_t* items;
    size_t count, capacity;
}Naredi_Jobs;

bool naredi_jobs_wait(Naredi_Jobs* jobs);

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

Naredi_Rule* naredi_find_rule_for_out(Naredi_Rules* rules, Naredi_Small_String out);
pid_t naredi_rule_start(Naredi_Jobs* jobs, size_t procs_to_use, Naredi_Rules* rules, Naredi_Rule rule);
bool naredi_rule_wait(pid_t pid);

#endif // RULE_H
