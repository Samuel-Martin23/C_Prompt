#include "prompt.h"

#define MAX_FORMAT                  3

static bool is_valid_str(const char *data)
{
    return ((data != NULL) && (data[0] != '\0'));
}

static size_t parse_prompt(char *input, const size_t MAX_SIZE, bool stop_at_space)
{
    int ch = 0;
    size_t i = 0;

    while (true)
    {
        ch = getchar();

        if (ch == '\n' || ch == EOF || (ch == ' ' && stop_at_space))
        {
            break;
        }
        else if (i < (MAX_SIZE - 1))
        {
            input[i++] = (char)ch;
        }
    }

    input[i] = '\0';

    return i;
}

static size_t parse_str(va_list args)
{
    size_t size = 0;
    char *input = va_arg(args, char*);
    const size_t MAX_STR_SIZE = va_arg(args, size_t);

    if (MAX_STR_SIZE != 0)
    {
        size = parse_prompt(input, MAX_STR_SIZE, false);
    }

    return size;
}

static size_t prase_double(va_list args)
{
    size_t size = 0;
    char input[21];
    double *arg_value = va_arg(args, double*);

    size = parse_prompt(input, sizeof(input), true);
    *arg_value = strtod(input, NULL);

    return size;
}

static size_t parse_short(va_list args)
{
    size_t size = 0;
    long number = 0;
    char input[7];
    short *arg_value = va_arg(args, short*);

    size = parse_prompt(input, sizeof(input), true);
    number = strtol(input, 0, 10);

    if (number < SHRT_MIN)
    {
        *arg_value = SHRT_MIN;
    }
    else if (number > SHRT_MAX)
    {
        *arg_value = SHRT_MAX;
    }
    else
    {
        *arg_value = (short)number;
    }

    return size;
}

static size_t prase_float(va_list args)
{
    size_t size = 0;
    char input[12];
    float *arg_value = va_arg(args, float*);

    size = parse_prompt(input, sizeof(input), true);
    *arg_value = strtof(input, NULL);

    return size;
}

static size_t parse_int(va_list args)
{
    size_t size = 0;
    long number = 0;
    char input[12];
    int *arg_value = va_arg(args, int*);

    size = parse_prompt(input, sizeof(input), true);
    number = strtol(input, 0, 10);

    if (number < INT32_MIN)
    {
        *arg_value = INT32_MIN;
    }
    else if (number > INT32_MAX)
    {
        *arg_value = INT32_MAX;
    }
    else
    {
        *arg_value = (int)number;
    }

    return size;
}

static size_t parse_char(va_list args)
{
    char input[2];
    char *arg_value = va_arg(args, char*);
    size_t size = 0;

    size = parse_prompt(input, sizeof(input), false);
    *arg_value = input[0];

    return size;
}

size_t prompt(const char *message, const char *format, ...)
{
    printf("%s", message);

    size_t size = 0;
    char *specifier = "";
    char *format_alloc = strdup(format);
    char *format_copy = format_alloc;

    va_list args;
    va_start(args, format);

    while (!(is_valid_str(specifier)))
    {
        specifier = strsep(&format_copy, "%");
    }
    
    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        size = parse_char(args);
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        size = parse_int(args);
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        size = prase_float(args);
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        size = parse_short(args);
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        size = prase_double(args);
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        size = parse_str(args);
    }

    free(format_alloc);

    va_end(args);

    return size;
}
