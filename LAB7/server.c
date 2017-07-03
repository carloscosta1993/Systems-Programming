#include "storyserver.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/mman.h>

int socket_stream_port;
int socket_stream_fd;
int socket_datagram_fd;
char *gateway_ip;
int gateway_port;

void socket_stream(){
	char *ip;
	struct sockaddr_in socket_stream_addr;
	socket_stream_fd= socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_stream_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(socket_stream_port);
    socket_stream_addr.sin_addr.s_addr= htons(INADDR_ANY);
    
       
    int err = bind(socket_stream_fd, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
}

void socket_datagram(){
	socket_datagram_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_fd == -1){
		perror("socket: ");
		exit(-1);
	}
}

void send_to_gateway(){
	struct sockaddr_in gateway_addr;
	msg_gw m;
	m.type = 1;
	m.port = socket_stream_port;
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(gateway_port);
    inet_aton(gateway_ip, &gateway_addr.sin_addr); 
    if(sendto(socket_datagram_fd, (msg_gw*)&m , sizeof(m),0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr))<0){
		perror("FAIL CONNECTION TO GATEWAY\n");
		exit(1);
	}
}

void sigint_handler(int sig){
	struct sockaddr_in gateway_addr;
	msg_gw msg_disconnect;
	msg_disconnect.type = 0;
	msg_disconnect.port = socket_stream_port;
	
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(gateway_port);
    inet_aton(gateway_ip, &gateway_addr.sin_addr);
    sendto(socket_datagram_fd, (msg_gw*)&msg_disconnect , sizeof(msg_disconnect),
						0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
						
    if (close(socket_datagram_fd) < 0)
        perror("Datagram socket not closed");
    
    if (close(socket_stream_fd) < 0)
        perror("Stream socket not closed");
        
    exit(0);
     
	
}

int main(int argc, char *argv[]){
	
	if (argc < 3){
        printf("First parameter: (IP GATEWAY). Second parameter: (GATEWAY PORT)");
        exit(1);
    }
    socket_stream_port = 1024 + getpid();
    gateway_ip = argv[1];
    gateway_port = atoi(argv[2]);
    
	int recv1;
	char word[100];
	struct sockaddr_in client_addr;
	socklen_t size_addr;
	
	struct sigaction sig;
    sig.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig, NULL);
     	
	socket_stream();
	socket_datagram();
	send_to_gateway();
	printf(" socket created and binded \n Ready to receive messages\n");
       
    while(1){
	  listen(socket_stream_fd, 5);
      int client_fd = accept(socket_stream_fd, (struct sockaddr *) & client_addr, &size_addr);
      if(fork()==0){ 
			while(1){	
			recv1 = recv(client_fd, word, 100, 0);
			printf("%s\n",word);
			send(client_fd, word, strlen(word)+1, 0);
		  }
		  close(client_fd);
		}
	}
    printf("OK\n");
    close(socket_stream_fd);
    close(socket_datagram_fd);
    exit(0);
    
}




