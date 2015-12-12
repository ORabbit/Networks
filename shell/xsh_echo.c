/**
 * @file     xsh_echo.c
 * @provides xsh_echo
 *
 */

#include <xinu.h>
#include <udp.h>
/**
 * Shell command (arp) is ARP.
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_echo(int nargs, char *args[])
{
	if (nargs != 3) {
		printf("ERROR: Invalid arguments\necho <ip addr> <port number>\n");
		return SYSERR;
	}

	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);

	printf("echo\n");

	if (SYSERR == echo(ip, atoi(args[2]), "echo"))
		printf("ERROR: could not complete echo\n");

	free(ip);

	return OK;
}


