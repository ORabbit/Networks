/**
 * @file netInit.c
 * @provides netInit.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include <arp.h>

int arpDaemonId;
uchar* myipaddr;
/*
 * Initialize network interface.
 */
void netInit(void)
{
	uchar * packet[PKTSZ];
	void *pkt = malloc(ETHER_SIZE + sizeof(struct arpPkt));
	uchar *mac = malloc(ETH_ADDR_LEN);
	struct arpPkt *temp2;
	uchar *temp3=malloc(4);

	open(ETH0);
	arpDaemonId = create((void *)arpDaemon, INITSTK, 3, "ARP_DAEMON", 0);
	control(ETH0, ETH_CTRL_GET_MAC, (long) mac, 0);
	do {
		read(ETH0, pkt, sizeof(pkt));
		struct ethergram *temp = (struct ethergram *) pkt;
		char *tempData = temp->data;
		temp2 = (struct arpPkt *) tempData;
		memcpy(temp3, &temp2->addrs[ARP_ADDR_DHA], IPv4_ADDR_LEN);
		printf("PktType:%08x\tMAC:%02x:%02x:%02x:%02x:%02x:%u\r\nOURMAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n", ntohs(temp2->prtype), temp2->addrs[ARP_ADDR_DHA], temp2->addrs[ARP_ADDR_DHA+1], temp2->addrs[ARP_ADDR_DHA+2], temp2->addrs[ARP_ADDR_DHA+3], temp2->addrs[ARP_ADDR_DHA+4], temp2->addrs[ARP_ADDR_DHA+5], mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	} while(memcmp(&temp2->addrs[ARP_ADDR_DHA], mac, sizeof(mac)) != 0);
	//we found the packet with our ip address. Let's get out ip address
	memcpy(myipaddr, &temp2->addrs[ARP_ADDR_DPA], IPv4_ADDR_LEN);
	printf("IP Address is: %u.%u.%u.%u\r\n", myipaddr[0], myipaddr[1], myipaddr[2], myipaddr[3]);
	ready(arpDaemonId, 1); /* Starts an ARP Daemon for the backend. */
	return;
}
