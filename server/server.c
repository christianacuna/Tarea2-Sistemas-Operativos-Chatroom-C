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
#include "../util/stringy.c"

#define MAX_CLIENTS 100
#define MAX_GROUPS 100
#define BUFFER_SIZE 2048

static _Atomic unsigned int clientCount = 0;
static int uid = 10;

/* Client structure */
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} Client;

typedef struct 
{
	Client listClients[MAX_CLIENTS];
	int gid;
} Group;

Group *groups[MAX_GROUPS];

Client *clients[MAX_CLIENTS];
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;

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
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (!groups[i])
		{
			groups[i] = gp;
			break;
		}
	}

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Removes Group from the Group List
 *
 * @param gid int group id
 */
void groupRemove(int gid)
{
	pthread_mutex_lock(&clientsMutex);

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

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Adds Group to the Group list
 *
 * @param cl Client struct
 * @param gid Int Group id
 */
void clientGroupAdd(Client *cl, int gid)
{
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (groups[i])
		{
			if (groups[i]->gid == gid)
			{
				for (int j = 0; j < MAX_CLIENTS; ++j)
				{
					if(!groups[i]->listCLients[j])
					{
						groups[i]->listClients[j] = cl;
						break;
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&clientsMutex);
}

/**
 * Removes Group from the Group List
 *
 * @param gid int group id
 * @param uid int user id
 */
void clientGroupRemove(int, uid int gid)
{
	pthread_mutex_lock(&clientsMutex);

	for (int i = 0; i < MAX_GROUPS; ++i)
	{
		if (groups[i])
		{
			if (groups[i]->gid == gid)
			{
				for (int j = 0; j < MAX_CLIENTS; ++j)
				{
					if(groups[i]->listCLients[j])
					{
						if(groups[i]->listCLients[j]->uid == uid)
						{
							groups[i]->listClients[j] = NULL;
							break;
						}
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&clientsMutex);
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
 * Handles all communication with the clients
 *
 * @param arg Client reference
 */
void *handleClient(void *arg)
{
	char buff_out[BUFFER_SIZE];
	char name[32];
	int leave_flag = 0;

	clientCount++;
	Client *cli = (Client *)arg;

	// Name
	if (recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32 - 1)
	{
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	}
	else
	{
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		sendMessage(buff_out, cli->uid, 0);
	}

	bzero(buff_out, BUFFER_SIZE);

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
				sendMessage(buff_out, cli->uid, 0);

				strTrimLf(buff_out, strlen(buff_out));
				printf("%s -> %s\n", buff_out, cli->name);
			}
		}
		else if (receive == 0 || strcmp(buff_out, "exit") == 0)
		{
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			sendMessage(buff_out, cli->uid, 0);
			leave_flag = 1;
		}
		else
		{
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SIZE);
	}

	/* Delete client from queue and yield thread */
	close(cli->sockfd);
	queueRemove(cli->uid);
	free(cli);
	clientCount--;
	pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
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
		perror("ERROR: setsockopt failed");
		return EXIT_FAILURE;
	}

	/* Bind */
	if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("ERROR: Socket binding failed");
		return EXIT_FAILURE;
	}

	/* Listen */
	if (listen(listener, 10) < 0)
	{
		perror("ERROR: Socket listening failed");
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
			printf("Max clients reached. Rejected: ");
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
		pthread_create(&threadId, NULL, &handleClient, (void *)cli);

		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
