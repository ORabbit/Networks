/**
 * @file arp.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
uchar packet[PKTSZ];
void printPacket(uchar *);
void printAddr(uchar *,ushort){
/**
 * Shell command arp that can be used to display the current table, to
 * request a new mapping, or to eliminate an existing mapping.
 */
command ping( uchar *ip)

{	
	uchar *mac = malloc(ETH_ADDR_LEN);
	 arpResolve(ip, mac)
	//construct ip packet
	bzero(packet, PKTSZ);
	//ethergram
	struct ethergram * eg= (struct ethergram*) packet;
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
	ipPkt->ver_ihl = htons(IPv4_VERSION + IPv4_MIN_IHL);
	ipPkt->tos = 0;
	ipPkt->len = htons(IPv4_HDR_LEN);
	ipPkt->id = 0;
	ipPkt->flags_froff = 0;
	ipPkt->ttl = htons(10);
	ipPkt->proto = htons(IPv4_PROTO_ICMP);
	memcpy(ipPkt->src, myipaddr, IP_ADDR_LEN);
	memcpy(ipPkt->dst, ip, IP_ADDR_LEN);
	
	ipPkt->chksum = htons( checksum(ipPkt, sizeof(ipPkt)));
	
	memcpy(&icmppkt->data[0], ipPkt,sizeof(ipPkt));
	memcpy(&eg->data[0], icmppkt,sizeof(icmppkt));
	
	write(ETH0, packet,ETHER_SIZE+ETHER_MINPAYLOAD);
}

/**
 * Replies to ARP requests destined for our router, and handles replies
 * from our router
 */

