#include "prompt.h"

// USHRT_MAX and UINT32_MAX could be unsigned,
// so I need to typecast them.
// I wanted to check when the user goes below the min limit.
// For example, using ushort, if the user entered -65535
// the result is 1 as it should be. However, if the user
// entered -65536, what should it be? So if what the user entered
// was less than or equal to USHRT_MIN(-65536), the value will
// be USHRT_MAX(65535). The same thing applies to UINT32_MIN.
#define USHRT_MIN                   (-((long)USHRT_MAX) - 1)
#define UINT32_MIN                  (-((long)UINT32_MAX) - 1)

// How I determined MAX_READ.
// https://stackoverflow.com/questions/1701055/
// what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value
#define MAX_READ                    1080

#define MAX_FORMAT                  2

#define MULTIPLE_SPECIFIERS         (1 << 0)
#define STOP_AT_SPACE               (1 << 1)
#define NUMERICS_ONLY               (1 << 2)

#define READ_NONE                   0
#define READ_EOF                    (1 << 0)
#define READ_FAILURE                (1 << 1)
#define READ_SUCCESS                (1 << 2)
#define READ_NON_NUMERIC            (1 << 3)

typedef struct ArgumentType
{
    int status;
    int options;
    void *(*get)(va_list *args);
    void (*set)(void *arg, const char *str);
} ArgumentType;

typedef void (*ArgumentParser)(ArgumentType *arg_type, va_list *args);

// Forward declarations.
static char *str_alloc(const char *s);
static char *strsep_chars(char **data, const char *separator);
static int parse_format(va_list *args, const char *specifier,
                        bool multple_specifiers, int *successfully_read);
static void parse_types(ArgumentType *arg_type, va_list *args);
static void *va_arg_char(va_list *args);
static void parse_char(void *arg, const char *str);
static void *va_arg_int(va_list *args);
static void parse_int(void *arg, const char *str);
static void *va_arg_float(va_list *args);
static void parse_float(void *arg, const char *str);
static void *va_arg_short(va_list *args);
static void parse_short(void *arg, const char *str);
static void *va_arg_ushort(va_list *args);
static void parse_ushort(void *arg, const char *str);
static void *va_arg_long(va_list *args);
static void parse_long(void *arg, const char *str);
static void *va_arg_double(va_list *args);
static void parse_double(void *arg, const char *str);
static void *va_arg_ulong(va_list *args);
static void parse_ulong(void *arg, const char *str);
static void *va_arg_uint(va_list *args);
static void parse_uint(void *arg, const char *str);
static void parse_str(ArgumentType *arg_type, va_list *args);
static void parse_prompt(char *input, const size_t BUFFER_SIZE, ArgumentType *arg_type,
                         const char *delim, bool matched_delim, FILE *stream);
static bool is_multiple_specifiers(ArgumentType *arg_type, int ch);
static bool is_strchr(const char *s, int ch);
static bool is_space(ArgumentType *arg_type, int ch);
static bool is_non_numeric(ArgumentType *arg_type, int ch);

int prompt(const char *message, const char *format, ...)
{
    printf("%s", message);

    int result = READ_NONE;
    int successfully_read = 0;
    char *format_alloc = str_alloc(format);
    char *format_copy = format_alloc;
    char *specifier = strsep_chars(&format_copy, "%");

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

    return (result == READ_EOF) ? EOF : successfully_read;
}

int prompt_gets(const char *message, char *input, const size_t BUFFER_SIZE)
{
    printf("%s", message);
    return prompt_gets_delim_stream(input, BUFFER_SIZE, "\n", true, stdin);
}

int prompt_gets_delim(const char *message, char *input,
                      const size_t BUFFER_SIZE, const char *delim,
                      bool matched_delim)
{
    printf("%s", message);
    return prompt_gets_delim_stream(input, BUFFER_SIZE, delim, matched_delim, stdin);
}

int prompt_gets_stream(char *input, const size_t BUFFER_SIZE, FILE *stream)
{
    return prompt_gets_delim_stream(input, BUFFER_SIZE, "\n", true, stream);
}

int prompt_gets_delim_stream(char *input, const size_t BUFFER_SIZE,
                             const char *delim, bool matched_delim,
                             FILE *stream)
{
    if (BUFFER_SIZE == 0 || stream == stdout || stream == stderr)
    {
        return 0;
    }

    if (feof(stream))
    {
        return EOF;
    }

    parse_prompt(input, BUFFER_SIZE, NULL, delim, matched_delim, stream);

    return 1;
}

