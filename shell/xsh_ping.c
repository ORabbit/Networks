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
	if (nargs != 3) {
		printf("ERROR: Invalid arguments\nping <ip addr> <num of pings>\n");
		return SYSERR;
	}

	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);
	ulong timeStart, timeStartMs;
	int msg1, msg2, i, max;
	ushort *data;

	max = atoi(args[2]);
	//clockinit();

	for (i = 1; i <= max; i++) {
		//kprintf("calling ping\r\n");
		if (ping(ip, i) == SYSERR)
			continue;
		clock_update(platform.time_base_freq / (1000*CLKTICKS_PER_SEC));
		timeStart = clocktime;
		timeStartMs = ctr_mS;
		//sleep(100);
		//kprintf("resched -> xsh_ping\r\n");
		//resched();
		send(icmpDaemonId, currpid);
		//kprintf("sent to icmpDaemonId\r\n");
		resched();
		msg1 = recvtime(10*CLKTICKS_PER_SEC);
		//kprintf("recved from message -> xsh_ping\r\n");
		if (msg1 == TIMEOUT)
			continue;

		msg2 = recvtime(10*CLKTICKS_PER_SEC);
		//kprintf("recved from message -> xsh_ping\r\n");
		if (msg2 == TIMEOUT)
			continue;
		//kprintf("No timeout!\r\n");
		//data = (ushort *)(long)msg;
		clock_update(platform.time_base_freq / (1000*CLKTICKS_PER_SEC));
		kprintf("%d bytes from %u.%u.%u.%u: icmp_seq=%d ttl=%d time=%u.%u s\r\n", msg1, ip[0], ip[1], ip[2], ip[3], i, msg2, clocktime - timeStart, ctr_mS - timeStartMs);
		sleep(1000);
	}
	kprintf("ping done\r\n");
	free(ip);

	return OK;
}


