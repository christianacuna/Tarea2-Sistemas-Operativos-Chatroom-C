#include <stdio.h>

void strOverwriteStdout()
{
    printf("\r%s", "> ");
    fflush(stdout);
}

void strTrimLf(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
    { // trim \n
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}