/*
 * base.h
 *
 *  Created on: 2015年7月19日
 *      Author: April
 */

#ifndef BASE_H_
#define BASE_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
        #include <netpacket/packet.h>
        #include <net/ethernet.h>
#else
        #include <asm/types.h>
        #include <linux/if_packet.h>
        #include <linux/if_ether.h>
#endif

#endif /* BASE_H_ */
