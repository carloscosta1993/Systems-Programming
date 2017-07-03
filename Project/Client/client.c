#include "gallery.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/** Client socket port **/
int socket_port;


void print_options(){
	printf("\n<<<<<      Options      >>>>>\n1 - Help\n2 - List Photos\n3 - Add Photos\n4 - Add Keyword to Photo\n5 - Search Photos by Keyword\n6 - Delete Photo\n7 - Search photo by ID\n8 - Get photo\n9 - Exit\n");
}

void read_option(int socket_tcp){
	int opt, res, j;
	uint32_t *id, id1;
	char ID[12], *name, *fname=malloc(sizeof(char) * 100);
	char *key=malloc(sizeof(char) * 20);
	while(1){
		printf("\n<cmd>");
		opt=0;
		id1=0;
		memset(key, '\0', 20);
		scanf("%d", &opt);
		while(getchar()!='\n');
		switch(opt) {
		   case 1 :
		      print_options();
		      break; /* optional */
		   case 2:
		    	list_photos(socket_tcp);
		    	break;	
		   case 3 :
		        printf("\nEnter Image Name: ");
		        scanf("%s", fname);
		    	FILE *picture;
				picture = fopen(fname, "r");
		        if(picture==NULL){
			  		printf("\nFile does not exist\n");
			  		break;
			    }
			    fclose(picture);
			    gallery_add_photo(socket_tcp, fname);
		        break; /* optional */
		    case 4:
		    	printf("\nEnter Image ID: ");
		      	scanf("%s", ID);
		      	sscanf(ID, "%d", &id1);
		      	printf("\nEnter Keyword to add: ");
		      	scanf("%s", key);
		      	//P_id=&id;
		    	res=gallery_add_keyword(socket_tcp, key, id1);
				if(res) printf("\nKeyword Was Added\n");
				else printf("\nPhoto Was Not Found\n");
				break;
			case 5: 
				printf("\nEnter Keyword to Search: ");
		      	scanf("%s", key);
				res=gallery_search_photo(socket_tcp, key, &id);
				if (res<0) printf("Error Reaching Server\n");
				else{
					printf("<<<< There Were %d Matches >>>>\n", res);
					for(j=0; j<res; j++){
						printf("Match With Photo %d\n", *(id+j));
					}
				} 
				break;
			case 6:
				printf("\nEnter Image ID To Delete: ");
		      	scanf("%s", ID);
		      	sscanf(ID, "%d", &id1);
				res=gallery_delete_photo(socket_tcp, id1);
				if(res) printf("\nPhoto Was Sucessfuly Deleted\n");
				else if(res==0)printf("\nPhoto Was Not Found\n");
				else printf("\nSomething Went Wrong\n");
				break;
			case 7:
				printf("\nEnter Id of Photo to Search: ");
		      	scanf("%s", ID);
		      	sscanf(ID, "%d", &id1);
				res=gallery_get_photo_name(socket_tcp, id1, &name);
				if(res<0) printf("\nError ocurred\n");
		      	else if (res==0) printf("\nNo Photo Was Found With That ID\n");
		      	else printf("\nPhoto with ID %d is named %s\n", id1, name);
		      	break;
			case 8:
				printf("\nEnter Image ID To Download: ");
		      	scanf("%s", ID);
		      	sscanf(ID, "%d", &id1);
				printf("\nEnter Image Name: ");
		      	scanf("%s", fname);
		      	res=gallery_get_photo(socket_tcp, id1, fname);
		      	if(res<0) printf("\nError Ocurred\n");
		      	else if (res==0) printf("\nNo Photo Was Found With That ID\n");
		      	else printf("\nPhoto Was Downloaded Sucessfuly!\n");
				break;
			case 9:
				printf("\n<<<<<<<<<<<< Exiting. Good Bye! >>>>>>>>>>>>>>>\n");
				close(socket_tcp);
				exit(0);
		   default : break;/* Optional */
		}
	}
}

int main(int argc, char *argv[]){
	
	msg_gw *server=malloc(sizeof(msg_gw));
	int socket_tcp;
	printf("sizeof int %lu\n", sizeof(int));
	if (argc < 3){
        printf("First parameter: (IP GATEWAY). Second parameter: (GATEWAY PORT)");
        exit(1);
    }
        
    char *gateway_ip = argv[1];
    int gateway_port = atoi(argv[2]);
    
   	socket_tcp=gallery_connect(gateway_ip, gateway_port);
   	print_options();
   	read_option(socket_tcp);
	exit(0);    
}

