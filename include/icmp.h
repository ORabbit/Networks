
#define ECHO_REPLY 0x0000
#define ECHO_REQUEST 0x0008

#define ICMP_HDR_LEN 8
/* ICMP packet header */
struct icmpPkt {
	uchar type;
	uchar code; 
	ushort checksum;
	ushort id;
	ushort seq;
	uchar data[1]; 
};

struct icmpEcho {
	ushort id;
	ushort seq;
	ulong timesec;			// Clock time in seconds
	ulong timetic;			// Clock time in tics
	ulong timecyc;			// Clock time in cycles
	ulong arrivsec;
	ulong arrivtic;
	ulong arrivcyc;
};

extern int icmpDaemonId;

void printPacketICMP(uchar *);
command ping(uchar *, ushort,int);
syscall icmpDaemon(uchar[], uchar);
