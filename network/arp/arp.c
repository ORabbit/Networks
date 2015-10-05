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
command arp(int type, int nargs, uchar *args)
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

		if (SYSERR == arpResolve(&args[0], arpTab.arr[size].arpMac)) {
			printf("ERROR: Could not resolve the IP address\n");
			free(arpTab.arr[size].arpMac);
			free(arpTab.arr[size].arpIp);
		}
		else {
			printf("Resulting MAC address: %s\n", arpTab.arr[size].arpMac);
			arpTab.arr[size].arpIp = &args[0];
			arpTab.size++;
		}
	}else if (type == 3) { //Eliminate an existing mapping
		for (i = 0; i < arpTab.size; i++) {
			if (memcmp(arpTab.arr[i].arpIp, &args[0], sizeof(&args[0])) == 0) {
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
	struct ethergram *ethPkt = malloc(sizeof(struct ethergram));
	device* devptr = NULL;

	/* Ethernet Destination MAC address set to FF:FF:FF:FF:FF for broadcast */
	//ethPkt->dst = malloc(ETH_ADDR_LEN);
	//ethPkt->src = malloc(IPv4_ADDR_LEN);
	ethPkt->dst[0] = 0xFF;
	ethPkt->dst[1] = 0xFF;
	ethPkt->dst[2] = 0xFF;
	ethPkt->dst[3] = 0xFF;
	ethPkt->dst[4] = 0xFF;
	ethPkt->dst[5] = 0xFF;

	control(ETH0, ETH_CTRL_GET_MAC, (long)ethPkt->src, 0);
	ethPkt->type = ETYPE_ARP;
	//ethPkt->data[0] = &arpPkt;
	memcpy(ethPkt->data, arpPkt, sizeof(arpPkt));

	arpPkt->hwtype = 1; // Come back and check for constant or define
	arpPkt->prtype = ETYPE_ARP;
	arpPkt->hwalen = ETH_ADDR_LEN;
	arpPkt->pralen = IPv4_ADDR_LEN;
	arpPkt->op = htons(ARP_OP_RQST);
	
	control(ETH0, ETH_CTRL_GET_MAC, (long)arpPkt->addrs, 0); // SHA
	memcpy(&arpPkt->addrs[ARP_ADDR_SPA], myipaddr, arpPkt->pralen); // SPA
	memcpy(&arpPkt->addrs[ARP_ADDR_DPA], ipaddr, arpPkt->pralen); // DPA


	getpid();
	/* Attempting to receive an ARP response */
	ready(create((void *)arpResolveHelper, INITSTK, 2, "ARP_HELPER", 2, ethPkt,currpid), 1);
	msg = receive(); /* Wait until helper function has made 3 attempts to arpResolve */

	if (msg == TIMEOUT)
		return SYSERR;
	else
	{
		mac = (uchar *) msg;
		return OK;
	}
}

void arpResolveHelper(struct ethergram *ethPkt, int prevId)
{
	int msg = TIMEOUT;
	int i;
	device *devptr = NULL;

	/* Sending ARP Packet */
	for (i = 0; i < 3 && msg == TIMEOUT; i++)
	{
		write(ETH0, ethPkt, (sizeof(struct ethergram) + sizeof(struct arpPkt)));
		getpid(); //check syscall
		send(arpDaemonId, currpid);
		msg = recvtime(CLKTICKS_PER_SEC);
	}

	send(prevId, msg);
}
