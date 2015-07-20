/*
 * machine.h
 *
 *  Created on: 2015年7月19日
 *      Author: huih
 *      brief: 本文件主要是提供功能查询主机网卡相关的信息，包括网卡的名称，网卡的mac地址，网卡的ip地址，广播地址，子网掩码
 */

#ifndef HOSTINTF_H_
#define HOSTINTF_H_

typedef struct machine_intf
{
#define IFR_NAME_MAX_LENGTH 50
#define MAC_ADDRESS_MAX_LENGTH 6
#define IP_MAX_LENGTH 4
	char intf_name[IFR_NAME_MAX_LENGTH];
	unsigned char mac_addr[MAC_ADDRESS_MAX_LENGTH]; //0xbe 0x32 0x45 0x65 0x55 0x32
	unsigned char ip_addr[IP_MAX_LENGTH]; //172 148 23 56
	unsigned char broadcast_addr[IP_MAX_LENGTH];
	unsigned char netmask_addr[IP_MAX_LENGTH];
	int is_iff_promsic;//是否为混杂模式, 0:非混杂模式 1：混杂模式
	int is_interface_status; //网卡的状态， 0：没有启动状态 1：启动状态
}MachineIntf;

#define MAXINTERFACES 16 //最大接口数
MachineIntf machineInfo[MAXINTERFACES];  //当ifr_name为空得时候结束

/*
 * @brief 提供查询网卡的名称、mac地址、ip地址、广播地址、子网掩码、是否为混杂模式、网卡的状态信息
 * 如果获取成功，则在pMachineInfo结构中保存网卡的信息，并且以ifr_name作为网卡的结束。如果网卡的
 * 数量超过pMachineinfo结构能够保存的数量，则只返回pMachineInfo能够保存的网卡的信息，其他的丢弃。
 * pMachineInfo的大小应该为MAXINTERFACES
 * @param pMachineInfo 用于保存上面信息的结构
 * *      intfLen      表示pMachineinfo结构的大小
 * @return 成功返回获取网卡的数量，失败则返回-1.
 * */
int query_machineInfo(MachineIntf *pMachineInfo, int intfLen);

/***
 * @brief 获取指定网卡的ip地址,返回的地址格式类似为：127.0.0.1
 * @param interfaceName 网卡名称
 * @return 成功返回ip地址，失败返回null
 * **/
char* query_interfaceIp(const char* interfaceName);
unsigned char* query_interfaceIpUC(const char* interfaceName);

/***
 * @brief 获取指定网卡的mac地址,返回的地址格式类似为：ae.be.11.32.54.43
 * @param interfaceName 网卡名称
 * @return 成功返回mac地址，失败返回null
 * **/
char* query_interfaceMac(const char* interfaceName);
unsigned char* query_interfaceMacUC(const char* interfaceName);

/***
 * @brief 获取指定网卡的广播地址,返回的地址格式类似为：125.125.125.125
 * @param interfaceName 网卡名称
 * @return 成功返回广播地址，失败返回null
 * **/
char* query_interfaceBroadCast(const char* interfaceName);

/***
 * @brief 获取指定网卡的子网掩码地址,返回的地址格式类似为：255.255.255.0
 * @param interfaceName 网卡名称
 * @return 成功返回子网掩码地址，失败返回null
 * **/
char* query_interfaceNetMask(const char* interfaceName);

/**
 * @brief 根据网卡接口名称获取接口的索引值
 * @param interfaceName 网卡的名称
 * @return 成功返回网卡的索引值，否则返回-1.
 */
int get_interfaceIndex(const char* interfaceName);
#endif /* HOSTINTF_H_ */
