#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * Namespace struct
 *
 * @source https://stackoverflow.com/questions/389827/namespaces-in-c
 */
typedef struct
{
    void (*const log)(char *);
    void (*const error)(char *);
} namespace_struct;
extern namespace_struct const console;

/**
 * Starts a new log file
 *
 * @param descriptor addition to the filename
 */
void startLog(char *descriptor);

#endif // CONSOLE_H