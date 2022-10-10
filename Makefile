compile:
	rm -r .out; mkdir .out
	rm -r logs; mkdir logs
	gcc -Wall -g3 -fsanitize=address -pthread ./util/file_manager.c ./util/debugger.c ./server/server.c -o .out/server
	gcc -Wall -g3 -fsanitize=address -pthread ./util/file_manager.c ./util/debugger.c ./client/client.c -o .out/client
FLAGS    = -L /lib64
LIBS     = -lusb-1.0 -l pthread

