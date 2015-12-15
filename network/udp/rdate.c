#include <xinu.h>
#include <icmp.h>
#include <udp.h>

command rdate(uchar *ip, ushort port) {
	struct udpSocket *socket;

	if (ip == NULL || port < 0) return SYSERR;

	socket = malloc(sizeof(struct udpSocket));
	if (-1 == udpOpen(socket, ip, port)) return SYSERR;

	kprintf("ABOUT TO WRITE\r\n");
	udpWrite(socket, "", 0);
	sleep(1000);

	send(icmpDaemonId, -10);

	//struct userDataGram *udpGram = udpRead(socket);
	//if (udpGram == NULL) return SYSERR;

	//printf("%s\n", (char *)&udpGram->data[0]);

	return OK;
}
