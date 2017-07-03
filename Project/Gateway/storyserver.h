#define GW_CLIENT_PORT 2001
#define GW_SERVER_PORT 3001
#define MESSAGE_LEN 100
#define MAX_SOCKETS 10

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
