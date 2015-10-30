/**
 * @file     xsh_arp.c
 * @provides xsh_arp
 *
 */

#include <xinu.h>
#include <ping.h>

/**
 * Shell command (arp) is ARP.
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_ping(int nargs, char *args[])
{
	if (nargs != 2) {
		printf("ERROR: Invalid arguments\nping <ip addr>\n");
		return SYSERR;
	}

	uchar *ip = malloc(IPv4_ADDR_LEN);
	dot2ip(args[2], ip);
	ping(ip);
	return OK;
}
