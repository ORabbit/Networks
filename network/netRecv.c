#include <xinu.h>
#include <arp.h>
#include <icmp.h>

void netRecv(int arpId, int icmpId) {
	uchar pkt[PKTSZ];
	struct ethergram *eg;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	while (1) {
		bzero(pkt, PKTSZ);
		read(ETH0, pkt, PKTSZ);
		eg = (struct ethergram *)pkt;

		uchar brd[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (memcmp(eg->dst, myMac, ETH_ADDR_LEN) != 0 && memcmp(eg->dst, &brd[0], ETH_ADDR_LEN) != 0)
			continue;

		switch (ntohs(eg->type)) {
			case ETYPE_ARP:
				send(arpId, 1);
				resched();
				break;
			case ETYPE_IPv4:
				send(icmpId, 1);
				resched();
				break;
			default:
				break;
		}
	}
}
