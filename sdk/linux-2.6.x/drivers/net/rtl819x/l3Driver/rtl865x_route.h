/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : route table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_ROUTE_H
#define	RTL865X_ROUTE_H

#include <net/rtl/rtl865x_route_api.h>
#include "../common/rtl865x_netif_local.h"
#include "rtl865x_ppp_local.h"

typedef struct rtl865x_route_s
{
	struct rtl865x_route_s *next;
	ipaddr_t 	ipAddr;						/* Destination IP Address */
	ipaddr_t	ipMask;						/* Network mask */
	ipaddr_t 	nextHop;					/* next hop IP address */
	ipaddr_t	srcIp;						/* source IP address,only for multiple wan now*/
	uint32	valid:1, 		
			process:4,					/* 000: PPPoE, 001: L2, 010: ARP, 100: CPU, 101: NextHop, 110: Drop*/
			asicIdx:4,
			needInAsic:1;
				
	uint32            ref_count;                       /*referrence count*/
	char 	dstNetif[IFNAMSIZ];			/*destination network interface*/

	
	/* nexthop informaiton */
	union {
		struct
		{
			uint32	arpsta;						/* ARP Table Starting address */
			uint32	arpend;						/* ARP Table Ending address */
			uint32	arpIpIdx;					/* External IP selection index */
		} arp;

		struct 
		{
			void *macInfo;	/*direct nexthop's mac information*/
		}direct;

		struct
		{
			void *macInfo; /*pppoe server's mac information*/
			rtl865x_ppp_t *pppInfo;			
		}pppoe;
		
		struct
		{
			
			uint32 nxtHopSta;	/* pointer to Nexthop table: starting range */
			uint32 nxtHopEnd;		/* pointer to Nexthop table: ending range */				
			uint8 nhalog;							/* algo. for load balance */
			uint8 ipDomain;						/* IP domain */
		} nxthop;
		
	} un;

} rtl865x_route_t;

#define RT_DEFAULT_RT_NEXTHOP_CPU 		0x00
#define RT_DEFAULT_RT_NEXTHOP_NORMAL 	0x01

/* process: */
#define RT_PPPOE				0x00
#define RT_L2					0x01
#define RT_ARP					0x02
#define RT_CPU					0x04
#define RT_NEXTHOP				0x05
#define RT_DROP					0x06

/* nhalog: */
#define RT_ALOG_PACKET			0x00
#define RT_ALOG_SESSION		0x01
#define RT_ALOG_SIP				0x02

/* ipDomain: */
#define RT_DOMAIN_4_1				0x00
#define RT_DOMAIN_4_2				0x01
#define RT_DOMAIN_4_3				0x02
#define RT_DOMAIN_4_4				0x03
#define RT_DOMAIN_8_1				0x04
#define RT_DOMAIN_8_2				0x05
#define RT_DOMAIN_16_1				0x06

int32 rtl865x_initRouteTable(void);
int32 rtl865x_reinitRouteTable(void);

//int32 rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop, int8 * ifName);
//int32 rtl865x_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask);
int32 rtl865x_getRouteEntry(ipaddr_t dst, rtl865x_route_t *rt);
void rtl865x_refreshRouteArpExtIP(char* netifname,u32 extip);

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int rtl865x_getLanRoute(rtl865x_route_t routeTbl[], int tblSize);
#endif
#ifdef CONFIG_RTL_MULTI_ETH_WAN
extern int32 rtl865x_syncRouteToAsic(void);
extern int rtl865x_clearAsicRoutingTable(void);
#endif
extern rtl865x_route_t * rtl865x_getRouteList(void);


#endif
