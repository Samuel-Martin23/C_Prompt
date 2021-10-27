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

#define NO_PARSE_OPT                0u
#define MULTIPLE_SPECIFIERS         1u
#define STOP_AT_SPACE               2u
#define READ_NUMERICS_ONLY          4u

#define PURPLE                      "\033[1;95m"
#define RED                         "\033[1;91m"
#define WHITE                       "\033[1;97m"
#define RESET                       "\033[0m"

#define strchr_bool(s, c)           (bool)(strchr(s, c))

typedef struct parser
{
    unsigned char parse_opt;
    void *(*get_arg)(va_list *args);
    void (*parse_type)(const char *str, void *arg);
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

static bool is_numeric_rep(int c)
{
    if ((c >= '0' && c <= '9') || c == '.' || c == '-')
    {
        return true;
    }

    return false;
}

static bool check_eof(int ch, bool *is_eof)
{
    bool condition = (ch == EOF && is_eof != NULL);

    if (condition)
    {
        *is_eof = true;
    }

    return condition;
}

// This function was inspired by this video:
// https://youtu.be/NsB6dqvVu7Y?t=231
static void parse_prompt(char *input, const size_t MAX_SIZE, unsigned char parse_opt, char *delim, bool matched_delim, bool *is_eof, FILE *stream)
{
    int ch = getc(stream);
    
    if (check_eof(ch, is_eof))
    {
        return;
    }

    size_t i = 0;
    bool continue_reading = true;
    const size_t LAST_INDEX = MAX_SIZE - 1;

    if (stream == stdin && !(parse_opt == NO_PARSE_OPT))
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
            check_eof(ch, is_eof);
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

    // If you pass in multiple format specifiers
    // and one of them fails.
    if (((parse_opt & MULTIPLE_SPECIFIERS) && ch == ' ') && input[0] == '\0')
    {
        do
        {
            ch = getc(stream);
        } while (ch != '\n' && ch != EOF);

        check_eof(ch, is_eof);
    }
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

static int parse_arg(va_list *args, parser_t *p)
{
    bool is_eof = false;
    char input[MAX_READ] = {0};
    void *arg_value = p->get_arg(args);

    parse_prompt(input, MAX_READ, p->parse_opt, "", true, &is_eof, stdin);

    if (is_eof)
    {
        return EOF;
    }

    if (input[0] == '\0')
    {
        return 0;
    }

    p->parse_type(input, arg_value);

    return 1;
}

static int parse_str(va_list *args, unsigned char parse_opt)
{
    bool is_eof = false;
    char *input = va_arg(*args, char*);
    const size_t MAX_STR_SIZE = va_arg(*args, size_t);

    if (MAX_STR_SIZE == 0)
    {
        return 0;
    }

    parse_prompt(input, MAX_STR_SIZE, parse_opt, "", true, &is_eof, stdin);

    if (is_eof)
    {
        return EOF;
    }

    return 1;
}

static int parse_format(va_list *args, char *specifier, bool multple_specifiers)
{
    if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        return parse_str(args, (multple_specifiers | STOP_AT_SPACE));
    }

    parser_t p;
    p.parse_opt = (multple_specifiers | STOP_AT_SPACE | READ_NUMERICS_ONLY);

    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        p.parse_opt &= ~READ_NUMERICS_ONLY;
        p.get_arg = va_arg_char;
        p.parse_type = parse_char;
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        p.get_arg = va_arg_int;
        p.parse_type = parse_int;
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        p.get_arg = va_arg_float;
        p.parse_type = parse_float;
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        p.get_arg = va_arg_short;
        p.parse_type = parse_short;
    }
    else if (!(strncmp(specifier, "hu", MAX_FORMAT)))
    {
        p.get_arg = va_arg_ushort;
        p.parse_type = parse_ushort;
    }
    else if (!(strncmp(specifier, "ld", MAX_FORMAT)))
    {
        p.get_arg = va_arg_long;
        p.parse_type = parse_long;
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        p.get_arg = va_arg_double;
        p.parse_type = parse_double;
    }
    else if (!(strncmp(specifier, "lu", MAX_FORMAT)))
    {
        p.get_arg = va_arg_ulong;
        p.parse_type = parse_ulong;
    }
    else if (!(strncmp(specifier, "u", MAX_FORMAT)))
    {
        p.get_arg = va_arg_uint;
        p.parse_type = parse_uint;
    }
    else
    {
        printf("\n%serror:%s %%%s is not a specifier%s\n", RED, WHITE, specifier, RESET);
        exit(EXIT_FAILURE);
    }

    return parse_arg(args, &p);
}

int prompt_getline_delim(const char *message, char *input, const size_t MAX_STR_SIZE, char *delim, bool matched_delim, FILE *stream)
{
    if (stream == stdout || stream == stderr)
    {
        printf("\n%swarning:%s can not use current stream for getline functions.%s\n", PURPLE, WHITE, RESET);
        return 0;
    }

    if (MAX_STR_SIZE == 0)
    {
        return 0;
    }

    if (feof(stream))
    {
        return EOF;
    }

    printf("%s", message);

    parse_prompt(input, MAX_STR_SIZE, NO_PARSE_OPT, delim, matched_delim, NULL, stream);

    return 1;
}

int prompt_getline(const char *message, char *input, const size_t MAX_STR_SIZE, FILE *stream)
{
    return prompt_getline_delim(message, input, MAX_STR_SIZE, "", true, stream);
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
