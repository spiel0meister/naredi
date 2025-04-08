#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>

#include "lexer.h"

#include "common.c"
#include "lexer.c"

char* naredi_read_file(const char* filepath, int* out_size) {
    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        eprintf("Couldn't open %s: %s\n", filepath, strerror(errno));
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int n = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(n + 1);
    assert(content != NULL);
    
    if (out_size != NULL) *out_size = n;

    fread(content, 1, n, f);
    content[n] = 0;

    fclose(f);
    return content;
}

typedef struct {
    char** items;
    size_t count, capacity;
}Str_Array;

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

void naredi_rule_destroy(Naredi_Rule* rule) {
    da_free(&rule->in);
    da_free(&rule->deps);
    da_free(&rule->cmd);
}

bool naredi_parse_rule(Naredi_Lexer* lexer, Naredi_Rule* out_rule) {
    Naredi_Token t = {0};
    if (!naredi_lexer_next(lexer, &t)) return false;

    if (t.type == TK_LITERAL) {
        out_rule->out = t.value;
        if (!naredi_lexer_expect(lexer, TK_COLON, &t)) return false;

        while (naredi_lexer_next(lexer, &t) && t.type != TK_NEWLINE) {
            da_append(&out_rule->in, t.value);
        }

        if (naredi_lexer_next(lexer, &t)) {
            da_append(&out_rule->cmd, t.value);

            while (naredi_lexer_next(lexer, &t) && t.type != TK_NEWLINE) {
                da_append(&out_rule->cmd, t.value);
            }
        }
    } else {
        eprintf("%s\n", naredi_token_type_to_str(t.type));
    }

    return true;
}

void naredi_cmd_render(Str_Array cmd) {
    da_foreach(&cmd, char*, arg) {
        if (strchr(*arg, ' ') != NULL) {
            printf("'%s'", *arg);
        } else {
            printf("%s", *arg);
        }
        printf(" ");
    }
    printf("\n");
}

pid_t naredi_rule_start(Naredi_Rule rule) {
    Str_Array copy = {0};

    da_foreach(&rule.cmd, Naredi_Small_String, arg) {
        if (arg->value[0] == '$') {
            if (strcmp(arg->value, "$in") == 0) {
                da_foreach(&rule.in, Naredi_Small_String, in) {
                    da_append(&copy, naredi_small_string_to_cstr(*in));
                }
            } else if (strcmp(arg->value, "$out") == 0) {
                da_append(&copy, naredi_small_string_to_cstr(rule.out));
            }  else {
                todol("variables");
            }
        } else {
            da_append(&copy, naredi_small_string_to_cstr(*arg));
        }
    }

    naredi_cmd_render(copy);
    da_append(&copy, NULL);

    pid_t pid = fork();
    if (pid < 0) {
        eprintf("Couldn't fork: %s\n", strerror(errno));
        return pid;
    } else if (pid == 0) {
        execvp(copy.items[0], copy.items);

        eprintf("Couldn't start program: %s\n", strerror(errno));
        exit(1);
    }

    da_free(&copy);
    return pid;
}

bool naredi_rule_wait(pid_t pid) {
    for (;;) {
        int wstatus = 0;
        if (waitpid(pid, &wstatus, 0) < 0) {
            eprintf("Couldn't wait for %d: %s\n", pid, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int estatus = WEXITSTATUS(wstatus);
            if (estatus != 0) {
                eprintf("Process %d exited with %d\n", pid, estatus);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            eprintf("Process %d exited with signal %d\n", pid, WTERMSIG(wstatus));
            return false;
        }
    }

    return true;
}

int main(void) {
    int size;
    char* content = naredi_read_file("Naredifile", &size);
    defer(free(content)) {
        Naredi_Lexer lexer = {
            .start = content,
            .len = size,
            .pointer = 0,
        };

        Naredi_Rules rules = {0};

        Naredi_Rule rule = {0};
        while (naredi_parse_rule(&lexer, &rule)) {
            da_append(&rules, rule);
        }

        da_foreach(&rules, Naredi_Rule, rule) {
#if 0
            printf("%s\n", rule->out.value);

            printf("Ins: %lu\n", rule->in.count);
            printf("Cmd: %lu\n", rule->cmd.count);
#else
            pid_t pid = naredi_rule_start(*rule);
            if (pid == -1) return 1;
            
            if (!naredi_rule_wait(pid)) return 1;
#endif
        }

        da_foreach(&rules, Naredi_Rule, rule) {
            naredi_rule_destroy(rule);
        }
        da_free(&rules);
    }
    return 0;
}
