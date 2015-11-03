/**
 * @file netInit.c
 * @provides netInit.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include <arp.h>
#include <icmp.h>

int arpDaemonId;
int icmpDaemonId;
int netRecvId;
uchar* myipaddr;
/*
 * Initialize network interface.
 */
void netInit(void)
{
	myipaddr = malloc(IPv4_ADDR_LEN);	
	uchar *mac = malloc(ETH_ADDR_LEN);

	open(ETH0);
	arpDaemonId = create((void *)arpDaemon, INITSTK, 3, "ARP_DAEMON", 0);
	icmpDaemonId = create((void *)icmpDaemon, INITSTK, 3, "ICMP_DAEMON", 0);
	netRecvId = create((void *)netRecv, INITSTK, 3, "NET_RECV_DAEMON", 2, arpDaemonId, icmpDaemonId);
	//control(ETH0, ETH_CTRL_GET_MAC, (long) mac, 0);
	//do {
	//	read(ETH0, packet, PKTSZ);

	//printf("PktType:%08x\tMAC:%02x:%02x:%02x:%02x:%02x:%u\r\nOURMAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n", ntohs(eg->type), eg->dst[0], eg->dst[1], eg->dst[2], eg->dst[3], eg->dst[4], eg->dst[5], mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	//} while(memcmp(eg->dst, mac, sizeof(mac)) != 0);
	//we found the packet with our ip address. Let's get out ip address
	//struct arpPkt* tmpArpPkt = (struct arpPkt*) &eg->data[0];
	//memcpy(myipaddr, &tmpArpPkt->addrs[ARP_ADDR_DPA], IPv4_ADDR_LEN);
	//printf("IP Address is: %u.%u.%u.%u\r\n", myipaddr[0], myipaddr[1], myipaddr[2], myipaddr[3]);
	dot2ip(nvramGet("lan_ipaddr\0"), myipaddr);
printf("IP Address is: %u.%u.%u.%u\r\n", myipaddr[0], myipaddr[1], myipaddr[2], myipaddr[3]);

	ready(netRecvId, 0);
	ready(arpDaemonId, 0); /* Starts an ARP Daemon for the backend. */
	ready(icmpDaemonId, 1);
	return;
}
