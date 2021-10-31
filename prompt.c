#include "prompt.h"

// USHRT_MAX and UINT32_MAX could be unsigned,
// so I need to typecast them.
// I wanted to check when the user goes below the min limit.
// For example, using ushort, if the user entered -65535
// the result is 1 as it should be. However, if the user
// entered -65536, what should it be? So if what the user entered
// was less than or equal to USHRT_MIN(-65536), the value will
// be USHRT_MAX(65535). The same thing applies to UINT32_MIN.
#define USHRT_MIN                   -((long)USHRT_MAX) - 1
#define UINT32_MIN                  -((long)UINT32_MAX) - 1

// How I determined MAX_READ.
// https://stackoverflow.com/questions/1701055/
// what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value
#define MAX_READ                    1080u

#define MAX_FORMAT                  2u

#define MULTIPLE_SPECIFIERS         1u
#define STOP_AT_SPACE               2u
#define NUMERICS_ONLY               4u

#define NO_STATUS                   0u
#define EOF_STREAM                  1u
#define NOT_NUMERIC                 2u

typedef enum read_status
{
    READ_EOF = -1,
    READ_FAILURE,
    READ_SUCCESS,
    READ_BAD_CHAR
} read_status_t;

typedef struct parser
{
    unsigned char status;
    unsigned char options;
    void *(*get_arg)(va_list *args);
    void (*assign_to_arg)(const char *str, void *arg);
    read_status_t (*read_in_arg)(va_list *args, struct parser *parse);
} parser_t;

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

static bool is_strchr(const char *s, int c)
{
    return (bool)(strchr(s, c));
}

static bool is_not_numeric(int ch, parser_t *parse)
{
    if (parse && (parse->options & NUMERICS_ONLY)
        && !((ch >= '0' && ch <= '9') || ch == '.' || ch == '-'))
    {
        parse->status |= NOT_NUMERIC;
        return true;
    }

    return false;
}

static bool is_space(int ch, parser_t *parse)
{
    if (parse && (parse->options & STOP_AT_SPACE) && ch == ' ')
    {
        return true;
    }

    return false;
}

static bool is_multiple_specifiers(int ch, parser_t *parse)
{
    if (parse && (parse->options & MULTIPLE_SPECIFIERS) && ch == ' ')
    {
        return true;
    }

    return false;
}

static bool check_eof(int ch, parser_t *parse)
{
    bool condition = (ch == EOF);

    if (parse && condition)
    {
        parse->status |= EOF_STREAM;
    }

    return condition;
}

// This function was inspired by this video:
// https://youtu.be/NsB6dqvVu7Y?t=231
static void parse_prompt(char *input, const size_t MAX_SIZE,
    parser_t *parse, const char *delim,
    const bool matched_delim, FILE *stream)
{
    int ch = getc(stream);
    
    if (check_eof(ch, parse))
    {
        return;
    }

    size_t i = 0;
    const size_t LAST_INDEX = MAX_SIZE - 1;

    // If you are not using a getline function,
    // we don't want to read any newlines or spaces first.
    if (parse)
    {
        while (ch == '\n' || ch == ' ')
        {
            ch = getc(stream);
        }
    }

    while (true)
    {
        if (ch == EOF || is_multiple_specifiers(ch, parse))
        {
            break;
        }

        if (i == LAST_INDEX
            || is_space(ch, parse)
            || is_not_numeric(ch, parse)
            || is_strchr(delim, ch) == matched_delim)
        {
            if (stream == stdin)
            {
                // Clearing the buffer.
                while (ch != '\n' && ch != EOF)
                {
                    ch = getc(stream);
                }
            }

            break;
        }

        input[i++] = (char)ch;
        ch = getc(stream);
    }

    input[i] = '\0';

    check_eof(ch, parse);
}

static void *va_arg_uint(va_list *args) {return (void*)va_arg(*args, unsigned int*);}
static void *va_arg_ulong(va_list *args) {return (void*)va_arg(*args, unsigned long*);}
static void *va_arg_double(va_list *args) {return (void*)va_arg(*args, double*);}
static void *va_arg_long(va_list *args) {return (void*)va_arg(*args, long*);}
static void *va_arg_ushort(va_list *args) {return (void*)va_arg(*args, unsigned short*);}
static void *va_arg_short(va_list *args) {return (void*)va_arg(*args, short*);}
static void *va_arg_float(va_list *args) {return (void*)va_arg(*args, float*);}
static void *va_arg_int(va_list *args) {return (void*)va_arg(*args, int*);}
static void *va_arg_char(va_list *args) {return (void*)va_arg(*args, char*);}

static read_status_t parse_str(va_list *args, parser_t *parse)
{
    char *input = va_arg(*args, char*);
    const size_t MAX_STR_SIZE = va_arg(*args, size_t);

    if (MAX_STR_SIZE == 0)
    {
        return READ_FAILURE;
    }

    parse_prompt(input, MAX_STR_SIZE, parse, "\n", true, stdin);

    if (parse->status & EOF_STREAM)
    {
        return READ_EOF;
    }

    return READ_SUCCESS;
}

static void parse_uint(const char *str, void *arg) 
{
    long number = strtol(str, NULL, 10);
    unsigned int *uint_arg = (unsigned int*)arg;

    if (number > UINT32_MAX || number <= UINT32_MIN)
    {
        *uint_arg = UINT32_MAX;
    }
    else
    {
        *uint_arg = (unsigned int)number;
    }
}

static void parse_ulong(const char *str, void *arg) 
{
    unsigned long *ulong_arg = (unsigned long*)arg;
    *ulong_arg = strtoul(str, NULL, 10);
}

