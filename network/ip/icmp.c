
#include <xinu.h>
#include <icmp.h>
#include <arp.h>
uchar packet[PKTSZ];

void icmpDaemon(void)
{
	while (1)
	{
		//printf("arpDaemon started loop\n");
		//bzero(packet, PKTSZ);
		read(ETH0, packet, PKTSZ);
//		printPacket(packet);
		if (memcmp(eg->dst, mac, sizeof(mac)) != 0 && memcmp(eg->dst, brdcast, sizeof(brdcast)) != 0) /* Not our packet, drop */
			continue;
		//printf("arpDaemon We received a packet for our mac\n");
		arpPkt = (struct arpPkt *)&eg->data[0];
		//printf("eg->type=%d arpPkt->hwtype=%d arpPkt->prtype=%08x\n",ntohs(eg->type), ntohs(arpPkt->hwtype), ntohs(arpPkt->prtype));
		if (ntohs(eg->type)!=ETYPE_ARP||ntohs(arpPkt->hwtype) != 1 || ntohs(arpPkt->prtype) != ETYPE_IPv4
			 || arpPkt->hwalen != ETH_ADDR_LEN || arpPkt->pralen != IPv4_ADDR_LEN)
			continue;
		//printf("arpDaemon Got past first check\n");
		if (memcmp(myipaddr, &arpPkt->addrs[ARP_ADDR_DPA], sizeof(myipaddr)) != 0)
			continue;
		//printf("arpDaemon We received a packet for our mac and ip\n");

		if (arpPkt->op == ntohs(ARP_OP_RQST)) /* ARP Request */
		{
			//printf("arpDaemon ARP Request recveid\n");
			memcpy(eg->dst, eg->src, sizeof(eg->src));
			memcpy(eg->src, mac, sizeof(mac));
			eg->type = htons(ETYPE_ARP);
			arpPkt->prtype = htons(ETYPE_IPv4);
			arpPkt->op = htons(ARP_OP_REPLY);
			memcpy(&arpPkt->addrs[ARP_ADDR_DHA], &arpPkt->addrs[ARP_ADDR_SHA], ETH_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_DPA], &arpPkt->addrs[ARP_ADDR_SPA], IPv4_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_SHA], mac, ETH_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_SPA], myipaddr, IPv4_ADDR_LEN);
			write(ETH0, packet,ETHER_SIZE+ETHER_MINPAYLOAD);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		}else if (arpPkt->op == ntohs(ARP_OP_REPLY)) /* ARP Reply */
		{
			//printf("\narpDaemon Mac address recveived thee mac address %02x:%02x:%02x:%02x:%02x:%02x\n",arpPkt->addrs[ARP_ADDR_SHA],arpPkt->addrs[ARP_ADDR_SHA+1],arpPkt->addrs[ARP_ADDR_SHA+2],arpPkt->addrs[ARP_ADDR_SHA+3],arpPkt->addrs[ARP_ADDR_SHA+4],arpPkt->addrs[ARP_ADDR_SHA+5]);
			pid = recvtime(CLKTICKS_PER_SEC*10);
			//printf("arpDaemon recved succ, sending\n");
			if (pid == TIMEOUT)
				continue;
			sendMacAddress(pid, &arpPkt->addrs[ARP_ADDR_SHA]);
		}else /* Some other op type, drop */
			continue;
	}
}


