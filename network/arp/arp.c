/**
 * @file arp.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>
#include <arp.h>

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


/**
 * Shell command arp that can be used to display the current table, to
 * request a new mapping, or to eliminate an existing mapping.
 */
command arp(int type, uchar *ip)
{
	int i, j;

	wait(arpSem);

	int size = arpTab.size;
	
	if (type == 1) { //Display current arp table
		printf("IP ADDRESS\tMAC ADDRESS\n");
		for (i = 0; i < arpTab.size; i++) {
			printf("%s\t%s\n", arpTab.arr[i].arpIp, arpTab.arr[i].arpMac);
		}
	}else if (type == 2) { //Request new arp mapping
		arpTab.arr[size].arpMac = malloc(sizeof(uchar)*6);
		arpTab.arr[size].arpIp = malloc(sizeof(uchar)*4);

		if (SYSERR == arpResolve(ip, arpTab.arr[size].arpMac)) {
			printf("ERROR: Could not resolve the IP address\n");
			free(arpTab.arr[size].arpMac);
			free(arpTab.arr[size].arpIp);
		}
		else {
			printf("Resulting MAC address: %s\n", arpTab.arr[size].arpMac);
			arpTab.arr[size].arpIp = ip;
			arpTab.size++;
		}
	}else if (type == 3) { //Eliminate an existing mapping
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
	}
	
	signal(arpSem);
	return SYSERR; // COMEBACK		
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
	
	int pid;
	struct arpPkt *arpPkt;
	uchar *mac = malloc(ETH_ADDR_LEN);
	uchar *packet[PKTSZ];
	struct ethergram *eg = (struct ethergram*) packet;
	bzero(packet, PKTSZ);
	
	control(ETH0, ETH_CTRL_GET_MAC, (long)mac, 0);

	while (1)
	{
		bzero(packet, PKTSZ);
		read(ETH0, packet, PKTSZ);
		if (memcmp(eg->dst, mac, sizeof(mac)) != 0) /* Not our packet, drop */
			continue;
		arpPkt = (struct arpPkt *)&eg->data[0];
		if (arpPkt->hwtype != 1 || arpPkt->prtype != ETYPE_ARP
			 || arpPkt->hwalen != ETH_ADDR_LEN || arpPkt->pralen != IPv4_ADDR_LEN)
			continue;
		if (memcmp(mac, &arpPkt->addrs[ARP_ADDR_DHA], sizeof(mac)) != 0
			|| memcmp(myipaddr, &arpPkt->addrs[ARP_ADDR_DPA], sizeof(myipaddr)) != 0)
			continue;

		if (arpPkt->op != htons(ARP_OP_RQST)) /* ARP Request */
		{
			memcpy(eg->dst, eg->src, sizeof(eg->src));
			memcpy(eg->src, mac, sizeof(mac));
			arpPkt->prtype = ETYPE_ARP;
			arpPkt->op = htons(ARP_OP_REPLY);
			memcpy(&arpPkt->addrs[ARP_ADDR_DHA], &arpPkt->addrs[ARP_ADDR_SHA], ETH_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_DPA], &arpPkt->addrs[ARP_ADDR_SPA], IPv4_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_SHA], mac, ETH_ADDR_LEN);
			memcpy(&arpPkt->addrs[ARP_ADDR_SPA], myipaddr, IPv4_ADDR_LEN);
			write(ETH0, packet, PKTSZ);
		}else if (arpPkt->op != htons(ARP_OP_REPLY)) /* ARP Reply */
		{
			pid = recvtime(CLKTICKS_PER_SEC);
			if (pid == TIMEOUT)
				continue;
			send(pid, (message)arpPkt->addrs[ARP_ADDR_SHA]);
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
	uchar * packet[PKTSZ];
	struct ethergram *eg = (struct ethergram*) packet;
	bzero(packet,PKTSZ);
	// Check to see if the ip address is already mapped
	for (i = 0; i < arpTab.size; i++) {
		if (memcmp(arpTab.arr[i].arpIp, ipaddr, sizeof(ipaddr)) == 0)
		{
			mac = arpTab.arr[i].arpMac;
			return OK;
		}
	}

	//if not already mapped find it
	struct arpPkt *arpPkt = malloc(sizeof(struct arpPkt));
	bzero(eg->dst,ETH_ADDR_LEN);
	bzero(eg->src,ETH_ADDR_LEN);
	bzero(&eg->data[0],sizeof(struct arpPkt));
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
	eg->type = ETYPE_ARP;

	arpPkt->hwtype = 1; // Come back and check for constant or define
	arpPkt->prtype = ETYPE_ARP;
	arpPkt->hwalen = ETH_ADDR_LEN;
	arpPkt->pralen = IPv4_ADDR_LEN;
	arpPkt->op = htons(ARP_OP_RQST);
	
	control(ETH0, ETH_CTRL_GET_MAC, (long)&arpPkt->addrs[ARP_ADDR_SHA], 0); // SHA
	memcpy(&arpPkt->addrs[ARP_ADDR_SPA], myipaddr, arpPkt->pralen); // SPA
	memcpy(&arpPkt->addrs[ARP_ADDR_DPA], ipaddr, arpPkt->pralen); // DPA

	//put arpPckt in ethernet frame
	memcpy(&eg->data[0], arpPkt,sizeof(struct arpPkt));

	//getpid();
	/* Attempting to receive an ARP response */
	ready(create((void *)arpResolveHelper, INITSTK, 2, "ARP_HELPER", 2, packet,currpid), 1);
	msg = receive(); /* Wait until helper function has made 3 attempts to arpResolve */

	if (msg == TIMEOUT)
		return SYSERR;
	else
	{
		mac = (uchar *) msg;
		
		return OK;
	}
}

void arpResolveHelper(uchar* packet, int prevId)
{
	int msg = TIMEOUT;
	int i;

	/* Sending ARP Packet */
	for (i = 0; i < 3 && msg == TIMEOUT; i++)
	{
		write(ETH0, packet,ETHER_SIZE + ETHER_MINPAYLOAD );
		//getpid(); //check syscall
		send(arpDaemonId, currpid);
		msg = recvtime(CLKTICKS_PER_SEC);
	}

	send(prevId, msg);
}

void sendMacAddress(int pid, uchar* mac){
	int macFrame; 
	int i;
	for(i=0;i<ETH_ADDR_LEN;i++){
		macFrame = 0;
		macFrame+=mac[i];
		macFrame<<=2;
		macFrame+=i;
		send(pid,macFrame);
	}
}
void recvMacAddress(uchar* mac){
	int msgbuff[ETH_ADDR_LEN];
	int index;
	int i;
	for(i=0;i<ETH_ADDR_LEN;i++){
                msgbuff[i]=receive();
        }	

	for(i=0;i<ETH_ADDR_LEN;i++){
		index = msgbuff[i] & 0x00FF;
		mac[index]= (uchar)(msgbuff[i]>>2);		
        }	
}
