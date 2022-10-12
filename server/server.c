#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include "authenticator.c"
#include "../util/stringy.c"
#include "../util/debugger.h"

#define MAX_CLIENTS 100
#define MAX_GROUPS 100
#define BUFFER_SIZE 2048
#define MSG_BUFFER 32

#define SEVER_OK "200"
#define SEVER_ERROR "500"

/* Server commands accepted from Client */
#define GROUP_JOIN_CMD "$join"
#define GROUP_CREATE_CMD "$create"
#define GROUP_LIST_CMD "$glist"
#define GROUP_MEMBER_LIST_CMD "$gmemlist"

/* First character from client to allow server command execution */
#define SERVER_CMD_CHAR '$'

static _Atomic unsigned int clientCount = 0;
static int uid = 10;
int leave_flag = 0;

/* Client structure */
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[MSG_BUFFER];
} Client;

/* Group structure */
typedef struct 
{
	Client *listClients[MAX_CLIENTS];
	int gid;
} Group;

Group *groups[MAX_GROUPS];

Client *clients[MAX_CLIENTS];
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t groupMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Prints the address in IP format
 *
 * @param addr address to print
 */
void printClientAddr(struct sockaddr_in addr)
{
	printf("%d.%d.%d.%d",
		   addr.sin_addr.s_addr & 0xff,
		   (addr.sin_addr.s_addr & 0xff00) >> 8,
		   (addr.sin_addr.s_addr & 0xff0000) >> 16,
		   (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/**
 * Adds client to the queue
 *
 * @param cl Client struct
 */
void queueAdd(Client *cl)
{
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Removes client from the queue
 *
 * @param uid client user id
 */
void queueRemove(int uid)
{
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid == uid)
			{
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Adds Group to the Group list
 *
 * @param gp Group struct
 */
void groupAdd(Group *gp)
{
	pthread_mutex_lock(&groupMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (!groups[i])
		{
			groups[i] = gp;
			break;
		}
	}

	pthread_mutex_unlock(&groupMutex);
}

/**
 * Removes Group from the Group List
 *
 * @param gid int group id
 */
void groupRemove(int gid)
{
	pthread_mutex_lock(&groupMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (groups[i])
		{
			if (groups[i]->gid == gid)
			{
				groups[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&groupMutex);
}

/**
 * Adds Group to the Group list
 *
 * @param cl Client struct
 * @param gid Int Group id
 */
void clientGroupAdd(Client *cl, int gid)
{
	printf("its here:%d", gid);
	pthread_mutex_lock(&groupMutex);
	//char buff_out[BUFFER_SIZE];
	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (groups[i])
		{
			if (groups[i]->gid == gid)
			{
				for (int j = 0; j < MAX_CLIENTS; ++j)
				{
					if (!groups[i]->listClients[j])
					{
						groups[i]->listClients[j] = cl;
						//buff_out = "Group Joined\n";
						break;
					}
				}
			}
			else
			{
				//buff_out = "Group does not exist\n";
			}
		}
	}
	//sendMessage(buff_out, cl->uid, 0);
	pthread_mutex_unlock(&groupMutex);
}

/**
 * Removes Group from the Group List
 *
 * @param gid int group id
 * @param uid int user id
 */
void clientGroupRemove(int uid, int gid)
{
	pthread_mutex_lock(&groupMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (groups[i])
		{
			if (groups[i]->gid == gid)
			{
				for (int j = 0; j < MAX_CLIENTS; ++j)
				{
					if (groups[i]->listClients[j])
					{
						if (groups[i]->listClients[j]->uid == uid)
						{
							groups[i]->listClients[j] = NULL;
							break;
						}
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&groupMutex);
}

/**
 * Send message to all clients except sender
 *
 * @param s message to send
 * @param uid user id
 */
void sendMessage(char *s, int uid, int gid)
{
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid != uid)
			{
				if (write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Validates the user information with the Authenticator
 *
 * @param message buffered message received
 * @param name username to validate
 * @param sockfd socket file descriptor
 */
int validateClient(char message[MSG_BUFFER * 2], char name[MSG_BUFFER], int sockfd)
{
	int isValid = 0;
	char password[MSG_BUFFER];

	// Splits the message to extract its content
	memcpy(name, message, MSG_BUFFER);
	memcpy(password, message + MSG_BUFFER, MSG_BUFFER);

	if (strlen(name) < 2 || strlen(name) >= MSG_BUFFER - 1)
	{
		console.error("Invalid user name length");
	}
	else if (strlen(password) < 2 || strlen(password) >= MSG_BUFFER - 1)
	{
		console.error("Invalid password length");
	}
	else
	{

		char *response = malloc(sizeof(char) * MSG_BUFFER);
		isValid = authenticate(name, password);
		if (isValid == 1)
		{
			sprintf(response, "%s", SEVER_OK);
			write(sockfd, response, MSG_BUFFER);
			bzero(response, MSG_BUFFER);
		}
		else
		{
			sprintf(response, "%s", SEVER_ERROR);
			write(sockfd, response, MSG_BUFFER);
			console.error("Authentication failed, wrong password or username");
		}
		free(response);
	}

	return isValid;
}
/**
 * Eliminates empty spaces on an Char Array.
 * 
 * @param array char array to be cleaned.
*/
void cleanInputArray(char **array)
{
	int i = 0;
	while(array[i] != NULL){
		strTrimLf(array[i], strlen(array[i]));
		i++;
	}
}
/**
 * Operates which command was issued to the server by the client
 * 
 * @param buffer buffer recieved by the client
*/
void cmdHandler(char **buffer,Client *cl){
	pthread_mutex_lock(&clientsMutex);

	char *command = buffer[1];
	char *argument = buffer[2];
	console.log("Client sent command request");
	if (strstr(command, GROUP_JOIN_CMD) != NULL){
		console.log("User tried to Join Group");
		int group_id = atoi(argument);
		printf("stop here size: %ld", sizeof(group_id));
		clientGroupAdd(cl,group_id);
	}
	else if (strstr(command,  GROUP_CREATE_CMD) != NULL){
		console.log("User tried to Create Group");
	}
	else if (strstr(command, GROUP_LIST_CMD) != NULL){
		console.log("User tried to query Group List");
	}
	else if (strstr(command, GROUP_MEMBER_LIST_CMD) != NULL){
		console.log("User tried to query for Member List in a group");
	}
	else{
		console.log("User tried to use invalid command");
	}
	pthread_mutex_unlock(&clientsMutex);
}
/**
 * Splits a String to an Array using delimiter
 *
 * @param str String to split
 * @param array Char Array to store split output
 * @param delimiter Char guide split str
 */
void splitStrToArray(char *str, char **array, char *delimiter)
{
	pthread_mutex_lock(&clientsMutex);

    int i = 0;
	char *token = strtok (str, delimiter);
    while (token != NULL)
    {
        array[i++] = token;
        token = strtok (NULL, delimiter);
    }

	pthread_mutex_unlock(&clientsMutex);
}
/**
 * Handles all communication with the clients
 *
 * @param arg Client reference
 */
void *clientListener(Client *cli)
{
	char buff_out[BUFFER_SIZE];

	clientCount++;

	while (1)
	{
		if (leave_flag)
		{
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SIZE, 0);
		if (receive > 0)
		{
			if (strlen(buff_out) > 0)
			{
				char buff_out_copy[BUFFER_SIZE];
				//printf("test: %s",buff_out);
				// Makes copy of the buff so we can process it without damaging original
				strcpy(buff_out_copy, buff_out);

				// Array on which we store client input splited 
				char *stored[3];
				splitStrToArray(buff_out_copy,stored, " ");
				strTrimLf(stored[1], strlen(stored[1]));
				
				// Verify if there if private character is recognized as the start of new command
				if (stored[1][0] == SERVER_CMD_CHAR){
					cmdHandler(stored,cli);
				}
				sendMessage(buff_out, cli->uid, 0);
				strTrimLf(buff_out, strlen(buff_out));
				printf("%s -> %s\n", buff_out, cli->name);
			}
		}
		else if (receive == 0 || strcmp(buff_out, "/exit") == 0)
		{
			sprintf(buff_out, "%s has left", cli->name);
			console.log(buff_out);
			//sendMessage(buff_out, cli->uid, 0);
			bzero(buff_out, BUFFER_SIZE);
			break;
			//leave_flag = 1;
		}
		else
		{
			console.error("Buffering client");
			bzero(buff_out, BUFFER_SIZE);
			break;
			//leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SIZE);
	}

	/* Delete client from queue and yield thread */
	printf("TESTO 1\n");
	close(cli->sockfd);
	printf("TESTO 2\n");
	queueRemove(cli->uid);
	printf("TESTO 3\n");
	free(cli);
	printf("TESTO 4\n");
	clientCount--;
	printf("TESTO 9\n");
	pthread_detach(pthread_self());

	return NULL;
}

/**
 * Handler to wait until client successfully gets authenticated
 *
 * @param arg Client reference
 */
void *newClientHandler(void *arg)
{
	char buff_out[BUFFER_SIZE];
	char message[MSG_BUFFER * 2];
	char name[MSG_BUFFER];
	Client *cli = (Client *)arg;

	// Authenticate user
	int isNotAuthenticated = 1;
	while (isNotAuthenticated)
	{
		if (leave_flag)
		{
			break;
		}

		// Read message
		int result = recv(cli->sockfd, message, MSG_BUFFER * 2, 0) <= 0;
		if (result)
		{
			console.error("Invalid user buffered data");
			break;
			//leave_flag = 1;
		}

		// Authenticated correctly
		if (validateClient(message, name, cli->sockfd) == 1)
		{
			isNotAuthenticated = 0;
			strcpy(cli->name, name);
			sprintf(buff_out, "%s has joined", cli->name);
			console.log(buff_out);
			bzero(buff_out, BUFFER_SIZE);
		}
		bzero(message, MSG_BUFFER * 2);
		bzero(name, MSG_BUFFER);
	}

	// Passes priority to the listener handler
	return clientListener(cli);
}

int main(int argc, char **argv)
{
	startLog("server");
	if (argc > 3 || argc < 1)
	{
		printf("Usage: %s <port> <ip - (default:127.0.0.1)>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	if (argv[2] != NULL){
		ip = argv[2];
	}
	int port = atoi(argv[1]);
	int option = 1;
	int listener = 0, conn = 0;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	pthread_t threadId;

	/* Socket settings */
	listener = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	serverAddr.sin_port = htons(port);

	/* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if (setsockopt(listener, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
	{
		console.error("ERROR: setsockopt failed");
		return EXIT_FAILURE;
	}

	/* Bind */
	if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		console.error("ERROR: Socket binding failed");
		return EXIT_FAILURE;
	}

	/* Listen */
	if (listen(listener, 10) < 0)
	{
		console.error("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}

	printf("=== WELCOME TO THE CHATROOM ===\n");

	while (1)
	{
		socklen_t clientLen = sizeof(clientAddr);
		conn = accept(listener, (struct sockaddr *)&clientAddr, &clientLen);

		/* Check if max clients is reached */
		if ((clientCount + 1) == MAX_CLIENTS)
		{
			console.log("Max clients reached. Rejected: ");
			printClientAddr(clientAddr);
			printf(":%d\n", clientAddr.sin_port);
			close(conn);
			continue;
		}

		/* Client settings */
		Client *cli = (Client *)malloc(sizeof(Client));
		cli->address = clientAddr;
		cli->sockfd = conn;
		cli->uid = uid++;

		/* Add client to the queue and fork thread */
		queueAdd(cli);
		pthread_create(&threadId, NULL, &newClientHandler, (void *)cli);
		//leave_flag = 0;
		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
