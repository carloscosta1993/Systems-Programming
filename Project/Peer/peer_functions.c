#include "peer_functions.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <pthread.h>

/** Socket for communication with server **/
int socket_stream_fd, sock_peer;
/**Socket for communication with gateway **/
int socket_datagram_fd;
/** Server IP and Port received from Gateway **/
int server_port;
char server_ip[20];	

uint32_t generate_id(img_str * head_img){
	srand(time(NULL));   // should only be called once
	uint32_t hash = rand();  

	hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    img_str *current=head_img;
    while(current!=NULL){
   		if(current->id==hash){
   			hash=generate_id(head_img);
   		}
   		current=current->next;
   	}
   	return hash%2000000000;
}


int establish_p_t_p(char * host, in_port_t port, char * opt, img_str * head_img){
	///////////TCP SOCKET///////////////////
	struct sockaddr_in socket_stream_addr;
	int option = 1;
	socket_stream_fd= socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(socket_stream_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	int nbytes, recv1;
	
	if (socket_stream_fd == -1){
		perror("socket: ");
		return(-1);
	}
	
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(1024 + getpid()*getpid());
    socket_stream_addr.sin_addr.s_addr= INADDR_ANY;
    
    int err = bind(socket_stream_fd, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		return(-1);
	}

	char word[10];
	char receive_word[100];
	char peer[6]="peer\0";
	int socket_int;
	int receive;
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(port);

	if (!inet_aton(host, &server_addr.sin_addr)){
        perror("Error. adress IP:\n");
        return(-1);
    }
	
	if( -1 == connect(socket_stream_fd, 
			(const struct sockaddr *) &server_addr, sizeof(server_addr))){
		printf("Connection Error - 0\n");
		return(-1);
	}
	
	/////////////////////////SEND PEER///////////////////////////////////
	uint32_t id;
	socket_int = send(socket_stream_fd, peer, strlen(peer)+1, 0);
	if(socket_int<0){
		printf("Connection Error\n");
		return(-1);
	}
	recv1 = recv(socket_stream_fd, word, 10, 0);

	if(word[0]!='o'&&word[1]!='k') printf("Did not Recieve ACK\n");
	//////////////////////STATE////////////////////////////////////////////
	if(opt[0]=='a'&&word[1]!='d'){
		send_photo(socket_stream_fd, head_img);
	}else if(opt[0]=='d'&&word[1]!='e'&&word[2]!='l'){
		char *del=(char *)malloc(3);
		sscanf(opt, "%s %d", del, &id); free(del);
		delete_from_peer(socket_stream_fd, head_img, id);
	}else if(opt[0]=='k'&&word[1]!='e'&&word[2]!='y'){
		char *key=(char *)malloc(3);
		char *keyword=(char *)malloc(20);
		sscanf(opt, "%s %d %s", key, &id, keyword); free(key);
		add_key_to_peer(socket_stream_fd, id, keyword);
	}else if(opt[0]=='e'&&word[1]!='v'&&word[2]!='r'){
		send_evr(socket_stream_fd, head_img);
	}
	return(socket_stream_fd);
}

void send_evr(int sock, img_str * head_img){
	char word[10], id[12], num[8];
	char evr[6]="evr\0", NOK[20]="NOK\0"; 
	int n;
	img_str * current = head_img;
	if(current==NULL){
		n=0;
		sprintf(num, "%d", n);
		send(sock, num, 8, 0);
		recv(sock, word, 10, 0); 
		return;
	} 
	while(current!=NULL){
		memset(id, '\0', sizeof(char)*12 );
		memset(num, '\0', sizeof(char)*8 );
		////////////SEND SIZE/////////////////
		sprintf(num, "%d", current->size);
		send(sock, num, 8, 0);
		recv(sock, word, 10, 0); 
		if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
		/////////////Send ID/////////////////////
		sprintf(id, "%d", current->id);
		send(sock, id, 12, 0);
		recv(sock, word, 10, 0); 
		if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
		////////////SEND NAME//////////////////
		send(sock, current->name, 20, 0);
		recv(sock, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
		//////////////SEND KEYS//////////////////////
		for(int i=0; i<20; i++){
			memset(num, '\0', sizeof(char)*8 );
			if(current->keywords[i][0]=='\0'){
				send(sock, NOK, 20, 0);
				recv(sock, word, 10, 0); 
				if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
				break;
			}
			else{
				send(sock, current->keywords[i], 20, 0);
				recv(sock, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
			}

		}
		//////////////SEND FILE//////////////////
		FILE *picture;
		picture = fopen(current->name, "r");
		char send_buffer[current->size];

		while(!feof(picture)) {
		    fread(send_buffer, 1, sizeof(send_buffer), picture);
		    write(sock, send_buffer, sizeof(send_buffer));
		    bzero(send_buffer, sizeof(send_buffer));
		}

		recv(sock, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
		current=current->next;
	}
	n=0;
	sprintf(num, "%d", n);
	send(sock, num, 8, 0);
	printf("Sent Images To Peer\n"); 
}

void add_key_to_peer(int sock, uint32_t id_photo, char *keyword){
	char word[10], id[12];
	char key[6]="key\0";
	send(sock, key, strlen(key)+1, 0);
	recv(sock, word, 10, 0);
	if(word[0]=='o'&&word[1]=='k'){
		sprintf(id, "%d", id_photo);
		send(sock, id, 12, 0);
		recv(sock, word, 10, 0);
		if(word[0]=='o'&&word[1]=='k'){
			send(sock, keyword, 20, 0);
			recv(sock, word, 10, 0);
		}
	}
}

int delete_from_peer(int sock, img_str * head_img, uint32_t id_photo){
	char word[10], id[12];
	char del[6]="del\0";
	send(sock, del, strlen(del)+1, 0);
	recv(sock, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(-1);
	}
	sprintf(id, "%d", id_photo);
	send(sock, id, 12, 0);
	recv(sock, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k'){
		printf("Could not reach server\n");
		return(0);
	} 
	return(1);

}

img_str * get_photo_peer(int *sock, img_str * head_img){
	int size, recv1, status=0;
	char sizec[8], name[20];

	img_str * current = head_img;
	if(current==NULL){
		current= malloc(sizeof(img_str));
		current->next=NULL;
		status=1;
	} else{
		while(current->next!=NULL){
			current=current->next;
		}
		current->next=malloc(sizeof(img_str));
		current->next->next=NULL;
		current=current->next;
	}
	memset(current->keywords, '\0', sizeof(char)*400 );

	///////////////////////RECIEVE SIZE///////////////////////////////////////////////////////
	recv(*sock, sizec, 8, 0);
	sscanf(sizec, "%d", &size);
	char OK[10]={"ok"};
	send(*sock, OK, 10, 0);
	current->size=size;
	/////////////////////Recieve ID
	char idc[12];
	recv(*sock, idc, 12, 0);
	sscanf(idc, "%d", &current->id);
	send(*sock, OK, 10, 0);

	recv(*sock, name, 20, 0);
	send(*sock, OK, 10, 0);
	strcpy(current->name, name);

	char p_array[current->size];

	read(*sock, p_array, current->size);

	FILE *image;
	image = fopen(current->name, "w");
	fwrite(p_array, 1, sizeof(p_array), image);
	fclose(image);
	printf("Got Photo From Peer\n");
	if(status) head_img=current;
	return head_img;
	
}

void send_photo(int sock, img_str * head_img){
	char word[10];
	char addp[6]="addp\0";
	send(sock, addp, strlen(addp)+1, 0);
	recv(sock, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
	
	img_str * current = head_img;

	while(current->next!=NULL){

		current=current->next;

	}

	FILE *picture;
	picture = fopen(current->name, "r");
	
	int size=current->size;
	char n[8];
	sprintf(n, "%d", size);
	/////////////////////Send Size
	
	send(sock, n, 8, 0);
	recv(sock, word, 10, 0); 
	if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
	/////////////////////Send ID
	char send_buffer[size];
	char nid[12];
	int id=current->id;
	sprintf(nid, "%d", id);
	send(sock, nid, 12, 0);
	recv(sock, word, 10, 0); 
	if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
	/////////////////////Send Name	

	send(sock, current->name, 20, 0);
	recv(sock, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");

	while(!feof(picture)) {
	    fread(send_buffer, 1, sizeof(send_buffer), picture);
	    write(sock, send_buffer, sizeof(send_buffer));
	    bzero(send_buffer, sizeof(send_buffer));
	}
}

img_str * get_photo(int *sock, img_str * head_img){
	int size, recv1, status=0;
	char sizec[8], name[20];
	img_str * current = head_img;
	if(current==NULL){
		current= malloc(sizeof(img_str));
		current->next=NULL;
		status=1;
	} else{
		while(current->next!=NULL){
			current=current->next;
		}
		current->next=malloc(sizeof(img_str));
		current->next->next=NULL;
		current=current->next;
	}
	memset(current->keywords, '\0', sizeof(char)*400 );
	///////////////////////RECIEVE SIZE///////////////////////////////////////////////////////
	recv1=recv(*sock, sizec, 8, 0);
	sscanf(sizec, "%d", &size);
	char OK[10]={"ok"};
	send(*sock, OK, 10, 0);
	current->size=size;
	///////////////////////RECIEVE NAME///////////////////////////////////////////////////////
	recv1=recv(*sock, name, 20, 0);
	//sscanf(sizec, "%d", &size);
	send(*sock, OK, 10, 0);
	strcpy(current->name, name);
	///////////////////////RECIEVE PHOTO///////////////////////////////////////////////////////
	char p_array[size];
	read(*sock, p_array, size);
	///////////////////////GENERATE ID/////////////////////////////////////////////////////////
	
	current->id=generate_id(head_img);

	printf("Generated ID %" PRIu32 "\n", current->id);
	FILE *image;
	image = fopen(name, "w");
	fwrite(p_array, 1, sizeof(p_array), image);
	fclose(image);

	char n[12];
	int id=(int)current->id;
	sprintf(n, "%d", current->id);
	send(*sock, n, 12, 0);
	if(status) head_img=current;
	return head_img;
}

void list_photos(int *sock, img_str * head_img){
	char buffer[10000];
	memset(buffer, '\0', sizeof(char)*10000 );
	char temp[500];
	if (head_img==NULL){
			printf("<<<<<<<<<No Images In Database>>>>>>>>>>>\n");
			strcat(buffer, "<<<<<<<<<No Images In Database>>>>>>>>>>>\n");
	}else{
		img_str *current=head_img;
		printf("<<<<<<<< Listing Images >>>>>>>>>>>>>>>>>\n\n");
		strcat(buffer,"<<<<<<<< Listing Images >>>>>>>>>>>>>>>>>\n\n");
		int j, i=0;
		while(current!=NULL){
			i++;
			printf("%d - %s        ID - %d\n", i, current->name, current->id);
			sprintf(temp, "%d - %s        ID - %d\n", i, current->name, current->id);
			strcat(buffer, temp);
			printf("Keywords: ");
			strcat(buffer,"Keywords: ");
			j=0;
			while(current->keywords[j][0]!='\0'){
				printf("%s/ ", current->keywords[j]);
				sprintf(temp, "%s/ ", current->keywords[j]);
				strcat(buffer, temp);
				j++;
			}
			printf("\n");
			strcat(buffer, "\n");
			current=current->next;
		}
	}
	send(*sock, buffer, strlen(buffer)+1, 0);
}

void add_keyword(int *sock, img_str * head_img, char **key){
	char idc[12];
	int i;
	char nok[10]="nok\0";
	char ok[10]="ok\0";

	img_str *current=head_img;
	uint32_t id;
	send(*sock, ok, strlen(ok)+1, 0);
	recv(*sock, idc, 12, 0);
	sscanf(idc, "%d", &id);
	char *aux_key=(char *)malloc(sizeof(char)*40);
	memset(key, '\0', sizeof(char)*40);
	char aux[3]="key";

	if(current==NULL){
		printf("No photos available\n");
		send(*sock, nok, strlen(nok)+1, 0);
		return;
	}
	while(current!=NULL){
		int status=0;
		if(current->id==id){
			send(*sock, ok, strlen(ok)+1, 0);
			char keyword[20];
			recv(*sock, keyword, 20, 0);
			sprintf(aux_key, "%s %d %s", aux, id, keyword);
			*key=aux_key;
			for(i=0; i<20;i++){
				if(current->keywords[i][0]=='\0'){
					strcpy(current->keywords[i], keyword);
					status=1;
					break;
				}
			}
			if(status) send(*sock, ok, strlen(ok)+1, 0);
			else send(*sock, nok, strlen(nok)+1, 0);
			return;
		}
		current=current->next;
	}
	send(*sock, nok, strlen(nok)+1, 0);
	return;
}

img_str * delete_photo(int *sock, img_str * head_img, char** del){
	char idc[12];
	int i;
	char nok[10]="nok\0";
	char ok[10]="ok\0";
	/////////////For sending afterwards
	char *aux_del=(char *)malloc(sizeof(char)*20);
	memset(aux_del, '\0', sizeof(char)*20);
	img_str *aux, *current=head_img;
	uint32_t id;
	send(*sock, ok, strlen(ok)+1, 0);
	recv(*sock, idc, 12, 0);
	sscanf(idc, "%d", &id);
	char auxi[3]="del";
	sprintf(aux_del, "%s %d", auxi, id);
	*del=aux_del;
	printf("PHOTO DELETED\n");
	if(current==NULL){
		printf("No photos available\n");
		send(*sock, nok, strlen(nok)+1, 0);
		return head_img;
	}
	if(head_img==NULL){
		send(*sock, nok, strlen(nok)+1, 0);
		return head_img;
	} else{
		while(current!=NULL){
			if(current->id==id){
				if(current==head_img){
					if(current->next!=NULL){
						head_img=head_img->next;
						free(current);
						send(*sock, ok, strlen(ok)+1, 0);
						return(head_img);
					}else{
						free(current);
						send(*sock, ok, strlen(ok)+1, 0);
						return NULL;
					}
				} 
				else{
					aux->next=current->next;
					free(current);
					send(*sock, ok, strlen(ok)+1, 0);
					return head_img;
				}
			}
		aux=current;
		current=current->next;
		}
	send(*sock, nok, strlen(nok)+1, 0);
	return head_img;
	}
}

void search_key(int *sock, img_str * head_img){
	char keyword[20], n[8], word[10], id[12];
	int j, k=0, i=0, pos[20];
	char nok[10]="nok\0";
	char ok[10]="ok\0";
	img_str *current=head_img;
	send(*sock, ok, strlen(ok)+1, 0);
	recv(*sock, keyword, 20, 0);
	if(head_img==NULL){
		i=-1;
		sprintf(n, "%d", i);
		send(*sock, n, 8, 0);
		return;
	}
	while(current!=NULL){
		for(j=0; j<20; j++){
			if(current->keywords[j][0]!='\0'){
				if(strcmp(current->keywords[j], keyword)==0){
					pos[i]=k;
					i++; j=20;
				}
			}
		}
		k++;
		current=current->next;
	}
	sprintf(n, "%d", i);
	send(*sock, n, 8, 0);
	current=head_img;
	k=0; j=0;
	while(current!=NULL){
		if(pos[j]==k){
			recv(*sock, word, 10, 0);
			sprintf(id, "%d", current->id);
			send(*sock, id, 12, 0);
			j++;
		}
	current=current->next;
	k++;
	}
}

void send_photoc(int *sock, img_str * head_img){
	char keyword[20], n[8], word[10], id[12], name[20];
	int size, i=0, pos[20], found=0;
	char nok[10]="nok\0";
	char ok[10]="ok\0";
	uint32_t aux;
	img_str *current=head_img;
	send(*sock, ok, strlen(ok)+1, 0);
	recv(*sock, id, 12, 0);
	sscanf(id, "%d", &aux);
	if(head_img==NULL){
		i=-1;
		sprintf(n, "%d", i);
		send(*sock, n, 8, 0);
		return;
	}
	while(current!=NULL){
		if(current->id==aux){
			found=1;
			break;
		}
		current=current->next;
	}
	if(found){
		FILE *picture;
		picture = fopen(current->name, "r");
		////////////////////////////////////////GET SIZE///////////////////////////////////////////
		fseek(picture, 0, SEEK_END);
		size = ftell(picture);
		fseek(picture, 0, SEEK_SET);
		///////////////////////////////////////SEND SIZE//////////////////////////////////////////
		char n[8];
		sprintf(n, "%d", size);
		send(*sock, n, 8, 0);
		recv(*sock, word, 10, 0); if(word[0]!='o'&&word[1]!='k') printf("Did not recieve ACK\n");
		///////////////////////////////////////SEND NAME/////////////////////////////////////////

		char send_buffer[size];
		while(!feof(picture)) {
		    fread(send_buffer, 1, sizeof(send_buffer), picture);
		    write(*sock, send_buffer, sizeof(send_buffer));
		    bzero(send_buffer, sizeof(send_buffer));
		}
	}else{
		i=-1;
		sprintf(n, "%d", i);
		send(*sock, n, 8, 0);
		return;
	}
}

void find_name(int *sock, img_str * head_img){
	char keyword[20], n[8], word[10], id[12], name[20];
	int size, i=0, pos[20], found=0;
	char nok[10]="/nok\0";
	char ok[10]="ok\0";
	uint32_t aux;
	img_str *current=head_img;
	send(*sock, ok, strlen(ok)+1, 0);
	recv(*sock, id, 12, 0);
	sscanf(id, "%d", &aux);
	if(head_img==NULL){
		send(*sock, nok, 10, 0);
		return;
	}
	while(current!=NULL){
		if(current->id==aux){
			found=1;
			break;
		}
		current=current->next;
	}
	if(found){
		send(*sock, current->name, 20, 0);		
	}else{
		send(*sock, nok, 10, 0);		
	}
}

img_str * establish_to_download(char * host, in_port_t port, img_str * head_img){
	///////////TCP SOCKET///////////////////
	struct sockaddr_in socket_stream_addr;
	int option = 1;
	sock_peer= socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sock_peer, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	int nbytes, recv1;
	
	if (sock_peer == -1){
		perror("socket: ");
		return(NULL);
	}
	
    socket_stream_addr.sin_family = AF_INET;
    socket_stream_addr.sin_port= htons(1024 + getpid()*getpid());
    socket_stream_addr.sin_addr.s_addr= INADDR_ANY;
    
    int err = bind(sock_peer, (struct sockaddr *)&socket_stream_addr, sizeof(socket_stream_addr));
	if(err == -1) {
		perror("bind");
		return(NULL);
	}

	char word[10];
	char receive_word[100];
	char peer[6]="peer\0";
	int socket_int;
	int receive;
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(port);
	if (!inet_aton(host, &server_addr.sin_addr)){
        perror("Error. adress IP:\n");
        return(NULL);
    }
	
	if( -1 == connect(sock_peer, 
			(const struct sockaddr *) &server_addr, sizeof(server_addr))){
		printf("Connection Error - 0\n");
		return(NULL);
	}
	
	/////////////////////////SEND PEER///////////////////////////////////
	uint32_t id;
	socket_int = send(sock_peer, peer, strlen(peer)+1, 0);
	if(socket_int<0){
		printf("Connection Error\n");
		return(NULL);
	}

	recv1 = recv(sock_peer, word, 10, 0);
	if(word[0]!='o'&&word[1]!='k') printf("NO ACK\n");

	head_img=get_everything(sock_peer, head_img);
	//////////////////////STATE////////////////////////////////////////////
	close(sock_peer);
	return(head_img);
}

img_str * get_everything(int sock, img_str * head_img){
	char word[10], id[12], num[8];
	char evr[6]="evr\0", NOK[20]="NOK\0", OK[10]={"ok"};; 
	int size, i, recv1, status=0;
	char sizec[8], name[20];
	int n;
	send(sock, evr, strlen(evr)+1, 0);
	img_str * current=head_img;
	char idc[12], keyword[20];
	while(1){
		size=0;
		recv(sock, sizec, 8, 0);
		sscanf(sizec, "%d", &size);
		if(size==0){
			break;
		}
		if(current==NULL){
			current= malloc(sizeof(img_str));
			current->next=NULL;
		}else{
			current->next=malloc(sizeof(img_str));
			current->next->next=NULL;
			if(status==0){status=1; head_img=current;}
			current=current->next;
		}
		memset(current->keywords, '\0', sizeof(char)*400 );

		send(sock, OK, 10, 0);
		current->size=size;
		//////////////RECIEVE ID
		memset(idc, '\0', sizeof(char)*12 );
		recv(sock, idc, 12, 0);
		sscanf(idc, "%d", &current->id);
		send(sock, OK, 10, 0);
		//////////////RECIEVE NAME
		memset(name, '\0', sizeof(char)*20 );
		recv(sock, name, 20, 0);
		send(sock, OK, 10, 0);
		strcpy(current->name, name);
		//////////////RECIEVE KEYWORD
		for(i=0; i<20; i++){
			memset(keyword, '\0', sizeof(char)*20 );
			recv(sock, keyword, 20, 0);
			if(keyword[0]=='N'&&keyword[1]=='O'&&keyword[2]=='K'){
				send(sock, OK, 10, 0); break;
			}
			else{
				strcpy(current->keywords[i], keyword);
				send(sock, OK, 10, 0);
			}
		}
		char p_array[current->size];

		read(sock, p_array, current->size);

		FILE *image;
		image = fopen(current->name, "w");
		fwrite(p_array, 1, sizeof(p_array), image);
		fclose(image);
		send(sock, OK, 10, 0);
		printf("Photo Recieved\n");
	}
	if(status==0){status=1; head_img=current;}
	return head_img;
}