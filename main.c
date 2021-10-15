#include "prompt.h"

int main()
{
    char name[50] = {0};
    char color[10] = {0};
    int age = 0;
    float height = 0.0f;
    float weight = 0.0f;
    char movie[50] = {0};

    prompt_getline("Enter Name: ", name, sizeof(name));
    prompt("Enter favorite color: ", "%s", color, sizeof(color));
    prompt("Enter age: ", "%d", &age);
    prompt("Enter height followed by weight: ", "%f%f", &height, &weight);
    prompt_getline_delim("Enter Movie: ", movie, sizeof(movie), 's');

    printf("\nName: %s\n", name);
    printf("Color: %s\n", color);
    printf("Age: %d\n", age);
    printf("Height: %.4f\n", height);
    printf("Weight: %.4f\n", weight);
    printf("Movie: %s\n", movie);

    return 0;
}
