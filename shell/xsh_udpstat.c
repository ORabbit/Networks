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
	if (nargs != 1) {
		printf("ERROR:Invalid arguments\n udpstat takes no arguments.\n");
		return SYSERR;
	}
	kprintf("Opened UDP Sockets:\r\n");	
	int i;
	//wait(semSockTab);
	for(i=0;i<udpGlobalTable->size;i++){
		if(udpGlobalTable->sockets[i]!=0&&udpGlobalTable->sockets[i]->state==UDP_SOCK_OPEN){
			
			kprintf("Socket %d:\r\n",i);
			kprintf("remote ip: \r\n");
			printAddr(udpGlobalTable->sockets[i]->remote_ip,IP_ADDR_LEN);
			kprintf("local ip: \r\n");
			printAddr(udpGlobalTable->sockets[i]->local_ip,IP_ADDR_LEN);
			kprintf("source port: %u\r\n", udpGlobalTable->sockets[i]->local_port);
			kprintf("destination port: %u\r\n",udpGlobalTable->sockets[i]->remote_port);
			kprintf("incoming packets: %u packets\r\n",udpGlobalTable->sockets[i]->packets->size);
		}
	}
	//signal(semSockTab);
	return OK;
}
