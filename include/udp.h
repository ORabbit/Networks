#include "network.h"
#define MAX_QUEUE_SIZE 30
#define UDP_SOCK_OPEN 1
#define UDP_HDR_LEN 8
#define MAX_SOCKETS 10
struct udpSocketTable{
	udpSocket sockets[MAX_SOCKETS];
	uchar size;
};
struct udpSocket{
	uchar state;
	uchar remote_ip[IP_ADDR_LEN];	
	uchar local_ip[IP_ADDR_LEN];	
	ushort remote_port;	
	ushort local_port;	
	struct cqueue packets;
};
struct userDataGram{
	ushort src_port;
	ushort dst_port;
	ushort total_len;
	ushort checksum;
	uchar data[1];
};
struct cqueue{
	uchar head;
	uchar tail;
	uchar size;
	uchar* cqueue_arr[MAX_QUEUE_SIZE];
};
int udpOpen(struct udpSocket*);
void udpWrite(struct udpSocket*,void*);
void* udpRead(struct udpSocket*);
uchar* dequeue(struct cqueue);
syscall enqueue(struct cqueue,uchar*);
extern struct udpTable* udpGlobalTable;
