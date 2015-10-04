/**
 * @file netInit.c
 * @provides netInit.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>

/**
 * Initialize network interface.
 */
void netInit(void)
{
	open(ETH0);
	ready(create((void *)arpDaemon, INITSTK, 3, "ARPDAEMON", 0), 1); /* Starts an ARP Daemon for the backend. */
	return;
}
