/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer2 switch driver,following features are included:
*	PHY/MII/Port/STP/QOS
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
 
#ifndef RTL865X_ASICL2_H
#define RTL865X_ASICL2_H

// Use sw gpio to control led of Port0~Port4
//#define PATCH_GPIO_FOR_LED		1

#include "rtl865x_asicCom.h"
#define RTL865XC_QM_DESC_READROBUSTPARAMETER	10

 #define RTL8651_MII_PORTNUMBER                 	5
#define RTL8651_MII_PORTMASK                    	0x20
#define RTL8651_PHY_NUMBER				5

#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL	0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03
#define RTL8651_ETHER_AUTO_1000FULL	0x08
#define RTL8651_ETHER_AUTO_1000HALF	0x09
/* chhuang: patch for priority issue */
#define RTL8651_ETHER_FORCE_100FULL	0x04
#define RTL8651_ETHER_FORCE_100HALF	0x05
#define RTL8651_ETHER_FORCE_10FULL	0x06
#define RTL8651_ETHER_FORCE_10HALF	0x07

#define RTL865XC_MNQUEUE_OUTPUTQUEUE  1
#define RTL865XC_QOS_OUTPUTQUEUE 1

#if defined(RTL865XC_MNQUEUE_OUTPUTQUEUE) || defined(RTL865XC_QOS_OUTPUTQUEUE)
typedef struct rtl865xC_outputQueuePara_s {

	uint32	ifg;							/* default: Bandwidth Control Include/exclude Preamble & IFG */
	uint32	gap;							/* default: Per Queue Physical Length Gap = 20 */
	uint32	drop;						/* default: Descriptor Run Out Threshold = 500 */

	uint32	systemSBFCOFF;				/*System shared buffer flow control turn off threshold*/
	uint32	systemSBFCON;				/*System shared buffer flow control turn on threshold*/

	uint32	systemFCOFF;				/* system flow control turn off threshold */
	uint32	systemFCON;					/* system flow control turn on threshold */

	uint32	portFCOFF;					/* port base flow control turn off threshold */
	uint32	portFCON;					/* port base flow control turn on threshold */	

	uint32	queueDescFCOFF;				/* Queue-Descriptor=Based Flow Control turn off Threshold  */
	uint32	queueDescFCON;				/* Queue-Descriptor=Based Flow Control turn on Threshold  */

	uint32	queuePktFCOFF;				/* Queue-Packet=Based Flow Control turn off Threshold  */
	uint32	queuePktFCON;				/* Queue-Packet=Based Flow Control turn on Threshold  */
}	rtl865xC_outputQueuePara_t;
#endif


/*enum for duplex and speed*/
enum 
{
	PORT_DOWN=0,
	HALF_DUPLEX_10M,
	HALF_DUPLEX_100M,
	HALF_DUPLEX_1000M,
	DUPLEX_10M,
	DUPLEX_100M,
	DUPLEX_1000M,
	PORT_AUTO
};

/* enum for port ID */
enum PORTID
{
	PHY0 = 0,
	PHY1 = 1,
	PHY2 = 2,
	PHY3 = 3,
	PHY4 = 4,
	PHY5 = 5,
	CPU = 6,
	EXT1 = 7,
	EXT2 = 8,
	EXT3 = 9,
	MULTEXT = 10,
};

/* enum for queue ID */
enum QUEUEID
{
	QUEUE0 = 0,
	QUEUE1,
	QUEUE2,
	QUEUE3,
	QUEUE4,
	QUEUE5,
};

/* enum for queue type */
enum QUEUETYPE
{
	STR_PRIO = 0,
	WFQ_PRIO,
};

/* enum for output queue number */
enum QUEUENUM
{
	QNUM1 = 1,
	QNUM2,
	QNUM3,
	QNUM4,
	QNUM5,
	QNUM6,
};

