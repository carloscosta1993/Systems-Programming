#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include "sock_dg.h" 

int main(){
	struct sockaddr_un local_addr;
	char buff[100];
	int nbytes;
	
	int sock_fd= socket(AF_UNIX, SOCK_DGRAM, 0);
	
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
	/*
        struct sockaddr_un {
               sa_family_t sun_family;               // AF_UNIX 
               char        sun_path[108];            // pathname 
           }
	*/
	
	printf("%s\n", SOCK_ADDRESS);
	local_addr.sun_family = AF_UNIX;
	strcpy(local_addr.sun_path, SOCK_ADDRESS);
	int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
	
	printf(" socket created and binded \n Ready to receive messages\n");
	nbytes = recv(sock_fd, buff, 100, 0);
	printf("received %d bytes --- %s ---\n", nbytes, buff);
	//sleep(1);
	nbytes = recv(sock_fd, buff, 100, 0);
	printf("received %d bytes --- %s ---\n", nbytes, buff);

	//sleep(1);	
	nbytes = recv(sock_fd, buff, 100, 0);
	printf("received %d bytes --- %s ---\n", nbytes, buff);

	close(sock_fd);
	//unlink(SOCK_ADDRESS);
	exit(0);
}
