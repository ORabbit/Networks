/**
 * @file     xsh_rdate.c
 * @provides xsh_rdate
 *
 */

#include <xinu.h>
#include <udp.h>
/**
 * Shell command (rdate) is rdate.
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_rdate(int nargs, char *args[])
{
	if (nargs != 2) {
		printf("ERROR: Invalid arguments\nrdate <ip addr>\n");
		return SYSERR;
	}

	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);

	printf("Getting date\n");

	if (SYSERR == rdate(ip, 37))
		printf("ERROR: could not complete rdate\n");

	free(ip);

	sleep(1000);

	return OK;
}
