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

    do
    {
        ch = getchar();
    } 
    while (ch == '\n');

    while (true)
    {
        if (ch == '\n' || ch == EOF || (ch == ' ' && stop_at_space))
        {
            break;
        }
        else if (i < (MAX_SIZE - 1))
        {
            input[i++] = (char)ch;
        }

        ch = getchar();
    }

    input[i] = '\0';

    return i;
}

static size_t parse_str(va_list args, bool stop_at_space)
{
    size_t size = 0;
    char *input = va_arg(args, char*);
    const size_t MAX_STR_SIZE = va_arg(args, size_t);

    if (MAX_STR_SIZE != 0)
    {
        size = parse_prompt(input, MAX_STR_SIZE, stop_at_space);
    }

    return size;
}

static size_t prase_double(va_list args, bool stop_at_space)
{
    size_t size = 0;
    char input[21] = {0};
    double *arg_value = va_arg(args, double*);

    size = parse_prompt(input, sizeof(input), stop_at_space);
    *arg_value = strtod(input, NULL);

    return size;
}

static size_t parse_short(va_list args, bool stop_at_space)
{
    size_t size = 0;
    long number = 0;
    char input[7] = {0};
    short *arg_value = va_arg(args, short*);

    size = parse_prompt(input, sizeof(input), stop_at_space);
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

static size_t prase_float(va_list args, bool stop_at_space)
{
    size_t size = 0;
    char input[12] = {0};
    float *arg_value = va_arg(args, float*);

    size = parse_prompt(input, sizeof(input), stop_at_space);
    *arg_value = strtof(input, NULL);

    return size;
}

static size_t parse_int(va_list args, bool stop_at_space)
{
    size_t size = 0;
    long number = 0;
    char input[12] = {0};
    int *arg_value = va_arg(args, int*);

    size = parse_prompt(input, sizeof(input), stop_at_space);
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

static size_t parse_char(va_list args, bool stop_at_space)
{
    char input[2] = {0};
    char *arg_value = va_arg(args, char*);
    size_t size = 0;

    size = parse_prompt(input, sizeof(input), stop_at_space);
    *arg_value = input[0];

    return size;
}

static size_t parse_format(va_list args, char *specifier, bool stop_at_space)
{
    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        return parse_char(args, stop_at_space);
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        return parse_int(args, stop_at_space);
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        return prase_float(args, stop_at_space);
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        return parse_short(args, stop_at_space);
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        return prase_double(args, stop_at_space);
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        return parse_str(args, stop_at_space);
    }

    return 0;
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

    // Single specifier.
    size = parse_format(args, specifier, (format_copy != NULL));

    // Multiple specifiers.
    while (format_copy != NULL)
    {
        specifier = strsep(&format_copy, "%");
        size += parse_format(args, specifier, (format_copy != NULL));
    }

    free(format_alloc);

    va_end(args);

    return size;
}
