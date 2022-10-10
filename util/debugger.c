#include <stdio.h>
#include "debugger.h"
#include "date_time.c"
#include "file_manager.h"

#define DIR "../logs/"
#define BUFFER 128
#define DATE_LENGTH 28

char filename[BUFFER + DATE_LENGTH];
char content[BUFFER + DATE_LENGTH];

static void my_log(char *s)
{
    char dateTime[DATE_LENGTH];
    printf("[+] %s\n", s);
    getDateTime(dateTime);
    sprintf(content, "INFO (%s): %s\n", dateTime, s);
    writeFile(filename, content);
}
static void my_error(char *s)
{
    char dateTime[DATE_LENGTH];
    printf("[-] Error: %s\n", s);
    getDateTime(dateTime);
    sprintf(content, "ERROR (%s): %s\n", dateTime, s);
    writeFile(filename, content);
}
namespace_struct const console = {my_log, my_error};

/**
 * Starts a new log file
 *
 * @param descriptor addition to the filename
 */
void startLog(char *descriptor)
{
    char dateTime[DATE_LENGTH];
    getDateTime(dateTime);
    sprintf(filename, "%s%s-log-%s.txt", DIR, descriptor, dateTime);
    writeFile(filename, "Log started\n");
}