static void parse_double(const char *str, void *arg) 
{
    double *double_arg = (double*)arg;
    *double_arg = strtod(str, NULL);
}

static void parse_long(const char *str, void *arg) 
{
    long *long_arg = (long*)arg;
    *long_arg = strtol(str, NULL, 10);
}

static void parse_ushort(const char *str, void *arg) 
{
    long number = strtol(str, NULL, 10);
    unsigned short *ushort_arg = (unsigned short*)arg;

    if (number > USHRT_MAX || number <= USHRT_MIN)
    {
        *ushort_arg = USHRT_MAX;
    }
    else
    {
        *ushort_arg = (unsigned short)number;
    }
}

static void parse_short(const char *str, void *arg) 
{
    long number = strtol(str, NULL, 10);
    short *short_arg = (short*)arg;

    if (number < SHRT_MIN)
    {
        *short_arg = SHRT_MIN;
    }
    else if (number > SHRT_MAX)
    {
        *short_arg = SHRT_MAX;
    }
    else
    {
        *short_arg = (short)number;
    }
}

static void parse_float(const char *str, void *arg) 
{
    float *float_arg = (float*)arg;
    *float_arg = strtof(str, NULL);
}

static void parse_int(const char *str, void *arg) 
{
    long number = strtol(str, NULL, 10);
    int *int_arg = (int*)arg;

    if (number < INT32_MIN)
    {
        *int_arg = INT32_MIN;
    }
    else if (number > INT32_MAX)
    {
        *int_arg = INT32_MAX;
    }
    else
    {
        *int_arg = (int)number;
    }
}

static void parse_char(const char *str, void *arg) 
{
    char *char_arg = (char*)arg;
    *char_arg = str[0];
}

static read_status_t parse_types(va_list *args, parser_t *parse)
{
    char input[MAX_READ] = {0};
    void *arg_value = parse->get_arg(args);

    parse_prompt(input, MAX_READ, parse, "\n", true, stdin);

    if (parse->status & EOF_STREAM)
    {
        return READ_EOF;
    }

    if (input[0] == '\0')
    {
        return READ_FAILURE;
    }

    parse->assign_to_arg(input, arg_value);

    if (parse->status & NOT_NUMERIC)
    {
        return READ_BAD_CHAR;
    }

    return READ_SUCCESS;
}

static read_status_t parse_format(va_list *args, char *specifier,
    bool multple_specifiers, int *successfully_read)
{
    parser_t parse;
    read_status_t result = READ_FAILURE;

    parse.status = NO_STATUS;
    parse.options = (multple_specifiers | STOP_AT_SPACE | NUMERICS_ONLY);
    parse.read_in_arg = parse_types;

    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        parse.options &= ~NUMERICS_ONLY;
        parse.get_arg = va_arg_char;
        parse.assign_to_arg = parse_char;
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_int;
        parse.assign_to_arg = parse_int;
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_float;
        parse.assign_to_arg = parse_float;
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_short;
        parse.assign_to_arg = parse_short;
    }
    else if (!(strncmp(specifier, "hu", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_ushort;
        parse.assign_to_arg = parse_ushort;
    }
    else if (!(strncmp(specifier, "ld", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_long;
        parse.assign_to_arg = parse_long;
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_double;
        parse.assign_to_arg = parse_double;
    }
    else if (!(strncmp(specifier, "lu", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_ulong;
        parse.assign_to_arg = parse_ulong;
    }
    else if (!(strncmp(specifier, "u", MAX_FORMAT)))
    {
        parse.get_arg = va_arg_uint;
        parse.assign_to_arg = parse_uint;
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        parse.options &= ~NUMERICS_ONLY;
        parse.read_in_arg = parse_str;
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    result = parse.read_in_arg(args, &parse);
    
    // If the user enters in a series of numbers like:
    // 12L 5 9
    // we still want to read in '12' and also increament 
    // our successfully read counter. However we discard the
    // rest of the input because of the 'L' and stop reading
    // all together.
    if (result >= READ_SUCCESS)
    {
        (*successfully_read)++;
    }

    return result;
}

static bool is_output_stream(FILE *stream)
{
    if (stream == stdout || stream == stderr)
    {
        return true;
    }

    return false;
}

int prompt_getline_delim(const char *message, char *input,
    const size_t MAX_STR_SIZE, const char *delim,
    bool matched_delim, FILE *stream)
{
    if (MAX_STR_SIZE == 0 || is_output_stream(stream))
    {
        return READ_FAILURE;
    }

    if (feof(stream))
    {
        return READ_EOF;
    }

    printf("%s", message);

    parse_prompt(input, MAX_STR_SIZE, NULL, delim, matched_delim, stream);

    return READ_SUCCESS;
}

int prompt_getline(const char *message, char *input,
    const size_t MAX_STR_SIZE, FILE *stream)
{
    return prompt_getline_delim(message, input, MAX_STR_SIZE, "\n", true, stream);
}

int prompt(const char *message, const char *format, ...)
{
    printf("%s", message);

    int successfully_read = 0;
    char *format_alloc = alloc_str(format);
    char *format_copy = format_alloc;
    char *specifier = strsep_chars(&format_copy, "%");
    read_status_t result = READ_FAILURE;

    va_list args;
    va_start(args, format);

    while (format_copy != NULL)
    {
        specifier = strsep_chars(&format_copy, "%");
        result = parse_format(&args, specifier,
                    (format_copy != NULL), &successfully_read);

        if (result != READ_SUCCESS)
        {
            break;
        }
    }

    va_end(args);
    free(format_alloc);

    return (result == READ_EOF) ? READ_EOF : successfully_read;
}
