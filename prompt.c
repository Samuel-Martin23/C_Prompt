#include "prompt.h"

#define MAX_FORMAT                  3u

#define NO_PARSE_OPT                0u
#define MULTPLE_SPECIFIERS          1u
#define STOP_AT_SPACE               2u

static size_t parse_prompt(char *input, const size_t MAX_SIZE, unsigned char parse_opt, char delim)
{
    size_t i = 0;
    int ch = getchar();
    bool continue_reading = true;

    while (ch == '\n' || ch == ' ')
    {
        ch = getchar();
    }

    while (true)
    {
        if (ch == '\n' || ch == EOF || (ch == ' ' && (parse_opt & MULTPLE_SPECIFIERS)))
        {
            break;
        }
        else if (continue_reading)
        {
            if ((ch == ' ' && (parse_opt & STOP_AT_SPACE)) || ch == delim)
            {
                continue_reading = false;
            }
            else if (i < (MAX_SIZE - 1))
            {
                input[i++] = (char)ch;
            }
        }

        ch = getchar();
    }

    input[i] = '\0';

    return i;
}

static size_t parse_str(va_list args, bool multple_specifiers)
{
    char *input = va_arg(args, char*);
    const size_t MAX_STR_SIZE = va_arg(args, size_t);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, (multple_specifiers | STOP_AT_SPACE), '\0');
    }

    return 0;
}

static size_t prase_double(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[21] = {0};
    double *arg_value = va_arg(args, double*);

    size = parse_prompt(input, sizeof(input), (multple_specifiers | STOP_AT_SPACE), '\0');
    *arg_value = strtod(input, NULL);

    return size;
}

static size_t parse_short(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[7] = {0};
    short *arg_value = va_arg(args, short*);

    size = parse_prompt(input, sizeof(input), (multple_specifiers | STOP_AT_SPACE), '\0');
    number = strtol(input, NULL, 10);

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

static size_t prase_float(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[12] = {0};
    float *arg_value = va_arg(args, float*);

    size = parse_prompt(input, sizeof(input), (multple_specifiers | STOP_AT_SPACE), '\0');
    *arg_value = strtof(input, NULL);

    return size;
}

static size_t parse_int(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[12] = {0};
    int *arg_value = va_arg(args, int*);

    size = parse_prompt(input, sizeof(input), (multple_specifiers | STOP_AT_SPACE), '\0');
    number = strtol(input, NULL, 10);

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

static size_t parse_char(va_list args, bool multple_specifiers)
{
    char input[2] = {0};
    char *arg_value = va_arg(args, char*);
    size_t size = 0;

    size = parse_prompt(input, sizeof(input), (multple_specifiers | STOP_AT_SPACE), '\0');
    *arg_value = input[0];

    return size;
}

static size_t parse_format(va_list args, char *specifier, bool multple_specifiers)
{
    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        return parse_char(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        return parse_int(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        return prase_float(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        return parse_short(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        return prase_double(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        return parse_str(args, multple_specifiers);
    }

    return 0;
}

size_t prompt_getline_delim(const char *message, char *input, const size_t MAX_STR_SIZE, char delim)
{
    printf("%s", message);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, delim);
    }

    return 0;
}

size_t prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE)
{
    printf("%s", message);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, '\0');
    }

    return 0;
}

size_t prompt(const char *message, const char *format, ...)
{
    printf("%s", message);

    size_t size = 0;
    char *format_alloc = strdup(format);
    char *format_copy = format_alloc;
    char *specifier = strsep(&format_copy, "%");

    va_list args;
    va_start(args, format);

    while (format_copy != NULL)
    {
        specifier = strsep(&format_copy, "%");
        size += parse_format(args, specifier, (format_copy != NULL));
    }

    va_end(args);

    free(format_alloc);

    return size;
}
