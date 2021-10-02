#include "prompt.h"

int main()
{
    int age = 0;
    char first_name[20];

    prompt("Enter name: ", "%s", first_name, sizeof(first_name));
    prompt("Enter age: ", "%d", &age);

    printf("%s\n", first_name);
    printf("%d\n", age);

    return 0;
}
