/*
The prompt library provides a clean and easy way
to get input from the user. There are many advantages
to using the prompt library compared to other means such as
scanf or fgets.
(prompt currently does not support pattern matching.)

Advantages of the prompt library:
1. The buffer is flushed when using any prompt library functions.
scanf leaves \n in the buffer and is open to buffer overflow attacks.
fgets just reads a certain number of bytes from the buffer.
So a problem could arise when you use scanf or fgets
followed by another input function without clearing the buffer.
Even if you do clear the buffer, you might need to press
the enter/return key one or more times.

2. There is no need for explicit field width settings when using
prompt for strs. If you chose to use prompt to enter a str,
you can pass in the size of the str as an additional parameter.

3. prompt gives you protection from arithmetic overflow. If an
overflow happens, the strto... family will take care of it with
additional checking for types smaller than a long.

4. The prompt library also provides prompt_getline
and prompt_getline_delim. If you use prompt to enter a str,
it will stop reading when it encounters a space. Using prompt_getline
or prompt_getline_delim will solve this issue.

5. prompt_getline_delim has some interesting features.
prompt_getline_delim takes two additional parameters,
a delim, and matched_delim. delim is a char* as
I wanted a way for the user to pass in multiple delims. The order of
the delim does not matter. matched_delim is a bool that will
exclude(true) or include(false) the delim.
For example, if the delim is "01\n" and the matched_delim is true,
then it will stop reading when it encounters a '0', '1', or a '\n'.
However, if the delim is "01" and the matched_delim is false,
then it will stop reading only when it encounters a char
that is NOT a '0' or a '1'.

6. The prompt functions have an additional parameter for
printing out your message to the user instead of calling printf
before you use an input function.

Format specifiers supported by the prompt library:
Format Specifier | Data Type
%c  | char
%d  | int
%f  | float
%hi | short
%hu | unsigned short
%ld | long
%lf | double
%lu | unsigned long
%s  | string
%u  | unsigned int
*/

#ifndef PROMPT_H
#define PROMPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <stdint.h>
#else
#include <limits.h>
#endif

int prompt_getline_delim_stream(const char *message, char *input,
    const size_t MAX_STR_SIZE, const char *delim,
    bool matched_delim, FILE *stream);

int prompt_getline_delim(const char *message, char *input,
    const size_t MAX_STR_SIZE, const char *delim,
    bool matched_delim);

int prompt_getline_stream(const char *message, char *input,
    const size_t MAX_STR_SIZE, FILE *stream);

int prompt_getline(const char *message, char *input,
    const size_t MAX_STR_SIZE);

int prompt(const char *message, const char *format, ...);

#endif /* PROMPT_H */
