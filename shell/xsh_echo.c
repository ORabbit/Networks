/**
 * @file     xsh_echo.c
 * @provides xsh_echo
 *
 */

#include <xinu.h>
#include <udp.h>
#include <stdio.h>
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
	char c;
	int i = 0;
	uchar *ip = malloc(IP_ADDR_LEN);
	dot2ip(args[1], ip);
	char msg[113];
   bzero(msg,113);
	kprintf("only takes 112 byte long messages.\r\nPress ~ to quit.\r\n");
   while((c = getc(CONSOLE)) != '~'){
		if(c == '\n' || i > 111){
			if (SYSERR == echo(ip, atoi(args[2]), msg, i+1))
				  printf("ERROR: could not complete echo\n");
			i=0;
			bzero(msg,113);
			continue;
   	}
		msg[i] = c;
	   i++;
	}

		free(ip);

	return OK;
}


