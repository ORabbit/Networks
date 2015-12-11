
#include <xinu.h>
#include <icmp.h>
#include <arp.h>

//uchar packetICMP[MAX_ETH_LEN];
syscall fragmentPacketHelper(uchar packet[],uchar frag_pkts[])
{
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar zero_buff[MAX_ETH_LEN];
	bzero(zero_buff,MAX_ETH_LEN);
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];

	ip_ihl= ipPkt->ver_ihl & IPv4_IHL;
	if (ntohs(eg->type) != ETYPE_IPv4 
		|| ((ipPkt->ver_ihl & IPv4_VER) >> 4) != IPv4_VERSION || (ip_ihl < IPv4_MIN_IHL) 
		|| (ip_ihl > IPv4_MAX_IHL) || ntohs(ipPkt->len) < (IPv4_MIN_IHL * 4)
		|| checksum((uchar*)ipPkt, (ip_ihl * 4)) != 0){

		kprintf("corrupted ipPkt within\r\n");
		return SYSERR;//continue;
	}
	//initialize frag packets if it is empty with the first packet
	if(memcmp(frag_pkts,zero_buff,MAX_ETH_LEN)==0){
		memcpy(frag_pkts,eg,ETH_HEADER_LEN+ETH_MTU);
		return 1;
	}

	uchar morbius[] = {192,168,6,10};
	int morbOff = 0;
	if(memcmp(ipPkt->src, morbius, IPv4_ADDR_LEN) == 0)
		morbOff = 256;
//	kprintf("morbOff=%d\r\n",morbOff);
	
	if(ipPkt->id!=getIPpktId(frag_pkts)){
		//kprintf("wrong id-> incoming pkt id:%u. id of frag_pkts:%u\r\n",ipPkt->id,getIPpktId(frag_pkts));
		return SYSERR;
	}
	
	if(morbOff) {
		setIPpktId(frag_pkts, ipPkt->id+morbOff);
	}

	//passed all the ip header checks
		//last one?
	int overall_pkt_len = getIPpktLen(frag_pkts);
	syscall last_one;
	if(((ipPkt->flags_froff>>13) & 0b001) == 0){
//		kprintf("last one: true  %u\r\n", ipPkt->flags_froff);
		last_one = 0;
	}else{
//		kprintf("last one: false  %u\r\n", ipPkt->flags_froff);
		last_one = 1;
	}
	memcpy(frag_pkts+overall_pkt_len,&ipPkt->opts[0],ntohs(ipPkt->len)-ip_ihl*4);
//	addIcmpPktData(frag_pkts,ntohs(ipPkt->len)-ip_ihl*4);
	setIPpktLen(frag_pkts,overall_pkt_len+ntohs(ipPkt->len)-ip_ihl*4);
	calculateIPChcksum(frag_pkts);

	if(ipPkt->proto ==IPv4_PROTO_ICMP) calculateICMPChcksum(frag_pkts,ntohs(ipPkt->len)-ip_ihl*4);

	return last_one;
}
ushort getIPpktId(uchar packet[]){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	return ipPkt->id;

}
void setIPpktId(uchar packet[], ushort id){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	ipPkt->id = id;
}
ushort getIPpktLen(uchar packet[]){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	return ntohs(ipPkt->len);

}
void setIPpktLen(uchar packet[],ushort clen){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	ipPkt->len = ntohs(clen);

}
void calculateIPChcksum(uchar packet[]){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	ipPkt->chksum = 0;
	ipPkt->chksum = checksum((uchar*)ipPkt,IPv4_HDR_LEN);
}
void calculateICMPChcksum(uchar packet[],ushort clen){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	icmpPkt = (struct icmpPkt *)&ipPkt->opts[0];	
	icmpPkt->checksum=0;
	icmpPkt->checksum = checksum((uchar*)icmpPkt,clen);
}
/**
void addIcmpPktData (uchar packet[],ushort ip_data_len){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ipPkt = (struct ipgram *)&eg->data[0];
	ushort ip_ihl= ipPkt->ver_ihl & IPv4_IHL;

	icmpPkt = (struct icmpPkt *)&ipPkt->opts[0];
	memcpy(&icmpPkt->data[0]+(getIPpktLen(packet)-ip_ihl*4)-ICMP_HDR_LEN,ip_data_len);
}*/
/*void setIcmpPktLen(uchar packet[],ushort clen){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];
	icmpPkt = (struct icmpPkt *)&ipPkt->opts[0];
	icmpPkt->len = ntohs(clen);

}*/
int isFragmentedPacket(uchar packet[]){
	struct ethergram *eg;
	struct icmpPkt *icmpPkt;
	struct ipgram *ipPkt;
	uchar *myMac = malloc(ETH_ADDR_LEN);
	control(ETH0, ETH_CTRL_GET_MAC, (long)myMac, 0);

	eg = (struct ethergram *)packet;
	ushort ip_ihl;
	ipPkt = (struct ipgram *)&eg->data[0];

	return (ipPkt->flags_froff>>13) & 0b001;
}

