#include <xinu.h>
#include <arp.h>
#include <udp.h>
#include <icmp.h>

void netRecv(int arpId, int icmpId) {
	uchar pkt[PKTSZ];
	struct ethergram *eg;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);
	uchar frag_pkts[MAX_ETH_LEN];
	bzero(frag_pkts, MAX_ETH_LEN);
	int lookingForFragment = 0;
	ushort ret_val;
	uchar count = 0;
	while (1) {
		if (count > 0) {
			socketDaemon();
			count = 0;
		}
		count++;
		bzero(pkt, PKTSZ);
		read(ETH0, pkt, PKTSZ);
		eg = (struct ethergram *)pkt;

		uchar brd[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (memcmp(eg->dst, myMac, ETH_ADDR_LEN) != 0 && memcmp(eg->dst, &brd[0], ETH_ADDR_LEN) != 0)
			continue;

		switch (ntohs(eg->type)) {
			case ETYPE_ARP:
				arpDaemon(pkt);
				//send(arpId, 1);
				//resched();
				break;
			case ETYPE_IPv4:
				//using ICMP protocol
				//printPacketICMP(pkt);
				if(((struct ipgram*)&eg->data[0])->proto== IPv4_PROTO_ICMP){
					//kprintf("got icmp packet\r\n");
					if(!isFragmentedPacket(pkt)&&!lookingForFragment){
						//kprintf("1st case\r\n");
						icmpDaemon(pkt,0);
					}
					else if( (ret_val = fragmentPacketHelper(pkt,frag_pkts)) ==0){
					//	kprintf("2nd case\r\n");
						lookingForFragment = 0;
						icmpDaemon(frag_pkts,1);
						bzero(frag_pkts, MAX_ETH_LEN);
					}else if(ret_val == 1){
					//	kprintf("3rd case\r\n");
						lookingForFragment = 1;
					}else{ //SYSERR = ret_val ... different datagram or something else
						
					//	kprintf("4th case\r\n");
						bzero(frag_pkts, MAX_ETH_LEN);
						lookingForFragment = 0;
						continue;
					}
				//using udp protocol
				}else if(((struct ipgram*)&eg->data[0])->proto== IPv4_PROTO_UDP){ 
					if(!isFragmentedPacket(pkt)&&!lookingForFragment){
						//kprintf("1st case\r\n");
						udpDaemon(pkt);
					}
					else if( (ret_val = fragmentPacketHelper(pkt,frag_pkts)) ==0){
					//	kprintf("2nd case\r\n");
						lookingForFragment = 0;
						udpDaemon(frag_pkts);
						bzero(frag_pkts, MAX_ETH_LEN);
					}else if(ret_val == 1){
					//	kprintf("3rd case\r\n");
						lookingForFragment = 1;
					}else{ //SYSERR = ret_val ... different datagram or something else
						
					//	kprintf("4th case\r\n");
						bzero(frag_pkts, MAX_ETH_LEN);
						lookingForFragment = 0;
						continue;
					}
				}
				break;
			default:
				break;
		}
	}
}
