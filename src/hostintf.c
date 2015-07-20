/*
 * machine.c
 *
 *  Created on: 2015年7月19日
 *      Author: huih
 */

#include "base.h"
#include "hostintf.h"

int query_machineInfo(MachineIntf *pMachineInfo, int intfLen)
{
	register int fd, intrface, curInf;
	struct ifreq buf[MAXINTERFACES];
	struct ifconf ifc;

	//创建socket文件描述符
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("create socket error, errno： %d, errstr: %s\n", errno, strerror(errno));
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t) buf;

	if (ioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0)//siocgifconf只能获取ip层的网络信息，如果没有配置ip，则获取不到网卡的信息
	{
		printf("ioctl error, errno: %d, errstr: %s\n", errno, strerror(errno));
		return -1;
	}

	//获取接口数量信息
	intrface = ifc.ifc_len / sizeof (struct ifreq);

	//根据接口信息循环获取设备IP和MAC地址
	curInf = 0;
	while ( ((intrface--) > 0) && (curInf < intfLen))
	{
		//获取设备名称
		printf ("net device %s\n", buf[intrface].ifr_name);
		(pMachineInfo + curInf)->is_iff_promsic = 0; //设置为非混杂模式
		(pMachineInfo + curInf)->is_interface_status = 0; //设置网卡为启用状态

		//判断网卡类型
		if (ioctl (fd, SIOCGIFFLAGS, (char *) &buf[intrface]) < 0)
		{
			printf("ioctl error, errno: %d, errstr: %s\n", errno, strerror(errno));
			continue;
		}
		//判断是否为混杂模式
		if (buf[intrface].ifr_flags & IFF_PROMISC)
		{
			(pMachineInfo + curInf) -> is_iff_promsic = 1;
		}
		//判断网卡状态
		if (buf[intrface].ifr_flags & IFF_UP)
		{
			(pMachineInfo + curInf)->is_interface_status = 1;
			printf("the interface status is UP\n");
		}

		//获取当前网卡的IP地址
		if (ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]) < 0)
		{
			printf("ioctl error, errno: %d, errstr: %s\n", errno, strerror(errno));
			continue;
		}
		{
			struct in_addr ipAddr = ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr;
			(pMachineInfo + curInf)->ip_addr[3] = (ipAddr.s_addr >> 24) & 0xff;
			(pMachineInfo + curInf)->ip_addr[2] = (ipAddr.s_addr >> 16) & 0xff;
			(pMachineInfo + curInf)->ip_addr[1] = (ipAddr.s_addr >> 8) & 0xff;
			(pMachineInfo + curInf)->ip_addr[0] = (ipAddr.s_addr) & 0xff;
		}

		if (ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface]) < 0)
		{
			printf("get mac address error, errno: %d, errstr: %s\n", errno, strerror(errno));
			continue;
		}
		{
			(pMachineInfo + curInf)->mac_addr[0] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[0];
			(pMachineInfo + curInf)->mac_addr[1] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[1];
			(pMachineInfo + curInf)->mac_addr[2] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[2];
			(pMachineInfo + curInf)->mac_addr[3] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[3];
			(pMachineInfo + curInf)->mac_addr[4] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[4];
			(pMachineInfo + curInf)->mac_addr[5] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[5];
		}

		//子网掩码
		 if (ioctl(fd, SIOCGIFNETMASK, (char *) &buf[intrface]) < 0)
		 {
			 printf("ioctl error, errno: %d, errstr: %s\n", errno, strerror(errno));
			 continue;
		 }
		{
			struct in_addr ipAddr = ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr;
			(pMachineInfo + curInf)->netmask_addr[3] = (ipAddr.s_addr >> 24) & 0xff;
			(pMachineInfo + curInf)->netmask_addr[2] = (ipAddr.s_addr >> 16) & 0xff;
			(pMachineInfo + curInf)->netmask_addr[1] = (ipAddr.s_addr >> 8) & 0xff;
			(pMachineInfo + curInf)->netmask_addr[0] = (ipAddr.s_addr) & 0xff;
		}

	   //广播地址
	   if (ioctl(fd, SIOCGIFBRDADDR, (char *) &buf[intrface]) < 0)
	   {
		   printf("get broadcast address error, errno: %d, errstr: %s\n", errno, strerror(errno));
		   continue;
	   }
	   {
		   struct in_addr ipAddr = ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr;
		   (pMachineInfo + curInf)->broadcast_addr[3] = (ipAddr.s_addr >> 24) & 0xff;
		   (pMachineInfo + curInf)->broadcast_addr[2] = (ipAddr.s_addr >> 16) & 0xff;
		   (pMachineInfo + curInf)->broadcast_addr[1] = (ipAddr.s_addr >> 8) & 0xff;
		   (pMachineInfo + curInf)->broadcast_addr[0] = (ipAddr.s_addr) & 0xff;
	   }

	   strncpy((pMachineInfo + (curInf++))->intf_name, buf[intrface].ifr_name, IFR_NAME_MAX_LENGTH);
	} //while
	close (fd);

	return 0;
}


