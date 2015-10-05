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
	uchar *ip = malloc(sizeof(IPv4_ADDR_LEN));
	ip[0] = 192;
	ip[1] = 168;
	ip[2] = 6;
	ip[3] = 10;
	arp(2, ip);
	return OK;
}
