/**
 * @file ping.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
//uchar packetPING[PKTSZ];
 uchar *packetPING;
void printPacketICMP(uchar *);
void printAddr(uchar *,ushort);
/**
 * pings another ip address
 */
command ping(uchar *ip)
{
	packetPING = malloc(PKTSZ);
	uchar *mac = malloc(ETH_ADDR_LEN);
	if (SYSERR == arpResolve(ip, mac)) {
		printf("ERROR in ping->arpResolve\n");
		return SYSERR;
	}

	bzero(packetPING, PKTSZ);
	//ethergram
	struct ethergram * eg = (struct ethergram*) packetPING;
	memcpy(eg->dst, mac, ETH_ADDR_LEN); 
	control(ETH0, ETH_CTRL_GET_MAC, (long)eg->src, 0);
	eg->type = htons(ETYPE_IPv4);

	struct icmpPkt *icmppkt = malloc(sizeof(struct icmpPkt*));
	icmppkt->type = htons(ECHO_REQUEST);
	icmppkt->code = 0;
	icmppkt->checksum = htons(checksum(icmppkt, sizeof(struct icmpPkt*)));
	//icmppkt->data
	
	//constructing ip packet
	struct ipgram *ipPkt = malloc(sizeof(struct ipgram*));
	ipPkt->ver_ihl = (uchar)htons(IPv4_VERSION + IPv4_MIN_IHL);
	ipPkt->tos = 0;
	ipPkt->len = 0;//htons(IPv4_HDR_LEN);
	ipPkt->id = 0;
	ipPkt->flags_froff = 0;
	ipPkt->ttl = (uchar)htons(10);
	ipPkt->proto = (uchar)htons(IPv4_PROTO_ICMP);
	memcpy(ipPkt->src, myipaddr, IP_ADDR_LEN);
	memcpy(ipPkt->dst, ip, IP_ADDR_LEN);
	
	ipPkt->chksum = htons( checksum(ipPkt, sizeof(ipPkt)));
	
	memcpy(&ipPkt->opts[0], icmppkt, sizeof(icmppkt));
	memcpy(&eg->data[0], ipPkt, sizeof(ipPkt));
	
	//printPacketICMP(packetPING);
	printf("About to write packet to ETH0\n");
	write(ETH0, packetPING,ETHER_SIZE+ETHER_MINPAYLOAD);

	//free(packetPING);
	return OK;
}

//prints the ethernet frame with the icmp packet and ip packe within
void printPacketICMP(uchar *pkt) {
kprintf("--------ETHERNET FRAME----------\n");
kprintf("sizeof(pkt):%d\neg->src: %02x:%02x:%02x:%02x:%02x:%02x\n", sizeof(pkt), ((struct ethergram *)pkt)->src[0], ((struct ethergram *)pkt)->src[1], ((struct ethergram *)pkt)->src[2], ((struct ethergram *)pkt)->src[3],((struct ethergram *)pkt)->src[4],((struct ethergram *)pkt)->src[5]);
kprintf("eg->dst: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct ethergram *)pkt)->dst[0],((struct ethergram *)pkt)->dst[1],((struct ethergram *)pkt)->dst[2],((struct ethergram *)pkt)->dst[3],((struct ethergram *)pkt)->dst[4],((struct ethergram *)pkt)->dst[5]);
kprintf("eg->type:%u\n",((struct ethergram *)pkt)->type);
kprintf("eg->data\n");

kprintf("--------ICMP PACKET----------\n");
kprintf("icmpPkt->type: %u\n",((struct icmpPkt *)((struct ethergram *)pkt)->data)->type);
kprintf("icmpPkt->code: %u\n",((struct icmpPkt *)((struct ethergram *)pkt)->data)->code);
kprintf("icmpPkt->checksum: %02x\n",((struct icmpPkt *)((struct ethergram *)pkt)->data)->checksum);
kprintf("icmpPkt->data\n");

kprintf("--------IP PACKET-----------\n");
kprintf("ipPkt->ver_ihl: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->ver_ihl);
kprintf("ipPkt->tos: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->tos);
kprintf("ipPkt->len: %02x\n",  ((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->len);
kprintf("ipPkt->id: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->id);
kprintf("ipPkt->flags_froff: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->flags_froff);
kprintf("ipPkt->ttl: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->ttl);
kprintf("ipPkt->proto: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->proto);
kprintf("ipPkt->chksum: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->chksum);
kprintf("ipPkt->src: ");
printAddr(((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->src,IPv4_ADDR_LEN);
kprintf("ipPkt->dst: ");
printAddr(((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)pkt)->data)->data)->dst,IPv4_ADDR_LEN);
}
void printAddr(uchar * addr,ushort len){
	ushort i;
	for(i=0;i<len;i++){
		//last uchar to print in addr
		if(i==len-1){
			printf("%02x\n",addr[i]);
		}else{
			printf("%02x:",addr[i]);
		}
	}
}
