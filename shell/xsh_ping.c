/**
 * @file     xsh_arp.c
 * @provides xsh_arp
 *
 */

#include <xinu.h>
#include <icmp.h>
<<<<<<< HEAD
#include <arp.h>
=======

>>>>>>> 952e0982e5a463a5a96237c47d1e5c135cfea3cd
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

<<<<<<< HEAD
	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);
=======
	uchar *ip = malloc(IPv4_ADDR_LEN);
	dot2ip(args[2], ip);
>>>>>>> 952e0982e5a463a5a96237c47d1e5c135cfea3cd
	ping(ip);

	send(icmpDaemonId, currpid);
	int msg, i;
	ushort *data;
<<<<<<< HEAD

	for (i = 1; /*getc(CONSOLE) == 'd' ||*/ (msg = recvtime(3*CLKTICKS_PER_SEC)) != TIMEOUT; i++) {
		data = (ushort *)(long)msg;
		kprintf("%d bytes from %u.%u.%u.%u: icmp_seq=%d ttl=%u time=%u ms\r\n", data[0], ip[0], ip[1], ip[2], ip[3], i, data[1], data[2]);
		ping(ip);
		send(icmpDaemonId, currpid);
	}
	kprintf("ping done\r\n");
=======
	for (i = 1; getc(CONSOLE) == 'd' || (msg = recvtime(1)) == TIMEOUT; i++) {
		data = (ushort *)(long)msg;
		printf("%d bytes from %u.%u.%u.%u: icmp_seq=%d ttl=%u time=%u ms\n", data[0], ip[0], ip[1], ip[2], ip[3], i, data[1], data[2]);
		send(icmpDaemonId, 0);
	}

>>>>>>> 952e0982e5a463a5a96237c47d1e5c135cfea3cd
	free(ip);

	return OK;
}
