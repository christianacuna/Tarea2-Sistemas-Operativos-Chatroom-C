#include <stdio.h>
#include "../util/debugger.h"

#define MSG_BUFFER 32
#define FILE_SIZE 1024

// Validation code constant
char OK[MSG_BUFFER] = "200";

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

/**
 * Validates if the user credentials are  correct
 *
 * @param name username
 * @param password user's password
 * @param sockfd socket file descriptor
 * @return a boolean value
 */
int logIn(char *name, char *password, int sockfd)
{
    int isValid = 0;
    char message[MSG_BUFFER * 2];
    memcpy(message, name, MSG_BUFFER);
    memcpy(message + MSG_BUFFER, password, MSG_BUFFER);
    send(sockfd, message, MSG_BUFFER * 2, 0);

    // Server response
    char response[MSG_BUFFER];
    int receive = recv(sockfd, response, MSG_BUFFER, 0);
    if (receive > 0)
    {
        if (strcmp(response, OK) == 0)
        {
            isValid = 1;
        }
        else
        {
            console.error("Server respones validation error");
        }
    }
    else if (receive == 0)
    {
        console.error("Server timeout");
    }
    else
    {
        console.error("Unknown login error");
    }
    memset(response, 0, sizeof(response));

    return isValid;
}