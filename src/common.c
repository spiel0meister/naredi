#include <stdlib.h>
#include <string.h>

#include "common.h"

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

Naredi_Small_String naredi_small_string_from_cstr(const char* str) {
    Naredi_Small_String ss = {0};
    for (const char* p = str; *p != 0; ++p) {
        int i = p - str;
        if (i > 127) break;
        ss.value[i] = *p;
    }
    ss.value[127] = 0;

    return ss;
}

Naredi_Small_String naredi_small_string_from_sized(const char* start, int len) {
    if (len > 127) len = 127;

    Naredi_Small_String ss = {0};
    for (int i = 0; i < len; ++i) {
        ss.value[i] = start[i];
    }
    ss.value[127] = 0;

    return ss;
}

// TODO: we always allocate max size of string, so we "leak" some memory
// TODO: use arenas
char* naredi_small_string_to_cstr(Naredi_Small_String string) {
    char* copy = malloc(sizeof(string.value));
    memcpy(copy, string.value, sizeof(string.value));
    return copy;
}