/* enum for priority value type */
enum PRIORITYVALUE
{
	PRI0 = 0,
	PRI1,
	PRI2,
	PRI3,
	PRI4,
	PRI5,
	PRI6,
	PRI7,
};

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint16          mac39_24;
    uint16          mac23_8;

    /* word 1 */
    uint32          reserv0: 6;
    uint32          auth: 1;
    uint32          fid:2;
    uint32          nxtHostFlag : 1;
    uint32          srcBlock    : 1;
    uint32          agingTime   : 2;
    uint32          isStatic    : 1;
    uint32          toCPU       : 1;
    uint32          extMemberPort   : 3;
    uint32          memberPort : 6;
    uint32          mac47_40    : 8;

#else /*LITTLE_ENDIAN*/
    /* word 0 */
    uint16          mac23_8;
    uint16          mac39_24;
		
    /* word 1 */
    uint32          mac47_40    : 8;
    uint32          memberPort : 6;
    uint32          extMemberPort   : 3;
    uint32          toCPU       : 1;
    uint32          isStatic    : 1;
    uint32          agingTime   : 2;
    uint32          srcBlock    : 1;
    uint32          nxtHostFlag : 1;
    uint32          fid:2;
    uint32          auth:1;	
    uint32          reserv0:6;	

#endif /*LITTLE_ENDIAN*/
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_l2Table_t;



typedef struct rtl865x_tblAsicDrv_l2Param_s {
	ether_addr_t	macAddr;
	uint32 		memberPortMask; /*extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]*/
	uint32 		ageSec;
	uint32	 	cpu:1,
				srcBlk:1,
				isStatic:1,				
				nhFlag:1,
				fid:2,
				auth:1;

} rtl865x_tblAsicDrv_l2Param_t;

typedef struct rtl8651_tblAsic_ethernet_s {
	uint8	linkUp: 1,
			phyId: 5,
			isGPHY: 1;
} rtl8651_tblAsic_ethernet_t;

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
typedef struct rtl865x_tblAsicDrv_rateLimitParam_s {
	uint32 	token;
	uint32	maxToken;
	uint32	t_remainUnit;
	uint32 	t_intervalUnit;
	uint32	refill_number;
} rtl865x_tblAsicDrv_rateLimitParam_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 2;
    uint32          refillRemainTime    : 6;
    uint32          token       : 24;
    /* word 1 */
    uint32          reserv1     : 2;
    uint32          refillTime  : 6;
    uint32          maxToken    : 24;
    /* word 2 */
    uint32          reserv2     : 8;
    uint32          refill      : 24;
#else
    /* word 0 */
    uint32          token       : 24;
    uint32          refillRemainTime    : 6;
    uint32          reserv0     : 2;
    /* word 1 */
    uint32          maxToken    : 24;
    uint32          refillTime  : 6;
    uint32          reserv1     : 2;
    /* word 2 */
    uint32          refill      : 24;
    uint32          reserv2     : 8;
#endif /*_LITTLE_ENDIAN*/
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl8651_tblAsic_rateLimitTable_t;
#endif

extern int32 rtl865x_maxPreAllocRxSkb;
extern int32 rtl865x_rxSkbPktHdrDescNum;
extern int32 rtl865x_txSkbPktHdrDescNum;

extern int32	rtl865x_wanPortMask;
extern int32	rtl865x_lanPortMask;

int32 rtl865x_initAsicL2(rtl8651_tblAsic_InitPara_t *para);
int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p);
int32 rtl8651_delAsicL2Table(uint32 row, uint32 column);
unsigned int rtl8651_asicL2DAlookup(uint8 *dmac);
int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p);
int32 rtl8651_getAsicL2Table_ALL(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p);
int32 rtl8651_getAsicPortMirror(uint32 *mRxMask, uint32 *mTxMask, uint32 *mPortMask);
int32 rtl8651_setAsicPortMirror(uint32 mRxMask, uint32 mTxMask,uint32 mPortMask);
uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid);

int32 rtl8651_setAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag, int8 fid, int8 auth);
int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *asic_l2_t);

int32 rtl8651_setAsicPortPatternMatch(uint32 port, uint32 pattern, uint32 patternMask, int32 operation);
int32 rtl8651_getAsicPortPatternMatch(uint32 port, uint32 *pattern, uint32 *patternMask, int32 *operation);

int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled);
int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled);
int32 rtl8651_asicEthernetCableMeterInit(void);

