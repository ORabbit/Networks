
#include <xinu.h>
#include <icmp.h>
#include <arp.h>

uchar packetICMP[PKTSZ];

void icmpDaemon(void)
{
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packetICMP;
	ushort ip_ihl;
	//kprintf("icmpDaemon started loop\r\n");
	while (1)
	{
		bzero(packetICMP, PKTSZ);
		read(ETH0, packetICMP, PKTSZ);

		if (memcmp(eg->dst, myMac, sizeof(myMac)) != 0) /* Not our packet, drop */
			continue;

		ipPkt = (struct ipgram *)&eg->data[0];

		ip_ihl= ipPkt->ver_ihl & IPv4_IHL;
		if (ntohs(eg->type)!=ETYPE_IPv4 || ipPkt->proto != IPv4_PROTO_ICMP || ((ipPkt->ver_ihl & IPv4_VER) >> 4) != IPv4_VERSION || (ip_ihl < IPv4_MIN_IHL) || (ip_ihl > IPv4_MAX_IHL) || ipPkt->len < (IPv4_MIN_IHL * 4)
			 || checksum(ipPkt, (ip_ihl * 4)) != 0){
//			kprintf("not the packet we were looking for(Test[1]: %d,Test[2]: %d,Test[3]:%d,Test[4]:%d,Test[5]:%d,Test[6]:%d)\r\n.",ntohs(eg->type)==ETYPE_IPv4,ipPkt->proto == IPv4_PROTO_ICMP,((ipPkt->ver_ihl & IPv4_VER) >> 4) == IPv4_VERSION,!(ip_ihl < IPv4_MIN_IHL) && !(ip_ihl > IPv4_MAX_IHL), ipPkt->len >= (IPv4_MIN_IHL * 4), checksum(ipPkt, (ip_ihl * 4)) == 0);
//			printPacketICMP(packetICMP);
			continue;
		}
	//	kprintf("icmpDaemon Got past first check:(It is an uncorrupted IP packet)\r\n");
		icmpPkt = (struct icmpPkt *)&ipPkt->opts[0];
		if(checksum((uchar*)icmpPkt,ICMP_HDR_LEN)!=0){ 
			kprintf("corrupted icmp packet within\r\n ");
			continue;
		}
	//	kprintf("icmpDaemon has determined that the icmp packet withing is uncorrupted\r\n");

		if (icmpPkt->type == ECHO_REQUEST) /* ECHO Request */
		{
//			kprintf("icmpDaemon ECHO Request recevied\r\n");
			memcpy(eg->dst, eg->src, sizeof(eg->src));
			memcpy(eg->src, myMac, sizeof(myMac));
			eg->type = htons(ETYPE_IPv4);

			icmpPkt->type = ECHO_REPLY;
			icmpPkt->code = 0;
			icmpPkt->checksum = 0;
			icmpPkt->checksum = checksum(icmpPkt, ICMP_HDR_LEN);

			// Almost entire ipPkt is already set up. Just changing src, dst, and checksum
			memcpy(&ipPkt->dst, &ipPkt->src, IPv4_ADDR_LEN);
			memcpy(&ipPkt->src, myipaddr, IPv4_ADDR_LEN);
			ipPkt->chksum = 0;
			ipPkt->chksum = checksum(ipPkt, IPv4_HDR_LEN);

			write(ETH0, packetICMP,ETHER_SIZE+ETHER_MINPAYLOAD);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		}else if (icmpPkt->type == ECHO_REPLY) /* ECHO Reply */
		{
			
//			kprintf("icmpDaemon ECHO Reply recevied\r\n");
			ushort *data = malloc(sizeof(uchar) * 3);
			data[0] = sizeof(packetICMP);
			data[1] = htons(ipPkt->ttl);
			data[2] = (uchar)CLKTICKS_PER_SEC*10;
			send(recvtime(1*CLKTICKS_PER_SEC), (int)(long)data);
			//recvtime(1*CLKTICKS_PER_SEC);
			free(data);
		}else /* Some other op type, drop */
			continue;
	}
	//free(packet);
}
