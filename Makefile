CC := clang
FLAGS := -std=c17 -Wall -Wextra -Wconversion -Wunreachable-code -Wnewline-eof -Wno-error=unused-variable -Wshadow -Wfloat-equal -Wcovered-switch-default -Wunreachable-code-break
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))
HEADERS := $(wildcard *.h)

LIBRARY_NAME := prompt
LIBRARY_SOURCES := $(filter-out main.c,$(wildcard *.c))

EXECUTABLE_NAME := program

%.o: %.c $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

$(EXECUTABLE_NAME): $(OBJECTS)
	$(CC) $^ -o $@

shared:
	$(CC) $(FLAGS) -dynamiclib $(LIBRARY_SOURCES) -o lib$(LIBRARY_NAME).dylib -install_name @rpath/lib$(LIBRARY_NAME).dylib

clean:
	rm *.o
	rm $(EXECUTABLE_NAME)
