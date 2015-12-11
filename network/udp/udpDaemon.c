
#include <xinu.h>
#include <udp.h>
#include <arp.h>


syscall udpDaemon(uchar packetUDP[])
{
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct userDatagram *udpPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packetUDP;
	ushort ip_ihl;
		ipPkt = (struct ipgram *)&eg->data[0];

		ip_ihl= ipPkt->ver_ihl & IPv4_IHL;
		//ushort ipChksum = ntohs(ipPkt->chksum);
		//ipPkt->chksum = 0;

		//not for us
		if(memcmp(ipPkt->dst,myipaddr,IPv4_ADDR_LEN)!=0) return SYSERR;
		//if(!wasFragmented && (ntohs(ipPkt->len) < (IPv4_MIN_IHL * 4) 	|| (ip_ihl > IPv4_MAX_IHL))) return SYSERR;

		if (ntohs(eg->type)!=ETYPE_IPv4 || ipPkt->proto != IPv4_PROTO_ICMP  
			|| ((ipPkt->ver_ihl & IPv4_VER) >> 4) != IPv4_VERSION || (ip_ihl < IPv4_MIN_IHL) 
		
			|| checksum((uchar*)ipPkt, (ip_ihl * 4)) != 0){
		
			kprintf("corrupted ipPkt within\r\n");
			return SYSERR;//continue;
		}
		
		udpPkt = (struct userDatagram *)&ipPkt->opts[0];
	
		ushort lengthOfData = ntohs(ipPkt->len)-(ip_ihl*4);

		//kprintf("%u\r\n", lengthOfData);
		//was udp checksum optional?
		ushort chksumCheck=0;
		if((udpPkt->checksum!=0&&udpPkt->checksum!=~chksumCheck)/*checksum isn't optional*/&&checksum((uchar*)udpPkt, lengthOfData) != 0){
				//printf("ERROR: udp packet checksum didn't pass check");
				return SYSERR;
		}

		//find what socket packet corresponds to
		///if it doesn't correspond to a socket drop the packet.
		int i;
		for(i=0;i<MAX_SOCKETS;i++){
			//loop through open sockets
			if(udpGlobalTable->sockets[i]!=0		&&		udpGlobalTable->sockets[i]->state==UDP_SOCK_OPEN 	&&
				//does packet correspond to packet's source, end ports, and ip addresses.
			 udpPkt->src_port==udpGlobalTable->sockets[i]->src_port		&&		udpPkt->dst_port==udpGlobalTable->sockets[i]->dst_port	&&		

			 memcmp(ipPkt->src,udpGlobalTable->sockets[i]->local_ip,IP_ADDR_LEN) && memcmp(ipPkt->dst,udpGlobalTable->sockets[i]->remote_ip,IP_ADDR_LEN)){
				//add udp packet to circular queue: if queue is full packet is dropped
				enqueue(udpGlobalTable->sockets[i]->packets,udpPkt);
			}
		}	
	return OK;
}
