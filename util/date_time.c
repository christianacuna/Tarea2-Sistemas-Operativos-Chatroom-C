#include <time.h>

/**
 * Gets current date and time in yyyy/mm/dd_hh-mm-ss format
 *
 * @param dateTime string to write
 */
void getDateTime(char *dateTime)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(dateTime, "%d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
