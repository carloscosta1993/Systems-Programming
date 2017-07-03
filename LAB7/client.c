#include "storyserver.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int socket_stream_port;
int socket_stream_fd;
int socket_datagram_fd;
int server_port;
char server_ip[20];

void socket_datagram(){
	socket_datagram_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_fd == -1){
		perror("socket: ");
		exit(-1);
	}
}

void socket_stream(){
	struct sockaddr_in socket_stream_addr;
	socket_stream_fd= socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_stream_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(socket_stream_port);
    socket_stream_addr.sin_addr.s_addr= INADDR_ANY;
    
    int err = bind(socket_stream_fd, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
}

void send_to_gateway(char *gateway_ip, int port){
	struct sockaddr_in gateway_addr;
	msg_gw m;
	m.type = 1;
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(port);
    inet_aton(gateway_ip, &gateway_addr.sin_addr);
    int nbytes = sendto(socket_datagram_fd, (msg_gw*)&m , (1024+sizeof(m)),
						0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
}

msg_gw *receive_from_gateway(){
	struct sockaddr_in gateway_addr;
	msg_gw *server=malloc(sizeof(msg_gw));
	int size_addr = sizeof(gateway_addr);
	int nbytes = recvfrom(socket_datagram_fd, server , sizeof(*server)+1, 0, (struct sockaddr *) & gateway_addr, &size_addr);
	if(server->type == 0){
		printf("NO SERVERS AVAIABLE. TRY AGAIN LATER\n");
		exit(0);
	}
	printf("RECEBI SERVIDOR COM IP %s e PORT %d\n",server->address,server->port);
	return server;
}

void client_server(msg_gw *server){
	char word[100];
	char receive_word[100];
	int send1;
	int receive;
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(server->port);
	
	if (!inet_aton(server->address, &server_addr.sin_addr)){
        perror("Gateway IP incorrect");
        exit(1);
    } 
	
	if( -1 == connect(socket_stream_fd, 
			(const struct sockaddr *) &server_addr, sizeof(server_addr))){
				printf("Connection Error\n");
				exit(-1);
	}
	
	while(1){
		while(1){
		printf("message: ");
		fgets(word, sizeof(word), stdin);
		send1 = send(socket_stream_fd, word, strlen(word)+1, 0);
		receive = recv(socket_stream_fd, receive_word, 100, 0);
		printf("%s\n",receive_word);
		sleep(1);
		
		}
	}
	
}

int main(int argc, char *argv[]){
	
	msg_gw *server=malloc(sizeof(msg_gw));
	
	if (argc < 3){
        printf("First parameter: (IP GATEWAY). Second parameter: (GATEWAY PORT)");
        exit(1);
    }
    socket_stream_port = 1024 + getpid();
    char *gateway_ip = argv[1];
    int gateway_port = atoi(argv[2]);
    
    socket_stream();
    socket_datagram();
    send_to_gateway(gateway_ip, gateway_port);
    server = receive_from_gateway();
    client_server(server);
    
	while(1){
		
		
	}
    printf("OK\n");
    close(socket_stream_fd);
    close(socket_datagram_fd);
	exit(0);
    
}

