/**
 * @file ipWrite.c
 * @provides ipWrite
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
#define SUB_CONSTANT IPv4_HDR_LEN
/**
 * constructs an ip packet from the payload it recieves and sends that to the data link layer 
 */
command ipWrite(uchar *payload, ushort payload_len,ushort ipv4_type, uchar* dst_ip_addr,uchar* eg_dst)
{
	static ushort id = 0;
	uchar *mac = malloc(ETH_ADDR_LEN);
	if(eg_dst!=NULL){
		memcpy(mac, eg_dst, ETH_ADDR_LEN);
	}else if (SYSERR == arpResolve(dst_ip_addr, mac)) {
		printf("ERROR in ping->arpResolve\n");
		return SYSERR;
	}
	//how many fragmented datagrams do we have to produce.
	struct ipgram *ipPkt = (struct ipgram*)malloc(ETH_MTU);
	if(ipPkt==NULL){
		kprintf("ERROR: couldn't allocate memory for ip packet\r\n");
		return SYSERR;
	}
	int fr_grams = payload_len/(ETH_MTU-SUB_CONSTANT);
	//new ip datagram packet: increment unique packed it number.
	++id;
	int i;
	for(i=0;i<=fr_grams;i++){
		//constructing ip packet
		//kprintf("loop iteration:%d\r\n",i);
		bzero(ipPkt,ETH_MTU);
		ipPkt->ver_ihl = (uchar)(IPv4_VERSION << 4);
		ipPkt->ver_ihl += (IPv4_HDR_LEN) / 4;
		ipPkt->tos = 0;
		if(i==fr_grams){//last datagram
		//last fr_datagram
		//e.g payload is 1600 bytes last fr_datagram will hold 200 bytes
		//2nd case payload is exactly 1400 bytes : then len is ETH_MTU
			if(payload_len%(ETH_MTU-IPv4_HDR_LEN)!=0) 
				ipPkt->len = htons((payload_len%(ETH_MTU-SUB_CONSTANT))+IPv4_HDR_LEN);
			else
				ipPkt->len = htons(ETH_MTU);
			ipPkt->flags_froff = 0;

	//		kprintf("ip packet length:%d. ntohs_ver:%d\r\n",ntohs(ipPkt->len),ipPkt->len);
		}else{
			ipPkt->len = htons(ETH_MTU);//IPv4_HDR_LEN+ICMP_HDR_LEN);
	//		kprintf("ip packet length:%d. ntohs_ver:%d\r\n",ntohs(ipPkt->len),ipPkt->len);
			ipPkt->flags_froff = 1;
			ipPkt->flags_froff <<= 13;
		}	
		if(fr_grams==0)
	   	ipPkt->flags_froff =64;//(ushort)(((ETH_MTU-SUB_CONSTANT)*i)/8);
		else
    		ipPkt->flags_froff += (ETH_MTU-SUB_CONSTANT)-((ETH_MTU-SUB_CONSTANT)%8);//(ushort)(((ETH_MTU-SUB_CONSTANT)*i)/8);

		ipPkt->id = id;
		ipPkt->ttl = (uchar)IPv4_TTL;
		ipPkt->proto = ipv4_type;
		memcpy(ipPkt->src, myipaddr, IP_ADDR_LEN);
		memcpy(ipPkt->dst, dst_ip_addr, IP_ADDR_LEN);
		
		ipPkt->chksum = 0;
		ipPkt->chksum = checksum((uchar*)ipPkt, IPv4_HDR_LEN);
		bzero(ipPkt->opts,ETH_MTU-IPv4_HDR_LEN);
		memcpy(&ipPkt->opts[0], payload+(ETH_MTU-SUB_CONSTANT)*i, ETH_MTU-SUB_CONSTANT);

	//	kprintf("About to write packet to ETH0\r\n");
		
		ushort size = ETHER_MINPAYLOAD < ntohs(ipPkt->len) ? ntohs(ipPkt->len) : ETHER_MINPAYLOAD;
		netWrite((uchar*)ipPkt,ntohs(ipPkt->len),ETYPE_IPv4,mac);
		//cleanup

	}	
//		kprintf("freeing ipPkt\r\n");
		free(ipPkt);
//		kprintf("freeing mac\r\n");
		free(mac);
		return OK;
}



