#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef EOS
#define EOS '\0'
#endif

#define INITIAL_MAXARGC 8

static bool
is_space(const char c);

static void
consume_whitespace(const char **input);

char **buildargv(const char *input);

int
countargv (char * const *argv);