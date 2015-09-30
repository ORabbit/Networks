/**
 * @file arp.c
 * @provides arpResolve.
 *
 */

#include <xinu.h>

struct arp {
	uchar *arpIp;
	uchar *arpMac;
}

struct arpTable {
	struct arp arr[]; /* Using a array instead of linked-list because of infrequent deletions and set size */
	int size;
}

struct arpTable arpTab;
semaphore arpSem;
int isDaemonRunning = 0;


/**
 * Shell command arp that can be used to display the current table, to
 * request a new mapping, or to eliminate an existing mapping.
 */
command arp(int type, int nargs, uchar *args)
{
	int i, j;

	if (!isDaemonRunning)
		ready(create((void *)arpDaemon, INITSTK, 3, "ARPDAEMON", 0), 1);

	wait(arpSem);
	
	if (type == 1) { //Display current arp table
		printf("IP ADDRESS\tMAC ADDRESS\n");
		for (i = 0; i < arpTab->size; i++) {
			printf("%s\t%s\n", arpTab->arr[i]->arpIp, arpTab->arr[i]->arpMac);
		}
	}else if (type == 2) { //Request new arp mapping
		arpTab->arr[size]->arpMac = malloc(sizeof(uchar)*6);
		arpTab->arr[size]->arpIp = malloc(sizeof(uchar)*4);

		if (SYSERR == arpResolve(args[0], arpTab->arr[size]->arpMac)) {
			printf("ERROR: Could not resolve the IP address\n");
			free(arpTab->arr[size]->arpMac);
			free(arpTab->arr[size]->arpIp);
		}
		else {
			printf("Resulting MAC address: %s\n", arpTab->arr[size]->arpMac);
			arpTab->arr[size]->arpIp = args[0];
			arpTab->size++;
		}
	}else if (type == 3) { //Eliminate an existing mapping
		for (i = 0; i < arpTab->size; i++) {
			if (memcmp(arpTab->arr[i]->arpIp, args[0], sizeof(args[0])) == 0) {
				for (j = i; j < arpTab->size-1; j++)
					arpTab->arr[j] = arpTab->arr[j+1];
				free(arpTab->arr[size]);
				arpTab->size--;
				break;
			}
		}				
	}else { //Incorrect input
		printf("ERROR: Invalid input\n");
	}
	
	signal(arpSem);
			
}

/**
 * Replies to ARP requests destined for our router, and handles replies
 * from our router
 */
void arpDaemon(void)
{
	arpTab = malloc(sizeof(struct arpTable));
	arpTab->arr = malloc(sizeof(struct arp)*30);
	arpTab->size = 0;
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

	// Check to see if the ip address is already mapped
	for (i = 0; i < arpTab->size; i++) {
		if (memcmp(arpTab->arr[i]->arpIp, ipaddr, sizeof(ipaddr) == 0)
			return arpTab->arr[i]->arpMac;
	}
	//if not already mapped find it
	struct arpPkt *arppkt=malloc(sizeof(struct arppkt));
	device* devptr;

	etherControl(devptr,ETH_CTRL_GET_MAC, (long)arpPkt->hwtype,0);
	arpPkt->prtype = ETYPE_ARP;
	arpPkt->hwalen = ETH_ADDR_LEN;
	arpPkt->pralen = IPv4_ADDR_LEN;
	arpPkt->op = hs2net(ARP_OP_RQST);
	

	

}
