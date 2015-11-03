#include <xinu.h>
#include <arp.h>
#include <icmp.h>

void netRecv(int arpId, int icmpId) {
	uchar pkt[PKTSZ];
	struct ethergram *eg;	

	while (1) {
		read(ETH0, pkt, PKTSZ);
		eg = (struct ethergram *)pkt;


		switch (ntohs(eg->type)) {
			case ETYPE_ARP:
				send(arpId, 1);
				resched();
				break;
			case ETYPE_IPv4:
				send(icmpId, 1);
				resched();
				break;
		}
	}
}
