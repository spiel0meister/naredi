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

#define FLAG_IMPLEMENTATION
#include "external/flag.h"

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

void usage(FILE* f) {
    fprintf(f, "%s [OPTIONS]\n", flag_program_name());
    flag_print_options(f);
}

// TODO: multiprocessing
// TODO: creating variables
// TODO: if-elif-else, foreach, etc.
int main(int argc, char** argv) {
    bool* help = flag_bool("help", false, "Display this help message");
    bool* version = flag_bool("version", false, "Display the version");
    char** naredifile = flag_str("naredi", "Naredifile", "Path to Naredifile");

    if (!flag_parse(argc, argv)) {
        eprintf("Couldn't parse args\n");
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        usage(stdout);
        return 0;
    }

    if (*version) {
        printf("%s v%s\n", flag_program_name(), NAREDI_VERSION);
        return 0;
    }

    int size;
    char* content = naredi_read_file(*naredifile, &size);
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
