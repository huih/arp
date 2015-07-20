/*
 ============================================================================
 Name        : arp.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "base.h"
#include "hostintf.h"
#include "arp.h"

typedef struct arp_packet
{
	struct ether_header eh;
	struct ether_arp arp;
}ArpPacket;


/**
 * @brief 发送ARP请求包
 * @param
 * @return 发送成功返回0， 否则返回-1
 * **/
static int send_broadCastRequestPackage(int skfd, const char* interfaceName, const unsigned char* routeIp);

//接收ARP的响应包
static int recv_arpReplyPackage(const unsigned char* routeIp, ArpPacket* pArpPacket);

//打包请求包或者响应包
static void pack_arpPackage(const unsigned char *toMac, const unsigned char *toIp,
		const unsigned char *fromMac, const unsigned char *fromIP,
		const unsigned short int op, ArpPacket* arpPacket);

//设置发送的地址
static void set_destAddr(const int interfaceIndex, const unsigned char* toMac,
		const unsigned char* toIp, struct sockaddr_ll *pDest);

//打印package
static void print_arpPackage(ArpPacket* pArpPacket);
static void print_data(const char* headstr, const unsigned char* data, int len);


int send_broadCastReplyPackage(const int interfaceIndex, const unsigned char*ip, const unsigned char* mac)
{
	int err = 0;
	int sfd = 0;
	struct sockaddr_ll dest;
	ArpPacket arpPacket;
	unsigned char toMac[ETH_ALEN];
	unsigned char toIp[4];

	sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
	if (sfd < 0)
	{
		printf("create socket error, errno: %d, errstr: %s\n", errno, strerror(errno));
		return -1;
	}

	//设置目标地址
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_halen = ETH_ALEN;
	dest.sll_ifindex =interfaceIndex;
	memcpy(dest.sll_addr, mac, ETH_ALEN);

	//设置数据包
	memset(toMac, 0xff, ETH_ALEN);
	memset(toIp, 0, 4);//htons(ARPOP_REPLY)
	pack_arpPackage(toMac, toIp, mac, ip, htons(ARPOP_REPLY), &arpPacket);

	err = sendto(sfd, &arpPacket, sizeof(ArpPacket), 0, (struct sockaddr *)&dest, sizeof(dest));
	close(sfd);
	if (err < sizeof(arpPacket))
	{
		printf("sendto data error, errno: %d, errstr: %s\n", errno, strerror(errno));
		return -1;
	} else {
		printf("send data success\n");
	}
	return 0;
}

char* query_destMacAddr(const char* interfaceName, const unsigned char* destIp)
{
	int skfd = 0;
	ArpPacket arpPacket;
	char* destMac = NULL;

	//1. 创建一个socket
	skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
	if (skfd < 0)
	{
		printf("create socket error, errno: %d, errstr: %s\n", errno, strerror(errno));
		return destMac;
	}

	//2. 发送request数据
	if (send_broadCastRequestPackage(skfd, interfaceName, destIp) < 0)
	{
		printf("send request package error\n");
		close(skfd);
		return destMac;
	}
	close(skfd);

	//3. 接收响应包
	if(recv_arpReplyPackage(destIp, &arpPacket) < 0)
	{
		printf("recv arp package error\n");
		close(skfd);
		return destMac;
	}

	destMac = (char*)malloc(sizeof(char) * 18);
	if (destMac == NULL)
	{
		printf("malloc memory error\n");
		return NULL;
	}
	snprintf(destMac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)arpPacket.arp.arp_sha[0],
			(unsigned char)arpPacket.arp.arp_sha[1],
			(unsigned char)arpPacket.arp.arp_sha[2],
			(unsigned char)arpPacket.arp.arp_sha[3],
			(unsigned char)arpPacket.arp.arp_sha[4],
			(unsigned char)arpPacket.arp.arp_sha[5]);
	return destMac;
}

static int send_broadCastRequestPackage(int skfd, const char* interfaceName, const unsigned char* routeIp)
{
	int err = 0;
	ArpPacket arpPacket;
	struct sockaddr_ll dest;
	unsigned char toMac[ETH_ALEN];
	unsigned char* fromMac;
	unsigned char* fromIp;
	int interfIndex = 0;

	interfIndex = get_interfaceIndex(interfaceName);
	if (interfIndex < 0)
	{
		printf("get interface index error\n");
		return -1;
	}

	//1.获取到本地的ip和mac
	memset(toMac, 0xff, ETH_ALEN);
	fromMac = query_interfaceMacUC(interfaceName);
	fromIp = query_interfaceIpUC(interfaceName);

	//2.创建请求包
	pack_arpPackage(toMac, routeIp, fromMac, fromIp, htons(ARPOP_REQUEST), &arpPacket);
	if (fromMac != NULL)
	{
		free(fromMac);
		fromMac = NULL;
	}
	if (fromIp != NULL){
		free(fromIp);
		fromIp = NULL;
	}

	//3.设置发送地址
	set_destAddr(interfIndex, toMac, routeIp, &dest);
	print_arpPackage(&arpPacket);

	//4.发送数据
	err = sendto(skfd, &arpPacket, sizeof(ArpPacket), 0, (struct sockaddr *)&dest, sizeof(dest));
	if (err < sizeof(arpPacket))
	{
		printf("sendto data error, errno: %d, errstr: %s\n", errno, strerror(errno));
		return -1;
	} else {
		printf("send data success\n");
	}

	return 0;
}