void arpDaemon(void)
{
	//arpTab = malloc(sizeof(struct arpTable));
	//arpTab.arr = malloc(sizeof(struct arp)*MAX_ARP_TABLE);
	arpTab.size = 0;
	arpSem = semcreate(1);
	bzero(packet,PKTSZ);	
	int pid;
	struct arpPkt *arpPkt;
	uchar *mac = malloc(ETH_ADDR_LEN);
	uchar *brdcast = malloc(ETH_ADDR_LEN);

	struct ethergram *eg = (struct ethergram*) packet;
	//bzero(packet, PKTSZ);
	
	control(ETH0, ETH_CTRL_GET_MAC, (long)mac, 0);

	brdcast[0] = 0xFF;
	brdcast[1] = 0xFF;
	brdcast[2] = 0xFF;
	brdcast[3] = 0xFF;
	brdcast[4] = 0xFF;
	brdcast[5] = 0xFF;

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
/**
 * Takes an IP address as the first argument, and returns the resolved MAC
 * via the second argument. This function should return OK if ARP resolution
 * is successful, and SYSERR otherwise. This is the function that higher
 * layers of our stack will depend upon. If a previously unknown IP address
 * is requested, this function should block while a helper process makes up
 * to three attempts to ARP resolve, at 1 second intervals.
 */
devcall arpResolve(uchar *ipaddr, uchar *mac)
{
	int i;
	int msg;
	uchar packet[PKTSZ];
	struct ethergram *eg = (struct ethergram*) packet;
	
	//if not already mapped find it
	struct arpPkt *arpPkt = malloc(sizeof(struct arpPkt)+20);
	bzero(eg->dst,ETH_ADDR_LEN);
	bzero(eg->src,ETH_ADDR_LEN);
	bzero(eg->data,sizeof(struct arpPkt)+20+ETHER_MINPAYLOAD);
	/* Ethernet Destination MAC address set to FF:FF:FF:FF:FF for broadcast */
	//ethPkt->dst = malloc(ETH_ADDR_LEN);
	//ethPkt->src = malloc(IPv4_ADDR_LEN);
	eg->dst[0] = 0xFF;
	eg->dst[1] = 0xFF;
	eg->dst[2] = 0xFF;
	eg->dst[3] = 0xFF;
	eg->dst[4] = 0xFF;
	eg->dst[5] = 0xFF;

	control(ETH0, ETH_CTRL_GET_MAC, (long)eg->src, 0);
	eg->type = htons(ETYPE_ARP);

	arpPkt->hwtype = htons(1); // Come back and check for constant or define
	arpPkt->prtype = htons(ETYPE_IPv4);
	arpPkt->hwalen = ETH_ADDR_LEN;
	arpPkt->pralen = IPv4_ADDR_LEN;
	arpPkt->op = htons(ARP_OP_RQST);
	
	control(ETH0, ETH_CTRL_GET_MAC, (long)&arpPkt->addrs[ARP_ADDR_SHA], 0); // SHA
	memcpy(&arpPkt->addrs[ARP_ADDR_SPA], myipaddr, IPv4_ADDR_LEN); // SPA
	memcpy(&arpPkt->addrs[ARP_ADDR_DPA], ipaddr, IPv4_ADDR_LEN); // DPA

	//put arpPckt in ethernet frame
	memcpy(&eg->data[0], arpPkt,sizeof(struct arpPkt)+20);

	/* Attempting to receive an ARP response */
	//printf("arpResolve sizeof(packet):%d and eg:%d\n", sizeof(packet), sizeof(eg));
	ready(create((void *)arpResolveHelper, INITSTK, 2, "ARP_HELPER", 2, packet,currpid), 1);
	if(recvMacAddressTime(mac,0)==SYSERR) return SYSERR; /* Wait until helper function has made 3 attempts to arpResolve */
	//printf("ArpResolve: mac we are getting:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return OK;//don't trust the mac you get back
}

void arpResolveHelper(uchar *packet, int prevId)
{
	unsigned int didTimeout=TIMEOUT;
	int i;
	uchar *mac;
	uchar *ppkt = packet;	
	mac = malloc(ETH_ADDR_LEN);
	int err = 0xFFFF;
	/* Sending ARP Packet */
	for (i = 0; i < 3 && didTimeout == TIMEOUT; i++)
	{
		write(ETH0, packet,ETHER_SIZE+ETHER_MINPAYLOAD);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		send(arpDaemonId, currpid);
		didTimeout=recvMacAddressTime(mac,1);
		 kprintf("(%d) Trying to resolve ip address.\r\n",i+1);
        }
	if(didTimeout==TIMEOUT){
		send(prevId, err);//tell the process the mac is wrong
		//kprintf("sent error");
	}else sendMacAddress(prevId, mac);
}

void sendMacAddress(int pid, uchar* mac){
	int macFrame; 
	int i;
	//printf("ArpDaemon(sendMacAddres): mac we are sending:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	for(i=0;i<ETH_ADDR_LEN;i++){
		macFrame = 0;
		macFrame+=mac[i];
		macFrame<<=(2*8);
		macFrame+=i;
		send(pid,macFrame);
	//	kprintf("sent macFrame:%04x\r\n",macFrame);
	}
}
int recvMacAddressTime(uchar* mac,unsigned int time){
        int msgbuff[ETH_ADDR_LEN];
        int index;
        int i;
	int err = 0xFFFF;
        bzero(msgbuff,ETH_ADDR_LEN);
        if(time==0){
                for(i=0;i<ETH_ADDR_LEN;i++){
                        msgbuff[i]=receive();
			if(err==msgbuff[i]){
				return SYSERR;
			}
 //                       kprintf("LOOP: RECV macFrame:%02x, pos:%u\r\n",msgbuff[i]>>(2*8),msgbuff[i]&0x00FF);
                }
        }else{
        for(i=0;i<ETH_ADDR_LEN;i++){
                        msgbuff[i]=recvtime(CLKTICKS_PER_SEC*time);
                        if(msgbuff[i]==TIMEOUT){
                                return TIMEOUT;
                        }
			if(msgbuff[i]==err){
				return SYSERR;
			}
 //                       kprintf("LOOP: RECV macFrame:%02x, pos:%u\r\n",msgbuff[i]>>(2*8),msgbuff[i]&0x00FF);
                }
}

        for(i=0;i<ETH_ADDR_LEN;i++){
                index = msgbuff[i] & 0x00FF;
                mac[index]= (uchar)(msgbuff[i]>>(2*8));
        }
        return OK;
}
//prints the ethernet frame with the icmp packet and ip packe within
void printPacket(uchar *packet) {
printf("--------ETHERNET FRAME----------\n");
printf("sizeof(packet):%d\neg->src: %02x:%02x:%02x:%02x:%02x:%02x\n", sizeof(packet), ((struct ethergram *)packet)->src[0], ((struct ethergram *)packet)->src[1], ((struct ethergram *)packet)->src[2], ((struct ethergram *)packet)->src[3],((struct ethergram *)packet)->src[4],((struct ethergram *)packet)->src[5]);
printf("eg->dst: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct ethergram *)packet)->dst[0],((struct ethergram *)packet)->dst[1],((struct ethergram *)packet)->dst[2],((struct ethergram *)packet)->dst[3],((struct ethergram *)packet)->dst[4],((struct ethergram *)packet)->dst[5]);
printf("eg->type:%u\n",((struct ethergram *)packet)->type);
printf("eg->data\n");

printf("--------ICMP PACKET----------\n");
printf("icmpPkt->type: %u\n",((struct icmpPkt *)((struct ethergram *)packet)->data)->type);
printf("icmpPkt->code: %u\n",((struct icmpPkt *)((struct ethergram *)packet)->data)->code);
printf("icmpPkt->checksum: %02x\n",((struct icmpPkt *)((struct ethergram *)packet)->data)->checksum);
printf("icmpPkt->data: %u\n",((struct icmpPkt *)((struct ethergram *)packet)->data)->pralen);

printf("--------IP PACKET-----------\n");
printf("ipPkt->ver_ihl: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->ver_ihl);
printf("ipPkt->tos: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->tos);
printf("ipPkt->len: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->len);
printf("ipPkt->id: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->id);
printf("ipPkt->flags_froff: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->flags_froff);
printf("ipPkt->ttl: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->ttl);
printf("ipPkt->proto: %02x\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->proto);
printf("ipPkt->chksum: %u\n",((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->chksum);
printf("ipPkt->src: ");
printAddr(((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->src,IPv4_ADDR_LEN);
printf("ipPkt->dst: ");
printAddr(((struct ipgram*) ((struct icmpPkt *)((struct ethergram *)packet)->data)->data)->dst,IPv4_ADDR_LEN);
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
