
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

	while (1)
	{
		//printf("icmpDaemon started loop\n");
		bzero(packetICMP, PKTSZ);
		read(ETH0, packetICMP, PKTSZ);
//		printPacket(packet);
		eg = (struct ethergram *)packetICMP;
		if (memcmp(eg->dst, myMac, sizeof(myMac)) != 0) /* Not our packet, drop */
			continue;
		//printf("icmpDaemon We received a packet for our mac\n");
		icmpPkt = (struct icmpPkt *)&eg->data[0];
		//printf("eg->type=%d arpPkt->hwtype=%d arpPkt->prtype=%08x\n",ntohs(eg->type), ntohs(arpPkt->hwtype), ntohs(arpPkt->prtype));
		if (ntohs(eg->type)!=ETYPE_IPv4 || ntohs(icmpPkt->code) != 0
			 || htons(icmpPkt->checksum) != checksum(icmpPkt, sizeof(struct icmpPkt*)))
			continue;
		//printf("icmpDaemon Got past first check\n");
		ipPkt = (struct ipgram *)&icmpPkt->data[0];
		if (htons(ipPkt->ver_ihl) != (IPv4_VERSION + IPv4_MIN_IHL) || ipPkt->tos != 0
			|| htons(ipPkt->len) != IPv4_HDR_LEN || ipPkt->id != 0
			|| ipPkt->flags_froff != 0 || htons(ipPkt->ttl) < 0 
			|| htons(ipPkt->proto) != IPv4_PROTO_ICMP 
			|| memcmp(ipPkt->dst, myipaddr, IP_ADDR_LEN) 
			|| htons(ipPkt->chksum) != checksum(ipPkt, sizeof(ipPkt)))
			continue;

		//printf("icmpDaemon We received a packet for our mac and ip\n");

		if (ntohs(icmpPkt->type) == ECHO_REQUEST) /* ECHO Request */
		{
			//printf("icmpDaemon ECHO Request recevied\n");
			memcpy(eg->dst, eg->src, sizeof(eg->src));
			memcpy(eg->src, myMac, sizeof(myMac));
			eg->type = htons(ETYPE_IPv4);

			icmpPkt->type = htons(ECHO_REPLY);
			icmpPkt->code = 0;
			icmpPkt->checksum = htons(checksum(icmpPkt, sizeof(struct icmpPkt*)));

			// Almost entire ipPkt is already set up. Just changing src, dst, and checksum
			memcpy(&ipPkt->dst, &ipPkt->src, IPv4_ADDR_LEN);
			memcpy(&ipPkt->src, myipaddr, IPv4_ADDR_LEN);
			ipPkt->chksum = htons(checksum(ipPkt, sizeof(ipPkt)));

			write(ETH0, packetICMP,ETHER_SIZE+ETHER_MINPAYLOAD);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		}else if (ntohs(icmpPkt->type) == ECHO_REPLY) /* ECHO Reply */
		{
			ushort *data = malloc(sizeof(uchar) * 3);
			data[0] = sizeof(packetICMP);
			data[1] = htons(ipPkt->ttl);
			data[2] = (uchar)CLKTICKS_PER_SEC*10;
			send(recvtime(1), (int)(long)data);
			recvtime(1);
			free(data);
		}else /* Some other op type, drop */
			continue;
	}
	//free(packet);
}
