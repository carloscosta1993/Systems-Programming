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
#include <sys/wait.h>


int volatile sock_fd;


void intHandler(int dummy) {
   printf("EXIT\n");
   close(sock_fd);
   exit(0);
}

int main(){
	message m;
    char * story;
    char word[100];
    struct sockaddr_in local_addr;
    struct sockaddr_in client_addr;
    socklen_t size_addr;
    story = strdup("");
    int recv1,recv2;
    char client[100];
    char *buf = m.buffer;
    buf = mmap(NULL, MESSAGE_LEN*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port= htons(PORT);
    local_addr.sin_addr.s_addr= INADDR_ANY;
   
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
		
	int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
	
	printf(" socket created and binded \n Ready to receive messages\n");
	listen(sock_fd, 5);
    signal(SIGINT, intHandler);
       
    while(1){
        int client_fd = accept(sock_fd, (struct sockaddr *) & client_addr, &size_addr); 
        if(fork()==0){
			while(1){	
			recv1 = recv(client_fd, word, 100, 0);
			story = strcat(buf, word);
			printf("%s\n",story);
			send(client_fd, story, strlen(story)+1, 0);
		  }
		  close(client_fd);
		}
     }
    printf("OK\n");
    close(sock_fd);
    exit(0);
    
}




