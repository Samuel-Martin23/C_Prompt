CC = clang
flags = -std=c17 -Wall -Wextra -Wconversion -Wunreachable-code -Wnewline-eof -Wno-error=unused-variable -Wshadow -Wfloat-equal -Wcovered-switch-default -Wunreachable-code-break

name_of_lib = prompt
file_to_compile = prompt
object_files = main.o prompt.o
name_of_executable = program

$(name_of_executable): $(object_files)
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(flags) -c $^ -o $@

prompt.o: prompt.c prompt.h
	$(CC) $(flags) -c prompt.c -o $@

shared:
	$(CC) -dynamiclib $(file_to_compile).c -o lib$(name_of_lib).dylib -install_name @rpath/lib$(name_of_lib).dylib

clean:
	rm *.o $(name_of_executable)