int32 rtl865xC_setAsicSpanningTreePortState(uint32 port, uint32 portState);
int32 rtl865xC_getAsicSpanningTreePortState(uint32 port, uint32 *portState);
int32 rtl8651_setAsicMulticastSpanningTreePortState(uint32 port, uint32 portState);
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState);

int32 rtl865xC_setAsicPortPauseFlowControl(uint32 port, uint8 rxEn, uint8 txEn);
int32 rtl865xC_getAsicPortPauseFlowControl(uint32 port, uint8 *rxEn, uint8 *txEn);

int32 rtl8651_restartAsicEthernetPHYNway(uint32 port);

/*MDC/MDIO*/
int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData);
int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData);

int32 rtl865xC_setAsicEthernetMIIMode(uint32 port, uint32 mode);
int32 rtl865xC_setAsicEthernetRGMIITiming(uint32 port, uint32 Tcomp, uint32 Rcomp);
int32 rtl8651_setAsicEthernetMII(uint32 phyAddress, int32 mode, int32 enabled);
int32 rtl8651_getAsicEthernetMII(uint32 *phyAddress);

int32 rtl865x_setStormControl(uint32 type,uint32 enable,uint32 percentage);

/*flow control*/
int32 rtl8651_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri, uint32 natpri );
int32 rtl8651_setAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 enable);
int32 rtl8651_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold );
int32 rtl8651_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold );
int32 rtl8651_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr );
int32 rtl8651_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr );
int32 rtl8651_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth);
int32 rtl8651_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth );
int32 rtl8651_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth );
int32 rtl8651_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth );
int32 rtl8651_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 weight );
int32 rtl8651_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight );
int32 rtl8651_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum );
int32 rtl8651_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum );
int32 rtl8651_EnablePortRemark_8021p( enum PORTID port);
int32 rtl8651_DisablePortRemark_8021p( enum PORTID port);
int32 rtl8651_EnablePortRemark_dscp( enum PORTID port);
int32 rtl8651_DisablePortRemark_dscp( enum PORTID port);
int32 rtl8651_SetPortRemark(uint8 aclpriority,uint8 value_8021p,uint8 value_dscp);
int32 rtl8651_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid );
int32 rtl8651_setAsicCPUPriorityToQIDMappingTable( enum PORTID port, enum PRIORITYVALUE priority, enum QUEUEID qid );
int32 rtl8651_setAsicSystemBasedFlowControlRegister(uint32 sharedON, uint32 sharedOFF, uint32 fcON, uint32 fcOFF, uint32 drop);
int32 rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8651_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8651_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF);
int32 rtl8651_setAsicPerQueuePhysicalLengthGapRegister(uint32 gap);
int32 rtl8651_getAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 *enable);
#ifdef CONFIG_RTL_LINKCHG_PROCESS
int32 rtl8651_setAsicEthernetLinkStatus(uint32 port, int8 linkUp);
int32  rtl8651_updateAsicLinkAggregatorLMPR(int32 portmask);
#endif
int32 rtl865xC_waitForOutputQueueEmpty(void);
 int32 rtl8651_resetAsicOutputQueue(void);

 int32 rtl8651_setAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 rate);
 int32 rtl8651_setAsicFlowControlRegister(uint32 port, uint32 enable);
 int32 rtl8651_getAsicFlowControlRegister(uint32 port, uint32 *enable);
 int32 rtl8651_setAsicSystemInputFlowControlRegister(uint32 fcON, uint32 fcOFF);
 int32 rtl8651_getAsicSystemInputFlowControlRegister(uint32 *fcON, uint32 *fcOFF);
int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex);
int32 rtl8651_setAsic802D1xMacBaseAbility( enum PORTID port, uint32 isEnable );
int32 rtl8651_setAsic802D1xMacBaseDirection(int32 dir);
int32 rtl8651_setAsicGuestVlanProcessControl( uint32 process);


/* for 802.1p qos */
int32 rtl8651_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority );
int32 rtl8651_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority );
int32 rtl8651_flushAsicDot1qAbsolutelyPriority(void);


#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
int32 rtl8651_setAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t);
int32 rtl8651_delAsicRateLimitTable(uint32 index);
int32 rtl8651_getAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t);
#endif

#endif
