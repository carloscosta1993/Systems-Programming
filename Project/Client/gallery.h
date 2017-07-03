#define GW_CLIENT_PORT 2001
#define GW_SERVER_PORT 3001
#define MESSAGE_LEN 100
#define MAX_SOCKETS 10

#include <netinet/in.h>

typedef struct message{
    char buffer[MESSAGE_LEN];
} message;

typedef struct message_gw{
	int type;
	char address[20];
	int port;
} msg_gw;

typedef struct sv{
	char address[20];
	int port;
	struct sv * next;
}node_t;


int gallery_connect(char * host, in_port_t port);
uint32_t gallery_add_photo(int peer_socket, char *file_name);
void list_photos(int peer_socket);
int gallery_add_keyword(int peer_socket, char *keyword, uint32_t id_photos);
int gallery_delete_photo(int peer_socket, uint32_t id_photo);
int gallery_search_photo(int peer_socket, char *keyword, uint32_t **id_photos);
int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name);
int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name);
