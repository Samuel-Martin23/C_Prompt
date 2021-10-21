#ifndef PROMPT_H
#define PROMPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

int prompt_getline_delim(const char *message, char *input, const size_t MAX_STR_SIZE, char *delim, bool matched_delim);
int prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE);
int prompt(const char *message, const char *format, ...);

#endif /* PROMPT_H */
