

#define ARP_OP_RQST 1
#define ARP_OP_REPLY 2

#define MAX_ARP_TABLE 30
#define ARP_ADDR_SHA 0
#define ARP_ADDR_SPA ETH_ADDR_LEN
#define ARP_ADDR_DHA ARP_ADDR_SPA + IPv4_ADDR_LEN
#define ARP_ADDR_DPA ARP_ADDR_DHA + ETH_ADDR_LEN

#define CLKTICKS_PER_SEC 1000 // DOUBLE CHECK

/* ARP packet header */
struct arpPkt {
	ushort hwtype;
	ushort prtype; // Protocol type
	uchar hwalen;
	uchar pralen;
	ushort op;
	uchar addrs[]; /* 	Source Hardware Address (SHA)
								Source Protocol Address (SPA)
								Destination Hardware Address (DHA)
								Destination Protocol Address (DPA)
						 */
};

extern uchar *myipaddr;
extern int arpDaemonId;

command arp(int, int, uchar *);
void arpDaemon(void);
devcall arpResolve(uchar *, uchar *);
void arpResolveHelper(uchar *, int);
