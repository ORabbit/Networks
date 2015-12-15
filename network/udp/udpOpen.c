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
	//wait(semSockTab);
	if(udpGlobalTable->size>=MAX_SOCKETS){ free(socket); return -1;}
	if(globTableContains(socket)) { return 0;}
	//add socket to the table
	udpGlobalTable->sockets[udpGlobalTable->size]=socket;	
	udpGlobalTable->size++;
	//signal(semSockTab);
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
	kprintf("ABOUT TO CALL ipWrite from udpWrite\r\n");
	ipWrite((void *)udg,htons(udg->total_len),IPv4_PROTO_UDP,socket->remote_ip,macDst);

	//free(udg);
}
void* udpRead(struct udpSocket* socket){
	return cdequeue(socket->packets);
}

void socketDaemon(void){//(struct udpSocketTable *udpGlobalTable, semaphore semSock) {
	int i;
	struct udpSocket *socket;
	struct userDataGram *udg;
	struct ethergram *eg;


	//while (1) {
		//wait(semSock);
		kprintf("in socketDaemon\r\n");
		for (i = 0; i < udpGlobalTable->size; i++) {
			kprintf("in for of socketDaemon\r\n");
			if (udpGlobalTable->sockets[i] != 0 
					&& udpGlobalTable->sockets[i]->state == UDP_SOCK_OPEN
					&& udpGlobalTable->sockets[i]->packets->size > 0) {
				kprintf("got packet in socketDaemon\r\n");
				socket = udpGlobalTable->sockets[i];
				ushort port = socket->local_port;
				int msg = recvtime(1 * CLKTICKS_PER_SEC);
				eg = udpRead(socket);
				udg = (struct userDataGram*)&(((struct ipgram*)&eg->data[0])->opts[0]);
				kprintf("got past dequeue in socket daemon\r\n");
				switch (port) {
					case 7:
						kprintf("got echo in socketDaemon\r\n");
						if (msg == TIMEOUT) {
							kprintf("GOING TO WRITE BACK ECHO\r\n");
							//printPacketUDP((uchar*)eg);
							ipWrite((uchar*)udg, htons(udg->total_len), IPv4_PROTO_UDP, socket->remote_ip, eg->src);
							kprintf("FINISHED WRITING BACK ECHO\r\n");
						}else
							kprintf("%s\r\n",(char*)&udg->data[0]);
						break;
					case 37:
						kprintf("got rdate in socketDaemon\r\n");
						if (msg == TIMEOUT) {
							kprintf("GOING TO WRITE BACK RDATE\r\n");
							ipWrite((uchar*)udg, htons(udg->total_len), IPv4_PROTO_UDP, socket->remote_ip, eg->src);
							kprintf("FINISHED WRITING BACK RDATE\r\n");
						}else {
							kprintf("data: %s\r\n",(char*)&udg->data[0]);
							convertToDate(atoi((char*)&udg->data[0]));					
						}
					
						break;
					default:
						break;
				}
			}
		}
		//signal(semSock);
		//resched();
	//}
}

void convertToDate(int sec) {
	// Convert seconds since epoch to timestamp
	//char *timestamp[12];
	char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "November", "Decemeber"};
	char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

	//	12:00AM Jan 1st, 1900 GMT.
	int yearsPast = sec / 31556926;
	int monthsPast = sec / 2629743;
	int weeksPast = sec / 604800;
	int daysPast = sec / 86400;
	int hoursPast = sec / 3600;
	int minsPast = sec / 60;

	//char *hours = itoa(hoursPast%24);
	//strncpy(timestamp[0], hours, strlen(hours));
	kprintf("%d:",hoursPast%24);
	//char *mins = itoa(minsPast%60);
	//strncpy(timestamp[2], mins, strlen(mins));
	kprintf("%d:",minsPast%60);
	//char *secs = itoa(sec%60);
	kprintf("%d ",sec%60);
	//strncpy(timestamp[4], secs, strlen(secs));
	//timestamp[5] = " GMT ";
	kprintf("GMT, ");
	//strncpy(timestamp[6], days[daysPast%7], strlen(days[daysPast%7]));
	//timestamp[7] = ", ";
	//strncpy(timestamp[8], months[monthsPast%12], strlen(months[monthsPast%12]));
	kprintf("%s ",months[monthsPast%12]);
	//timestamp[9] = " ";
	//char *dayNum = itoa(daysPast%30);
	//strncpy(timestamp[10], dayNum, strlen(dayNum));
	kprintf("%d ",daysPast%30);
	//timestamp[11] = ", ";
	//char *year = itoa(yearsPast+1900);
	//strncpy(timestamp[12], year, strlen(year));

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
