# C_Prompt
The prompt library provides a clean and easy way
to get input from the user. There are many advantages
to using the prompt library compared to other means such as
scanf or fgets. prompt currently supports these format specifiers:
%c, %d, %f, %hi, %hu, %ld, %lf, %lu, %s, and %u.
prompt currently does not support pattern matching.

1. The buffer is flushed when using any prompt library functions.
scanf leaves \n in the buffer, and fgets just reads a certain number
of bytes from the buffer. So a problem could arise when
you use scanf or fgets followed by another input function
without clearing the buffer. Even if you do clear the buffer,
you might need to press the enter/return key one or more times.

2. prompt gives you protection from arithmetic overflow. If an
overflow happens, the strto... family will take care of it with
additional checking for types smaller than a long.

3. There is no need for explicit field width settings when using
prompt for strs. If you chose to use prompt to enter a str,
you can pass in the size of the str as an additional parameter.

4. The prompt library also provides a prompt_getline
and a prompt_getline_delim. If you use prompt to enter a str,
it will stop reading when it encounters a space. Using prompt_getline
or prompt_getline_delim will solve this issue.

5. prompt_getline_delim has some interesting features.
prompt_getline_delim takes two additional parameters,
a delim, and matched_delim. delim is a char* as
I wanted a way for the user to pass in multiple delims. The order of
the delim does not matter. matched_delim is a bool that will
exclude(true) or include(false) the delim.
For example, if the delim is "01" and the matched_delim is true,
then it will stop reading when it encounters a '0' or a '1'.
However, if matched_delim is false, then it will stop reading
only when it encounters a char that is NOT a '0' or a '1'.

6. The prompt functions have an additional parameter for
printing out your message to the user instead of calling printf
before you use an input function. It just makes
the code look cleaner imo.
