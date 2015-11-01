
#define ECHO_REPLY 0x0000
#define ECHO_REQUEST 0x0008

#define ICMP_HDR_LEN 8
/* ARP packet header */
struct icmpPkt {
	uchar type;
	uchar code; 
	ushort checksum;
	ushort id;
	ushort seq;
	uchar data[1]; //takes ip datagram 
};

extern int icmpDaemonId;

void printPacketICMP(uchar *);
command ping(uchar *);
void icmpDaemon(void);
