/**
 * @file     xsh_test.c
 * @provides xsh_test
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>
#include <icmp.h>
#include <udp.h>
#include <arp.h>

/**
 * Shell command (test) is testing hook.
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_udpstat(int nargs, char *args[])
{
	if (nargs != 0) {
		printf("ERROR:Invalid arguments\n udpstat takes no arguments.");
		return SYSERR;
	}
	printf("Opened UDP Sockets:\r\n");	
	int i;
	for(i=0;udpGlobalTable->size;i++){
		if(udpGlobalTable->sockets[i].state==UDP_SOCK_OPEN){
			
			printf("Socket %d:",i);
			printf("remote ip: ");
			printAddr(udpGlobalTable->sockets[i].remote_ip,IP_ADDR_LEN);
			printf("local ip: ");
			printAddr(udpGlobalTable->sockets[i].local_ip,IP_ADDR_LEN);
			printf("source port: %u", udpGlobalTable->sockets[i].source_port);
			printf("destination port: %u",udpGlobalTable->sockets[i].destination_port);
			printf("incoming packets: %u packets",udpGlobalTable->sockets[i].packets.tail-udpGlobalTable->sockets[i].packets.head);
		}
	}
	return OK;
}
