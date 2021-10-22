#include "prompt.h"

// USHRT_MAX and UINT32_MAX could be unsigned
// so I need to typecast them.
#define USHRT_MIN                   -((long)USHRT_MAX) - 1
#define UINT32_MIN                  -((long)UINT32_MAX) - 1

// How I determined MAX_READ.
// https://stackoverflow.com/questions/1701055/
// what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value
#define MAX_READ                    1080u

#define MAX_FORMAT                  2u

#define NO_PARSE_OPT                0u
#define MULTIPLE_SPECIFIERS         1u
#define STOP_AT_SPACE               2u
#define READ_NUMERICS_ONLY          4u
#define READ_EVERYTHING             8u

#define strchr_bool(s, c)           (bool)(strchr(s, c))

// Separates a str with a multi-character delimiter.
static char *strsep_chars(char **data, const char *separator) 
{
    if (*data == NULL)
    {
        return *data;
    }

    char *end = strstr(*data, separator);
    char *temp = *data;

    if (end == NULL)
    {
        *data = NULL;
        return temp;
    }

    *end = '\0';
    *data = end + strlen(separator);

    return temp;
}

static char *alloc_str(const char *s)
{
    size_t size = strlen(s) + 1;
    char *str = malloc(sizeof(char) * size);

    if (str != NULL)
    {
        memcpy(str, s, size);
    }

    return str;
}

static void check_format_specifiers(int last_ch_read, char beginning, bool *is_eof, FILE *stream)
{
    // If you pass in multiple format specifiers
    // and one of them fails.
    int ch = '\0';

    if (last_ch_read == ' ' && beginning == '\0')
    {
        ch = getc(stream);

        while (ch != '\n' && ch != EOF)
        {
            ch = getc(stream);
        }

        if (ch == EOF && is_eof != NULL)
        {
            *is_eof = true;
        }
    }
}

static bool is_numeric_rep(int c)
{
    if ((c >= '0' && c <= '9') || c == '.' || c == '-')
    {
        return true;
    }

    return false;
}

// This function was inspired by this video:
// https://youtu.be/NsB6dqvVu7Y?t=231
static void parse_prompt(char *input, const size_t MAX_SIZE, unsigned char parse_opt, char *delim, bool matched_delim, bool *is_eof, FILE *stream)
{
    size_t i = 0;
    int ch = getc(stream);
    bool continue_reading = true;
    const size_t LAST_INDEX = MAX_SIZE - 1;

    if (stream == stdin && !(parse_opt & READ_EVERYTHING))
    {
        while (ch == '\n' || ch == ' ')
        {
            ch = getc(stream);
        }
    }

    while (true)
    {
        if (ch == '\n' || ch == EOF || ((parse_opt & MULTIPLE_SPECIFIERS) && ch == ' '))
        {
            if (ch == EOF && is_eof != NULL)
            {
                *is_eof = true;
            }

            break;
        }
        else if (continue_reading)
        {
            if (i == LAST_INDEX
                || ((parse_opt & STOP_AT_SPACE) && ch == ' ')
                || ((parse_opt & READ_NUMERICS_ONLY) && !(is_numeric_rep(ch)))
                || strchr_bool(delim, ch) == matched_delim)
            {
                continue_reading = false;
            }
            else
            {
                input[i++] = (char)ch;
            }
        }

        ch = getc(stream);
    }

    input[i] = '\0';

    check_format_specifiers(ch, input[0], is_eof, stream);
}

static void parse_number(char *input, const size_t MAX_SIZE, bool multple_specifiers, bool *is_eof)
{
    parse_prompt(input, MAX_SIZE, (multple_specifiers | STOP_AT_SPACE | READ_NUMERICS_ONLY), "\0", true, is_eof, stdin);
}

static void parse_chars(char *input, const size_t MAX_SIZE, bool multple_specifiers, bool *is_eof)
{
    parse_prompt(input, MAX_SIZE, (multple_specifiers | STOP_AT_SPACE), "\0", true, is_eof, stdin);
}

