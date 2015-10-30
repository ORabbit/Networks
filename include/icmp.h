
#define ECHO_REPLY 0
#define ECHO_REQUEST 8

/* ARP packet header */
struct icmpPkt {
	ushort type;
	ushort code; 
	uchar checksum;
	char data[1]; //takes ip datagram 
};

extern int icmpDaemonId;

command ping(uchar *);
void icmpDaemon(void);