char* query_interfaceIp(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	printf("ifr_name: %s\n",strcpy(ifr.ifr_name, interfaceName));
	if(ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return NULL;
	}
	close(skfd);

	struct in_addr addr = ((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr;
	return inet_ntoa(addr);
}

unsigned char* query_interfaceIpUC(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;
	unsigned char* ip = NULL;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	printf("ifr_name: %s\n",strcpy(ifr.ifr_name, interfaceName));
	if(ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return ip;
	}
	close(skfd);

	struct in_addr addr = ((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr;
	ip = (unsigned char*) malloc(sizeof(unsigned char) * 4);
	if (ip == NULL)
	{
		printf("malloc ip error\n");
		return ip;
	}
	ip[3] = (unsigned char)(addr.s_addr >> 24) & 0xff;
	ip[2] = (unsigned char)(addr.s_addr >> 16) & 0xff;
	ip[1] = (unsigned char)(addr.s_addr >> 8) & 0xff;
	ip[0] = (unsigned char)(addr.s_addr ) & 0xff;

	return ip;
}


char* query_interfaceMac(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;
	char *mac;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	strcpy(ifr.ifr_name, interfaceName);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return NULL;
	}
	close(skfd);

	mac = (char *)malloc(sizeof(char) * 18);
	if (mac == NULL)
	{
		printf("malloc mac memory error\n");
		return NULL;
	}

	memset(mac, 0, 18);
	snprintf(mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
	mac[17] = '\0';

	return mac;
}

unsigned char* query_interfaceMacUC(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;
	unsigned char *mac = NULL;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	strcpy(ifr.ifr_name, interfaceName);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return mac;
	}
	close(skfd);

	mac = (unsigned char *)malloc(sizeof(unsigned char) * ETH_ALEN);
	if (mac == NULL)
	{
		printf("malloc mac memory error\n");
		return mac;
	}

	memset(mac, 0xff, ETH_ALEN);
	mac[0] = (unsigned char)ifr.ifr_hwaddr.sa_data[0];
	mac[1] = (unsigned char)ifr.ifr_hwaddr.sa_data[1];
	mac[2] = (unsigned char)ifr.ifr_hwaddr.sa_data[2];
	mac[3] = (unsigned char)ifr.ifr_hwaddr.sa_data[3];
	mac[4] = (unsigned char)ifr.ifr_hwaddr.sa_data[4];
	mac[5] = (unsigned char)ifr.ifr_hwaddr.sa_data[5];
	return mac;
}

char* query_interfaceBroadCast(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	strcpy(ifr.ifr_name, interfaceName);
	if(ioctl(skfd, SIOCGIFBRDADDR, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return NULL;
	}
	close(skfd);

	struct in_addr addr = ((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr;
	return inet_ntoa(addr);
}

char* query_interfaceNetMask(const char* interfaceName)
{
	int skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	//指定网卡名称
	strcpy(ifr.ifr_name, interfaceName);
	if(ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0)
	{
		printf("ioctl error, errno: %d, errnostr: %s\n", errno, strerror(errno));
		return NULL;
	}
	close(skfd);

	struct in_addr addr = ((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr;
	return inet_ntoa(addr);
}


int get_interfaceIndex(const char* interfaceName)
{
	struct ifreq req;
	int skfd = 0;

	//创建原始socket
	skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP));

	//查询索引值
	strcpy(req.ifr_name, interfaceName);
	if (ioctl(skfd, SIOCGIFINDEX, &req) < 0)
	{
		printf("ioctl error, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return req.ifr_ifindex;
}
