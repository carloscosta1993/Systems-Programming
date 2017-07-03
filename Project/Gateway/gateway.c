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

/** UDP Socket to handle client **/
int socket_datagram_client;
/** UDP Socket to handle server **/
int socket_datagram_server;
/** Linked list inicialization. Stores the servers **/
node_t * head = NULL;
/** Index of Round Robin **/
int rr_index = -1;
/** Number of servers variable **/
int nb_servers;
/** Lock Thread **/
pthread_mutex_t node_mutex;

void print_list();

/** Function to create UDP socket for server and client **/
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

/** Function to delete the server from linked list by port **/
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

void send_updated_list(int port, char address[20]){
	node_t *current=head;
	msg_gw server_update;
	struct sockaddr_in socket_stream_addr;
	int nbytes;
	
	socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_addr.s_addr= INADDR_ANY;
	
	
	do{
		if(current->port!=port){
			//send to new peer
			socket_stream_addr.sin_port= htons(port);
			inet_aton(address, &socket_stream_addr.sin_addr);
			server_update.type = 2;
			server_update.port = current->port;
			strcpy(server_update.address, current->address);
			nbytes = sendto(socket_datagram_client, (msg_gw*)&server_update , sizeof(server_update),0, (const struct sockaddr *) &socket_stream_addr, sizeof(socket_stream_addr));	
			
			//send to old peers

			inet_aton(current->address, &socket_stream_addr.sin_addr);
			socket_stream_addr.sin_port= htons(current->port);
			if(head->next->next==NULL&&current==head){
				server_update.type = 4;
			}else server_update.type = 2;
			server_update.port = port;
			strcpy(server_update.address, current->address);
			nbytes = sendto(socket_datagram_client, (msg_gw*)&server_update , sizeof(server_update),0, (const struct sockaddr *) &socket_stream_addr, sizeof(socket_stream_addr));	
		}
		current=current->next;
	}while(current!=NULL);
}


