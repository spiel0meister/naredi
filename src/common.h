#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <errno.h>

#define eprintf(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

#define todo() do { eprintf("%s:%d: TODO\n", __FILE__, __LINE__); __builtin_trap(); } while (0)
#define todol(label) do { eprintf("%s:%d: TODO: %s\n", __FILE__, __LINE__, label); __builtin_trap(); } while (0)

#define unused(thing) (void)(thing)
#define unreachable() do { eprintf("%s:%d: UNREACHABLE\n", __FILE__, __LINE__); __builtin_trap(); } while (0)

#define defer(expr) for (int i = 0; i < 1; (i++, (expr)))

#define da_append(da, item) do { \
        if ((da)->count >= (da)->capacity) { \
            if ((da)->capacity == 0) (da)->capacity = 8; \
            while ((da)->count >= (da)->capacity) (da)->capacity *= 3; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof((da)->items[0])); \
        } \
        (da)->items[(da)->count++] = (item); \
    } while (0)
#define da_clear(da) ((da)->count = 0)
#define da_free(da) (free((da)->items), (da)->items = NULL, (da)->count = 0)
#define da_foreach(da, Type, ptr) for (Type* ptr = (da)->items; ptr < (da)->items + (da)->count; ptr++)
#define da_remove(da, i) (assert(i < (da)->count), (da)->items[i] = (da)->items[--(da)->count])

typedef struct {
    char** items;
    size_t count, capacity;
}Str_Array;

void naredi_cmd_render(Str_Array cmd);

typedef struct {
    char value[128];
}Naredi_Small_String;

typedef struct {
    Naredi_Small_String* items;
    size_t count, capacity;
}Naredi_Small_Strings;

Naredi_Small_String naredi_small_string_from_cstr(const char* str);
Naredi_Small_String naredi_small_string_from_sized(const char* start, int len);
char* naredi_small_string_to_cstr(Naredi_Small_String string);

bool is_file1_modified_after_file2(const char* filepath1, const char* filepath2);
bool is_file1_modified_after_file2_small_string(Naredi_Small_String filepath1, Naredi_Small_String filepath2);

#endif // COMMON_H
