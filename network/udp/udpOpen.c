#include <xinu.h>
#include <udp.h>
#include <arp.h>
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
	//add socket to the table
	udpGlobalTable->sockets[udpGlobalTable->size]=socket;	
	udpGlobalTable->size++;
	//signal(semSockTab);
	return 0;
}

void udpWrite(struct udpSocket* socket,void* data,unsigned int len){
	struct userDataGram* udg = malloc(sizeof(struct userDataGram));
	udg->src_port=ntohs(socket->local_port);
	udg->dst_port=ntohs(socket->remote_port);
	udg->total_len=ntohs(len+UDP_HDR_LEN);
	udg->checksum = 0;
	memcpy(&udg->data[0],data,len);
	uchar *macDst = malloc(ETH_ADDR_LEN);
	arpResolve(socket->remote_ip, macDst);
	kprintf("ABOUT TO CALL ipWrite from udpWrite\r\n");
	ipWrite((void *)udg,htons(udg->total_len),IPv4_PROTO_UDP,socket->remote_ip,macDst);

	free(udg);
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
				switch (port) {
					case 7:
						kprintf("got echo in socketDaemon\r\n");
						int msg = recvtime(1 * CLKTICKS_PER_SEC);
						eg = udpRead(socket);
						udg = (struct userDataGram*)&(((struct ipgram*)&eg->data[0])->opts[0]);
						kprintf("got past dequeue in socket daemon\r\n");
						if (msg == TIMEOUT) {
							ipWrite((uchar*)udg, htons(udg->total_len), IPv4_PROTO_UDP, socket->remote_ip, eg->src);
						}else
							kprintf("%s\r\n",(char*)&udg->data[0]);
						break;
					case 37:
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

uchar* cdequeue(struct cqueue* cQueue){
	if(cQueue->cqueue_arr[cQueue->head]==0 || cQueue->size<=0) return NULL;
	else{
		uchar* packet =  cQueue->cqueue_arr[cQueue->head];	
		cQueue->head=(cQueue->head-1)%MAX_QUEUE_SIZE;
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
