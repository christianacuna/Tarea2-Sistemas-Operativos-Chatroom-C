#include <stdio.h>
#include "file_manager.h"
#include "../util/debugger.h"

/**
 * Opens the file if available and can be read
 *
 * @param filename name of thefile to open
 * @return a File type reference
 */
FILE *openFile(char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        console.error("Reading file");
    }
    return fp;
}

/**
 * Writes to a new file
 *
 * @param filename
 * @param content
 */
void writeFile(char *filename, char *content)
{
    FILE *fp = fopen(filename, "a");

    if (fp == NULL)
    {
        printf("File %s can't be opened\n", filename);
    }

    // Write
    fflush(stdin);
    fputs(content, fp);

    fclose(fp);
}