static int recv_arpReplyPackage(const unsigned char* routeIp, ArpPacket* pArpPacket)
{
	struct sockaddr_ll destAddr;
	int skfd = 0;
	int destLen = sizeof(destAddr);

	//创建socket
	skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	bzero(pArpPacket, sizeof(ArpPacket));
	bzero(&destAddr, sizeof(destAddr));

	//接收数据
	while(1)
	{
		if (recvfrom(skfd, pArpPacket, sizeof(ArpPacket), 0, (struct sockaddr *)&destAddr, (socklen_t*)&destLen) <= 0)
		{
			printf("recvfrom function error\n");
			close(skfd);
			return -1;
		}

		//检测是否为ARP包，并且响应端是路由器
		if(pArpPacket->eh.ether_type != htons(ETHERTYPE_ARP)
				|| pArpPacket->arp.ea_hdr.ar_op != htons(ARPOP_REPLY)
				|| memcmp(pArpPacket->arp.arp_spa, routeIp, 4) != 0)
		{
			continue;
		}
		break;
	}
	close(skfd);
	return 0;
}

//打包请求包或者响应包
static void pack_arpPackage(const unsigned char *toMac, const unsigned char *toIp,
		const unsigned char *fromMac, const unsigned char *fromIp,
		const unsigned short int op, ArpPacket* pArpPacket)
{
	pArpPacket->eh.ether_type = htons(ETHERTYPE_ARP);
	memcpy(pArpPacket->eh.ether_dhost, toMac, ETH_ALEN);
	memcpy(pArpPacket->eh.ether_shost, fromMac, ETH_ALEN);
	pArpPacket->arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
	pArpPacket->arp.ea_hdr.ar_pro = htons(ETH_P_IP);
	pArpPacket->arp.ea_hdr.ar_hln = ETH_ALEN;
	pArpPacket->arp.ea_hdr.ar_pln = 4;
	pArpPacket->arp.ea_hdr.ar_op = op;
	memcpy(pArpPacket->arp.arp_sha, fromMac, ETH_ALEN);
	memcpy(pArpPacket->arp.arp_spa, fromIp, 4);
	memcpy(pArpPacket->arp.arp_tha, toMac, ETH_ALEN);
	memcpy(pArpPacket->arp.arp_tpa, toIp, 4);
}

static void set_destAddr(const int interfaceIndex, const unsigned char* toMac,
		const unsigned char* toIp, struct sockaddr_ll *pDest)
{
	memset(pDest, 0, sizeof(struct sockaddr_ll));
	pDest->sll_family = AF_PACKET;
	//pDest->sll_halen = ETH_ALEN;
	pDest->sll_ifindex =interfaceIndex;//IFF_BROADCAST
	//memcpy(pDest->sll_addr, toMac, ETH_ALEN);
}

static void print_arpPackage(ArpPacket* pArpPacket)
{
	print_data("eh.ether_dhost: ", pArpPacket->eh.ether_dhost, 6);
	print_data("eh.ether_shost: ", pArpPacket->eh.ether_shost, 6);
	printf("eh.ether_type: %02x\n", ntohs(pArpPacket->eh.ether_type));

	printf("ar_hln: %x\n", pArpPacket->arp.ea_hdr.ar_hln);
	printf("ar_hrd: %x\n", ntohs(pArpPacket->arp.ea_hdr.ar_hrd));
	printf("ar_op: %x\n", ntohs(pArpPacket->arp.ea_hdr.ar_op));
	printf("ar_pln: %x\n", pArpPacket->arp.ea_hdr.ar_pln);
	printf("ar_pro: %x\n", ntohs(pArpPacket->arp.ea_hdr.ar_pro));

	print_data("arp.arp_sha: ", pArpPacket->arp.arp_sha, 6);
	print_data("arp.arp_spa: ", pArpPacket->arp.arp_spa, 4);
	print_data("arp.arp_tha: ", pArpPacket->arp.arp_tha, 6);
	print_data("arp.arp_tpa: ", pArpPacket->arp.arp_tpa, 4);

}
static void print_data(const char* headStr, const unsigned char* data, int len)
{
	int i = 0;
	printf("%s\n", headStr);
	for (i = 0; i < len; ++i)
	{
		printf("%02x ", data[i]);
	}
	printf("\n");
}

