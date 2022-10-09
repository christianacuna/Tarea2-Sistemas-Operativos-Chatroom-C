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

/**
 * Removes all unecessary characters of the string
 * @param arr string reference
 * @param length array size
 */
void strTrimAll(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        switch (arr[i])
        {
        case '\n':
            arr[i] = '\0';
            break;
        case '\t':
            arr[i] = '\0';
            break;
        case ' ':
            arr[i] = '\0';
            break;
        }
    }
}