/** Communication Server-Gateway Only **/
void *receive_from_server(void * arg){
		
	while(1){
		printf("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("THREAD OF SERVER COMMUNICATION. PORT: %d\n",GW_SERVER_PORT);
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		
		struct sockaddr_in sv_addr;
		socklen_t size_addr = sizeof(sv_addr);
		
		msg_gw *server=malloc(sizeof(msg_gw));
		int nbytes = recvfrom(socket_datagram_server, server , sizeof(*server)+1, 0, (struct sockaddr *) & sv_addr, &size_addr);
		if(nbytes < 0){
			perror("RECEIVED FROM SERVER. ERROR:");
			exit(0);
		}
		printf("RECEIVED MSG FROM SERVER\n");
		/** Type = 1 means that it is to ADD the server **/
		if(server->type == 1){
			printf("SERVER: PORT-> %d, TYPE->%d\n", server->port, server->type);
			if(head == NULL){
				head = malloc(sizeof(node_t));
				head->port = server->port;
				//strncpy(head->address, inet_ntoa(sv_addr.sin_addr), 20);
				strcpy(head->address,server->address);
				head->next = NULL;
			}else{
				
				node_t * current = head;
				
				while (current->next != NULL) {
					current = current->next;
				}
			 
				current->next = malloc(sizeof(node_t));
				current->next->port = server->port;
				strcpy(current->next->address, server->address);
				current->next->next = NULL;
			}			
			nb_servers++;
			send_updated_list(server->port, server->address);
		/** Type = 0 means that it is to DELETE the server **/
		}else{
			pthread_mutex_lock(&node_mutex);
			printf("DELETE SERVER WITH PORT: %d\n", server->port); 
			head = delete(head, server->port);
			nb_servers--;
			pthread_mutex_unlock(&node_mutex);
			//DELETE FROM LISTS
			node_t *current=head;
			msg_gw server_update;
			struct sockaddr_in socket_stream_addr; int nbytes;
			socket_stream_addr.sin_family = AF_INET;
			socket_stream_addr.sin_addr.s_addr= INADDR_ANY;			
			while(current!=NULL){
				//send to new peer
				inet_aton(current->address, &socket_stream_addr.sin_addr);
				socket_stream_addr.sin_port= htons(current->port);
				server_update.type = 3;
				server_update.port = server->port;
				strcpy(server_update.address, server->address);
				nbytes = sendto(socket_datagram_client, (msg_gw*)&server_update , sizeof(server_update),0, (const struct sockaddr *) &socket_stream_addr, sizeof(socket_stream_addr));	
				current=current->next;
			}
		}
		print_list();
	}	
}

int count_servers(){
	int i=0;
	node_t * current=head;
	while(current!=NULL){
		i++;
		current=current->next;
	}
	return i;
}
/** Communication Gateway-Client and Client-Gateway **/
void *receive_from_client(void * arg){

	while(1){
		
		printf("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("THREAD OF CLIENT COMMUNICATION. PORT: %d\n", GW_CLIENT_PORT);
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		struct sockaddr_in sv_addr;
		int size_addr = sizeof(sv_addr);
		msg_gw *server=malloc(sizeof(msg_gw));
		
		int nbytes = recvfrom(socket_datagram_client, server , sizeof(*server)+1, 0, (struct sockaddr *) & sv_addr, &size_addr);
		msg_gw server_assigned;
		node_t *current = head;
		int i = 0, n;
		
		/** Send MSG with type = 0 if there are no avaiable server **/
		if(nb_servers == 0){
			server_assigned.type = 0;
			int nbytes = sendto(socket_datagram_client, (msg_gw*)&server_assigned , sizeof(server_assigned), 0, (const struct sockaddr *) &sv_addr, sizeof(sv_addr));
			if(nbytes < 0){
				perror("RECEIVED FROM CLIENT. ERROR:");
				exit(0);
			}
		/** Send MSG with type = 1 if there are avaiable servers **/
		}else{
			pthread_mutex_lock(&node_mutex);
			n=count_servers();
			if(rr_index>(n+1)) rr_index=0;
			rr_index=(rr_index+1)%n;
			while (current->next != NULL && i < rr_index){ 
				current = current->next;
				i++;
				}
			pthread_mutex_unlock(&node_mutex);
			
			//Send choosen server to client
			server_assigned.type = 1;
			server_assigned.port = current->port;
			strcpy(server_assigned.address, current->address);
			int nbytes = sendto(socket_datagram_client, (msg_gw*)&server_assigned , sizeof(server_assigned),0, (const struct sockaddr *) &sv_addr, sizeof(sv_addr));
			if(nbytes < 0){
				perror("RECEIVED FROM SERVER. ERROR:");
				exit(0);
			}
		}
		
	}
}

/** Print avaiable servers **/
void print_list(){
	if (head == NULL){printf("NO SERVERS AVAIABLE\n");}
    printf("NUMBER OF AVAIABLE SERVER: %d\n",nb_servers);    
	node_t * current = head;
	while(current!=NULL){
			printf("%d - %s\n", current->port, current->address);
			current =  current->next;
		}
		
}

/** SIGNAL handler. Close the sockets **/
void sigint_handler(int sig){
						
    if (close(socket_datagram_server) < 0)
        perror("GATEWAY socket:");
    
    if (close(socket_datagram_client) < 0)
        perror("GATEWAY socket:");
    
    printf("GATEWAY CLOSED.................\n");
    exit(0);
     	
}

int main(){
	/*
	struct sockaddr_in sv_addr;
	int size_addr = sizeof(sv_addr);
	msg_gw *server=malloc(sizeof(msg_gw));
	char server_address[20];
	*/
	socket_datagram();
	printf(" SOCKET CREATED AND BINDED. READY TO RECEIVE MESSAGES!\n");
	
	struct sigaction sig;
    sig.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig, NULL);
	
	pthread_t client_thread;
	pthread_t server_thread;
	
	/** Thread to handle clients. **/
	int thd_create1 = pthread_create(&client_thread, NULL, receive_from_client, NULL);
	if(thd_create1 != 0){
		perror("THREAD: ERROR.");
		exit(-1);
	}
	
	/** Thread to handler servers. **/
	int thd_create2 = pthread_create(&server_thread, NULL, receive_from_server, NULL);
	if(thd_create2 != 0){
		perror("THREAD: ERROR.");
		exit(-1);
	}

	/** Wait for all threads **/
	pthread_join(client_thread, NULL);
	pthread_join(server_thread, NULL);
    exit(0);
    
}




