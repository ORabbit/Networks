#define MAX_QUEUE_SIZE 30
#define UDP_SOCK_OPEN 1
#define UDP_HDR_LEN 8
#define MAX_SOCKETS 10
struct udpSocketTable{
	struct udpSocket* sockets[MAX_SOCKETS];
	uchar size;
};
struct checksumUDPHeader{
	uchar ip_src[IP_ADDR_LEN];
	uchar ip_dst[IP_ADDR_LEN];
	ushort proto;
	ushort total_len;
	ushort src_port;
	ushort dst_port;
	ushort total_len2;
	ushort checksum;
	uchar* data[1];
};
struct udpSocket{
	uchar state;
	uchar remote_ip[IP_ADDR_LEN];	
	uchar local_ip[IP_ADDR_LEN];	
	ushort remote_port;	
	ushort local_port;	
	struct cqueue* packets;
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
int udpOpen(struct udpSocket*,uchar*,ushort);
void udpWrite(struct udpSocket*,void*,unsigned int);
void* udpRead(struct udpSocket*);
syscall udpDaemon(uchar*);
uchar* cdequeue(struct cqueue*);
syscall cenqueue(struct cqueue*,uchar*);
void socketDaemon(void);
command echo(uchar*,ushort,char[],ushort);
void printPacketUDP(uchar*);
int globTableContains(struct udpSocket*);
void convertToDate(unsigned int);
command rdate(uchar*,ushort);

extern struct udpSocketTable* udpGlobalTable;
