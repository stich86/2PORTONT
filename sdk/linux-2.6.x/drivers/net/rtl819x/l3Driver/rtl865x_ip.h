/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : ip table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_IP_H
#define	RTL865X_IP_H

#include <net/rtl/rtl865x_ip_api.h>
#include <linux/if.h>

#if !defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#define IP_NUMBER	8
#else
#if 0//defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define IP_NUMBER	2
#else
#define IP_NUMBER	16
#endif
#endif

typedef struct rtl865x_ip_entry_s
{
	ipaddr_t intIp; /*internal ip address*/
	ipaddr_t extIp; /*external ip address*/
	uint32 valid:1, /*valid*/
		type:2,  /*napt/nat/ls/reserved*/
		ref_count:5;  /* reference count */
		//defNaptIp:1; /*this ip entry is default napt ip address?*/

#if 1
	/*now, this information is invalid...*/
	void *nexthop; /*point to a nexthop entry,in preversion SoC, 
				     *used for multiple session hardware forwarding...
				     */	
#endif

	char output_netif[IFNAMSIZ];
}rtl865x_ip_entry_t;


int32 rtl865x_initIpTable(void);
int32 rtl865x_reinitIpTable(void);
//int32 rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type);
//int32 rtl865x_delIp(ipaddr_t extIp);
int32 rtl865x_getIPIdx(rtl865x_ip_entry_t *entry, int32 *idx);
int32 rtl865x_getIpIdxByExtIp(ipaddr_t extIp, int32 *idx);
void rtl865x_extIP_show(void);

#ifdef CONFIG_RTL_MULTI_ETH_WAN
rtl865x_ip_entry_t* rtl865x_getIpEntryByIdx(int32 idx);
#endif
#endif

