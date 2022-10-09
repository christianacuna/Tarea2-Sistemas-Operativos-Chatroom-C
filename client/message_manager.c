#include <stdio.h>
#include "../util/debugger.h"

#define MSG_BUFFER 32
#define FILE_SIZE 1024

/**
 * Sends a message via socket
 *
 * @param name username
 * @param msg message to send
 * @param length message char length
 * @param sockfd socket file descriptor
 */
void sendMsg(char *name, char *msg, int length, int sockfd)
{
    int size = length + MSG_BUFFER + 2;
    char *buffer = malloc(sizeof(char) * size);
    sprintf(buffer, "%s: %s\n", name, msg);
    send(sockfd, buffer, strlen(buffer), 0);
    free(buffer);
}

/**
 * Sends a file via socket
 *
 * @param fp file reference to send
 * @param sockfd socket file descriptor
 */
void sendFile(FILE *fp, int sockfd)
{
    char data[FILE_SIZE] = {0};

    while (fgets(data, FILE_SIZE, fp) != NULL)
    {
        if (send(sockfd, data, sizeof(data), 0) == -1)
        {
            console.error("Sending file");
            exit(1);
        }
        bzero(data, FILE_SIZE);
    }
}
