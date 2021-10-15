#ifndef PROMPT_H
#define PROMPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

size_t prompt(const char *message, const char *format, ...);
size_t prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE);

#endif /* PROMPT_H */
