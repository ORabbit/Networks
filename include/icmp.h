
#define ECHO_REPLY 0
#define ECHO_REQUEST 8

/* ARP packet header */
struct icmpPkt {
	ushort type;
	ushort code; 
	uchar checksum;
	uchar data[]; //takes ip datagram 
};

command ping(uchar *);
void icmpDaemon(void);
