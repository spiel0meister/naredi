#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>

#include "rule.h"
#include "common.h"
#include "lexer.h"

bool naredi_jobs_wait(Naredi_Jobs* jobs) {
    for (size_t i = 0; i < jobs->count; ++i) {
        if (!naredi_rule_wait(jobs->items[i])) {
            eprintf("Couldn't wait for job\n");
            return false;
        }

        da_remove(jobs, i);
        i--;
    }
    return true;
}

void naredi_rule_destroy(Naredi_Rule* rule) {
    da_free(&rule->in);
    da_free(&rule->deps);
    da_free(&rule->cmd);
}

bool naredi_parse_rule(Naredi_Lexer* lexer, Naredi_Rule* out_rule) {
    do {
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

            return true;
        } else if (t.type == TK_NEWLINE) {
            // ignore
        } else {
            eprintf("Unexpected %s\n", naredi_token_type_to_str(t.type));
            return false;
        }
    } while (true);

    return false;
}

pid_t naredi_rule_start(Naredi_Jobs* jobs, size_t procs_to_use, Naredi_Rule rule) {
    if (jobs->count >= procs_to_use) 
        if (!naredi_jobs_wait(jobs)) return -1;

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

    da_foreach(&copy, char*, arg) {
        free(*arg);
    }
    da_free(&copy);

    da_append(jobs, pid);
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