static int parse_uint(va_list *args, bool multple_specifiers)
{
    long number = 0;
    bool is_eof = false;
    char input[MAX_READ] = {0};
    unsigned int *arg_value = va_arg(*args, unsigned int*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    number = strtol(input, NULL, 10);

    if (number > UINT32_MAX || number <= UINT32_MIN)
    {
        *arg_value = UINT32_MAX;
    }

    *arg_value = (unsigned int)number;

    return 1;
}

static int parse_str(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char *input = va_arg(*args, char*);
    const size_t MAX_STR_SIZE = va_arg(*args, size_t);

    if (MAX_STR_SIZE == 0)
    {
        return 0;
    }

    parse_chars(input, MAX_STR_SIZE, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }

    return 1;
}

static int parse_ulong(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    unsigned long *arg_value = va_arg(*args, unsigned long*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    *arg_value = strtoul(input, NULL, 10);

    return 1;
}

static int parse_double(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    double *arg_value = va_arg(*args, double*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    *arg_value = strtod(input, NULL);

    return 1;
}

static int parse_long(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    long *arg_value = va_arg(*args, long*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    *arg_value = strtol(input, NULL, 10);

    return 1;
}

static int parse_ushort(va_list *args, bool multple_specifiers)
{
    long number = 0;
    bool is_eof = false;
    char input[MAX_READ] = {0};
    unsigned short *arg_value = va_arg(*args, unsigned short*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    number = strtol(input, NULL, 10);

    if (number > USHRT_MAX || number <= USHRT_MIN)
    {
        *arg_value = USHRT_MAX;
    }
    else
    {
        *arg_value = (unsigned short)number;
    }

    return 1;
}

static int parse_short(va_list *args, bool multple_specifiers)
{
    long number = 0;
    bool is_eof = false;
    char input[MAX_READ] = {0};
    short *arg_value = va_arg(*args, short*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

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

    return 1;
}

static int parse_float(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    float *arg_value = va_arg(*args, float*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    *arg_value = strtof(input, NULL);

    return 1;
}

static int parse_int(va_list *args, bool multple_specifiers)
{
    long number = 0;
    bool is_eof = false;
    char input[MAX_READ] = {0};
    int *arg_value = va_arg(*args, int*);

    parse_number(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

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

    return 1;
}

static int parse_char(va_list *args, bool multple_specifiers)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    char *arg_value = va_arg(*args, char*);

    parse_chars(input, MAX_READ, multple_specifiers, &is_eof);

    if (is_eof)
    {
        return EOF;
    }
    else if (input[0] == '\0')
    {
        return 0;
    }

    *arg_value = input[0];

    return 1;
}

static int parse_format(va_list *args, char *specifier, bool multple_specifiers)
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

int prompt_getline_delim(const char *message, char *input, const size_t MAX_STR_SIZE, char *delim, bool matched_delim, FILE *stream)
{
    if (MAX_STR_SIZE == 0)
    {
        return 0;
    }
    else if (feof(stream))
    {
        return EOF;
    }

    printf("%s", message);

    parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, delim, matched_delim, NULL, stream);

    return 1;
}

int prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE, FILE *stream)
{
    return prompt_getline_delim(message, input, MAX_STR_SIZE, "\0", true, stream);
}

int prompt(const char *message, const char *format, ...)
{
    printf("%s", message);

    int result = 0;
    int successfully_read = 0;
    char *format_alloc = alloc_str(format);
    char *format_copy = format_alloc;
    char *specifier = strsep_chars(&format_copy, "%");

    va_list args;
    va_start(args, format);

    while (format_copy != NULL)
    {
        specifier = strsep_chars(&format_copy, "%");
        result = parse_format(&args, specifier, (format_copy != NULL));

        if (result == EOF || result == 0)
        {
            break;
        }

        successfully_read += result;
    }

    va_end(args);
    free(format_alloc);

    return (result == EOF) ? EOF : successfully_read;
}