// FIXME: Make input pass by ref.
char *prompt_getline_alloc(const char *message)
{
    printf("%s", message);
    return prompt_getline_stream_alloc(stdin);
}

char *prompt_getline_stream_alloc(FILE *stream)
{
    size_t i = 0;
    size_t capacity = 8;
    int ch = getc(stream);
    // FIXME: Check malloc
    char *input = malloc(sizeof(char) * (capacity + 1));

    // FIXME: Add support for delim.
    while (ch != EOF && ch != '\n')
    {
        if (i == capacity)
        {
            capacity *= 2;
            // FIXME: Check realloc
            input = realloc(input, capacity + 1);
        }

        input[i] = (char)ch;
        i++;

        ch = getc(stream);
    }

    input[i] = '\0';

    return input;
}

static char *str_alloc(const char *s)
{
    size_t size = strlen(s) + 1;
    char *str = malloc(sizeof(char) * size);

    if (str != NULL)
    {
        memcpy(str, s, size);
    }

    return str;
}

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

static int parse_format(va_list *args, const char *specifier,
                        bool multple_specifiers, int *successfully_read)
{
    ArgumentType arg_type;
    ArgumentParser parse_arg = parse_types;

    arg_type.status = READ_NONE;
    arg_type.options = (multple_specifiers | STOP_AT_SPACE | NUMERICS_ONLY);

    if (!(strncmp(specifier, "c", MAX_FORMAT)))
    {
        arg_type.options &= ~NUMERICS_ONLY;
        arg_type.get = va_arg_char;
        arg_type.set = parse_char;
    }
    else if (!(strncmp(specifier, "d", MAX_FORMAT)))
    {
        arg_type.get = va_arg_int;
        arg_type.set = parse_int;
    }
    else if (!(strncmp(specifier, "f", MAX_FORMAT)))
    {
        arg_type.get = va_arg_float;
        arg_type.set = parse_float;
    }
    else if (!(strncmp(specifier, "hi", MAX_FORMAT)))
    {
        arg_type.get = va_arg_short;
        arg_type.set = parse_short;
    }
    else if (!(strncmp(specifier, "hu", MAX_FORMAT)))
    {
        arg_type.get = va_arg_ushort;
        arg_type.set = parse_ushort;
    }
    else if (!(strncmp(specifier, "ld", MAX_FORMAT)))
    {
        arg_type.get = va_arg_long;
        arg_type.set = parse_long;
    }
    else if (!(strncmp(specifier, "lf", MAX_FORMAT)))
    {
        arg_type.get = va_arg_double;
        arg_type.set = parse_double;
    }
    else if (!(strncmp(specifier, "lu", MAX_FORMAT)))
    {
        arg_type.get = va_arg_ulong;
        arg_type.set = parse_ulong;
    }
    else if (!(strncmp(specifier, "u", MAX_FORMAT)))
    {
        arg_type.get = va_arg_uint;
        arg_type.set = parse_uint;
    }
    else if (!(strncmp(specifier, "s", MAX_FORMAT)))
    {
        arg_type.options &= ~NUMERICS_ONLY;
        parse_arg = parse_str;
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    parse_arg(&arg_type, args);

    // If the user enters in a series of numbers like:
    // 12L 5 9
    // we still want to read in "12" and also increament 
    // our successfully read counter. However we discard the
    // rest of the input because of the 'L' and stop reading
    // all together.
    if (arg_type.status & (READ_SUCCESS|READ_NON_NUMERIC))
    {
        (*successfully_read)++;
    }

    return arg_type.status;
}

static void parse_types(ArgumentType *arg_type, va_list *args)
{
    char input[MAX_READ] = {0};
    void *arg_value = arg_type->get(args);

    parse_prompt(input, MAX_READ, arg_type, "\n", true, stdin);

    if (arg_type->status & (READ_EOF|READ_FAILURE))
    {
        return;
    }

    arg_type->set(arg_value, input);

    if (arg_type->status == READ_NON_NUMERIC)
    {
        return;
    }

    arg_type->status = READ_SUCCESS;
}

static void *va_arg_char(va_list *args)
{
    return (void*)va_arg(*args, char*);
}

static void parse_char(void *arg, const char *str)
{
    char *char_arg = (char*)arg;
    *char_arg = str[0];
}

static void *va_arg_int(va_list *args)
{
    return (void*)va_arg(*args, int*);
}

static void parse_int(void *arg, const char *str)
{
    const long number = strtol(str, NULL, 10);
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

static void *va_arg_float(va_list *args)
{
    return (void*)va_arg(*args, float*);
}

static void parse_float(void *arg, const char *str)
{
    float *float_arg = (float*)arg;
    *float_arg = strtof(str, NULL);
}

static void *va_arg_short(va_list *args)
{
    return (void*)va_arg(*args, short*);
}

static void parse_short(void *arg, const char *str)
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

static void *va_arg_ushort(va_list *args)
{
    return (void*)va_arg(*args, unsigned short*);
}

static void parse_ushort(void *arg, const char *str)
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

static void *va_arg_long(va_list *args)
{
    return (void*)va_arg(*args, long*);
}

static void parse_long(void *arg, const char *str)
{
    long *long_arg = (long*)arg;
    *long_arg = strtol(str, NULL, 10);
}

static void *va_arg_double(va_list *args)
{
    return (void*)va_arg(*args, double*);
}

static void parse_double(void *arg, const char *str)
{
    double *double_arg = (double*)arg;
    *double_arg = strtod(str, NULL);
}

static void *va_arg_ulong(va_list *args)
{
    return (void*)va_arg(*args, unsigned long*);
}

static void parse_ulong(void *arg, const char *str)
{
    unsigned long *ulong_arg = (unsigned long*)arg;
    *ulong_arg = strtoul(str, NULL, 10);
}

static void *va_arg_uint(va_list *args)
{
    return (void*)va_arg(*args, unsigned int*);
}

static void parse_uint(void *arg, const char *str)
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

static void parse_str(ArgumentType *arg_type, va_list *args)
{
    char *input = va_arg(*args, char*);
    const size_t MAX_STR_SIZE = va_arg(*args, size_t);

    if (MAX_STR_SIZE == 0)
    {
        arg_type->status = READ_FAILURE;
        return;
    }

    parse_prompt(input, MAX_STR_SIZE, arg_type, "\n", true, stdin);

    if (arg_type->status == READ_EOF)
    {
        return;
    }

    arg_type->status = READ_SUCCESS;
}

// This function was inspired by this video:
// https://youtu.be/NsB6dqvVu7Y?t=231
static void parse_prompt(char *input, const size_t BUFFER_SIZE, ArgumentType *arg_type,
                         const char *delim, bool matched_delim, FILE *stream)
{
    int ch = getc(stream);
    size_t i = 0;
    const size_t LAST_INDEX = BUFFER_SIZE - 1;

    // If you are only using the prompt function,
    // we do not want you to read any newlines or spaces first.
    if (arg_type)
    {
        while (ch == '\n' || ch == ' ')
        {
            ch = getc(stream);
        }
    }

    while (ch != EOF && !(is_multiple_specifiers(arg_type, ch)))
    {
        // I can't remember why is_strchr has to go first.
        // I think it caused some bug, but I don't remember
        // what bug or how to replicate it.
        if (is_strchr(delim, ch) == matched_delim
            || is_space(arg_type, ch)
            || is_non_numeric(arg_type, ch))
        {
            // Clear only the input buffer. We do not want 
            // to clear any file buffers.
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

        // Even if we reach the MAX_SIZE we should
        // not clear the input buffer as there could be
        // multiple specifiers.
        if (i != LAST_INDEX)
        {
            input[i] = (char)ch;
            i++;
        }

        ch = getc(stream);
    }

    input[i] = '\0';

    // It should be a failure if the first index is 0.
    if (arg_type && input[0] == '\0')
    {
        arg_type->status = READ_FAILURE;
    }

    // It could be the eof.
    if (arg_type && ch == EOF)
    {
        arg_type->status = READ_EOF;
    }
}

static bool is_multiple_specifiers(ArgumentType *arg_type, int ch)
{
    return (arg_type && (arg_type->options & MULTIPLE_SPECIFIERS) && ch == ' ');
}

static bool is_strchr(const char *s, int ch)
{
    return (bool)(strchr(s, ch));
}

static bool is_space(ArgumentType *arg_type, int ch)
{
    return (arg_type && (arg_type->options & STOP_AT_SPACE) && ch == ' ');
}

static bool is_non_numeric(ArgumentType *arg_type, int ch)
{
    if (arg_type && (arg_type->options & NUMERICS_ONLY)
        && !((ch >= '0' && ch <= '9') || ch == '.' || ch == '-' ||
            ch == '+'  || ch == 'e' || ch == 'E'))
    {
        arg_type->status = READ_NON_NUMERIC;
        return true;
    }

    return false;
}
