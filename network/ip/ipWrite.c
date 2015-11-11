/**
 * @file ipWrite.c
 * @provides ipWrite
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>

/**
 * constructs an ip packet from the payload it recieves and sends that to the data link layer 
 */
command ipWrite(uchar *payload, ushort payload_len,uchar ipv4_type, uchar* dst_ip_addr)
{
	uchar *mac = malloc(ETH_ADDR_LEN);

	if (macOther != NULL)
		memcpy(mac, macOther, ETH_ADDR_LEN);
	else if (SYSERR == arpResolve(ip, mac)) {
		printf("ERROR in ping->arpResolve\n");
		return SYSERR;
	}else {
		macOther = malloc(ETH_ADDR_LEN);
		memcpy(macOther, mac, ETH_ADDR_LEN);
	}

	//constructing ip packet
	struct ipgram *ipPkt = malloc(IPv4_HDR_LEN+payload_len);
	if(ipPkt==NULL){
		kprintf("ERROR: couldn't allocate memory for ip packet\r\n");
		return SYSERR;
	}else{
		//kprintf("ip packet address:%08x: \r\n",ipPkt);
	}
	ipPkt->ver_ihl = (uchar)(IPv4_VERSION << 4);
	ipPkt->ver_ihl += (IPv4_HDR_LEN) / 4;
	ipPkt->tos = 0;
	ipPkt->len = htons(IPv4_HDR_LEN+payload_len);//IPv4_HDR_LEN+ICMP_HDR_LEN);
	ipPkt->id = 1;
	ipPkt->flags_froff = 64;
	ipPkt->ttl = (uchar)IPv4_TTL;
	ipPkt->proto = ipv4_type;
	memcpy(ipPkt->src, myipaddr, IP_ADDR_LEN);
	memcpy(ipPkt->dst, dst_ip_addr, IP_ADDR_LEN);
	
	ipPkt->chksum = 0;
	ipPkt->chksum = checksum((uchar*)ipPkt, IPv4_HDR_LEN);
	bzero(ipPkt->opts,payload_len);
	memcpy(&ipPkt->opts[0], payload, payload_len);

	kprintf("About to write packet to ETH0\r\n");
	ushort size = ETHER_MINPAYLOAD < ntohs(ipPkt->len) ? ntohs(ipPkt->len) : ETHER_MINPAYLOAD;
	netwrite(ipPkt,IPv4_HDR_LEN+payload_len,ETYPE_IPv4,mac);
	//cleanup
	free(mac);
	free(ipPkt);
	return OK;
}


