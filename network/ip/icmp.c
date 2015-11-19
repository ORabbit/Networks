
#include <xinu.h>
#include <icmp.h>
#include <arp.h>

//uchar packetICMP[PKTSZ];

syscall icmpDaemon(uchar packetICMP[], uchar wasFragmented)
{
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packetICMP;
	ushort ip_ihl;
	//kprintf("icmpDaemon started loop\r\n");
	//while (1)
	//{
		//if (receive() != 1)
			//continue;
		//bzero(packetICMP, PKTSZ);
		//read(ETH0, packetICMP, PKTSZ);

	//	printPacketICMP(packetICMP);

		ipPkt = (struct ipgram *)&eg->data[0];

		ip_ihl= ipPkt->ver_ihl & IPv4_IHL;
		//ushort ipChksum = ntohs(ipPkt->chksum);
		//ipPkt->chksum = 0;

		//not for us
		if(memcmp(ipPkt->dst,myipaddr,IPv4_ADDR_LEN)!=0) return SYSERR;
		if(!wasFragmented && (ntohs(ipPkt->len) < (IPv4_MIN_IHL * 4) 	|| (ip_ihl > IPv4_MAX_IHL))) return SYSERR;

		if (ntohs(eg->type)!=ETYPE_IPv4 || ipPkt->proto != IPv4_PROTO_ICMP  
			|| ((ipPkt->ver_ihl & IPv4_VER) >> 4) != IPv4_VERSION || (ip_ihl < IPv4_MIN_IHL) 
		
			|| checksum((uchar*)ipPkt, (ip_ihl * 4)) != 0){
			//kprintf("not the packet we were looking for(Test[1]: %d,Test[2]: %d,Test[3]:%d,Test[4]:%d,Test[5]:%d,Test[6]:%d)\r\n.",ntohs(eg->type)==ETYPE_IPv4,ipPkt->proto == IPv4_PROTO_ICMP,((ipPkt->ver_ihl & IPv4_VER) >> 4) == IPv4_VERSION,!(ip_ihl < IPv4_MIN_IHL) && !(ip_ihl > IPv4_MAX_IHL), ntohs(ipPkt->len) >= (IPv4_MIN_IHL * 4), checksum((uchar*)ipPkt, (ip_ihl * 4)) == 0);
//			printPacketICMP(packetICMP);
//			
			kprintf("corrupted ipPkt within\r\n");
			return SYSERR;//continue;
		}
		//kprintf("icmpDaemon Got past first check:(It is an uncorrupted IP packet)\r\n");
		
		icmpPkt = (struct icmpPkt *)&ipPkt->opts[0];
		//icmpPkt += 2;
		//icmpPkt = (struct icmpPkt *)ipPkt->opts;
		//icmpPkt += (ntohs(ipPkt->len) - IPv4_HDR_LEN - ICMP_HDR_LEN);
//		ushort icmpChksum = icmpPkt->checksum;
		//icmpPkt->checksum = 0;
		
		ushort lengthOfData = ntohs(ipPkt->len)-(ip_ihl*4);

		//kprintf("%u\r\n", lengthOfData);
		if(checksum((uchar*)icmpPkt, lengthOfData) != 0){ 
			kprintf("corrupted icmp packet within\r\n ");
			return SYSERR;//continue;
		}
	//	kprintf("icmpDaemon has determined that the icmp packet withing is uncorrupted\r\n");

		if (icmpPkt->type == ECHO_REQUEST) /* ECHO Request */
		{
//			kprintf("icmpDaemon ECHO Request recevied\r\n");
			memcpy(eg->dst, eg->src, ETH_ADDR_LEN);
			memcpy(eg->src, myMac, ETH_ADDR_LEN);
			eg->type = htons(ETYPE_IPv4);

			icmpPkt->type = ECHO_REPLY;
			icmpPkt->code = 0;
			//struct icmpEcho *icmpE = (struct icmpEcho *)&icmpPkt->data[0];
			//icmpE->arrivsec = clocktime;
			//icmpE->arrivtic = CLKTICKS_PER_SEC * clocktime;
			//icmpE->arrivcyc = CLKTICKS_PER_SEC * clocktime;

			icmpPkt->checksum = 0;
			icmpPkt->checksum = checksum((uchar*)icmpPkt, lengthOfData);//ICMP_HDR_LEN);

			// Almost entire ipPkt is already set up. Just changing src, dst, and checksum
			memcpy(&ipPkt->dst, &ipPkt->src, IPv4_ADDR_LEN);
			memcpy(&ipPkt->src, myipaddr, IPv4_ADDR_LEN);
			ipPkt->chksum = 0;
			ipPkt->chksum = checksum((uchar*)ipPkt, (ip_ihl * 4));

			//printf("SENDING BACK:\n");
			//printPacketICMP(packetICMP);
			//kprintf("ipoPkt->len:%u\r\n",htons(ipPkt->len));
			int size = ETHER_MINPAYLOAD < htons(ipPkt->len) ? htons(ipPkt->len) : ETHER_MINPAYLOAD;
			ipWrite(&ipPkt->opts[0],lengthOfData,IPv4_PROTO_ICMP,ipPkt->dst,eg->dst);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		}else if (icmpPkt->type == ECHO_REPLY) /* ECHO Reply */
		{
			
		//	kprintf("icmpDaemon ECHO Reply recevied\r\n");	
		//	kprintf("GOT A REPLY!\r\n");
			//ushort *data = malloc(sizeof(ushort) * 2);
			//data[0] = ntohs(ipPkt->len)-(ip_ihl*4);//ntohs(ipPkt->len);
			//data[1] = ipPkt->ttl;
		//	kprintf("(0)bytes: %d and ttl %d\r\n",data[0],data[1]);
			//kprintf("MADE IT HERE\r\n");
			//data[2] = (uchar)CLKTICKS_PER_SEC*10;
			int pid = recvtime(10*CLKTICKS_PER_SEC);
			if (pid != TIMEOUT) {
				//kprintf("NO TIMEOUT IN ICMPDAEMON\r\n");
			//	kprintf("(1)bytes: %d and ttl %d\r\n",data[0],data[1]);
				send(pid, (int)(ntohs(ipPkt->len) - (ip_ihl*4)));
				send(pid, (int)ipPkt->ttl);
				//recvtime(10*CLKTICKS_PER_SEC);
			}else {
				//kprintf("TIMEOUT IN ICMPDAEMON\r\n");
				return SYSERR;
			}
		}else /* Some other op type, drop */
			return SYSERR;//continue;
	//}
	//free(packet);
	return OK;
}
