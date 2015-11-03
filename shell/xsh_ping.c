/**
 * @file     xsh_arp.c
 * @provides xsh_arp
 *
 */

#include <xinu.h>
#include <icmp.h>
#include <arp.h>
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

	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);
	int timeStart;
	int msg, i;
	ushort *data;

	for (i = 1; /*getc(CONSOLE) == 'd' ||*/ 1 ; i++) {
		if (ping(ip, i) == SYSERR)
			continue;
		timeStart = clocktime;
		sleep(100);
		send(icmpDaemonId, currpid);

		msg = recvtime(10*CLKTICKS_PER_SEC);
		if (msg == TIMEOUT)
			continue;
		data = (ushort *)(long)msg;
		kprintf("%d bytes from %u.%u.%u.%u: icmp_seq=%d ttl=%u time=%u ms\r\n", data[0], ip[0], ip[1], ip[2], ip[3], i, data[1], timeStart - clocktime);
	}
	kprintf("ping done\r\n");
	free(ip);

	return OK;
}
