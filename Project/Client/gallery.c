#include "gallery.h"
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
#include <inttypes.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/** Socket for communication with server **/
int socket_stream_fd;
/**Socket for communication with gateway **/
int socket_datagram_fd;
/** Server IP and Port received from Gateway **/
int server_port;
char server_ip[20];

//////////////////////////////////////////////////////GALLERY CONNECT/////////////////////////////////////////////////////////
int gallery_connect(char * host, in_port_t port){
	///////////TCP SOCKET///////////////////
	struct sockaddr_in socket_stream_addr;
	int option = 1;
	socket_stream_fd= socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(socket_stream_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	int nbytes;
	
	if (socket_stream_fd == -1){
		perror("socket: ");
		return(-1);
	}
	
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(1024 + getpid());
    socket_stream_addr.sin_addr.s_addr= INADDR_ANY;
    
    int err = bind(socket_stream_fd, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		return(-1);
	}

	////////////UDP///////////////////////////////
	socket_datagram_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_datagram_fd == -1){
		perror("socket: ");
		return(-1);
	}

	/** Function that sends MSG to gateway to receive a Server to connect **/

	struct sockaddr_in gateway_addr;
	msg_gw m;
	m.type = 1;
	
	gateway_addr.sin_family = AF_INET;
    gateway_addr.sin_port = htons(port);
    inet_aton(host, &gateway_addr.sin_addr);
    
    nbytes = sendto(socket_datagram_fd, (msg_gw*)&m , (1024+sizeof(m)),0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
	if(nbytes<0){
		perror("FAIL CONNECTION TO GATEWAY\n");
		return(-1);
	}

	/** Function that receives the Server from gateway **/
	msg_gw *server=malloc(sizeof(msg_gw));
	int size_addr = sizeof(gateway_addr);
	
	nbytes = recvfrom(socket_datagram_fd, server , sizeof(*server)+1, 0, (struct sockaddr *) & gateway_addr, &size_addr);
	if(nbytes<0){
		printf("Error. Receive from gateway:\n");
		return(-1);
	}
	if(server->type == 0){
		printf("NO SERVERS AVAIABLE. TRY AGAIN LATER\n");
		return(-1);
	}
	
	printf("RECEBI SERVIDOR COM IP %s e PORT %d\n",server->address,server->port);

	//Establishes peer conection

	char word[100];
	char receive_word[100];
	char cli[5]="cli\0";
	int socket_int;
	int receive;
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(server->port);
	
	if (!inet_aton(server->address, &server_addr.sin_addr)){
        perror("Error. Gateway IP:\n");
        return(-1);
    } 
	
	if( -1 == connect(socket_stream_fd, 
			(const struct sockaddr *) &server_addr, sizeof(server_addr))){
				printf("Connection Error\n");
				return(-1);
	}
	
	socket_int = send(socket_stream_fd, cli, strlen(cli)+1, 0);
	if(socket_int<0){
		printf("Connection Error\n");
		return(-1);
	}
	return(socket_stream_fd);
}

////////////////////////////////////////////////////////ADD PHOTO///////////////////////////////////////////

uint32_t gallery_add_photo(int peer_socket, char *file_name){
	char word[10];
	char addp[6]="addp\0";
	send(peer_socket, addp, strlen(addp)+1, 0);
	int recv1 = recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k') printf("Did nor recieve ack\n");

	FILE *picture;
	picture = fopen(file_name, "r");
	////////////////////////////////////////GET SIZE///////////////////////////////////////////
	int size;
	fseek(picture, 0, SEEK_END);
	size = ftell(picture);
	fseek(picture, 0, SEEK_SET);
	///////////////////////////////////////SEND SIZE//////////////////////////////////////////
	char n[8];
	sprintf(n, "%d", size);
	send(peer_socket, n, 8, 0);
	recv(peer_socket, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
	///////////////////////////////////////SEND NAME/////////////////////////////////////////
	send(peer_socket, file_name, 20, 0);
	recv(peer_socket, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");

	char send_buffer[size];
	while(!feof(picture)) {
	    fread(send_buffer, 1, sizeof(send_buffer), picture);
	    write(peer_socket, send_buffer, sizeof(send_buffer));
	    bzero(send_buffer, sizeof(send_buffer));
	}

	char sizec[12];
	recv(peer_socket, sizec, 12, 0);
	uint32_t ID;
	sscanf(sizec, "%d", &ID);
	printf("Image Uploaded Sucessfully With ID - %" PRIu32 "\n", ID);
	return (ID);
}

void list_photos(int peer_socket){
	char word[10000];
	memset(word, '\0', sizeof(char)*10000 );
	char list[6]="list\0";
	send(peer_socket, list, strlen(list)+1, 0);
	recv(peer_socket, word, 10000, 0);
	printf("%s\n", word);
}

int gallery_add_keyword(int peer_socket, char *keyword, uint32_t id_photos){
	char word[10], id[12];
	char key[6]="key\0";
	send(peer_socket, key, strlen(key)+1, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]=='o'&&word[1]=='k'){
		sprintf(id, "%d", id_photos);
		send(peer_socket, id, 12, 0);
		recv(peer_socket, word, 10, 0);
		if(word[0]=='o'&&word[1]=='k'){
			send(peer_socket, keyword, 20, 0);
			recv(peer_socket, word, 10, 0);
			if(word[0]=='o'&&word[1]=='k'){
				return(1);
			}
		}
	}
	return(0);
}

int gallery_delete_photo(int peer_socket, uint32_t id_photo){
	char word[10], id[12];
	char del[6]="del\0";
	send(peer_socket, del, strlen(del)+1, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(-1);
	}
	sprintf(id, "%d", id_photo);
	send(peer_socket, id, 12, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		return(0);
	} 
	return(1);
}

int gallery_search_photo(int peer_socket, char *keyword, uint32_t **id_photos){
	char word[10], id[12];
	char sizec[8], skey[6]="skey\0";
	char ok[10]="ok\0";
	int i, size;
	send(peer_socket, skey, strlen(skey)+1, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(-1);
	} 
	send(peer_socket, keyword, 20, 0);
	//recieve number of photos
	recv(peer_socket, sizec, 8, 0);
	sscanf(sizec, "%d", &size);
	//*id_photos=(uint32_t *)malloc(sizeof(uint32_t)*size);
	uint32_t *aux=(uint32_t *)malloc(sizeof(uint32_t)*size);
	for(int i=0; i<size;i++){
		send(peer_socket, ok, strlen(ok)+1, 0);
		recv(peer_socket, id, 12, 0);////////Not working
		sscanf(id, "%d", &aux[i]);
	}
	*id_photos=aux;
	return size;
}

int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name){
	char word[10], id[12];
	char sizec[8], getp[6]="getp\0";
	char ok[10]="ok\0";
	int i, size;
	send(peer_socket, getp, strlen(getp)+1, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(-1);
	} 
	sprintf(id, "%d", id_photo);
	send(peer_socket, id, 12, 0);
	recv(peer_socket, sizec, 8, 0);
	sscanf(sizec, "%d", &size);
	if(size==-1) return(0);
	send(peer_socket, ok, strlen(ok)+1, 0);
	char p_array[size];
	read(peer_socket, p_array, size);
	FILE *image;
	image = fopen(file_name, "w");
	fwrite(p_array, 1, sizeof(p_array), image);
	fclose(image);
	return 1;
}

int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name){
	char word[10], id[12], name[20];
	char sizec[8], idph[6]="idph\0";
	char ok[10]="ok\0";
	int i, size;
	send(peer_socket, idph, strlen(idph)+1, 0);
	recv(peer_socket, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(-1);
	} 
	sprintf(id, "%d", id_photo);
	send(peer_socket, id, 12, 0);
	recv(peer_socket, name, 20, 0);
	char *aux=(char *)malloc(sizeof(name));
	if(name[0]=='/'&&name[1]=='n'&&name[2]=='o'&&name[3]=='k'){
		return 0;
	}
	aux=name;
	*photo_name=aux;
	return 1;
}