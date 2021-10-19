#include "prompt.h"

// They could be unsigned so I need to
// typecast them to long.
#define USHRT_MIN                   -((long)USHRT_MAX) - 1
#define UINT32_MIN                  -((long)UINT32_MAX) - 1

#define MAX_READ_CHAR               2u
#define MAX_READ_SHORT              8u
#define MAX_READ_INT                13u
#define MAX_READ_FLOAT              60u
#define MAX_READ_LONG               22u
#define MAX_READ_ULONG              23u
#define MAX_READ_DOUBLE             329u

#define MAX_NEG_SHORT               (MAX_READ_SHORT - 2u)
#define MAX_POS_SHORT               (MAX_READ_SHORT - 3u)
#define MAX_NEG_INT                 (MAX_READ_INT - 2u)
#define MAX_POS_INT                 (MAX_READ_INT - 3u)
#define MAX_NEG_LONG                (MAX_READ_LONG - 2u)
#define MAX_POS_LONG                (MAX_READ_LONG - 3u)
#define MAX_POS_ULONG               (MAX_READ_ULONG - 3u)

#define MAX_FORMAT                  2u

#define NO_PARSE_OPT                0u
#define MULTIPLE_SPECIFIERS         1u
#define STOP_AT_SPACE               2u
#define STOP_AT_ALPHA               4u

#define strchr_bool(s, c)           (bool)(strchr(s, c))

static size_t get_unsigned_size(size_t value)
{
    // I really hate doing this but I can't think of a way
    // to get the size after.
    size_t count = 0;

    while (value != 0)
    {
        value /= 10;
        count++;
    }

    return count;
}

static size_t parse_prompt(char *input, const size_t MAX_SIZE, unsigned char parse_opt, char *delim, bool matched_delim)
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
        if (ch == '\n' || ch == EOF || ((parse_opt & MULTIPLE_SPECIFIERS) && ch == ' '))
        {
            break;
        }
        else if (continue_reading)
        {
            if (((parse_opt & STOP_AT_SPACE) && ch == ' ')
                || ((parse_opt & STOP_AT_ALPHA) && isalpha(ch))
                || strchr_bool(delim, ch) == matched_delim)
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

static size_t parse_uint(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[MAX_READ_INT] = {0};
    unsigned int *arg_value = va_arg(args, unsigned int*);

    size = parse_prompt(input, MAX_READ_INT, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    number = strtol(input, NULL, 10);

    if (number > UINT32_MAX || number <= UINT32_MIN)
    {
        *arg_value = UINT32_MAX;
        return MAX_POS_INT;
    }

    *arg_value = (unsigned int)number;
    return (input[0] == '-') ? get_unsigned_size((size_t)*arg_value) : size;
}

static size_t parse_str(va_list args, bool multple_specifiers)
{
    char *input = va_arg(args, char*);
    const size_t MAX_STR_SIZE = va_arg(args, size_t);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, (multple_specifiers | STOP_AT_SPACE), "\0", true);
    }

    return 0;
}

static size_t parse_ulong(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[MAX_READ_ULONG] = {0};
    unsigned long *arg_value = va_arg(args, unsigned long*);

    errno = 0;
    size = parse_prompt(input, MAX_READ_ULONG, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    *arg_value = strtoul(input, NULL, 10);

    if (errno == ERANGE && *arg_value == ULONG_MAX)
    {
        return MAX_POS_ULONG;
    }

    return (input[0] == '-') ? get_unsigned_size(*arg_value) : size;
}

static size_t parse_double(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[MAX_READ_DOUBLE] = {0};
    double *arg_value = va_arg(args, double*);

    size = parse_prompt(input, MAX_READ_DOUBLE, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    *arg_value = strtod(input, NULL);

    return size;
}

static size_t parse_long(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[MAX_READ_LONG] = {0};
    long *arg_value = va_arg(args, long*);

    errno = 0;
    size = parse_prompt(input, MAX_READ_LONG, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    *arg_value = strtol(input, NULL, 10);

    if (errno == ERANGE && *arg_value == LONG_MIN)
    {
        return MAX_NEG_LONG;
    }
    else if (errno == ERANGE && *arg_value == LONG_MAX)
    {
        return MAX_POS_LONG;
    } 

    return size;
}

static size_t parse_ushort(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[MAX_READ_SHORT] = {0};
    unsigned short *arg_value = va_arg(args, unsigned short*);

    size = parse_prompt(input, MAX_READ_SHORT, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    number = strtol(input, NULL, 10);

    if (number > USHRT_MAX || number <= USHRT_MIN)
    {
        *arg_value = USHRT_MAX;
        return MAX_POS_SHORT;
    }

    *arg_value = (unsigned short)number;
    return (input[0] == '-') ? get_unsigned_size((size_t)*arg_value) : size;
}

static size_t parse_short(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[MAX_READ_SHORT] = {0};
    short *arg_value = va_arg(args, short*);

    size = parse_prompt(input, MAX_READ_SHORT, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    number = strtol(input, NULL, 10);

    if (number < SHRT_MIN)
    {
        *arg_value = SHRT_MIN;
        return MAX_NEG_SHORT;
    }
    else if (number > SHRT_MAX)
    {
        *arg_value = SHRT_MAX;
        return MAX_POS_SHORT;
    }

    *arg_value = (short)number;
    return size;
}

static size_t parse_float(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    char input[MAX_READ_FLOAT] = {0};
    float *arg_value = va_arg(args, float*);

    size = parse_prompt(input, MAX_READ_FLOAT, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), "\0", true);
    *arg_value = strtof(input, NULL);

    return size;
}

static size_t parse_int(va_list args, bool multple_specifiers)
{
    size_t size = 0;
    long number = 0;
    char input[MAX_READ_INT] = {0};
    int *arg_value = va_arg(args, int*);

    size = parse_prompt(input, MAX_READ_INT, (multple_specifiers | STOP_AT_SPACE | STOP_AT_ALPHA), '\0', true);
    number = strtol(input, NULL, 10);

    if (number < INT32_MIN)
    {
        *arg_value = INT32_MIN;
        return MAX_NEG_INT;
    }
    else if (number > INT32_MAX)
    {
        *arg_value = INT32_MAX;
        return MAX_POS_INT;
    }

    *arg_value = (int)number;
    return size;
}

static size_t parse_char(va_list args, bool multple_specifiers)
{
    char input[MAX_READ_CHAR] = {0};
    char *arg_value = va_arg(args, char*);
    size_t size = 0;

    size = parse_prompt(input, MAX_READ_CHAR, (multple_specifiers | STOP_AT_SPACE), "\0", true);
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
        return parse_float(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        return parse_short(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "hu", MAX_FORMAT)))
    {
        return parse_ushort(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "ld", MAX_FORMAT)))
    {
        return parse_long(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        return parse_double(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "lu", MAX_FORMAT)))
    {
        return parse_ulong(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        return parse_str(args, multple_specifiers);
    }
    else if (!(strncmp(specifier, "u", MAX_FORMAT)))
    {
        return parse_uint(args, multple_specifiers);
    }

    return 0;
}

size_t prompt_getline_delim(const char *message, char *input, const size_t MAX_STR_SIZE, char *delim, bool matched_delim)
{
    printf("%s", message);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, delim, matched_delim);
    }

    return 0;
}

size_t prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE)
{
    printf("%s", message);

    if (MAX_STR_SIZE != 0)
    {
        return parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, "\0", true);
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
