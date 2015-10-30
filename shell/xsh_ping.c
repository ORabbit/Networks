/**
 * @file     xsh_arp.c
 * @provides xsh_arp
 *
 */

#include <xinu.h>
#include <icmp.h>

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

	send(icmpDaemonId, currpid);
	int msg, i;
	ushort *data;
	for (i = 1; getc(CONSOLE) == 'd' || (msg = recvtime(1)) == TIMEOUT; i++) {
		data = (ushort *)(long)msg;
		printf("%d bytes from %u.%u.%u.%u: icmp_seq=%d ttl=%u time=%u ms\n", data[0], ip[0], ip[1], ip[2], ip[3], i, data[1], data[2]);
		send(icmpDaemonId, 0);
	}

	free(ip);

	return OK;
}
