#include "udp.h"
#include "arp.h"
int udpOpen(udpSocket* socket){
	//initialize socket
	socket->state = UDP_SOCK_OPEN; 
	socket->local_ip = myipaddr;
	socket->packets.head = 0;
	socket->packets.tail = 0;
	socket->packets.size = 0;
	//table is full: return error
	if(udpGlobalTable.size>=MAX_SOCKETS) return -1;
	//add socket to the table
	udpGlobalTable->sockets[udpGlobalTable.size++]=socket;	
	return 0;
}

void udpWrite(udpSocket* socket,void* data,unsigned int len){
	
	struct userDataGram* udg= malloc(sizeof(struct userDatagram));

	udg->src_port=socket->local_port;
	udg->dst_port=socket->remote_port;
	udg->total_len=len+UDP_HDR_LEN;
	memcpy(&udg->data[0],data,len);
	ipWrite((void *)udg,udg->total_len,IPv4_PROTO_UDP,socket->local_ip,socket->remote_ip);

	free(udg);
}
void* udpRead(udpSocket* socket){
	return dequeue(socket->packets);
}
uchar* dequeue(struct cqueue queue){
	if(queue.cqueue_arr[queue.head]==0 || queue.size<=0) return NULL;
	else{
		uchar* packet =  queue.cqueue_arr[queue.head];	
		queue.head=(queue.head-1)%MAX_QUEUE_SIZE;
		queue.size--;
		return packet;
	}
}

syscall enqueue(struct cqueue queue, uchar* pkt){

		if(queue.size>=MAX_QUEUE_SIZE) return SYSERR;

		queue.cqueue_arr[queue.tail]=pkt;

		queue.tail = (queue.tail+1)%MAX_QUEUE_SIZE;

		queue.size++;
		return OK;
}
