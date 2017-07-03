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

int volatile sock_fd;

void intHandler(int dummy) {
   printf("EXIT\n");
   close(sock_fd);
   exit(0);
}


int main(){
 
    message m;
    struct sockaddr_in server_addr;
	sock_fd= socket(AF_INET, SOCK_STREAM, 0);
	char word[100];
	char story[100];
	int send1,send2,receive;
	char client[100];
    
    if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(PORT);
	server_addr.sin_addr.s_addr= INADDR_ANY; 
   
    
    if( -1 == connect(sock_fd, 
			(const struct sockaddr *) &server_addr, 
			sizeof(server_addr))){
				printf("Error connecting\n");
				exit(-1);
	}
   
    signal(SIGINT, intHandler);
	printf(" socket created \n Ready to send\n");
    
	while(1){
		printf("message: ");
		fgets(word, sizeof(word), stdin);
		send1 = send(sock_fd, word, strlen(word)+1, 0);
		receive = recv(sock_fd, story, 100, 0);
		printf("%s\n",story);
		sleep(1);
		
	}
    printf("OK\n");
    close(sock_fd);
	exit(0);
    
}

