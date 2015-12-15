
#include <xinu.h>
#include <udp.h>
#include <icmp.h>
#include <arp.h>


syscall udpDaemon(uchar packetUDP[])
{
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct userDataGram *udpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	//printPacketUDP(packetUDP);

	eg = (struct ethergram *)packetUDP;
	ushort ip_ihl;
		ipPkt = (struct ipgram *)&eg->data[0];

		ip_ihl= ipPkt->ver_ihl & IPv4_IHL;
		//ushort ipChksum = ntohs(ipPkt->chksum);
		//ipPkt->chksum = 0;

		//not for us
		if(memcmp(ipPkt->dst,myipaddr,IPv4_ADDR_LEN)!=0) return SYSERR;
		//if(!wasFragmented && (ntohs(ipPkt->len) < (IPv4_MIN_IHL * 4) 	|| (ip_ihl > IPv4_MAX_IHL))) return SYSERR;

		if (ntohs(eg->type)!=ETYPE_IPv4 || ipPkt->proto != IPv4_PROTO_UDP  
			|| ((ipPkt->ver_ihl & IPv4_VER) >> 4) != IPv4_VERSION || (ip_ihl < IPv4_MIN_IHL) 
		
			|| checksum((uchar*)ipPkt, (ip_ihl * 4)) != 0){
		
			kprintf("corrupted ipPkt within\r\n");
			return SYSERR;//continue;
		}
		
		udpPkt = (struct userDataGram *)&ipPkt->opts[0];
	
		ushort lengthOfData = ntohs(ipPkt->len)-(ip_ihl*4);

		//kprintf("%u\r\n", lengthOfData);
		//was udp checksum optional?
		ushort chksumCheck=0;
//checksum
		struct checksumUDPHeader* chksumUDPHeader = malloc(12+udpPkt->total_len);
	memcpy(chksumUDPHeader->ip_src,ipPkt->src,IP_ADDR_LEN);
	memcpy(chksumUDPHeader->ip_dst,ipPkt->dst,IP_ADDR_LEN);
	chksumUDPHeader->proto=ipPkt->proto;
	chksumUDPHeader->total_len=udpPkt->total_len;
	chksumUDPHeader->src_port=udpPkt->src_port;
	chksumUDPHeader->dst_port=udpPkt->dst_port;
	chksumUDPHeader->total_len2=udpPkt->total_len;
	chksumUDPHeader->checksum=udpPkt->checksum;
	memcpy(&chksumUDPHeader->data[0],&udpPkt->data[0],udpPkt->total_len-UDP_HDR_LEN);
		printPacketUDP((uchar*)eg);
		/**if((udpPkt->checksum!=0&&udpPkt->checksum!=~chksumCheck)  &&  checksum((uchar*)chksumUDPHeader, udpPkt->total_len+12) != 0){
				kprintf("ERROR: udp packet checksum didn't pass check\r\n");
				return SYSERR;
		}**/

		//find what socket packet corresponds to
		///if it doesn't correspond to a socket drop the packet.
		kprintf("GOT UDP PACKET ENTIRELY\r\n");
		printPacketUDP(packetUDP);
		int i, hasSock = 0;
		//wait(semSockTab);
		kprintf("PAST WAIT\r\n");
		struct ethergram* eg_2 = malloc(ntohs(ipPkt->len)+ETH_HEADER_LEN);
	  for(i=0;i<udpGlobalTable->size;i++){
			//loop through open sockets
			kprintf("Socket is %u\r\n",udpGlobalTable->sockets[i]);
			if(udpGlobalTable->sockets[i]!=0){
			 kprintf("Socket %d\r\n",i);
			kprintf("Socket state: %u",udpGlobalTable->sockets[i]->state);
			kprintf("remote ip: \r\n");
			printAddr(udpGlobalTable->sockets[i]->remote_ip,IP_ADDR_LEN);
			kprintf("local ip: \r\n");
			printAddr(udpGlobalTable->sockets[i]->local_ip,IP_ADDR_LEN);
			kprintf("source port: %u\r\n", udpGlobalTable->sockets[i]->local_port);
			kprintf("destination port: %u\r\n",udpGlobalTable->sockets[i]->remote_port);
			kprintf("incoming packets: %u packets\r\n",udpGlobalTable->sockets[i]->packets->size);
			
		}
			if(udpGlobalTable->sockets[i]!=0		&&		udpGlobalTable->sockets[i]->state==UDP_SOCK_OPEN 	&&
				//does packet correspond to packet's source, end ports, and ip addresses.
			 htons(udpPkt->src_port)==udpGlobalTable->sockets[i]->local_port		&&		htons(udpPkt->dst_port)==udpGlobalTable->sockets[i]->remote_port	&&		

			 memcmp(ipPkt->src,udpGlobalTable->sockets[i]->remote_ip,IP_ADDR_LEN)==0 && memcmp(ipPkt->dst,udpGlobalTable->sockets[i]->local_ip,IP_ADDR_LEN)==0){
				//add udp packet to circular queue: if queue is full packet is dropped
				kprintf("ABOUT TO DO SHIT\r\n");
	  			memcpy(eg_2,eg,ntohs(ipPkt->len)+ETH_HEADER_LEN);
				cenqueue(udpGlobalTable->sockets[i]->packets,(uchar*)eg_2);
				hasSock = 1;
				break;
			}else
				kprintf("ERROR: didn't pass UDP checks\r\n");
		}
	kprintf("wtf is hasSock:%d\r\n",hasSock);
	if (!hasSock) {
		kprintf("in !hasSock\r\n");
		struct udpSocket *socket = malloc(sizeof(struct udpSocket));
		udpOpen(socket, ipPkt->src, htons(udpPkt->dst_port));
		kprintf("initialized first socket\r\n");
	   memcpy(eg_2,eg,ntohs(ipPkt->len)+ETH_HEADER_LEN);
		cenqueue(udpGlobalTable->sockets[udpGlobalTable->size-1]->packets, (uchar*)eg_2);
		kprintf("enqueued new socket\r\n");
	}
	//signal(semSockTab);
	kprintf("FINISHED\r\n");
	return OK;
}

void printPacketUDP(uchar *pkt) {
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

kprintf("--------UDP PACKET----------\r\n");
kprintf("udp->src_port: %u\r\n",((struct userDataGram *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->src_port);
kprintf("udp->dst_port: %u\r\n",((struct userDataGram *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->dst_port);
kprintf("udp->total_len: %u\r\n",((struct userDataGram *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->total_len);
kprintf("udp->checksum: %u\r\n",((struct userDataGram *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->checksum);
kprintf("udp->data: %s\r\n",(char*)&((struct userDataGram *)((struct ipgram*)((struct ethergram *)pkt)->data)->opts)->data[0]);
kprintf("icmpPkt->data\r\n");
}
