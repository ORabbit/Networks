/**
 * @file ping.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
//uchar packetPING[PKTSZ];
//uchar *packetPING;
//uchar *macOther;
void printAddr(uchar *,ushort);

/**
 * pings another ip address
 */
command ping(uchar *ip, ushort seq)
{
	
	/*packetPING = malloc(PKTSZ);
	uchar *mac = malloc(ETH_ADDR_LEN);

	//kprintf("ip addr dst:");
	//printAddr(ip,IP_ADDR_LEN);
	//kprintf("ip addr src:");
	//printAddr(myipaddr,IP_ADDR_LEN);

	if (macOther != NULL)
		memcpy(mac, macOther, ETH_ADDR_LEN);
	else if (SYSERR == arpResolve(ip, mac)) {
		printf("ERROR in ping->arpResolve\n");
		return SYSERR;
	}else {
		macOther = malloc(ETH_ADDR_LEN);
		memcpy(macOther, mac, ETH_ADDR_LEN);
	}

	bzero(packetPING, PKTSZ);
	//construct ethergram
	struct ethergram * eg = (struct ethergram*) packetPING;
	memcpy(eg->dst, mac, ETH_ADDR_LEN); 
	control(ETH0, ETH_CTRL_GET_MAC, (long)eg->src, 0);
	eg->type = htons(ETYPE_IPv4);
*/
	//construct icmpPkt
	struct icmpPkt *icmppkt = malloc(ICMP_HDR_LEN+56);
	//struct icmpPkt *icmppkt = malloc(ICMP_HDR_LEN+56);
	bzero(icmppkt, ICMP_HDR_LEN+56);
	if(icmppkt==NULL){
                kprintf("ERROR: couldn't allocate memory for icmp packet\r\n");
                return SYSERR;
        }
	icmppkt->type = ECHO_REQUEST;
	icmppkt->code = 0;
	icmppkt->id = 1;
	icmppkt->seq = htons(seq);
	icmppkt->checksum = 0;
	icmppkt->checksum = checksum((uchar*)icmppkt, ICMP_HDR_LEN+56);

	ipWrite((uchar*)icmppkt, (ushort)(ICMP_HDR_LEN+56), IPv4_PROTO_ICMP, ip);
	free(icmppkt);
	return OK;
/*
	//constructing ip packet
	struct ipgram *ipPkt = malloc(IPv4_HDR_LEN+ICMP_HDR_LEN+56);
	bzero(ipPkt, IPv4_HDR_LEN+ICMP_HDR_LEN+56);
	if(ipPkt==NULL){
		kprintf("ERROR: couldn't allocate memory for ip packet\r\n");
		return SYSERR;
	}else{
		//kprintf("ip packet address:%08x: \r\n",ipPkt);
	}
	ipPkt->ver_ihl = (uchar)(IPv4_VERSION << 4);
   ipPkt->ver_ihl += (IPv4_HDR_LEN) / 4;
	ipPkt->tos = 0;
	ipPkt->len = htons(IPv4_HDR_LEN+ICMP_HDR_LEN+56);//IPv4_HDR_LEN+ICMP_HDR_LEN);
	ipPkt->id = 1;
	ipPkt->flags_froff = 64;
	ipPkt->ttl = (uchar)IPv4_TTL;
	ipPkt->proto = IPv4_PROTO_ICMP;
	bzero(ipPkt->src,IP_ADDR_LEN);
	bzero(ipPkt->dst,IP_ADDR_LEN);
	memcpy(ipPkt->src, myipaddr, IP_ADDR_LEN);
	memcpy(ipPkt->dst, ip, IP_ADDR_LEN);
	
	ipPkt->chksum = 0;
	ipPkt->chksum = checksum((uchar*)ipPkt, IPv4_HDR_LEN);

	bzero(eg->data,IPv4_HDR_LEN+ICMP_HDR_LEN);
	bzero(ipPkt->opts,ICMP_HDR_LEN);
	memcpy(&ipPkt->opts[0], icmppkt, ICMP_HDR_LEN+56);
	memcpy(&eg->data[0], ipPkt, ntohs(ipPkt->len));
	//printPacketICMP(packetPING);
	//kprintf("About to write packet to ETH0\r\n");
	ushort size = ETHER_MINPAYLOAD < ntohs(ipPkt->len) ? ntohs(ipPkt->len) : ETHER_MINPAYLOAD;
	write(ETH0, packetPING,ETHER_SIZE+size);

	//free(packetPING);
	return OK;*/
}

//prints the ethernet frame with the icmp packet and ip packe within
void printPacketICMP(uchar *pkt) {
kprintf("--------ETHERNET FRAME----------\r\n");
kprintf("sizeof(pkt):%d\r\neg->src: %02x:%02x:%02x:%02x:%02x:%02x\r\n", sizeof(pkt), ((struct ethergram *)pkt)->src[0], ((struct ethergram *)pkt)->src[1], ((struct ethergram *)pkt)->src[2], ((struct ethergram *)pkt)->src[3],((struct ethergram *)pkt)->src[4],((struct ethergram *)pkt)->src[5]);
kprintf("eg->dst: %02x:%02x:%02x:%02x:%02x:%02x\r\n",((struct ethergram *)pkt)->dst[0],((struct ethergram *)pkt)->dst[1],((struct ethergram *)pkt)->dst[2],((struct ethergram *)pkt)->dst[3],((struct ethergram *)pkt)->dst[4],((struct ethergram *)pkt)->dst[5]);
kprintf("eg->type:%u\r\n",ntohs(((struct ethergram *)pkt)->type));
kprintf("eg->data\r\n");

kprintf("--------IP PACKET-----------: (address:%08x)\r\n",(struct ipgram*)((struct ethergram *)pkt)->data);
kprintf("ipPkt->ver_ihl: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->ver_ihl);
kprintf("ipPkt->tos: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->tos);
kprintf("ipPkt->len: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->len);
kprintf("ipPkt->id: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->id);
kprintf("ipPkt->flags_froff: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->flags_froff);
kprintf("ipPkt->ttl: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->ttl);
kprintf("ipPkt->proto: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->proto);
kprintf("ipPkt->chksum: %u\r\n",((struct ipgram*)((struct ethergram *)pkt)->data)->chksum);
kprintf("ipPkt->src: ");
printAddr(((struct ipgram*)((struct ethergram *)pkt)->data)->src,IPv4_ADDR_LEN);
kprintf("ipPkt->dst: ");
printAddr(((struct ipgram*)((struct ethergram *)pkt)->data)->dst,IPv4_ADDR_LEN);

kprintf("--------ICMP PACKET----------\r\n");
kprintf("icmpPkt->type: %u\r\n",((struct icmpPkt *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->type);
kprintf("icmpPkt->code: %u\r\n",((struct icmpPkt *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->code);
kprintf("icmpPkt->checksum: %u\r\n",((struct icmpPkt *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->checksum);
kprintf("icmpPkt->data\r\n");
}
void printAddr(uchar * addr,ushort len){
	ushort i;
	for(i=0;i<len;i++){
		//last uchar to print in addr
		if(i==len-1){
			kprintf("%02x\r\n",addr[i]);
		}else{
			kprintf("%02x:",addr[i]);
		}
	}
}


