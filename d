[1mdiff --git a/Makefile b/Makefile[m
[1mindex aa0758d..056f38a 100644[m
[1m--- a/Makefile[m
[1m+++ b/Makefile[m
[36m@@ -1,6 +1,7 @@[m
 compile:[m
[31m-	gcc -Wall -g3 -fsanitize=address -pthread server.c -o server[m
[31m-	gcc -Wall -g3 -fsanitize=address -pthread client.c -o client[m
[32m+[m	[32mrm -r .out; mkdir .out[m
[32m+[m	[32mgcc -Wall -g3 -fsanitize=address -pthread ./server/server.c -o .out/server[m
[32m+[m	[32mgcc -Wall -g3 -fsanitize=address -pthread ./client/client.c -o .out/client[m
 FLAGS    = -L /lib64[m
 LIBS     = -lusb-1.0 -l pthread[m
 [m
[1mdiff --git a/client.c b/client.c[m
[1mdeleted file mode 100644[m
[1mindex 4064dc0..0000000[m
[1m--- a/client.c[m
[1m+++ /dev/null[m
[36m@@ -1,140 +0,0 @@[m
[31m-#include <stdio.h>[m
[31m-#include <stdlib.h>[m
[31m-#include <string.h>[m
[31m-#include <signal.h>[m
[31m-#include <unistd.h>[m
[31m-#include <sys/types.h>[m
[31m-#include <sys/socket.h>[m
[31m-#include <netinet/in.h>[m
[31m-#include <arpa/inet.h>[m
[31m-#include <pthread.h>[m
[31m-[m
[31m-#define LENGTH 2048[m
[31m-[m
[31m-// Global variables[m
[31m-volatile sig_atomic_t flag = 0;[m
[31m-int sockfd = 0;[m
[31m-char name[32];[m
[31m-[m
[31m-void str_overwrite_stdout() {[m
[31m-  printf("%s", "> ");[m
[31m-  fflush(stdout);[m
[31m-}[m
[31m-[m
[31m-void str_trim_lf (char* arr, int length) {[m
[31m-  int i;[m
[31m-  for (i = 0; i < length; i++) { // trim \n[m
[31m-    if (arr[i] == '\n') {[m
[31m-      arr[i] = '\0';[m
[31m-      break;[m
[31m-    }[m
[31m-  }[m
[31m-}[m
[31m-[m
[31m-void catch_ctrl_c_and_exit(int sig) {[m
[31m-    flag = 1;[m
[31m-}[m
[31m-[m
[31m-void send_msg_handler() {[m
[31m-  char message[LENGTH] = {};[m
[31m-	char buffer[LENGTH + 32] = {};[m
[31m-[m
[31m-  while(1) {[m
[31m-  	str_overwrite_stdout();[m
[31m-    fgets(message, LENGTH, stdin);[m
[31m-    str_trim_lf(message, LENGTH);[m
[31m-[m
[31m-    if (strcmp(message, "exit") == 0) {[m
[31m-			break;[m
[31m-    } else {[m
[31m-      sprintf(buffer, "%s: %s\n", name, message);[m
[31m-      send(sockfd, buffer, strlen(buffer), 0);[m
[31m-    }[m
[31m-[m
[31m-		bzero(message, LENGTH);[m
[31m-    bzero(buffer, LENGTH + 32);[m
[31m-  }[m
[31m-  catch_ctrl_c_and_exit(2);[m
[31m-}[m
[31m-[m
[31m-void recv_msg_handler() {[m
[31m-	char message[LENGTH] = {};[m
[31m-  while (1) {[m
[31m-		int receive = recv(sockfd, message, LENGTH, 0);[m
[31m-    if (receive > 0) {[m
[31m-      printf("%s", message);[m
[31m-      str_overwrite_stdout();[m
[31m-    } else if (receive == 0) {[m
[31m-			break;[m
[31m-    } else {[m
[31m-			// -1[m
[31m-		}[m
[31m-		memset(message, 0, sizeof(message));[m
[31m-  }[m
[31m-}[m
[31m-[m
[31m-int main(int argc, char **argv){[m
[31m-	if(argc != 2){[m
[31m-		printf("Usage: %s <port>\n", argv[0]);[m
[31m-		return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	char *ip = "127.0.0.1";[m
[31m-	int port = atoi(argv[1]);[m
[31m-[m
[31m-	signal(SIGINT, catch_ctrl_c_and_exit);[m
[31m-[m
[31m-	printf("Please enter your name: ");[m
[31m-  fgets(name, 32, stdin);[m
[31m-  str_trim_lf(name, strlen(name));[m
[31m-[m
[31m-[m
[31m-	if (strlen(name) > 32 || strlen(name) < 2){[m
[31m-		printf("Name must be less than 30 and more than 2 characters.\n");[m
[31m-		return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	struct sockaddr_in server_addr;[m
[31m-[m
[31m-	/* Socket settings */[m
[31m-	sockfd = socket(AF_INET, SOCK_STREAM, 0);[m
[31m-  server_addr.sin_family = AF_INET;[m
[31m-  server_addr.sin_addr.s_addr = inet_addr(ip);[m
[31m-  server_addr.sin_port = htons(port);[m
[31m-[m
[31m-[m
[31m-  // Connect to Server[m
[31m-  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));[m
[31m-  if (err == -1) {[m
[31m-		printf("ERROR: connect\n");[m
[31m-		return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	// Send name[m
[31m-	send(sockfd, name, 32, 0);[m
[31m-[m
[31m-	printf("=== WELCOME TO THE CHATROOM ===\n");[m
[31m-[m
[31m-	pthread_t send_msg_thread;[m
[31m-  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){[m
[31m-		printf("ERROR: pthread\n");[m
[31m-    return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	pthread_t recv_msg_thread;[m
[31m-  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){[m
[31m-		printf("ERROR: pthread\n");[m
[31m-		return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	while (1){[m
[31m-		if(flag){[m
[31m-			printf("\nBye\n");[m
[31m-			break;[m
[31m-    }[m
[31m-	}[m
[31m-[m
[31m-	close(sockfd);[m
[31m-[m
[31m-	return EXIT_SUCCESS;[m
[31m-}[m
[1mdiff --git a/server.c b/server.c[m
[1mdeleted file mode 100644[m
[1mindex 34346a6..0000000[m
[1m--- a/server.c[m
[1m+++ /dev/null[m
[36m@@ -1,230 +0,0 @@[m
[31m-#include <sys/socket.h>[m
[31m-#include <netinet/in.h>[m
[31m-#include <arpa/inet.h>[m
[31m-#include <stdio.h>[m
[31m-#include <stdlib.h>[m
[31m-#include <unistd.h>[m
[31m-#include <errno.h>[m
[31m-#include <string.h>[m
[31m-#include <pthread.h>[m
[31m-#include <sys/types.h>[m
[31m-#include <signal.h>[m
[31m-[m
[31m-#define MAX_CLIENTS 100[m
[31m-#define BUFFER_SZ 2048[m
[31m-[m
[31m-static _Atomic unsigned int cli_count = 0;[m
[31m-static int uid = 10;[m
[31m-[m
[31m-/* Client structure */[m
[31m-typedef struct{[m
[31m-	struct sockaddr_in address;[m
[31m-	int sockfd;[m
[31m-	int uid;[m
[31m-	char name[32];[m
[31m-} client_t;[m
[31m-[m
[31m-client_t *clients[MAX_CLIENTS];[m
[31m-[m
[31m-pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;[m
[31m-[m
[31m-void str_overwrite_stdout() {[m
[31m-    printf("\r%s", "> ");[m
[31m-    fflush(stdout);[m
[31m-}[m
[31m-[m
[31m-void str_trim_lf (char* arr, int length) {[m
[31m-  int i;[m
[31m-  for (i = 0; i < length; i++) { // trim \n[m
[31m-    if (arr[i] == '\n') {[m
[31m-      arr[i] = '\0';[m
[31m-      break;[m
[31m-    }[m
[31m-  }[m
[31m-}[m
[31m-[m
[31m-void print_client_addr(struct sockaddr_in addr){[m
[31m-    printf("%d.%d.%d.%d",[m
[31m-        addr.sin_addr.s_addr & 0xff,[m
[31m-        (addr.sin_addr.s_addr & 0xff00) >> 8,[m
[31m-        (addr.sin_addr.s_addr & 0xff0000) >> 16,[m
[31m-        (addr.sin_addr.s_addr & 0xff000000) >> 24);[m
[31m-}[m
[31m-[m
[31m-/* Add clients to queue */[m
[31m-void queue_add(client_t *cl){[m
[31m-	pthread_mutex_lock(&clients_mutex);[m
[31m-[m
[31m-	for(int i=0; i < MAX_CLIENTS; ++i){[m
[31m-		if(!clients[i]){[m
[31m-			clients[i] = cl;[m
[31m-			break;[m
[31m-		}[m
[31m-	}[m
[31m-[m
[31m-	pthread_mutex_unlock(&clients_mutex);[m
[31m-}[m
[31m-[m
[31m-/* Remove clients to queue */[m
[31m-void queue_remove(int uid){[m
[31m-	pthread_mutex_lock(&clients_mutex);[m
[31m-[m
[31m-	for(int i=0; i < MAX_CLIENTS; ++i){[m
[31m-		if(clients[i]){[m
[31m-			if(clients[i]->uid == uid){[m
[31m-				clients[i] = NULL;[m
[31m-				break;[m
[31m-			}[m
[31m-		}[m
[31m-	}[m
[31m-[m
[31m-	pthread_mutex_unlock(&clients_mutex);[m
[31m-}[m
[31m-[m
[31m-/* Send message to all clients except sender */[m
[31m-void send_message(char *s, int uid){[m
[31m-	pthread_mutex_lock(&clients_mutex);[m
[31m-[m
[31m-	for(int i=0; i<MAX_CLIENTS; ++i){[m
[31m-		if(clients[i]){[m
[31m-			if(clients[i]->uid != uid){[m
[31m-				if(write(clients[i]->sockfd, s, strlen(s)) < 0){[m
[31m-					perror("ERROR: write to descriptor failed");[m
[31m-					break;[m
[31m-				}[m
[31m-			}[m
[31m-		}[m
[31m-	}[m
[31m-[m
[31m-	pthread_mutex_unlock(&clients_mutex);[m
[31m-}[m
[31m-[m
[31m-/* Handle all communication with the client */[m
[31m-void *handle_client(void *arg){[m
[31m-	char buff_out[BUFFER_SZ];[m
[31m-	char name[32];[m
[31m-	int leave_flag = 0;[m
[31m-[m
[31m-	cli_count++;[m
[31m-	client_t *cli = (client_t *)arg;[m
[31m-[m
[31m-	// Name[m
[31m-	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){[m
[31m-		printf("Didn't enter the name.\n");[m
[31m-		leave_flag = 1;[m
[31m-	} else{[m
[31m-		strcpy(cli->name, name);[m
[31m-		sprintf(buff_out, "%s has joined\n", cli->name);[m
[31m-		printf("%s", buff_out);[m
[31m-		send_message(buff_out, cli->uid);[m
[31m-	}[m
[31m-[m
[31m-	bzero(buff_out, BUFFER_SZ);[m
[31m-[m
[31m-	while(1){[m
[31m-		if (leave_flag) {[m
[31m-			break;[m
[31m-		}[m
[31m-[m
[31m-		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);[m
[31m-		if (receive > 0){[m
[31m-			if(strlen(buff_out) > 0){[m
[31m-				send_message(buff_out, cli->uid);[m
[31m-[m
[31m-				str_trim_lf(buff_out, strlen(buff_out));[m
[31m-				printf("%s -> %s\n", buff_out, cli->name);[m
[31m-			}[m
[31m-		} else if (receive == 0 || strcmp(buff_out, "exit") == 0){[m
[31m-			sprintf(buff_out, "%s has left\n", cli->name);[m
[31m-			printf("%s", buff_out);[m
[31m-			send_message(buff_out, cli->uid);[m
[31m-			leave_flag = 1;[m
[31m-		} else {[m
[31m-			printf("ERROR: -1\n");[m
[31m-			leave_flag = 1;[m
[31m-		}[m
[31m-[m
[31m-		bzero(buff_out, BUFFER_SZ);[m
[31m-	}[m
[31m-[m
[31m-  /* Delete client from queue and yield thread */[m
[31m-	close(cli->sockfd);[m
[31m-  queue_remove(cli->uid);[m
[31m-  free(cli);[m
[31m-  cli_count--;[m
[31m-  pthread_detach(pthread_self());[m
[31m-[m
[31m-	return NULL;[m
[31m-}[m
[31m-[m
[31m-int main(int argc, char **argv){[m
[31m-	if(argc != 2){[m
[31m-		printf("Usage: %s <port>\n", argv[0]);[m
[31m-		return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	char *ip = "127.0.0.1";[m
[31m-	int port = atoi(argv[1]);[m
[31m-	int option = 1;[m
[31m-	int listenfd = 0, connfd = 0;[m
[31m-  struct sockaddr_in serv_addr;[m
[31m-  struct sockaddr_in cli_addr;[m
[31m-  pthread_t tid;[m
[31m-[m
[31m-  /* Socket settings */[m
[31m-  listenfd = socket(AF_INET, SOCK_STREAM, 0);[m
[31m-  serv_addr.sin_family = AF_INET;[m
[31m-  serv_addr.sin_addr.s_addr = inet_addr(ip);[m
[31m-  serv_addr.sin_port = htons(port);[m
[31m-[m
[31m-  /* Ignore pipe signals */[m
[31m-	signal(SIGPIPE, SIG_IGN);[m
[31m-[m
[31m-	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){[m
[31m-		perror("ERROR: setsockopt failed");[m
[31m-    return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	/* Bind */[m
[31m-  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {[m
[31m-    perror("ERROR: Socket binding failed");[m
[31m-    return EXIT_FAILURE;[m
[31m-  }[m
[31m-[m
[31m-  /* Listen */[m
[31m-  if (listen(listenfd, 10) < 0) {[m
[31m-    perror("ERROR: Socket listening failed");[m
[31m-    return EXIT_FAILURE;[m
[31m-	}[m
[31m-[m
[31m-	printf("=== WELCOME TO THE CHATROOM ===\n");[m
[31m-[m
[31m-	while(1){[m
[31m-		socklen_t clilen = sizeof(cli_addr);[m
[31m-		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);[m
[31m-[m
[31m-		/* Check if max clients is reached */[m
[31m-		if((cli_count + 1) == MAX_CLIENTS){[m
[31m-			printf("Max clients reached. Rejected: ");[m
[31m-			print_client_addr(cli_addr);[m
[31m-			printf(":%d\n", cli_addr.sin_port);[m
[31m-			close(connfd);[m
[31m-			continue;[m
[31m-		}[m
[31m-[m
[31m-		/* Client settings */[m
[31m-		client_t *cli = (client_t *)malloc(sizeof(client_t));[m
[31m-		cli->address = cli_addr;[m
[31m-		cli->sockfd = connfd;[m
[31m-		cli->uid = uid++;[m
[31m-[m
[31m-		/* Add client to the queue and fork thread */[m
[31m-		queue_add(cli);[m
[31m-		pthread_create(&tid, NULL, &handle_client, (void*)cli);[m
[31m-[m
[31m-		/* Reduce CPU usage */[m
[31m-		sleep(1);[m
[31m-	}[m
[31m-[m
[31m-	return EXIT_SUCCESS;[m
[31m-}[m
