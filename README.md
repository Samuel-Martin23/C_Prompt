# C_Prompt
The prompt library provides a clean and easy way
to get input from the user. There are many advantages
to using the prompt library compared to other means such as
`scanf` or `fgets`.
(prompt currently does not support pattern matching.)

### Advantages of the prompt library:
1. When using any prompt library functions, the input buffer is flushed
and a '\n' is NOT added at the very end of an input string.
scanf leaves a '\n' in the buffer and is open to buffer overflow attacks.
fgets just reads a certain number of bytes from the buffer and includes
a '\n' at the very end of the input string.
So a problem could arise when you use scanf or fgets
followed by another input function without clearing the buffer.
Even if you do clear the buffer, you might need to press
the enter/return key one or more times.

2. There is no need for explicit field width settings when using
prompt for strs. If you chose to use prompt to enter a str,
you must pass in the size of the str as an additional parameter.
	```c
	char name[50] = "";
	prompt("Enter name: ", "%s", name, 50);
	```

3. prompt gives you protection from arithmetic overflow. If an
overflow happens, the strto... family will take care of it with
additional checking for types smaller than a long. 
	```c
	int x = 0;
	int y = 0;
	prompt("Enter two numbers: ", "%d%d", &x, &y);	// 43654786443577646 378786
	printf("The two numbers are: %d %d", x, y); 	// The two numbers are: 2147483647 378786
	```

4. The prompt library also provides prompt_getline,
prompt_getline_stream, prompt_getline_delim,
and prompt_getline_delim_stream. If you use prompt
to enter a str, it will stop reading when it encounters a space.
	```c
	char name[50] = "";
	prompt("Enter name: ", "%s", name, 50);	// Samuel Martin
	printf("Your name is %s", name);	// Your name is Samuel
	```
	Using prompt_getline will solve this issue.
	```c
	char name[50] = "";
	prompt_getline("Enter name: ", name, 50);	// Samuel Martin
	printf("Your name is %s", name);		// Your name is Samuel Martin
	```
	
	You can also read files using prompt_getline_stream or prompt_getline_delim_stream.
	```c
	char line[1000] = "";
	FILE *fp = fopen("file.txt", "r");
	
	while (prompt_getline_stream("", line, 1000, fp) == 1)
	{
	    printf("%s\n", line);
	}

	fclose(fp);
	```

5. prompt_getline_delim has some interesting features.
prompt_getline_delim takes two additional parameters,
a delim, and a matched_delim. delim is a const char* as
a way for the user to pass in multiple delims. The order of
the delims does not matter. matched_delim is a bool that will
exclude(true) or include(false) the delim.
For example, if the delim is "01\n" and the matched_delim is true,
then it will stop reading when it encounters a '0', a '1', or a '\n'.
	```c
	char exper[50] = "";
	prompt_getline_delim("Enter experiment: ", exper, 50, "01\n", true);	// Test 23123
	printf("Your experiment is %s", name);					// Your experiment is Test 23
	```
	However, if the delim is "01" and the matched_delim is false,
	then it will stop reading only when it encounters a char that is NOT a '0' or a '1'.
	```c
	char bi_num[50] = "";
	prompt_getline_delim("Enter binary number: ", bi_num, 50, "01", false);		// 1012010 
	printf("Your binary number is %s", bi_num);                 			// Your binary number is 101
	```

6. The prompt functions have an additional parameter for
printing out your message to the user instead of calling printf
before you use an input function.

### Format specifiers supported by the prompt function:
Format Specifier  | Data Type
------------- | -------------
%c  | char
%d  | int
%f  | float
%hi  | short
%hu  | unsigned short
%ld  | long
%lf  | double
%lu  | unsigned long
%s  | string
%u | unsigned int
