#include <xinu.h>
#include <icmp.h>
#include <udp.h>

command echo(uchar *ip, ushort port, char msg[]) {
	struct udpSocket *socket;

	if (ip == NULL || port < 0 || msg == NULL) return SYSERR;

	socket = malloc(sizeof(struct udpSocket));
	if (-1 == udpOpen(socket, ip, port)) return SYSERR;

	kprintf("ABOUT TO WRITE\r\n");
	udpWrite(socket, msg, strlen(msg)+1);
	sleep(1000);

	send(icmpDaemonId, -5);	

	//struct userDataGram *udpGram = udpRead(socket);
	//if (udpGram == NULL) return SYSERR;

	//printf("%s\n", (char *)&udpGram->data[0]);

	return OK;
}