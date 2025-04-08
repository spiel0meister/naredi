#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stddef.h>

#define eprintf(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

#define todo() do { eprintf("%s:%d: TODO\n", __FILE__, __LINE__); __builtin_trap(); } while (0)
#define todol(label) do { eprintf("%s:%d: TODO: %s\n", __FILE__, __LINE__, label); __builtin_trap(); } while (0)

#define unused(thing) (void)(thing)
#define unreachable() do { eprintf("%s:%d: UNREACHABLE\n", __FILE__, __LINE__); __builtin_trap(); } while (0)

#define defer(expr) for (int i = 0; i < 1; (i++, (expr)))

#define da_free(da) (free((da)->items), (da)->items = NULL, (da)->count = 0)
#define da_append(da, item) do { \
        if ((da)->count >= (da)->capacity) { \
            if ((da)->capacity == 0) (da)->capacity = 8; \
            while ((da)->count >= (da)->capacity) (da)->capacity *= 3; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof((da)->items[0])); \
        } \
        (da)->items[(da)->count++] = (item); \
    } while (0)
#define da_foreach(da, Type, ptr) for (Type* ptr = (da)->items; ptr < (da)->items + (da)->count; ptr++)

typedef struct {
    char value[128];
}Naredi_Small_String;

typedef struct {
    Naredi_Small_String* items;
    size_t count, capacity;
}Naredi_Small_Strings;

Naredi_Small_String naredi_small_string_from_cstr(const char* str);
Naredi_Small_String naredi_small_string_from_sized(const char* start, int len);

#endif // COMMON_H
