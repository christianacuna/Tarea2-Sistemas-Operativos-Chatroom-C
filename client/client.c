#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../util/stringy.c"

#define LENGTH 2048
#define MSG_BUFFER 32

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[MSG_BUFFER];
char password[MSG_BUFFER];

/**
 * Used for Ctrl + C exit command
 *
 * @param sig
 */
void exitOnCommand(int sig)
{
	flag = 1;
}

/**
 * Handler that sends the message to the Server
 */
void sendMsgHandler()
{
	char message[LENGTH] = {};
	char buffer[LENGTH + MSG_BUFFER + 2] = {};

	while (1)
	{
		strOverwriteStdout();
		fgets(message, LENGTH, stdin);
		strTrimLf(message, LENGTH);

		if (strcmp(message, "exit") == 0)
		{
			break;
		}
		else
		{
			sprintf(buffer, "%s: %s\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + MSG_BUFFER);
	}
	exitOnCommand(2);
}

/**
 * Handler that receives a Server message
 */
void receiveMsgHandler()
{
	char message[LENGTH] = {};
	while (1)
	{
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0)
		{
			printf("%s", message);
			strOverwriteStdout();
		}
		else if (receive == 0)
		{
			break;
		}
		else
		{
			// -1
		}
		memset(message, 0, sizeof(message));
	}
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

	signal(SIGINT, exitOnCommand);

	// User name input
	printf("Please enter your name: ");
	fgets(name, MSG_BUFFER, stdin);
	strTrimAll(name, strlen(name));

	if (strlen(name) > MSG_BUFFER || strlen(name) < 2)
	{
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	// User password input
	printf("Please enter your password: ");
	fgets(password, MSG_BUFFER, stdin);
	strTrimAll(password, strlen(password));

	if (strlen(password) > MSG_BUFFER || strlen(password) < 2)
	{
		printf("Password must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	// Socket settings
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	// Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1)
	{
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send message
	char message[64];
	memcpy(message, name, MSG_BUFFER);
	memcpy(message + MSG_BUFFER, password, MSG_BUFFER);
	send(sockfd, message, 64, 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");

	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, (void *)sendMsgHandler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, (void *)receiveMsgHandler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1)
	{
		if (flag)
		{
			printf("\nBye\n");
			break;
		}
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
