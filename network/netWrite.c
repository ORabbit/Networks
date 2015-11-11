/**
 * @file ipWrite.c
 * @provides ipWrite
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
//uchar *packetPING;
//uchar *macOther;

/**
 * data link layer: which encapsulates what it recieves in an ethernet frame
 *
 */
command netWrite(uchar *payload, ushort payload_len,uchar e_type, uchar* mac)
{
	//construct ethergram
	uchar packet[PKTSZ];
	bzero(packet,PKTSZ);

	struct ethergram * eg = (struct ethergram*) packet;
	memcpy(eg->dst, mac, ETH_ADDR_LEN); 
	control(ETH0, ETH_CTRL_GET_MAC, (long)eg->src, 0);
	eg->type = htons(e_type);

	bzero(eg->data,payload_len);
	memcpy(&eg->data[0], payload, payload_len);


	kprintf("About to write packet to ETH0\r\n");
	ushort size = ETHER_MINPAYLOAD < ntohs(ipPkt->len) ? ntohs(ipPkt->len) : ETHER_MINPAYLOAD;
	write(ETH0, packet,ETHER_SIZE+size);

	return OK;
}

