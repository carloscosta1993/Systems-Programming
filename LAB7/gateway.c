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
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>


int socket_datagram_client;
int socket_datagram_server;
node_t * head = NULL;
int rr_index = 0;
int nb_servers;

void print_list();

void socket_datagram(){
	
	socket_datagram_client = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_client == -1){
		perror("socket: ");
		exit(-1);
	}
	
	struct sockaddr_in gateway_addr;
	gateway_addr.sin_family = AF_INET;
	gateway_addr.sin_port = htons(GW_CLIENT_PORT);
	gateway_addr.sin_addr.s_addr= htons(INADDR_ANY);
	
	int err1 = bind(socket_datagram_client, (struct sockaddr *)&gateway_addr, sizeof(gateway_addr));
	if(err1 == -1) {
		perror("bind1");
		exit(-1);
	}
	
	socket_datagram_server = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_server == -1){
		perror("socket: ");
		exit(-1);
	}
	
	gateway_addr.sin_family = AF_INET;
	gateway_addr.sin_port = htons(GW_SERVER_PORT);
	gateway_addr.sin_addr.s_addr= INADDR_ANY;
	
	int err2 = bind(socket_datagram_server, (struct sockaddr *)&gateway_addr, sizeof(gateway_addr));
	if(err2 == -1) {
		perror("bind2");
		exit(-1);
	}
}

node_t * delete(node_t *currP, int port) {


  if (currP == NULL)
    return NULL;

  if (currP->port == port) {
    node_t *tempNextP;
    tempNextP = currP->next;
    free(currP);
    return tempNextP;
  }

  currP->next = delete(currP->next, port);
  return currP;
}



void *receive_from_server(){
	while(1){
		printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		struct sockaddr_in sv_addr;
		int size_addr = sizeof(sv_addr);
		msg_gw *server=malloc(sizeof(msg_gw));
		printf("SERVER COMMUNICATION\n");
		int nbytes = recvfrom(socket_datagram_server, server , sizeof(*server)+1, 0, (struct sockaddr *) & sv_addr, &size_addr);
		
		if(server->type == 1){
			
			if(head == NULL){
				head = malloc(sizeof(node_t));
				head->port = server->port;
				strncpy(head->address, inet_ntoa(sv_addr.sin_addr), 20);
				head->next = NULL;
			}else{
				
			node_t * current = head;
			
			while (current->next != NULL) {
				current = current->next;
			}
		 
			current->next = malloc(sizeof(node_t));
			current->next->port = server->port;
			strncpy(current->next->address, inet_ntoa(sv_addr.sin_addr), 20);
			current->next->next = NULL;
			}
			
			nb_servers++;
		
		}else{
			head = delete(head, server->port);
			nb_servers--;
		}
		print_list();
		printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
	}	
}

void *receive_from_client(){
	while(1){
		printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		printf("CLIENT COMMUNICATION\n");
		struct sockaddr_in sv_addr;
		msg_gw *server=malloc(sizeof(msg_gw));
		int size_addr = sizeof(sv_addr);
		int nbytes = recvfrom(socket_datagram_client, server , sizeof(*server)+1, 0, (struct sockaddr *) & sv_addr, &size_addr);
		msg_gw server_assigned;
		node_t *current = head;
		int i = 0;
		
		if(nb_servers == 0){
			server_assigned.type = 0;
			int nbytes = sendto(socket_datagram_client, (msg_gw*)&server_assigned , sizeof(server_assigned),
							0, (const struct sockaddr *) &sv_addr, sizeof(sv_addr));
		}else{
			
			while (current->next != NULL && i < rr_index){ 
				current = current->next;
				i++;
			}
			if(current->next == NULL){
				rr_index = 0;
			}else{
				rr_index++;
			}
			
			//Send choosen server to client
			server_assigned.type = 1;
			server_assigned.port = current->port;
			strcpy(server_assigned.address, current->address);
			int nbytes = sendto(socket_datagram_client, (msg_gw*)&server_assigned , sizeof(server_assigned),
								0, (const struct sockaddr *) &sv_addr, sizeof(sv_addr));
		}
		printf("|||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
	}
}

void print_list(){
	if (head == NULL){printf("NO SERVERS AVAIABLE\n");}
    printf("NUMBER OF AVAIABLE SERVER: %d\n",nb_servers);    
	node_t * current = head;
	while(current!=NULL){
			printf("%d - %s\n", current->port, current->address);
			current =  current->next;
		}
		
}


int main(){
	/*
	struct sockaddr_in sv_addr;
	int size_addr = sizeof(sv_addr);
	msg_gw *server=malloc(sizeof(msg_gw));
	char server_address[20];
	*/
	socket_datagram();
	printf(" socket created and binded \n Ready to receive messages\n");
	pthread_t client_thread;
	pthread_t server_thread;
	
	int error;
	error = pthread_create(&client_thread, NULL, receive_from_client, NULL);
	if(error != 0){
		perror("Thread creation error");
		exit(-1);
	}

	error = pthread_create(&server_thread, NULL, receive_from_server, NULL);
	if(error != 0){
		perror("Thread creation error");
		exit(-1);
	}
	
     /* 
    while(1){
		int nbytes = recvfrom(socket_datagram_fd, server , sizeof(*server)+1, 0, (struct sockaddr *) & sv_addr, &size_addr);
		if(server->type == 0){
			receive_from_server(*server, inet_ntoa(sv_addr.sin_addr));
			 print_list();
		}else{
			receive_from_client(sv_addr);
		}
		printf("DONE\n");
     }
	*/
	pthread_join(client_thread, NULL);
	pthread_join(server_thread, NULL);
	while(1){}
    close(socket_datagram_client);
    close(socket_datagram_server);
    printf("HERE\n");
    exit(0);
    
}




