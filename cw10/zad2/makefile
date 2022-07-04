all: server client

server: server.c common_udp.h vector.h game_common.h game_server.h
	gcc -Wall -pthread -lm server.c -o server

client: client.c game_common.h vector.h common_udp.h
	gcc -Wall -pthread -lm client.c -o client

clean:
	rm -f ./udp_socket