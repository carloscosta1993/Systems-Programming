#include "peer_functions.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>


pthread_mutex_t img_str_mutex;
pthread_mutex_t node_mutex;
/** Server socket port **/
int socket_port;
/** TCP Socket **/
int socket_stream_fd;
/** UDP Socket **/
int socket_datagram_fd;
int socket_handler;
int ret1=2, status=0;
/** Gateway IP and Port **/
char *gateway_ip, p_address[20];
int gateway_port;
node_t * head = NULL;
img_str * head_img=NULL;

/** Function to create TCP socket **/
void socket_stream(){
	char *ip;
	struct sockaddr_in socket_stream_addr;
	socket_stream_fd= socket(AF_INET, SOCK_STREAM, 0);
	if (socket_stream_fd == -1){
		perror("socket: ");
		exit(-1);
	}
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(socket_port);
    socket_stream_addr.sin_addr.s_addr= htons(INADDR_ANY);
       
    int err = bind(socket_stream_fd, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
}

void print_list(){
	if (head == NULL){printf("NO SERVERS AVAIABLE\n");}  
	node_t * current = head;
	while(current!=NULL){
			printf("%d - %s\n", current->port, current->address);
			current =  current->next;
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

void add_server(msg_gw *server){
	pthread_mutex_lock(&node_mutex);
	if(head == NULL){
		head = malloc(sizeof(node_t));
		head->port = server->port;
		strncpy(head->address, server->address, 20);
		head->next = NULL;
	}else{		
		node_t * current = head;
				
		while (current->next != NULL) {
				current = current->next;
		}	 
		current->next = malloc(sizeof(node_t));
		current->next->port = server->port;
		strncpy(current->next->address, server->address, 20);
		current->next->next = NULL;
		}
	pthread_mutex_unlock(&node_mutex);					
}

/** Function to create UDP socket **/
void socket_datagram(){
	socket_datagram_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	socket_handler = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_handler == -1){
		perror("socket: ");
		exit(-1);
	}
	
	struct sockaddr_in peer_addr;
	peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(socket_port);
    peer_addr.sin_addr.s_addr= htons(INADDR_ANY);
	
	int err = bind(socket_datagram_fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}
}

void *download_all(){
	if(head==NULL){
		pthread_exit(NULL);
	}
	node_t *current=head;
	char *host=head->address;
	head_img=establish_to_download(host, current->port, head_img);
	pthread_exit(NULL);
}

void *receive_from_gateway(){
	struct sockaddr_in peer_addr;
	socklen_t size_addr = sizeof(peer_addr);
    msg_gw *server=malloc(sizeof(msg_gw));
    msg_gw *new_server=malloc(sizeof(msg_gw));
    pthread_t thds_download;

	while(1){
		recvfrom(socket_datagram_fd, server , sizeof(*server)+1, 0, (struct sockaddr *) & peer_addr, &size_addr);
		if(server->type == 2){
			add_server(server);	
		}else if(server->type == 3){
			pthread_mutex_lock(&node_mutex);
			head = delete(head, server->port);
			pthread_mutex_unlock(&node_mutex);
		}else if(server->type == 4){
			status=1;
			add_server(server);
		}
		if(status==0&&head_img==NULL&&head!=NULL){
			status=1;
			int error = pthread_create(&thds_download, NULL, download_all, NULL);
			if(error != 0){
				perror("pthread_create: ");
				exit(-1);
			}
		}
	}	
}

/** Function to send server IP and port to Gateway **/
void send_to_gateway(){	
	struct sockaddr_in gateway_addr;
	msg_gw m;
	m.type = 1;
	m.port = socket_port;
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(gateway_port);
    inet_aton(gateway_ip, &gateway_addr.sin_addr);


    strcpy(m.address, p_address);
    printf("%s\n", m.address);
    
    if(sendto(socket_datagram_fd, (msg_gw*)&m , sizeof(m),0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr))<0){
		perror("FAIL CONNECTION TO GATEWAY\n");
		exit(1);
	}
		
}
/** Signal handler function. Send MSG to gateway to remove server. Close the sockets **/
void sigint_handler(int sig){
	struct sockaddr_in gateway_addr;
	msg_gw m;
	m.type = 0;
	m.port = socket_port;
	
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(gateway_port);
    inet_aton(gateway_ip, &gateway_addr.sin_addr); 
    
    printf("\nCLOSING SERVER.................\n");
    printf("SERVER PORT: %d |GATEWAY ADDRESS: %s, PORT: %d\n",m.port, gateway_ip, gateway_port);
    
   int r=sendto(socket_handler, (msg_gw*)&m , sizeof(m),0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
    
    if(r<0){
		perror("FAIL CONNECTION TO GATEWAY\n");
		exit(1);
	}
	printf("r=%d\n",r);
						
    if (close(socket_datagram_fd) < 0)
        perror("Server-Gateway socket:");
    if (close(socket_stream_fd) < 0)
        perror("Server-Client socket:");
    
    printf("SERVER CLOSED.................\n");
     
    exit(0);
}

void *updating_peers(void *opt){
	if(head==NULL){
		pthread_exit(NULL);
	}
	node_t *current=head;
	int peer_fd;
	while(current!=NULL){
		char *host=current->address;
		pthread_mutex_lock(&img_str_mutex);
		peer_fd=establish_p_t_p(host, current->port, opt, head_img);
		pthread_mutex_unlock(&img_str_mutex);
		close(peer_fd);
		current=current->next;
	}
	printf("<<<<<<<<<<<<<<<<<< DISCONNECTING PEER >>>>>>>>>>>>>>>>>>\n");
	pthread_exit(NULL);
}

void handle_client(int *client){
	int recv1;
	char word[100], OK[10]={"ok"};
	printf("<<<<<<<<<<<<<<<<<< A CLIENT CONNECTED >>>>>>>>>>>>>>>>>>\n");
	pthread_t thds_peer;
	char *del, *key;
	while(1){
		memset(word, '\0', 100);
		recv1 = recv(*client, word, 100, 0);
		if(word[0]=='a'&&word[1]=='d'&&word[2]=='d'&&word[3]=='p'){
			send(*client, OK, strlen(OK)+1, 0);
			pthread_mutex_lock(&img_str_mutex);
			head_img=get_photo(client, head_img);
			pthread_mutex_unlock(&img_str_mutex);
			char *opt="addp/0";
			int error = pthread_create(&thds_peer, NULL, updating_peers, opt);
			if(error != 0){
				perror("pthread_create: ");
				exit(-1);
			}
		}else if(word[0]=='l'&&word[1]=='i'&&word[2]=='s'&&word[3]=='t'){
			pthread_mutex_lock(&img_str_mutex);
			list_photos(client, head_img);
			pthread_mutex_unlock(&img_str_mutex);
		}else if(word[0]=='k'&&word[1]=='e'&&word[2]=='y'){
			pthread_mutex_lock(&img_str_mutex);
			add_keyword(client, head_img, &key);
			pthread_mutex_unlock(&img_str_mutex);
			int error = pthread_create(&thds_peer, NULL, updating_peers, key);
			if(error != 0){
				perror("pthread_create: ");
				exit(-1);
			}
		}else if(word[0]=='d'&&word[1]=='e'&&word[2]=='l'){
			pthread_mutex_lock(&img_str_mutex);
			head_img=delete_photo(client, head_img, &del);
			pthread_mutex_unlock(&img_str_mutex);
			int error = pthread_create(&thds_peer, NULL, updating_peers, del);
			if(error != 0){
				perror("pthread_create: ");
				exit(-1);
			}
		}else if(word[0]=='s'&&word[1]=='k'&&word[2]=='e'){
			pthread_mutex_lock(&img_str_mutex);
			search_key(client, head_img);
			pthread_mutex_unlock(&img_str_mutex);
		}else if(word[0]=='g'&&word[1]=='e'&&word[2]=='t'){
			pthread_mutex_lock(&img_str_mutex);
			send_photoc(client, head_img);
			pthread_mutex_unlock(&img_str_mutex);
		}else if(word[0]=='i'&&word[1]=='d'&&word[2]=='p'){
			pthread_mutex_lock(&img_str_mutex);
			find_name(client, head_img);
			pthread_mutex_unlock(&img_str_mutex);
		}
	}
	printf("<<<<<<<<<<<<<<<<<< DISCONNECTING CLIENT >>>>>>>>>>>>>>>>>>\n");
	pthread_exit(NULL);
}

void handle_peer(int *peer){
	int recv1, socket;
	char *del, *key, word[6], OK[10]={"ok"};
	printf("<<<<<<<<<<<<<<<<<< A PEER CONNECTED >>>>>>>>>>>>>>>>>>\n");
	send(*peer, OK, strlen(OK)+1, 0);
	recv(*peer, word, 6, 0);
	pthread_mutex_lock(&img_str_mutex);
	if(word[0]=='a'&&word[1]=='d'&&word[2]=='d'){
		send(*peer, OK, strlen(OK)+1, 0);
		head_img=get_photo_peer(peer, head_img);
	}else if(word[0]=='d'&&word[1]=='e'&&word[2]=='l'){
		head_img=delete_photo(peer, head_img, &del);
	}else if(word[0]=='k'&&word[1]=='e'&&word[2]=='y'){
		add_keyword(peer, head_img, &key);
	}else if(word[0]=='e'&&word[1]=='v'&&word[2]=='r'){
		socket=*peer;
		send_evr(socket, head_img);
	}
	pthread_mutex_unlock(&img_str_mutex);
	printf("<<<<<<<<<<<<<<<<<< DISCONNECTING PEER >>>>>>>>>>>>>>>>>>\n");
	pthread_exit(NULL);
}

/** Communication Server-Client anc Client-Server **/
void *server_client(void *client_fd){
	int *client = (int *)client_fd;
	int recv1;
	char word[100];
	recv1 = recv(*client, word, 100, 0);
	if(word[0]=='c'&&word[1]=='l'&&word[2]=='i'){
		handle_client(client);
	}else if(word[0]=='p'&&word[1]=='e'&&word[2]=='e'&&word[3]=='r'){
		handle_peer(client);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////	
int main(int argc, char *argv[]){
	
	if (argc < 3){
        printf("First parameter: (IP GATEWAY). Second parameter: (GATEWAY PORT).\nThird parameter: (PEER IP (Optional))");
        exit(1);
    }
    char default_ip[20]="127.0.0.1\0";
    if(argc==4){
    	strcpy(p_address, argv[3]);
    } else strcpy(p_address, default_ip);

    socket_port = 1024 + getpid();
    gateway_ip = argv[1];
    gateway_port = atoi(argv[2]);
	printf("Port: %d\n", socket_port);    
    int i = 0;
	struct sockaddr_in client_addr;
	socklen_t size_addr;
	
	struct sigaction sig;
    sig.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig, NULL);
     	
	socket_stream();
	socket_datagram();
	send_to_gateway();
	printf("\nSocket created and binded \n Ready to receive messages\n");
	
	pthread_t handle_gateway;

	int thd = pthread_create(&handle_gateway, NULL, receive_from_gateway, NULL);
	if(thd != 0){
		perror("THREAD: ERROR.");
		exit(-1);
	}
	
	pthread_t thds_client[MAX_SOCKETS];
    int client_fd[MAX_SOCKETS];
    memset(&client_fd, -1, MAX_SOCKETS*sizeof(int));
    listen(socket_stream_fd, MAX_SOCKETS);
    int aux=socket_stream_fd;
    while(1){		
    	client_fd[i] = accept(aux, (struct sockaddr *) & client_addr, &size_addr);
        if(client_fd[i]>0){
			int error = pthread_create(&thds_client[i], NULL, server_client, &client_fd[i]);
			if(error != 0){
				perror("pthread_create: ");
				exit(-1);
			}
		    i++;
		}
		sleep(1);
	} 
    exit(0);   
}
