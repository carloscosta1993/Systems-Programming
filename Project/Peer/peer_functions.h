#define GW_CLIENT_PORT 2001
#define GW_SERVER_PORT 3001
#define MESSAGE_LEN 100
#define MAX_SOCKETS 10

#include <netinet/in.h>

typedef struct message{
    char buffer[MESSAGE_LEN];
} message;

typedef struct img_str{
	int size;
	uint32_t id;
	char name[20];
	char keywords[20][20];
	struct img_str * next;
}img_str;

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

int establish_p_t_p(char * host, in_port_t port, char * opt, img_str * head_img);
void send_photo(int sock, img_str * head_img);
int delete_from_peer(int sock, img_str * head_img, uint32_t id_photo);
img_str * get_photo(int *sock, img_str * head_img);
img_str * delete_photo(int *sock, img_str * head_img, char **del);
void add_key_to_peer(int sock, uint32_t id_photo, char *keyword);
void list_photos(int *sock, img_str * head_img);
void send_evr(int sock, img_str * head_img);
void add_keyword(int *sock, img_str * head_img, char **key);
void search_key(int *sock, img_str * head_img);
void send_photoc(int *sock, img_str * head_img);
void find_name(int *sock, img_str * head_img);
img_str * get_photo_peer(int *sock, img_str * head_img);

img_str * establish_to_download(char * host, in_port_t port, img_str * head_img);
img_str * get_everything(int sock, img_str * head_img);