/*
 * arp.h
 *
 *  Created on: 2015年7月17日
 *      Author: April
 */

#ifndef ARP_H_
#define ARP_H_

/**
 *@brief 发送广播包，修改ARP数据项
 *@param interfaceIndex:网卡的索引值，可以通过hostintf.h中的get_interfaceIndex函数查询得到
 *       ip 需要操作的ip地址
 *       mac 需要与ip进行绑定的物理地址
 *@return 处理成功返回0， 否则返回-1
 * **/
int send_broadCastReplyPackage(const int interfaceIndex, const unsigned char*ip, const unsigned char* mac);

/**
 * @brief 查询局域网中其他主机的mac地址
 * @param interfaceName 是网卡的名字，比如:eth0,可以通过hostintf.h中的query_machineInfo函数查询得到所有的接口名称
 * @return 成功返回目标主机的mac地址，失败则返回null
 * **/
char* query_destMacAddr(const char* interfaceName, const unsigned char* destIp);

#endif /* ARP_H_ */
