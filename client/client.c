#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "message_manager.c"
#include "../util/file_manager.h"
#include "../util/stringy.c"
#include "../util/debugger.h"

// Message constants
#define CMD_LENGTH 64
#define MSG_LENGTH 2048
#define MSG_BUFFER 32

// User commands
#define EXIT_CMD "/exit"
#define MENU_CMD "/menu"
#define CHAT_CMD "/chat"
#define FILE_CMD "/file"
#define CREATE_GROUP_CMD "/cgroup"
#define JOIN_GROUP_CMD "/jgroup"
#define CMD_COUNT 6 // Update this base on the number of commands

// Client Private Characters
#define SERVER_CMD '&'
#define NEW_REPLACE '$'

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[MSG_BUFFER];
char *commands[CMD_COUNT] = {CHAT_CMD, FILE_CMD, MENU_CMD, CREATE_GROUP_CMD, JOIN_GROUP_CMD ,EXIT_CMD};

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
 * Handler that makes sure user input does not contain restricted characters, 
 * if so replaces them
 * @param input user input
 * @param length user input expected length
*/
void userInputCleaner(char *input, int length)
{
	if(input[0] == SERVER_CMD)
	{
		console.log("User tried to send message with restricted character, replacing with new");
		input[0] = NEW_REPLACE;
	}
}


/**
 * Handler that waits for any user input
 *
 * @param input user input
 * @param length user input expected length
 */
void userInputHandler(char *input, int length)
{
	strOverwriteStdout();
	fgets(input, length, stdin);
	strTrimLf(input, length);
	userInputCleaner(input, length);
}

/**
 * Handler that waits for user text message
 */
void sendMsgHandler()
{
	char message[MSG_LENGTH];
	printf("Please input a group id or user name:\n");
	userInputHandler(message, MSG_LENGTH);
	sendMsg(name, message, MSG_LENGTH, sockfd);
	bzero(message, MSG_LENGTH);
}
/**
 * Handler that sends the correct command to the server to join groups
*/
void sendGroupJoinHandler()
{
	char message[MSG_LENGTH];
	printf("Please input group id(Numbers) you want to join [0-99]");
	userInputHandler(message, MSG_LENGTH);
	sendMsg(name,message,MSG_LENGTH, sockfd);
	bzero(message, MSG_LENGTH);

}

/**
 * Handler that sends the correct command to the server to join groups
*/
void sendGroupCreateHandler()
{
	int new_msg_length = MSG_LENGTH - 5;
	char message[new_msg_length];
	char join_command[5] = "join_";
	printf("Please input group id(Numbers) you want to create [0-99]");
	userInputHandler(message, new_msg_length);
	strcat(join_command,message);
	sendMsg(name,join_command,MSG_LENGTH, sockfd);
	bzero(message, MSG_LENGTH);

}
/**
 * Handler that waits for user file to send
 */
void sendFileHandler()
{
	FILE *fp = openFile("../README.md");
	sendFile(fp, sockfd);
	fclose(fp);
	console.log("File data sent successfully");
}

/**
 * Handler that waits for user commands
 */
void userCmdHandler()
{
	char command[CMD_LENGTH];
	printf("Please input a command:\n");
	// Prints the commands list
	for (int i = 0; i < CMD_COUNT; i++)
	{
		printf("%s\n", commands[i]);
	}
	// Waits for user input
	userInputHandler(command, CMD_LENGTH);
	// Switch statement
	if (strcmp(command, EXIT_CMD) == 0)
	{
		console.log("User exiting...");
		//sendMsg(name, EXIT_CMD, 5, sockfd);
		bzero(command, CMD_LENGTH);
		exitOnCommand(2);
	}
	else if (strcmp(command, MENU_CMD) == 0)
	{
		console.log("Returning to menu...");
		bzero(command, CMD_LENGTH);
	}
	else if (strcmp(command, CHAT_CMD) == 0)
	{
		bzero(command, CMD_LENGTH);
		sendMsgHandler();
	}
	else if (strcmp(command, FILE_CMD) == 0)
	{
		console.log("User sending file...");
		bzero(command, CMD_LENGTH);
		sendFileHandler();
	}
	else if (strcmp(command, CREATE_GROUP_CMD) == 0)
	{
		console.log("User trying to create a group...");
		bzero(command, CMD_LENGTH);
		sendMsgHandler();
	}
	else if (strcmp(command, JOIN_GROUP_CMD) == 0)
	{
		console.log("User trying to join a group...");
		bzero(command, CMD_LENGTH);
		sendMsgHandler();
	}
	else
	{
		console.error("Invalid command");
		bzero(command, CMD_LENGTH);
	}
}

/**
 * Handler that receives a Server message
 */
void receiveMsgHandler()
{
	char message[MSG_LENGTH];
	while (flag)
	{
		int receive = recv(sockfd, message, MSG_LENGTH, 0);
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

/**
 * Handles the user authentication with the server
 */
void loginHandler()
{
	char password[MSG_BUFFER];

	// User name input
	while (flag == 0)
	{
		printf("Please enter your name: ");
		fgets(name, MSG_BUFFER, stdin);
		strTrimAll(name, strlen(name));

		// Validate name size
		if (strlen(name) > MSG_BUFFER || strlen(name) < 2)
		{
			console.error("Name must be less than 30 and more than 2 characters");
			continue;
		}

		break;
	}

	// User password input
	while (flag == 0)
	{
		printf("Please enter your password: ");
		fgets(password, MSG_BUFFER, stdin);
		strTrimAll(password, strlen(password));

		// Validate password size
		if (strlen(password) > MSG_BUFFER || strlen(password) < 2)
		{
			console.error("Password must be less than 30 and more than 2 characters");
			continue;
		}

		break;
	}

	// Serve validation
	if (logIn(name, password, sockfd) == 1)
	{
		console.log("User logged in successfully");
	}
	else
	{
		console.error("Sever authentication failed");
		loginHandler();
	}

	bzero(password, MSG_BUFFER);
}

/**
 * Main program cycle
 */
void start()
{
	while (flag == 0)
	{
		userCmdHandler();
	}
}

int main(int argc, char **argv)
{
	startLog("client");
	// Port in use
	if (argc > 3 || argc < 1)
	{
		char errorMsg[48];
		sprintf(errorMsg, "Port already in use: %s <port> <ip>\n", argv[0]);
		console.error(errorMsg);
		bzero(errorMsg, 48);
		return EXIT_FAILURE;
	}

	// Subscribes the exit on command signal
	signal(SIGINT, exitOnCommand);

	// Socket settings
	char *ip = "127.0.0.1";
	if (argv[2] != NULL){
		ip = argv[2];
	}
	int port = atoi(argv[1]);
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	// Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1)
	{
		console.error("Connection to server failed");
		return EXIT_FAILURE;
	}

	// Authenticate user
	loginHandler();

	printf("=== WELCOME TO THE CHATROOM ===\n");

	// Start main program thread
	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, (void *)start, NULL) != 0)
	{
		console.error("Pthread could not be created");
		return EXIT_FAILURE;
	}

	// Start listening server thread
	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, (void *)receiveMsgHandler, NULL) != 0)
	{
		console.error("Pthread could not be created");
		return EXIT_FAILURE;
	}

	while (1)
	{
		if (flag)
		{
			console.log("User session ended");
			break;
		}
	}
	printf("TEST 9");
	close(sockfd);

	return EXIT_SUCCESS;
}
