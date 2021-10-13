#include "prompt.h"

int main()
{
    int age = 0;
    char name[50];
    float height = 0.0f;
    float weight = 0.0f;

    prompt("Enter name: ", "%s", name, sizeof(name));
    prompt("Enter age: ", "%d", &age);
    prompt("Enter height followed by weight: ", "%f%f", &height, &weight);

    printf("\nName: %s\n", name);
    printf("Age: %d\n", age);
    printf("Height: %.4f\n", height);
    printf("Weight: %.4f\n", weight);

    return 0;
}
