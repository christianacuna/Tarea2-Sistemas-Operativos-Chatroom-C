compile:
	rm -r .out; mkdir .out
	gcc -Wall -g3 -fsanitize=address -pthread ./server/server.c -o .out/server
	gcc -Wall -g3 -fsanitize=address -pthread ./client/client.c -o .out/client
FLAGS    = -L /lib64
LIBS     = -lusb-1.0 -l pthread

