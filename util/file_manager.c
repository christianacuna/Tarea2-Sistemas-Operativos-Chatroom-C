#include <stdio.h>
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
        exit(1);
    }
    return fp;
}
