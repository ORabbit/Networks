#include <xinu.h>
#include <udp.h>
#include <arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int udpOpen(struct udpSocket* socket, uchar *ip, ushort port){
	//initialize socket
	socket->state = UDP_SOCK_OPEN; 
	memcpy(socket->local_ip,myipaddr,IP_ADDR_LEN);
	memcpy(socket->remote_ip,ip,IP_ADDR_LEN);
	socket->local_port = port;
	socket->remote_port = port;
	socket->packets = malloc(sizeof(struct cqueue));
	bzero(socket->packets->cqueue_arr,MAX_QUEUE_SIZE);

	if(socket->packets==NULL) return -1;

	socket->packets->head = 0;
	socket->packets->tail = 0;
	socket->packets->size = 0;
	//table is full: return error
	if(udpGlobalTable->size>=MAX_SOCKETS){ free(socket); return -1;}
	if(globTableContains(socket)) { return 0;}
	//add socket to the table
	udpGlobalTable->sockets[udpGlobalTable->size]=socket;	
	udpGlobalTable->size++;
	return 0;
}

int globTableContains(struct udpSocket *socket) {
	int i;
	for (i = 0; i < udpGlobalTable->size; i++) {
		struct udpSocket *tmpSock = udpGlobalTable->sockets[i];
		if (memcmp(socket->local_ip, tmpSock->local_ip, IPv4_ADDR_LEN) == 0
			&& memcmp(socket->remote_ip, tmpSock->remote_ip, IPv4_ADDR_LEN) == 0
			&& socket->local_port == tmpSock->local_port && socket->remote_port == tmpSock->remote_port) {
			if (tmpSock->state == UDP_SOCK_OPEN){ socket = tmpSock; return 1;}
			else {
				tmpSock->state = UDP_SOCK_OPEN;
				socket = tmpSock;
				return 1;
			}
		}
	}
	return 0;
}

void udpWrite(struct udpSocket* socket,void* data,unsigned int len){
	struct userDataGram* udg = malloc(sizeof(struct userDataGram)+len);
	udg->src_port=ntohs(socket->local_port);
	udg->dst_port=ntohs(socket->remote_port);
	udg->total_len=ntohs(len+UDP_HDR_LEN);
	udg->checksum = 0;
	memcpy(&udg->data[0],data,len);
	uchar *macDst = malloc(ETH_ADDR_LEN);
	arpResolve(socket->remote_ip, macDst);
	//kprintf("ABOUT TO CALL ipWrite from udpWrite\r\n");
	ipWrite((void *)udg,htons(udg->total_len),IPv4_PROTO_UDP,socket->remote_ip,macDst);
}

void* udpRead(struct udpSocket* socket){
	return cdequeue(socket->packets);
}

void socketDaemon(void){//(struct udpSocketTable *udpGlobalTable, semaphore semSock) {
	int i;
	struct udpSocket *socket;
	struct userDataGram *udg;
	struct ethergram *eg;
	uchar buf[4];

		//kprintf("in socketDaemon\r\n");
		for (i = 0; i < udpGlobalTable->size; i++) {
			if (udpGlobalTable->sockets[i] != 0 
					&& udpGlobalTable->sockets[i]->state == UDP_SOCK_OPEN
					&& udpGlobalTable->sockets[i]->packets->size > 0) {
				//kprintf("got packet in socketDaemon\r\n");
				socket = udpGlobalTable->sockets[i];
				ushort port = socket->local_port;
				int msg = recvtime(1 * CLKTICKS_PER_SEC);
				eg = udpRead(socket);
				udg = (struct userDataGram*)&(((struct ipgram*)&eg->data[0])->opts[0]);
				//kprintf("got past dequeue in socket daemon\r\n");
				switch (port) {
					case 7:
						//kprintf("got echo in socketDaemon\r\n");
						if (msg == TIMEOUT) {
							//kprintf("GOING TO WRITE BACK ECHO\r\n");
							//printPacketUDP((uchar*)eg);
							ipWrite((uchar*)udg, htons(udg->total_len), IPv4_PROTO_UDP, socket->remote_ip, eg->src);
							//kprintf("FINISHED WRITING BACK ECHO\r\n");
						}else
							kprintf("%s\r\n",(char*)&udg->data[0]);
						break;
					case 37:
						//kprintf("got rdate in socketDaemon\r\n");
						memcpy(buf,udg->data,4);
						//kprintf("data in buffer: %u\r\n",(buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
						convertToDate((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
						//}
						break;
					default:
						break;
				}
			}
		}
}


void convertToDate(unsigned int sec) {
	// Convert seconds since epoch to timestamp
	//char *timestamp[12];
	char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September","October", "November", "Decemeber"};
	char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

	//	12:00AM Jan 1st, 1900 GMT.

	int yearsPast = sec / 31557600;
	int monthsPast = sec / 2629743;
	int weeksPast = sec / 604800;
	int daysPast = sec / 86400;
	int hoursPast = sec / 3600;
	int minsPast = sec / 60;

	kprintf("%d:",(sec/(60*60))%24);
	kprintf("%d:",(sec/60)%60);
	kprintf("%d ",sec%60);
	kprintf("GMT, ");
	kprintf("%s ",days[daysPast%7]);
	kprintf("%s ",months[monthsPast%12]);
	kprintf("%d ",daysPast%28);
	kprintf("%d \r\n",yearsPast+1900);	
}

uchar* cdequeue(struct cqueue* cQueue){
	if(cQueue->cqueue_arr[cQueue->head]==0 || cQueue->size<=0) return NULL;
	else{
		uchar* packet =  cQueue->cqueue_arr[cQueue->head];	
		cQueue->head=(cQueue->head+1)%MAX_QUEUE_SIZE;
		cQueue->size--;
		return packet;
	}
}

syscall cenqueue(struct cqueue* cQueue, uchar* pkt){

		if(cQueue->size>=MAX_QUEUE_SIZE) return SYSERR;

		cQueue->cqueue_arr[cQueue->tail]=pkt;

		cQueue->tail = (cQueue->tail+1)%MAX_QUEUE_SIZE;

		cQueue->size++;
		return OK;
}
