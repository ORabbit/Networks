/**
 * @file     xsh_arp.c
 * @provides xsh_arp
 *
 */

#include <xinu.h>
#include <arp.h>

/**
 * Shell command (arp) is ARP.
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_arp(int nargs, char *args[])
{
	if (nargs != 2 && nargs != 3) {
		printf("ERROR: Invalid arguments\narp --display\narp --add <ipAddr>\narp --delete <ipAddr>\n");
		return SYSERR;
	}

	uchar *ip = malloc(IPv4_ADDR_LEN);
	dot2ip(args[2], ip);
	//printf("%u.%u.%u.%u\n", ip[0],ip[1],ip[2],ip[3]);
	
	if (memcmp(args[1], "--display", 9) == 0)
		if (SYSERR == arp(ARP_DISPLAY, NULL))
			printf("ERROR: Could not display table\n");
	else if (memcmp(args[1], "--add", 5) == 0 && sizeof(args[2]) <= 15)
		if (SYSERR == arp(ARP_ADD, ip))
			printf("ERROR: Could not add ip: %s\n", args[2]);
	else if (memcmp(args[1], "--delete", 8) == 0 && sizeof(args[2]) <= 15)
		if (SYSERR == arp(ARP_DELETE, ip))
			printf("ERROR: Could not delete ip: %s\n", args[2]);
	else {
		printf("ERROR\n");
		return SYSERR;
	}

	return OK;
}
