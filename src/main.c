#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>

#include "lexer.h"
#include "rule.h"

// See https://www.wikiwand.com/en/articles/Single_compilation_unit
#include "common.c"
#include "lexer.c"
#include "rule.c"

#define NAREDI_VERSION "0.0.1"

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
            pid_t pid = naredi_rule_start(*rule);
            if (pid == -1) return 1;
            if (!naredi_rule_wait(pid)) return 1;
        }

        da_foreach(&rules, Naredi_Rule, rule) {
            naredi_rule_destroy(rule);
        }
        da_free(&rules);
    }
    return 0;
}
