/**
 * @file arp.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>
#include <arp.h>

void sendMacAddress(int, uchar *);
void recvMacAddress(uchar *);
void printPacket(uchar *);

struct arp {
	uchar *arpIp;
	uchar *arpMac;
};

struct arpTable {
	struct arp arr[MAX_ARP_TABLE]; /* Using a array instead of linked-list because of infrequent deletions and set size */
	int size;
};

struct arpTable arpTab;
semaphore arpSem;
uchar packet[PKTSZ];

/**
 * Shell command arp that can be used to display the current table, to
 * request a new mapping, or to eliminate an existing mapping.
 */
command arp(int type, uchar *ip)
{
	int i, j, size, ret = OK;

	wait(arpSem);

	size = arpTab.size;
	
	if (type == ARP_DISPLAY) { //Display current arp table
		printf("IP ADDRESS\tMAC ADDRESS\n");
		for (i = 0; i < arpTab.size; i++) {
			printf("%u.%u.%u.%u\t%02x:%02x:%02x:%02x:%02x:%02x\n", arpTab.arr[i].arpIp[0], arpTab.arr[i].arpIp[1], arpTab.arr[i].arpIp[2], arpTab.arr[i].arpIp[3], arpTab.arr[i].arpMac[0], arpTab.arr[i].arpMac[1], arpTab.arr[i].arpMac[2], arpTab.arr[i].arpMac[3], arpTab.arr[i].arpMac[4], arpTab.arr[i].arpMac[5]);
		}
	}else if (type == ARP_ADD) { //Request new arp mapping
		// Check to see if the ip address is already mapped
		for (i = 0; i < arpTab.size; i++) {
			if (memcmp(arpTab.arr[i].arpIp, ip, sizeof(ip)) == 0)
			{
				printf("Resulting MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arpTab.arr[i].arpMac[0], arpTab.arr[i].arpMac[1], arpTab.arr[i].arpMac[2], arpTab.arr[i].arpMac[3], arpTab.arr[i].arpMac[4], arpTab.arr[i].arpMac[5]);
				signal(arpSem);
				return OK;
			}
		}

		// Attempt to ARP for it
		arpTab.arr[size].arpMac = malloc(sizeof(uchar)*6);
		arpTab.arr[size].arpIp = malloc(sizeof(uchar)*4);

		if (SYSERR == arpResolve(ip, arpTab.arr[size].arpMac)) {
			printf("ERROR: Could not resolve the IP address\n");
			free(arpTab.arr[size].arpMac);
			free(arpTab.arr[size].arpIp);
			ret = SYSERR;
		}
		else{
			printf("Resulting MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arpTab.arr[size].arpMac[0], arpTab.arr[size].arpMac[1], arpTab.arr[size].arpMac[2], arpTab.arr[size].arpMac[3], arpTab.arr[size].arpMac[4], arpTab.arr[size].arpMac[5]);
			arpTab.arr[size].arpIp = ip;
			arpTab.size++;
		}
	}else if (type == ARP_DELETE) { //Eliminate an existing mapping
		for (i = 0; i < arpTab.size; i++) {
			if (memcmp(arpTab.arr[i].arpIp, ip, sizeof(ip)) == 0) {
				for (j = i; j < arpTab.size-1; j++)
					arpTab.arr[j] = arpTab.arr[j+1];
				free(arpTab.arr[size].arpMac);
				free(arpTab.arr[size].arpIp);
				arpTab.size--;
				break;
			}
		}				
	}else { //Incorrect input
		printf("ERROR: Invalid input\n");
		ret = SYSERR;
	}
	
	signal(arpSem);
	return ret; // COMEBACK		
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
			arpPkt->prtype = ETYPE_IPv4;
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

	//getpid();
	/* Attempting to receive an ARP response */
	//printf("arpResolve sizeof(packet):%d and eg:%d\n", sizeof(packet), sizeof(eg));
	ready(create((void *)arpResolveHelper, INITSTK, 2, "ARP_HELPER", 2, packet,currpid), 1);
	recvMacAddress(mac); /* Wait until helper function has made 3 attempts to arpResolve */
	//printf("ArpResolve: mac we are getting:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	//if (msg == TIMEOUT)
	//	return SYSERR;
	//else
	//{
		//mac = (uchar *) msg;
		return OK;
	//}
}

void arpResolveHelper(uchar *packet, int prevId)
{
	int msg = TIMEOUT;
	int i;
	uchar *mac;
	uchar *ppkt = packet;	

	mac = malloc(ETH_ADDR_LEN);

	/* Sending ARP Packet */
	for (i = 0; i < 3 && msg == TIMEOUT; i++)
	{
		printf("sizeof(packet):%d\neg->src: %02x:%02x:%02x:%02x:%02x:%02x\n", sizeof(packet), ((struct ethergram *)packet)->src[0], ((struct ethergram *)packet)->src[1], ((struct ethergram *)packet)->src[2], ((struct ethergram *)packet)->src[3],((struct ethergram *)packet)->src[4],((struct ethergram *)packet)->src[5]);
printf("eg->dst: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct ethergram *)packet)->dst[0],((struct ethergram *)packet)->dst[1],((struct ethergram *)packet)->dst[2],((struct ethergram *)packet)->dst[3],((struct ethergram *)packet)->dst[4],((struct ethergram *)packet)->dst[5]);
printf("eg->data\n");
printf("arpPkt->hwtype: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->hwtype);
printf("arpPkt->prtype: %08x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->prtype);
printf("arpPkt->hwalen: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->hwalen);
printf("arpPkt->pralen: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->pralen);
printf("arpPkt->op: %u\n",htons(((struct arpPkt *)((struct ethergram *)packet)->data)->op));
printf("arpPkt->sha: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+3],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+4]);
printf("arpPkt->spa: %u.%u.%u.%u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+3]);
printf("arpPkt->dha: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+3],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+4]);
printf("arpPkt->dpa: %u.%u.%u.%u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+3]);
		write(ETH0, packet,ETHER_SIZE+ETHER_MINPAYLOAD);//(uchar*)((struct arpPkt *)((struct ethergram *)packet)->data)-packet);
		//getpid(); //check syscall
		send(arpDaemonId, currpid);
		//msg = recvtime(CLKTICKS_PER_SEC);
		recvMacAddress(mac);
	}

	printf("ArpResolveHelper:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//send(prevId, msg);
	sendMacAddress(prevId, mac);
}

void sendMacAddress(int pid, uchar* mac){
	int macFrame; 
	int i;
	printf("ArpDaemon(sendMacAddres): mac we are sending:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	for(i=0;i<ETH_ADDR_LEN;i++){
		macFrame = 0;
		macFrame+=mac[i];
		macFrame<<=(2*8);
		macFrame+=i;
		send(pid,macFrame);
		kprintf("sent macFrame:%04x\r\n",macFrame);
	}
}
void recvMacAddress(uchar* mac){
	int msgbuff[ETH_ADDR_LEN];
	int index;
	int i;
	bzero(msgbuff,ETH_ADDR_LEN);
	for(i=0;i<ETH_ADDR_LEN;i++){
                msgbuff[i]=receive();
		kprintf("LOOP: RECV macFrame:%02x, pos:%u\r\n",msgbuff[i]>>(2*8),msgbuff[i]&0x00FF);
        }	

	for(i=0;i<ETH_ADDR_LEN;i++){
		index = msgbuff[i] & 0x00FF;
		mac[index]= (uchar)(msgbuff[i]>>(2*8));		
        }	
}

void printPacket(uchar *packet) {
printf("sizeof(packet):%d\neg->src: %02x:%02x:%02x:%02x:%02x:%02x\n", sizeof(packet), ((struct ethergram *)packet)->src[0], ((struct ethergram *)packet)->src[1], ((struct ethergram *)packet)->src[2], ((struct ethergram *)packet)->src[3],((struct ethergram *)packet)->src[4],((struct ethergram *)packet)->src[5]);
printf("eg->dst: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct ethergram *)packet)->dst[0],((struct ethergram *)packet)->dst[1],((struct ethergram *)packet)->dst[2],((struct ethergram *)packet)->dst[3],((struct ethergram *)packet)->dst[4],((struct ethergram *)packet)->dst[5]);

printf("eg->type:%u\n",((struct ethergram *)packet)->type);
printf("eg->data\n");
printf("arpPkt->hwtype: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->hwtype);
printf("arpPkt->prtype: %08x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->prtype);
printf("arpPkt->hwalen: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->hwalen);
printf("arpPkt->pralen: %u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->pralen);
printf("arpPkt->op: %u\n",htons(((struct arpPkt *)((struct ethergram *)packet)->data)->op));
printf("arpPkt->sha: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+3],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SHA+4]);
printf("arpPkt->spa: %u.%u.%u.%u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_SPA+3]);
printf("arpPkt->dha: %02x:%02x:%02x:%02x:%02x:%02x\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+3],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DHA+4]);
printf("arpPkt->dpa: %u.%u.%u.%u\n",((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+1],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+2],((struct arpPkt *)((struct ethergram *)packet)->data)->addrs[ARP_ADDR_DPA+3]);
}
