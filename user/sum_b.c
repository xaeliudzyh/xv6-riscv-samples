//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main()
{
    const int BUFFER_SIZE = 100;
    char buffer[BUFFER_SIZE];
    int number1, number2;
    if (!gets(buffer, BUFFER_SIZE))
    {
        fprintf(2, "Read error.\n");
        return 1;
    }

    if (strlen(buffer) == 1)
    {
        fprintf(2, "Error: empty string.\n");
        return 1;
    }

    char *space = strchr(buffer, ' ');
    if (!space)
    {
        fprintf(2, "Error: numbers should be separated by a space.\n");
        return 1;
    }

    *space = '\0';
    number1 = atoi(buffer);
    number2 = atoi(space + 1);
    for (char *p = buffer; *p; p++)
        if (*p != ' ' && *p != '-' && !(*p >= '0' && *p <= '9'))
        {
            fprintf(2, "Error: the first number contains invalid characters.\n");
            return 1;
        }

    for (char *p = space + 1; *p; p++)
        if (*p != ' ' && *p != '\n' && *p != '-' && !(*p >= '0' && *p <= '9'))
        {
            fprintf(2, "Error: the second number contains invalid characters.\n");
            return 1;
        }

    printf("%d\n", number1 + number2);
    return 0;
}
// Created by George Tsagol on 05.03.2024.
//
