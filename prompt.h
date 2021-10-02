#ifndef PROMPT_H
#define PROMPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

size_t prompt(const char *message, const char *format, ...);

#endif /* PROMPT_H */
