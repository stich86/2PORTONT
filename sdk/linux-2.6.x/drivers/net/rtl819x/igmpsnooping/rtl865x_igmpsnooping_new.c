/*
* Copyright c                  Realsil Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program :  igmp snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/

#ifdef __linux__
#include <linux/config.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#ifdef CONFIG_PROC_FS
#include <linux/seq_file.h>
#endif
#endif


#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include "rtl865x_igmpsnooping_local.h"
//#include "../common/assert.h"




static struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];    
#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list igmpSysTimer;	/*igmp timer*/
#endif
/*global system resources declaration*/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxClientCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxSourceCnt;   /*maximum total group entry count,  default is 3000*/

void *rtl_groupMemory=NULL;
void *rtl_clientMemory=NULL;
void *rtl_sourceMemory=NULL;
void *rtl_mcastFlowMemory=NULL;
extern int  igmpsnoopenabled;
extern int mldSnoopEnabled;

static struct rtl_groupEntry *rtl_groupEntryPool=NULL;
static struct rtl_clientEntry *rtl_clientEntryPool=NULL;
static struct rtl_sourceEntry *rtl_sourceEntryPool=NULL;
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry *rtl_mcastFlowEntryPool=NULL;
#endif
static struct rtl_mCastTimerParameters rtl_mCastTimerParas;  /*IGMP snooping parameters */

static uint32 rtl_hashTableSize=0;
static uint32 rtl_hashMask=0;

/*the system up time*/
static uint32 rtl_startTime;
static uint32 rtl_sysUpSeconds;       

static rtl_multicastEventContext_t reportEventContext;
static rtl_multicastEventContext_t timerEventContext;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_multicast.h>
static rtl_multicastEventContext_t linkEventContext;
int rtl_handle_igmpgroup_change(rtl_multicastEventContext_t* param);
#endif


/*******************************internal function declaration*****************************/


/**************************
	resource managment
**************************/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;


static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize);
static  struct rtl_clientEntry* rtl_allocateClientEntry(void);
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr) ;

static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize);
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void);
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) ;
#ifdef CONFIG_RECORD_MCAST_FLOW
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize);
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void);
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry) ;
#endif
/**********************************Structure Maintenance*************************/

static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,  struct rtl_groupEntry ** hashTable);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,  struct rtl_groupEntry ** hashTable);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);


static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion,struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr);
static void rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntryPtr);
static void rtl_deleteClientEntry(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry);
static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode);
static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* entryNode);
static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr);
static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry);
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr);
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry);
static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable);
#endif
static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);

#ifdef CONFIG_RTL_MLD_SNOOPING
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr);
#endif
static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);
// Mason Yu. type error 
//static uint8 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry,  uint32 sysTime);
static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry,  uint32 sysTime);
static void rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry);
static uint32 rtl_getGroupSourceFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 * sourceAddr, uint32 sysTime);
static uint32 rtl_getClientSourceFwdPortMask(uint32 ipVersion, struct rtl_clientEntry * clientEntry, uint32 * sourceAddr, uint32 sysTime);
  
static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable);
static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime);


/*hash table operation*/
static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize);


/************************************Pkt Process**********************************/
/*MAC frame analyze function*/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* MacFrame, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo);

/*Process Query Packet*/
static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum);
static uint32 rtl_processQueries(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen);
/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); // process join report packet 
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process leave/done report packet
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet 
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static  int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet 
static  int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf);//process BLOCK_OLD_SOURCES report packet
static  uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8 *pktBuf);

/*******************different protocol process function**********************************/
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processMospf(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,  uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint8* pktBuf, uint32 pktLen);

#ifdef CONFIG_RECORD_MCAST_FLOW
static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo);
static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr);
static void rtl_doMcastFlowRecycle(uint32 moduleIndex,uint32 ipVersion);
#endif

#if  defined(__linux__) && defined(__KERNEL__)
static void rtl_multicastSysTimerExpired(uint32 expireDada);
static void rtl_multicastSysTimerInit(void);
static void rtl_multicastSysTimerDestroy(void);
#endif

static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable);
/************************************************
			Implementation
  ************************************************/
  
/**************************
	Initialize
**************************/

int32 rtl_initMulticastSnooping(struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig)
{
	int i,j;
	uint32 maxHashTableSize=MAX_HASH_TABLE_SIZE;
	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		memset(&(rtl_mCastModuleArray[i]), 0,sizeof(struct rtl_multicastModule));	
		
		for(j=0; j<6; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayMac[j]=0;
		}
		
		rtl_mCastModuleArray[i].rtl_gatewayIpv4Addr=0;
		rtl_mCastModuleArray[i].rtl_ipv4HashTable=NULL;	
		
		#ifdef CONFIG_RTL_MLD_SNOOPING	
		for(j=0; j<4; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayIpv6Addr[j]=0;
		}
		rtl_mCastModuleArray[i].rtl_ipv6HashTable=NULL;
		#endif
#ifdef CONFIG_RECORD_MCAST_FLOW		
		rtl_mCastModuleArray[i].flowHashTable=NULL;	
#endif		
		rtl_mCastModuleArray[i].enableSnooping=FALSE;
		rtl_mCastModuleArray[i].enableFastLeave=FALSE;

	}


       /*set multicast snooping parameters, use default value*/
      if(mCastSnoopingGlobalConfig.groupMemberAgingTime==0)
      {
	      rtl_mCastTimerParas.groupMemberAgingTime= DEFAULT_GROUP_MEMBER_INTERVAL;
      }
      else
      {
	      rtl_mCastTimerParas.groupMemberAgingTime= mCastSnoopingGlobalConfig.groupMemberAgingTime;
      }

      if(mCastSnoopingGlobalConfig.lastMemberAgingTime==0)
      {
             rtl_mCastTimerParas.lastMemberAgingTime= 0;
      }
      else
      {
	      rtl_mCastTimerParas.lastMemberAgingTime= mCastSnoopingGlobalConfig.lastMemberAgingTime;
      }

      if(mCastSnoopingGlobalConfig.querierPresentInterval==0)
      {
	      rtl_mCastTimerParas.querierPresentInterval= DEFAULT_QUERIER_PRESENT_TIMEOUT;
      }
	else
      {
	      rtl_mCastTimerParas.querierPresentInterval=mCastSnoopingGlobalConfig.querierPresentInterval;
      }


      if(mCastSnoopingGlobalConfig.dvmrpRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.dvmrpRouterAgingTime=DEFAULT_DVMRP_AGING_TIME;
      }
      else
      {
	      rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastSnoopingGlobalConfig.dvmrpRouterAgingTime;
      }

      if(mCastSnoopingGlobalConfig.mospfRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.mospfRouterAgingTime=DEFAULT_MOSPF_AGING_TIME;
      }
  	else
      {
	      rtl_mCastTimerParas.mospfRouterAgingTime=mCastSnoopingGlobalConfig.mospfRouterAgingTime;
      }

      if(mCastSnoopingGlobalConfig.pimRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.pimRouterAgingTime=DEFAULT_PIM_AGING_TIME;
      }
      else
      {
	      rtl_mCastTimerParas.pimRouterAgingTime=mCastSnoopingGlobalConfig.pimRouterAgingTime;
      }

	 /* set hash table size and hash mask*/
       if(mCastSnoopingGlobalConfig.hashTableSize==0)
        {
	      rtl_hashTableSize=DEFAULT_HASH_TABLE_SIZE;   /*default hash table size*/
        }
        else
        {
  	        for(i=0;i<11;i++)
	        {
		      if(mCastSnoopingGlobalConfig.hashTableSize>=maxHashTableSize)
		      {
			      rtl_hashTableSize=maxHashTableSize;
		
			      break;
		      }
	 	      maxHashTableSize=maxHashTableSize>>1;
		
	        }
        }

      rtl_hashMask=rtl_hashTableSize-1;
	  
	
      rtl_groupMemory=NULL;
      rtl_clientMemory=NULL;
      rtl_sourceMemory=NULL;
      rtl_mcastFlowMemory=NULL;
	
	/*initialize group entry pool*/
      if(mCastSnoopingGlobalConfig.maxGroupNum==0)
      {
	      rtl_totalMaxGroupCnt=DEFAULT_MAX_GROUP_COUNT;
      }	
      else
      {
		rtl_totalMaxGroupCnt=mCastSnoopingGlobalConfig.maxGroupNum;
      }

      rtl_groupEntryPool=rtl_initGroupEntryPool(rtl_totalMaxGroupCnt); 
      if(rtl_groupEntryPool==NULL)
      {
	      return FAILED;
      }
	  
	  /*initialize client entry pool*/
      if(mCastSnoopingGlobalConfig.maxClientNum==0)
      {
	      rtl_totalMaxClientCnt=DEFAULT_MAX_CLIENT_COUNT;
      }	
      else
      {
	      rtl_totalMaxClientCnt=mCastSnoopingGlobalConfig.maxClientNum;
      }

      rtl_clientEntryPool=rtl_initClientEntryPool(rtl_totalMaxClientCnt); 
      if(rtl_clientEntryPool==NULL)
      {
	      return FAILED;
      }
#ifdef CONFIG_RECORD_MCAST_FLOW
      rtl_mcastFlowEntryPool=rtl_initMcastFlowEntryPool(DEFAULT_MAX_FLOW_COUNT); 
      if(rtl_mcastFlowEntryPool==NULL)
      {
	      return FAILED;
      }
#endif
	/*initialize source entry pool*/
	if(mCastSnoopingGlobalConfig.maxSourceNum==0)
      {
	      rtl_totalMaxSourceCnt=DEFAULT_MAX_SOURCE_COUNT;
      }	
      else
      {
            	rtl_totalMaxSourceCnt=mCastSnoopingGlobalConfig.maxSourceNum;
      }
	  
	rtl_sourceEntryPool=rtl_initSourceEntryPool(rtl_totalMaxSourceCnt); 
      if(rtl_sourceEntryPool==NULL)
      {
	      rtl_totalMaxSourceCnt=0;
	      return FAILED;
      }
	  
#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerInit();
#endif

	return SUCCESS;

}
int32 rtl_flushAllIgmpRecord(void)
{
	/* maintain current time */
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			/*maintain ipv4 group entry  timer */
			if ( !igmpsnoopenabled)
			{				
				for(i=0; i<rtl_hashTableSize; i++)
				{
					/*scan the hash table*/
					if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
					{
						timerEventContext.ipVersion=IP_VERSION4;
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
						while(groupEntryPtr)              /*traverse each group list*/
						{	
							nextEntry=groupEntryPtr->next; 
							rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
							groupEntryPtr=nextEntry;
						}
					}
				}
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING		
			/*maintain ipv6 group entry  timer */
			if ( !mldSnoopEnabled)
			{				
				for(i=0; i<rtl_hashTableSize; i++)
				{
					/*scan the hash table*/
					if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
					{
						timerEventContext.ipVersion=IP_VERSION6;
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
						while(groupEntryPtr)              /*traverse each group list*/
						{	
							nextEntry=groupEntryPtr->next; 
							rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
							groupEntryPtr=nextEntry;
						}
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}
static inline uint32 rtl_igmpHashAlgorithm(uint32 ipVersion,uint32 *groupAddr)
{
	uint32 hashIndex=0;
	
	if(ipVersion==IP_VERSION4)
	{
		/*to do:change hash algorithm*/
		hashIndex=rtl_hashMask&groupAddr[0];
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		hashIndex=rtl_hashMask&groupAddr[3];
	}
#endif

	return hashIndex;
}

int32 rtl_exitMulticastSnooping(void)
{

	uint32 moduleIndex;
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		rtl_unregisterIgmpSnoopingModule(moduleIndex);
	}

	rtl_hashTableSize=0;
	rtl_hashMask=0;
	memset(&rtl_mCastTimerParas,0,sizeof(struct rtl_mCastTimerParameters));
	
	if(rtl_groupMemory!=NULL)
	{
		rtl_glueFree(rtl_groupMemory);	
	}
	
	rtl_totalMaxGroupCnt=0;
	rtl_groupMemory=NULL;
	rtl_groupEntryPool=NULL;
	
	if(rtl_clientMemory!=NULL)
	{
		rtl_glueFree(rtl_clientMemory);	
	}
	
	rtl_totalMaxClientCnt=0;
	rtl_clientMemory=NULL;
	rtl_clientEntryPool=NULL;

	if(rtl_sourceMemory!=NULL)
	{
		rtl_glueFree(rtl_sourceMemory);
	}	  

	rtl_totalMaxSourceCnt=0;
	rtl_sourceMemory=NULL;
	rtl_sourceEntryPool=NULL;

#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerDestroy();
#endif

	 return SUCCESS;
	
}

/*group entry memory management*/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_groupEntry *poolHead=NULL;
	struct rtl_groupEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_groupEntry *)rtl_glueMalloc(sizeof(struct rtl_groupEntry) * poolSize);
	rtl_groupMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_groupEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}
	
out:

	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;
	
}

// allocate a group entry from the group entry pool
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void)
{
	struct rtl_groupEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_groupEntryPool!=NULL)
		{
			ret = rtl_groupEntryPool;
			if(rtl_groupEntryPool->next!=NULL)
			{
				rtl_groupEntryPool->next->previous=NULL;
			}
			rtl_groupEntryPool = rtl_groupEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_groupEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a group entry and link it back to the group entry pool, default is link to the pool head
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) 
{
	if (!groupEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
		groupEntryPtr->next = rtl_groupEntryPool;
		if(rtl_groupEntryPool!=NULL)
		{
			rtl_groupEntryPool->previous=groupEntryPtr;
		}
		rtl_groupEntryPool=groupEntryPtr;	
	rtl_glueMutexUnlock();	
}

/*client entry memory management*/
static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_clientEntry *poolHead=NULL;
	struct rtl_clientEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_clientEntry *)rtl_glueMalloc(sizeof(struct rtl_clientEntry) * poolSize);
	rtl_clientMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_clientEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}
	
out:

	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;
	
}


// allocate a client entry from the client entry pool
static  struct rtl_clientEntry* rtl_allocateClientEntry(void)
{
	struct rtl_clientEntry *ret = NULL;

	rtl_glueMutexLock();	
	if (rtl_clientEntryPool!=NULL)
	{
		ret = rtl_clientEntryPool;
		if(rtl_clientEntryPool->next!=NULL)
		{
			rtl_clientEntryPool->next->previous=NULL;
		}
		rtl_clientEntryPool = rtl_clientEntryPool->next;
		memset(ret, 0, sizeof(struct rtl_clientEntry));
	}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a client entry and link it back to the client entry pool, default is link to the pool head
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr) 
{
	if (!clientEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
	clientEntryPtr->next = rtl_clientEntryPool;
	if(rtl_clientEntryPool!=NULL)
	{
		rtl_clientEntryPool->previous=clientEntryPtr;
	}
	rtl_clientEntryPool=clientEntryPtr;	
	rtl_glueMutexUnlock();	
}

/*source entry memory management*/
static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize)
{

	uint32 idx=0;
	struct rtl_sourceEntry *poolHead=NULL;
	struct rtl_sourceEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_sourceEntry *)rtl_glueMalloc(sizeof(struct rtl_sourceEntry) * poolSize);
	rtl_sourceMemory=(void *)poolHead;
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_sourceEntry)));
		entryPtr = poolHead;

		/* link the whole source entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			
		}
	}
	
out:
	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;

}


// allocate a source entry from the source entry pool
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void)
{
	struct rtl_sourceEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_sourceEntryPool!=NULL)
		{	
			ret = rtl_sourceEntryPool;
			if(rtl_sourceEntryPool->next!=NULL)
			{
				rtl_sourceEntryPool->next->previous=NULL;
			}
			rtl_sourceEntryPool = rtl_sourceEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_sourceEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a source entry and link it back to the source entry pool, default is link to the pool head
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) 
{
	if (!sourceEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
		sourceEntryPtr->next = rtl_sourceEntryPool;
		if(rtl_sourceEntryPool!=NULL)
		{
			rtl_sourceEntryPool->previous=sourceEntryPtr;
		}

		rtl_sourceEntryPool=sourceEntryPtr;	

	rtl_glueMutexUnlock();	
}
#ifdef CONFIG_RECORD_MCAST_FLOW
/*multicast flow entry memory management*/
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_mcastFlowEntry *poolHead=NULL;
	struct rtl_mcastFlowEntry *entryPtr=NULL;
	
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_mcastFlowEntry *)rtl_glueMalloc(sizeof(struct rtl_mcastFlowEntry) * poolSize);
	rtl_mcastFlowMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_mcastFlowEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}
	
out:

	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;
	
}


// allocate a multicast flow entry  from the multicast flow pool
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void)
{
	struct rtl_mcastFlowEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_mcastFlowEntryPool!=NULL)
		{
			ret = rtl_mcastFlowEntryPool;
			if(rtl_mcastFlowEntryPool->next!=NULL)
			{
				rtl_mcastFlowEntryPool->next->previous=NULL;
			}
			rtl_mcastFlowEntryPool = rtl_mcastFlowEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_mcastFlowEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a multicast flow entry and link it back to the multicast flow entry pool, default is link to the pool head
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry) 
{
	if (NULL==mcastFlowEntry)
	{
		return;
	}
		
	rtl_glueMutexLock();	
	mcastFlowEntry->next = rtl_mcastFlowEntryPool;
	if(rtl_mcastFlowEntryPool!=NULL)
	{
		rtl_mcastFlowEntryPool->previous=mcastFlowEntry;
	}
	rtl_mcastFlowEntryPool=mcastFlowEntry;	
	rtl_glueMutexUnlock();	
}

#endif

/*********************************************
			Group list operation
 *********************************************/

/*       find a group address in a group list    */

struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr)
{
	struct rtl_groupEntry* groupPtr = NULL;
	int32 hashIndex=0;

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, multicastAddr);
	
	if(ipVersion==IP_VERSION4)
	{
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
	}
#endif	

	while (groupPtr!=NULL)
	{	
		if(ipVersion==IP_VERSION4)
		{
			if(multicastAddr[0]==groupPtr->groupAddr[0])
			{
				return groupPtr;
			}
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			if(	(multicastAddr[0]==groupPtr->groupAddr[0])&&
				(multicastAddr[1]==groupPtr->groupAddr[1])&&
				(multicastAddr[2]==groupPtr->groupAddr[2])&&
				(multicastAddr[3]==groupPtr->groupAddr[3])
			)
			{
				return groupPtr;
				
			}
		}
#endif	
		groupPtr = groupPtr->next;

	}

	return NULL;
}


/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* groupEntry ,  struct rtl_groupEntry ** hashTable)
{
	uint32 hashIndex=0;
	
	rtl_glueMutexLock();//Lock resource
	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	if(NULL==groupEntry)
	{
		return;
	}
	
	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=groupEntry;
	}
	groupEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=groupEntry;
	hashTable[hashIndex]->previous=NULL;
		
	rtl_glueMutexUnlock();//UnLock resource

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* groupEntry,  struct rtl_groupEntry ** hashTable)
{	
	uint32 hashIndex=0;
	
	if(NULL==groupEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	

	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	/* unlink entry node*/
	if(groupEntry==hashTable[hashIndex]) /*unlink group list head*/
	{
		hashTable[hashIndex]=groupEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
	}
	else
	{
		if(groupEntry->previous!=NULL)
		{
			groupEntry->previous->next=groupEntry->next;
		}
		 
		if(groupEntry->next!=NULL)
		{
			groupEntry->next->previous=groupEntry->previous;
		}
	}
	
	groupEntry->previous=NULL;
	groupEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
}


/* clear the content of group entry */
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntry)
{
	rtl_glueMutexLock();
	if (NULL!=groupEntry)
	{
		memset(groupEntry, 0, sizeof(struct rtl_groupEntry));
	}
	rtl_glueMutexUnlock();
}

/*********************************************
			Client list operation
 *********************************************/
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion, struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr)
{
	struct rtl_clientEntry* clientPtr = groupEntry->clientList;

	if(clientAddr==NULL)
	{
		return NULL;
	}
	while (clientPtr!=NULL)
	{	
		if(ipVersion==IP_VERSION4)
		{
			if((clientPtr->clientAddr[0]==clientAddr[0]))
			{
				if(portNum<MAX_SUPPORT_PORT_NUMBER) 
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
				}
				return clientPtr;
			}
			
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			if(	((clientPtr->clientAddr[0]==clientAddr[0])
				&&(clientPtr->clientAddr[1]==clientAddr[1])
				&&(clientPtr->clientAddr[2]==clientAddr[2])
				&&(clientPtr->clientAddr[3]==clientAddr[3])))
			{
		
				if(portNum<MAX_SUPPORT_PORT_NUMBER) 
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
				}
				return clientPtr;
			}
		}
#endif	
		clientPtr = clientPtr->next;

	}

	return NULL;
}


/* link client Entry in the front of group client list */
static void  rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry )
{
	rtl_glueMutexLock();//Lock resource
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}


	if(groupEntry->clientList!=NULL)
	{
		groupEntry->clientList->previous=clientEntry;
	}
	clientEntry->next = groupEntry->clientList;
	
	groupEntry->clientList=clientEntry;
	groupEntry->clientList->previous=NULL;
		
	rtl_glueMutexUnlock();//UnLock resource

}


/* unlink a client entry from group client list */
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry)
{	
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	
	
	/* unlink entry node*/
	if(clientEntry==groupEntry->clientList) /*unlink group list head*/
	{
		groupEntry->clientList=groupEntry->clientList->next;
		if(groupEntry->clientList!=NULL)
		{
			groupEntry->clientList->previous=NULL;
		}

	}
	else
	{
		if(clientEntry->previous!=NULL)
		{
			clientEntry->previous->next=clientEntry->next;
		}
		 
		if(clientEntry->next!=NULL)
		{
			clientEntry->next->previous=clientEntry->previous;
		}
	}
	
	clientEntry->previous=NULL;
	clientEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
}


/* clear the content of client entry */
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntry)
{
	rtl_glueMutexLock();
	if (NULL!=clientEntry)
	{
		memset(clientEntry, 0, sizeof(struct rtl_clientEntry));
	}
	rtl_glueMutexUnlock();
}


/*********************************************
			source list operation
 *********************************************/
static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry)
{
	struct rtl_sourceEntry *sourcePtr=clientEntry->sourceList;
	while(sourcePtr!=NULL)
	{
		if(ipVersion==IP_VERSION4)
		{
			if(sourceAddr[0]==sourcePtr->sourceAddr[0])
			{
				return sourcePtr;
			}
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		else
		{
			if(	(sourceAddr[0]==sourcePtr->sourceAddr[0])&&
				(sourceAddr[1]==sourcePtr->sourceAddr[1])&&
				(sourceAddr[2]==sourcePtr->sourceAddr[2])&&
				(sourceAddr[3]==sourcePtr->sourceAddr[3])
			)
			{
				return sourcePtr;
			}
		}
#endif
		sourcePtr=sourcePtr->next;
	}

	return NULL;
}

#if 0
static int32 rtl_searchSourceAddr(uint32 ipVersion, uint32 *sourceAddr, uint32 *sourceArray, uint32 elementCount)
{
	uint32 i=0;
	uint32 *srcPtr=sourceArray;
	
	for(i=0; i<elementCount; i++)
	{
		if(ipVersion==IP_VERSION4)
		{
			if(sourceAddr[0]==srcPtr[0])
			{
				return TRUE;
			}
			srcPtr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		if(ipVersion==IP_VERSION6)
		{
			if(	(sourceAddr[0]==srcPtr[0])&&\
				(sourceAddr[1]==srcPtr[1])&&\
				(sourceAddr[2]==srcPtr[2])&&\
				(sourceAddr[3]==srcPtr[3]))
			{
			
				return TRUE;
			}
			
			srcPtr=srcPtr+4;
		}
#endif
	}
	
	return FALSE;
}
#endif

static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode)
{
	if(NULL==entryNode)
	{
		return;
	}
	
	if(NULL==clientEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	

	if(clientEntry->sourceList!=NULL)
	{
		clientEntry->sourceList->previous=entryNode;
	}
	entryNode->next=clientEntry->sourceList;
	clientEntry->sourceList=entryNode;
	clientEntry->sourceList->previous=NULL;
	
	rtl_glueMutexUnlock();  /* lock resource*/	
}

static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(NULL==sourceEntry)
	{
		return;
	}
	
	if(NULL==clientEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	
	/* unlink entry node*/ 
	if(sourceEntry==clientEntry->sourceList) /*unlink group list head*/
	{

		clientEntry->sourceList=sourceEntry->next;
		if(clientEntry->sourceList!=NULL)
		{
			clientEntry->sourceList ->previous=NULL;
		}
	}
	else
	{	
		if(sourceEntry->previous!=NULL)
		{
			sourceEntry->previous->next=sourceEntry->next;
		}

		if(sourceEntry->next!=NULL)
		{
			sourceEntry->next->previous=sourceEntry->previous;
		}
	}
	
	sourceEntry->previous=NULL;
	sourceEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource

}

static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr)
{
	rtl_glueMutexLock();
	if (NULL!=sourceEntryPtr)
	{
		memset(sourceEntryPtr, 0, sizeof(struct rtl_sourceEntry));
	}
	rtl_glueMutexUnlock();
}

/*********************************************
			multicast flow list operation
 *********************************************/
 
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr)
{
	struct rtl_mcastFlowEntry* mcastFlowPtr = NULL;
	uint32 hashIndex=0;

	if(NULL==serverAddr)
	{
		return NULL;
	}

	if(NULL==groupAddr)
	{
		return NULL;
	}
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	mcastFlowPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
	while (mcastFlowPtr!=NULL)
	{	

		if(mcastFlowPtr->ipVersion!=ipVersion)
		{
			goto nextFlow;
		}
	
		if(ipVersion==IP_VERSION4)
		{
			if( (serverAddr[0]==mcastFlowPtr->serverAddr[0]) && (groupAddr[0]==mcastFlowPtr->groupAddr[0]) )
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			
			if(	(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(groupAddr[0]==mcastFlowPtr->groupAddr[0])
				&&(groupAddr[1]==mcastFlowPtr->groupAddr[1])
				&&(groupAddr[2]==mcastFlowPtr->groupAddr[2])
				&&(groupAddr[3]==mcastFlowPtr->groupAddr[3]))
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}

nextFlow:
#endif	
		mcastFlowPtr = mcastFlowPtr->next;

	}

	return NULL;
}

/* link multicast flow entry in the front of a forwarding flow list */
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable)
{
	uint32 hashIndex=0;
	rtl_glueMutexLock();//Lock resource
	if(NULL==mcastFlowEntry)
	{
		return;
	}

	if(NULL==hashTable)
	{
		return;
	}

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);

	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=mcastFlowEntry;
	}
	mcastFlowEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=mcastFlowEntry;
	hashTable[hashIndex]->previous=NULL;

	rtl_glueMutexUnlock();//UnLock resource
	return;

}

/* unlink a multicast flow entry*/
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable)
{	 
	uint32 hashIndex=0;
	if(NULL==mcastFlowEntry)
	{
		return;
	}

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);

	rtl_glueMutexLock();  /* lock resource*/	
	/* unlink entry node*/
	if(mcastFlowEntry==hashTable[hashIndex]) /*unlink flow list head*/
	{
		hashTable[hashIndex]=mcastFlowEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
	}
	else
	{
		if(mcastFlowEntry->previous!=NULL)
		{
			mcastFlowEntry->previous->next=mcastFlowEntry->next;
		}
		 
		if(mcastFlowEntry->next!=NULL)
		{
			mcastFlowEntry->next->previous=mcastFlowEntry->previous;
		}
	}
	
	mcastFlowEntry->previous=NULL;
	mcastFlowEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
}


/* clear the content of multicast flow entry */
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry)
{
	rtl_glueMutexLock();
	if (NULL!=mcastFlowEntry)
	{
		memset(mcastFlowEntry, 0, sizeof(struct rtl_mcastFlowEntry));
	}
	rtl_glueMutexUnlock();
}


static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable)
{	
	if(mcastFlowEntry!=NULL)
	{

		rtl_unlinkMcastFlowEntry(mcastFlowEntry, hashTable);
		rtl_clearMcastFlowEntry(mcastFlowEntry);
		rtl_freeMcastFlowEntry(mcastFlowEntry);
	}

	return;	
}
#endif
	
/*****source entry/client entry/group entry/flow entry operation*****/

static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(clientEntry==NULL)
	{
		return;
	}
	
	if(sourceEntry!=NULL)
	{
		rtl_unlinkSourceEntry(clientEntry,sourceEntry);
		rtl_clearSourceEntry(sourceEntry);
		rtl_freeSourceEntry(sourceEntry);
	}
}
	
static void rtl_deleteSourceList(struct rtl_clientEntry* clientEntry)
{
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	sourceEntry=clientEntry->sourceList;
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
		sourceEntry=nextSourceEntry;
	}
}

static void rtl_deleteClientEntry(struct rtl_groupEntry* groupEntry,struct rtl_clientEntry *clientEntry)
{	
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}

	rtl_deleteSourceList(clientEntry);
	rtl_unlinkClientEntry(groupEntry,clientEntry);
	rtl_clearClientEntry(clientEntry);
	rtl_freeClientEntry(clientEntry);
	return;	
		
}

static void rtl_deleteClientList(struct rtl_groupEntry* groupEntry)
{

	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_clientEntry *nextClientEntry=NULL;

	if(NULL==groupEntry)
	{
		return;
	}
	
	clientEntry=groupEntry->clientList;
	while(clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		rtl_deleteClientEntry(groupEntry,clientEntry);
		clientEntry=nextClientEntry;
	}
}


static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable)
{	
	if(groupEntry!=NULL)
	{
	
		rtl_deleteClientList(groupEntry);
		rtl_unlinkGroupEntry(groupEntry, hashTable);
		rtl_clearGroupEntry(groupEntry);
		rtl_freeGroupEntry(groupEntry);
		
	}
		
}


static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr)
{
	if(ipVersion==IP_VERSION4)
	{
		if(macAddr[0]!=0x01)
		{
			return FALSE;
		}

		if((macAddr[3]&0x7f)!=(uint8)((ipAddr[0]&0x007f0000)>>16))
		{
			return FALSE;
		}
		
		if(macAddr[4]!=(uint8)((ipAddr[0]&0x0000ff00)>>8))
		{
			return FALSE;
		}

		if(macAddr[5]!=(uint8)(ipAddr[0]&0x000000ff))
		{
			return FALSE;
		}

		return TRUE;
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		if(macAddr[0]!=0x33)
		{
			return FALSE;
		}

		if(macAddr[1]!=0x33)
		{
			return FALSE;
		}

		if(macAddr[2]!=(uint8)((ipAddr[3]&0xff000000)>>24))
		{
			return FALSE;
		}
		
		if(macAddr[3]!=(uint8)((ipAddr[3]&0x00ff0000)>>16))
		{
			return FALSE;
		}

		if(macAddr[4]!=(uint8)((ipAddr[3]&0x0000ff00)>>8))
		{
			return FALSE;
		}
		
		if(macAddr[5]!=(uint8)(ipAddr[3]&0x000000ff))
		{
			return FALSE;
		}
		
		return TRUE;
	}
#endif
	return FALSE;
}

#ifdef CONFIG_RTL_MLD_SNOOPING	
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2)
{
	int i;
	for(i=0; i<4; i++)
	{
		if(ipv6Addr1[i]!=ipv6Addr2[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}
#endif

static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
{
	int i;
	for(i=0; i<6; i++)
	{
		if(macAddr1[i]!=macAddr2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	uint32  checksum=0;
	uint32 count=packetLen;
	uint16   *ptr= (uint16 *) (packetBuf);	
	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(packetBuf+packetLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);

}

#ifdef CONFIG_RTL_MLD_SNOOPING
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr)
{
	uint32  checksum=0;
	uint32 count=pktLen;
	uint16   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (uint16 *) (ipv6PseudoHdr);	
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		  checksum+= ntohs(*ptr);
		  ptr++;
	}
	
	/*compute the checksum of mld buffer*/
	 count=pktLen;
	 ptr=(uint16 *) (pktBuf);	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
	
}
#endif

// Mason Yu. type error
//static uint8 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry, uint32 sysTime)
static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry, uint32 sysTime)
{
	// Mason Yu. type error 
	//uint8 portMask=(1<<clientEntry->portNum);
	//uint8 fwdPortMask=0;
	uint32 portMask=(1<<clientEntry->portNum);
	uint32 fwdPortMask=0;
	
	struct rtl_sourceEntry * sourcePtr=NULL;;
	
	if(clientEntry->groupFilterTimer>sysTime) /*exclude mode never expired*/
	{
		fwdPortMask|=portMask;
	}
	else/*include mode*/
	{
		sourcePtr=clientEntry->sourceList;
		while(sourcePtr!=NULL)
		{
			if(sourcePtr->portTimer>sysTime)
			{
				fwdPortMask|=portMask;
				break;
			}
			sourcePtr=sourcePtr->next;
		
		}
		
	}

	return fwdPortMask;
}

static void rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry)
{
	uint8 deleteFlag=FALSE;
	uint8 oldFwdState,newFwdState;

	oldFwdState=sourceEntry->fwdState;
	
	if(sourceEntry->portTimer<=rtl_sysUpSeconds) /*means time out*/
	{
		if(clientEntry->groupFilterTimer<=rtl_sysUpSeconds) /* include mode*/
		{
			deleteFlag=TRUE;
		}
	
		sourceEntry->fwdState=FALSE;
	}
	else
	{
		deleteFlag=FALSE;
		sourceEntry->fwdState=TRUE;
	}
	
	newFwdState=sourceEntry->fwdState;
	
	if(deleteFlag==TRUE) /*means INCLUDE mode and expired*/
	{
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
	}

	if((deleteFlag==TRUE) || (newFwdState!=oldFwdState))
	{
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
			rtl_handle_igmpgroup_change(&timerEventContext);
		}
		#endif
	}

}

static uint32 rtl_getClientSourceFwdPortMask(uint32 ipVersion, struct rtl_clientEntry * clientEntry,uint32 *sourceAddr, uint32 sysTime)
{
	// Mason Yu. type error 
	//uint8 portMask=(1<<clientEntry->portNum);
	//uint8 fwdPortMask=0;
	uint32 portMask=(1<<clientEntry->portNum);
	uint32 fwdPortMask=0;
	struct rtl_sourceEntry * sourceEntry=NULL;
	if(clientEntry==NULL)
	{
		return 0xFFFFFFFF; /*broadcast*/
	}
	else
	{
		sourceEntry=rtl_searchSourceEntry(ipVersion,sourceAddr, clientEntry);
	
		if(clientEntry->groupFilterTimer<=sysTime)	/*include mode*/
		{	
			if(sourceEntry!=NULL)
			{
				if( sourceEntry->portTimer>sysTime)
				{
					fwdPortMask|=portMask;
				}
			}
		}
		else/*exclude mode*/
		{	
			if(sourceEntry==NULL)
			{
				fwdPortMask|=portMask;
			}
			else
			{
				if(sourceEntry->portTimer>sysTime)
				{
					fwdPortMask|=portMask;
				}
			}
		}
		
		return fwdPortMask;
		
	}
}

static uint32 rtl_getGroupSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime)
{
	// Mason Yu. type error 
	//uint8 fwdPortMask=0;
	uint32 fwdPortMask=0;
	struct rtl_clientEntry * clientEntry=NULL;
	if(groupEntry==NULL)
	{
		return 0xFFFFFFFF; /*broadcast*/
	}
	else
	{
		
		clientEntry=groupEntry->clientList;
		while(clientEntry!=NULL)
		{
			fwdPortMask|= rtl_getClientSourceFwdPortMask(groupEntry->ipVersion, clientEntry, sourceAddr, sysTime);
			clientEntry=clientEntry->next;
		}
		
	}
	return fwdPortMask;
}


static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry)
{
	// Mason Yu. type error 
	//uint8 oldFwdPortMask=0;
	//uint8 newFwdPortMask=0;
	uint32 oldFwdPortMask=0;
	uint32 newFwdPortMask=0;
	struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	oldFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);
	
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_checkSourceTimer(clientEntry, sourceEntry);
		sourceEntry=nextSourceEntry;
	}
	
	newFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);


	if(newFwdPortMask==0) /*none active port*/
	{
		rtl_deleteClientEntry(groupEntry,clientEntry);	
	}

	if((oldFwdPortMask!=newFwdPortMask) || (newFwdPortMask==0))
	{	
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
			rtl_handle_igmpgroup_change(&timerEventContext);
		}
		#endif
		
	}

}

static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable)
{
	uint32 deleteFlag=FALSE;
	struct rtl_clientEntry *clientEntry=groupEntry->clientList;
	struct rtl_clientEntry *nextClientEntry=NULL;
	
		
	while(clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		timerEventContext.portMask=1<<(clientEntry->portNum);
		rtl_checkClientEntryTimer(groupEntry, clientEntry);
		clientEntry=nextClientEntry;
	}
	
	if(groupEntry->clientList==NULL) /*none active client*/
	{
		deleteFlag=TRUE;
		rtl_deleteGroupEntry(groupEntry,hashTable);	
	}

	if(deleteFlag==TRUE)
	{	
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS		
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
			rtl_handle_igmpgroup_change(&timerEventContext);
		}
		#endif
		
	}

}



static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize)
{
	uint32 i=0;
	
	/* Allocate memory */
	rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable = (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);

	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!= NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	
			rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i]=NULL;
		}

	}
	else
	{
		return FAILED;
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=  (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	

			rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		
		return FAILED;

	}
#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_mCastModuleArray[moduleIndex].flowHashTable= (struct rtl_mcastFlowEntry **)rtl_glueMalloc(4 * hashTableSize);
	
	if (rtl_mCastModuleArray[moduleIndex].flowHashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	

			rtl_mCastModuleArray[moduleIndex].flowHashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif		
		return FAILED;


	}
	#endif
return SUCCESS;

}



/**************************
	Utility
**************************/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* macFrame, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo) 
{
	
//MAC Frame :DA(6 bytes)+SA(6 bytes)+ CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
//                   +Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes )+Data(46~1500 bytes)+CRC(4 bytes)

	uint8 *ptr=macFrame;

#ifdef CONFIG_RTL_MLD_SNOOPING	
	int i=0;
	uint8 nextHeader=0;
	uint16 extensionHdrLen=0;
	uint8 routingHead=FALSE;
	
	uint8 optionDataLen=0;
	uint8 optionType=0;
	uint32 ipv6RAO=0;
#endif

	uint32 ipAddr[4]={0,0,0,0};
	union pseudoHeader pHeader;
	
	memset(macInfo,0,sizeof(struct rtl_macFrameInfo));
	memset(&pHeader, 0, sizeof(union pseudoHeader));

	ptr=ptr+12;


	/*check the presence of VLAN tag*/	
	if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
	{
		ptr=ptr+4;
	}

	/*ignore packet with PPPOE header*/	
	if(*(int16 *)(ptr)==(int16)htons(PPPOE_ETHER_TYPE))
	{
		return;	
	}

	
	/*check the presence of ipv4 type*/
	if(*(int16 *)(ptr)==(int16)htons(IPV4_ETHER_TYPE))
	{
		ptr=ptr+2;
		macInfo->ipBuf=ptr;
		macInfo->ipVersion=IP_VERSION4;
	}
	else
	{
		/*check the presence of ipv4 type*/
		if(*(int16 *)(ptr)==(int16)htons(IPV6_ETHER_TYPE))
		{
			ptr=ptr+2;
			macInfo->ipBuf=ptr;
			macInfo->ipVersion=IP_VERSION6;
		}
	}

	if((macInfo->ipVersion!=IP_VERSION4) && (macInfo->ipVersion!=IP_VERSION6))
	{
		return;
	}
	macInfo->checksumFlag=FAILED;
	
	if(macInfo->ipVersion==IP_VERSION4)
	{
		macInfo->ipHdrLen=(uint16)((((struct ipv4Pkt *)(macInfo->ipBuf))->vhl&0x0f)<<2);
		macInfo->l3PktLen=ntohs(((struct ipv4Pkt *)(macInfo->ipBuf))->length)-macInfo->ipHdrLen;
		ptr=ptr+macInfo->ipHdrLen;
		macInfo->l3PktBuf=ptr;
		macInfo->macFrameLen=(uint16)((ptr-macFrame)+macInfo->l3PktLen);
		macInfo->srcIpAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->sourceIp);
		macInfo->dstIpAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp);
/*distinguish different IGMP packet:
                                                    ip_header_length      destination_ip      igmp_packet_length   igmp_type   group_address         	
IGMPv1_general_query:                            20                   224.0.0.1                       8                    0x11                 0
IGMPv2_general_query:                            24                   224.0.0.1                       8                    0x11                 0                     
IGMPv2_group_specific_query:			24                   224.0.0.1                       8                    0x11               !=0  
IGMPv3 _query:                                        24                   224.0.0.1                   >=12                  0x11        according_to_different_situation 

IGMPv1_join:                                            20          actual_multicast_address         8                    0x12           actual_multicast_address
IGMPv2_join:                                            24          actual_multicast_address         8                    0x16           actual_multicast_address
IGMPv2_leave:                                          24          actual_multicast_address         8                    0x17           actual_multicast_address
IGMPv3_report:                                         24          actual_multicast_address       >=12                0x22           actual_multicast_address*/

	/* parse IGMP type and version*/	
		if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==IGMP_PROTOCOL)
		{	
			/*check DVMRP*/
			if((macInfo->l3PktBuf[0]==DVMRP_TYPE) && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(DVMRP_ADDR)) )
			{
				macInfo->l3Protocol=DVMRP_PROTOCOL;
			}
			else
			{
				/*means unicast*/
				if((macFrame[0]&0x01)==0)
				{	
						if(rtl_compareMacAddr(macFrame, rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac)==TRUE) 
				       	{
								if(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr))
								{
									macInfo->l3Protocol=IGMP_PROTOCOL;
               							goto otherpro;
								}
						}
			      									
				}
				else /*means multicast*/
				{	
					ipAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp);
					if(rtl_checkMCastAddrMapping(IP_VERSION4,ipAddr,macFrame)==TRUE)
					{
						macInfo->l3Protocol=IGMP_PROTOCOL;
					}
					else
					{
						return;
					}
				}
			}
			
		}

otherpro:	
			if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==MOSPF_PROTOCOL &&\
			((((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_MOSPF_ADDR1)) ||\
			(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_MOSPF_ADDR2))))
		{
			macInfo->l3Protocol=MOSPF_PROTOCOL;
		}

		if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==PIM_PROTOCOL && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_PIM_ADDR)))
		{
			macInfo->l3Protocol=PIM_PROTOCOL;
		}

		if(verifyCheckSum==TRUE)
		{
			if(rtl_checksum(macInfo->l3PktBuf, macInfo->l3PktLen)!=0)
			{
				macInfo->checksumFlag=FAILED;
			}
			else
			{
				macInfo->checksumFlag=SUCCESS;
			}
		}
		else
		{
			macInfo->checksumFlag=SUCCESS;
		}
	}

#ifdef CONFIG_RTL_MLD_SNOOPING
	if(macInfo->ipVersion==IP_VERSION6)
	{
		macInfo->srcIpAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[0]);
		macInfo->srcIpAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[1]);
		macInfo->srcIpAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[2]);
		macInfo->srcIpAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[3]);
		
		macInfo->dstIpAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
		macInfo->dstIpAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
		macInfo->dstIpAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
		macInfo->dstIpAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
		
		macInfo->macFrameLen=(uint16)(ptr-macFrame+IPV6_HEADER_LENGTH+ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth));
		macInfo->ipHdrLen=IPV6_HEADER_LENGTH;
		
		nextHeader=((struct ipv6Pkt *)(macInfo->ipBuf))->nextHeader;
		ptr=ptr+IPV6_HEADER_LENGTH;
		while((ptr-macInfo->ipBuf)<(ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)+IPV6_HEADER_LENGTH))
		{
			switch(nextHeader) 
			{
				case HOP_BY_HOP_OPTIONS_HEADER:
					/*parse hop-by-hop option*/
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					ptr=ptr+2;
					
					while((ptr-macInfo->ipBuf-40)<extensionHdrLen)
					{
						optionType=ptr[0];
						/*pad1 option*/
						if(optionType==0)
						{
							ptr=ptr+1;
							continue;
						}

						/*padN option*/
						if(optionType==1)
						{
							optionDataLen=ptr[1];
							ptr=ptr+optionDataLen+2;
							continue;
						}

						/*router alter option*/
						if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
						{
							ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
							ptr=ptr+4;	
							continue;
						}

						/*other TLV option*/
						if((optionType!=0) && (optionType!=1))
						{
							optionDataLen=ptr[1];
							ptr=ptr+optionDataLen+2;
							continue;
						}
					

					}
					/*
					if((ptr-macInfo->ipBuf-40)!=extensionHdrLen)
					{
						rtl_gluePrintf("ipv6 packet parse error\n");
					}*/
					
				break;
				
				case ROUTING_HEADER:
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					
                                  
					 if (ptr[3]>0)
				   	{
                                          ptr=ptr+extensionHdrLen;
						for(i=0; i<4; i++)
						{
						      pHeader.ipv6_pHdr.destinationAddr[i]=*((uint32 *)(ptr)-4+i);

						}
					     routingHead=TRUE;
					     
					     
					}
					else
					{
                                          ptr=ptr+extensionHdrLen;
					}
					
					
					
				break;
				
				case FRAGMENT_HEADER:
					nextHeader=ptr[0];
					ptr=ptr+8;
				break;
				
				case DESTINATION_OPTION_HEADER:
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					ptr=ptr+extensionHdrLen;
				break;
				
				case ICMP_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					if((ptr[0]==MLD_QUERY) ||(ptr[0]==MLDV1_REPORT) ||(ptr[0]==MLDV1_DONE) ||(ptr[0]==MLDV2_REPORT))
					{
						/*means multicast*/
						if(	(macFrame[0]==0x33)&&\
							(macFrame[1]==0x33))
						{
							ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
							ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
							ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
							ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
							
							if(rtl_checkMCastAddrMapping(IP_VERSION6, ipAddr, macFrame)==TRUE)
							{
								macInfo->l3Protocol=ICMP_PROTOCOL;
							}
							
						}
						else /*means multicast*/
						{	
							
								ipAddr[0]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[0]);
								ipAddr[1]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[1]);
								ipAddr[2]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[2]);
								ipAddr[3]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[3]);
								if(	(rtl_compareMacAddr(macFrame, rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac)==TRUE) &&\
								(rtl_compareIpv6Addr(((struct ipv6Pkt *)macInfo->ipBuf)->destinationAddr, ipAddr) == TRUE))
								{
									macInfo->l3Protocol=ICMP_PROTOCOL;
								}		

							
						}

						/*
						if(ipv6RAO!=IPV6_ROUTER_ALTER_OPTION)
						{
							rtl_gluePrintf("router alter option error\n");
						}*/
					}
				
					
				break;
				
				case PIM_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					
					ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
					ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
					ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
					ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
					if(IS_IPV6_PIM_ADDR(ipAddr))
					{
						macInfo->l3Protocol=PIM_PROTOCOL;
					}
				
				break;
				
				case MOSPF_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					
					ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
					ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
					ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
					ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
					
					if(IS_IPV6_MOSPF_ADDR1(ipAddr) || IS_IPV6_MOSPF_ADDR2(ipAddr))
					{
						macInfo->l3Protocol=MOSPF_PROTOCOL;
					}

				break;
				
				default:		
					goto out;
				break;
			}
		
		}

out:
		
		
       	if(verifyCheckSum==TRUE)
       	{
       		if(macInfo->l3PktBuf==NULL)
       		{
				return;	
			}
			
       		/*generate pseudo header*/
		       for(i=0; i<4; i++)
		      {
			     pHeader.ipv6_pHdr.sourceAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[i];
			     
		       }
           
		      if(routingHead==FALSE)
		      {
		             for(i=0;i<4;i++)
		      	      {
			            pHeader.ipv6_pHdr.destinationAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[i];
		      	      }
		       }
	      
		
		       pHeader.ipv6_pHdr.nextHeader=macInfo->l3Protocol;
		       pHeader.ipv6_pHdr.upperLayerPacketLength=htonl((uint32)(macInfo->l3PktLen));
		       pHeader.ipv6_pHdr.zeroData[0]=0;
		       pHeader.ipv6_pHdr.zeroData[1]=0;
		       pHeader.ipv6_pHdr.zeroData[2]=0;

			/*verify checksum*/
		      if(rtl_ipv6L3Checksum(macInfo->l3PktBuf, macInfo->l3PktLen,&pHeader)!=0)
		      {
			      macInfo->checksumFlag=FAILED;
		       }
		       else
		       {
			      macInfo->checksumFlag=SUCCESS;
		       }
       	}
		else
		{
			macInfo->checksumFlag=SUCCESS;
		}
       }	
#endif
	return;
}


static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime)
{
	uint32 portIndex=0;
	uint8 portMaskn=PORT0_MASK;
	uint32 routerPortmask=0;
	
	if(ipVersion==IP_VERSION4)
	{
		for(portIndex=0; portIndex<MAX_SUPPORT_PORT_NUMBER; portIndex++)
		{
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}
			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	

			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}		


			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
		
			portMaskn=portMaskn<<1;  /*shift to next port mask*/
			
		}
	
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		for(portIndex=0; portIndex<MAX_SUPPORT_PORT_NUMBER; portIndex++)
		{
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{	

				routerPortmask=routerPortmask|portMaskn;
			}		

			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
			
			portMaskn=portMaskn<<1;  /*shift to next port mask*/
			
		}

	}
#endif	

	routerPortmask= routerPortmask |rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;

	return routerPortmask;
}

static uint32 rtl_processQueries(uint32 moduleIndex,uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	struct rtl_groupEntry *groupEntry=NULL;
	struct rtl_clientEntry * clientEntry=NULL;
	struct rtl_sourceEntry*sourceEntry=NULL;
	uint32 hashIndex=0; 
	uint32 groupAddress[4]={0,0,0,0};
	uint32 suppressFlag=0;
	uint32 *sourceAddr=NULL;
	uint32 numOfSrc=0;
	uint32 i=0;
	
	/*querier timer update and election process*/
	rtl_snoopQuerier(moduleIndex, ipVersion, portNum);
	
	if(ipVersion==IP_VERSION4)
	{	
		if(pktLen>=12) /*means igmpv3 query*/
		{
			groupAddress[0]=ntohl(((struct igmpv3Query*)pktBuf)->groupAddr);
			suppressFlag=((struct igmpv3Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct igmpv3Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct igmpv3Query*)pktBuf)->numOfSrc);

		}
		else
		{
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		}
		
		if(groupAddress[0]==0) /*means general query*/
		{
			goto out;
		}
		else
		{
			hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
		}
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		if(pktLen>=28) /*means mldv2 query*/
		{
			groupAddress[0]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[0]);
			groupAddress[1]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[1]);
			groupAddress[2]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[2]);
			groupAddress[3]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[3]);

			suppressFlag=((struct mldv2Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct mldv2Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct mldv2Query*)pktBuf)->numOfSrc);

		}
		else /*means mldv1 query*/
		{
			groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
			groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
			groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
			groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
			
		}
		
		if(	(groupAddress[0]==0)&& 
			(groupAddress[1]==0)&&
			(groupAddress[2]==0)&&
			(groupAddress[3]==0)	)/*means general query*/
		{
			goto out;
		}
		else
		{
			hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
		}
	}
#endif	
	if(suppressFlag==0)
	{
	
		groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
		if((groupEntry!=NULL))
		{	
			
			if(numOfSrc==0) /*means group specific query*/
			{
				clientEntry=groupEntry->clientList;
				while(clientEntry!=NULL)
				{
					if(clientEntry->groupFilterTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
					{
						clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}	
					clientEntry=clientEntry->next;
				}
				
			}
			else /*means group and source specific query*/
			{
				clientEntry=groupEntry->clientList;
				while(clientEntry!=NULL)
				{
					for(i=0; i<numOfSrc; i++)
					{	
						
						sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr, clientEntry);
						
						if(sourceEntry!=NULL)
						{
							if(sourceEntry->portTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
							{
								sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
							}
							
						}

						if(ipVersion==IP_VERSION4)
						{
							sourceAddr++;
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						else
						{
							sourceAddr=sourceAddr+4;
						}
#endif
					}
					
					clientEntry=clientEntry->next;
				}
				
			}
		}
		
		
	}
	
	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

out:	
	return (~(1<<portNum) & ((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}


static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum)
{
	
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
	}
#endif	
	return;
}


/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	
	
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);

	if(ipVersion==IP_VERSION4)
	{
		if(pktBuf[0]==0x12)
		{ 
			groupAddress[0]=ntohl(((struct igmpv1Pkt *)pktBuf)->groupAddr);
		}

		if(pktBuf[0]==0x16)
		{
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		}
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
	}
#endif	

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			goto out;
		}

		assert(newGroupEntry->clientList==NULL);
#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
	
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif			
		groupEntry=newGroupEntry;
		
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			goto out;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->portNum=portNum;
		newClientEntry->igmpVersion=IGMP_V2;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	

	rtl_deleteSourceList(clientEntry);
	clientEntry->igmpVersion=IGMP_V2;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;	

	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

out:
	return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}

static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
	}
#endif	

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	
	if(groupEntry!=NULL)
	{   
		clientEntry=rtl_searchClientEntry( ipVersion, groupEntry, portNum, clientAddr);
		if(clientEntry!=NULL) 
		{

			if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
			{
				rtl_deleteClientEntry(groupEntry, clientEntry);
			}
			else
			{
				while(sourceEntry!=NULL)
				{
					nextSourceEntry=sourceEntry->next;
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
					sourceEntry=nextSourceEntry;
				}
				
				if(clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
				{
					clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
				}
			
			}

		}
		
	}	
	

	
	reportEventContext.ipVersion=ipVersion;

#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
#else
	reportEventContext.groupAddr[0]=groupAddress[0];
#endif
	
	if((groupEntry!=NULL) && (groupEntry->clientList==NULL))
	{
		if(ipVersion==IP_VERSION4)
		{
			rtl_deleteGroupEntry(groupEntry,rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}		
	}
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
			rtl_handle_igmpgroup_change(&reportEventContext);
		}

	}
	#endif
	
	//return (multicastRouterPortMask&(~(1<<portNum))&0x3f);
	return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}

static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{

	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;

	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);

	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif	

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

		assert(newGroupEntry->clientList==NULL);
		/*set new multicast entry*/
#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}
	
	/*from here groupEntry is the same as newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
		
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	/*from here client entry is the same as the newClientEntry*/
	rtl_deleteSourceList(clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds;
	assert(clientEntry->sourceList==NULL);
	
	/*here to handle the source list*/
	for(j=0; j<numOfSrc; j++)
	{
		
		newSourceEntry=rtl_allocateSourceEntry();
		if(newSourceEntry==NULL)
		{
			rtl_gluePrintf("run out of source entry!\n");
			return FAILED;
		}
	
		if(ipVersion==IP_VERSION4)
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];
			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];
			newSourceEntry->sourceAddr[1]=sourceAddr[1];
			newSourceEntry->sourceAddr[2]=sourceAddr[2];
			newSourceEntry->sourceAddr[3]=sourceAddr[3];

			sourceAddr=sourceAddr+4;
		}
#endif						
		newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
		rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
	}
	


	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif
	


	return SUCCESS;
}

static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif

	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
	
		assert(newGroupEntry->clientList==NULL);
		/*set new multicast entry*/		
#ifdef CONFIG_RTL_MLD_SNOOPING	
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}
	
	/*from here groupEntry is the same as  newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}

		assert(newClientEntry->sourceList==NULL);
		
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif	
		
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;

	}
	
	/*from here clientEntry  is the same as newClientEntry*/
	
	/*flush the old source list*/
	rtl_deleteSourceList( clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
	assert(clientEntry->sourceList==NULL);
	
	/*link the new source list*/
	for(j=0; j<numOfSrc; j++)
	{
		newSourceEntry=rtl_allocateSourceEntry();
		if(newSourceEntry==NULL)
		{
			rtl_gluePrintf("run out of source entry!\n");
			return FAILED;
		}

		if(ipVersion==IP_VERSION4)
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];

			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];
			newSourceEntry->sourceAddr[1]=sourceAddr[1];
			newSourceEntry->sourceAddr[2]=sourceAddr[2];
			newSourceEntry->sourceAddr[3]=sourceAddr[3];

			sourceAddr=sourceAddr+4;
		}
#endif						
		/*time out the sources included in the MODE_IS_EXCLUDE report*/
		newSourceEntry->portTimer=rtl_sysUpSeconds;
		rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
	}

	

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);		
	}
	#endif
	
	return SUCCESS;

}

static int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	
	}
#endif

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{	
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
	
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif			
		groupEntry=newGroupEntry;
	}

	/*from here groupEntry is the same as newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	
	if(clientEntry==NULL)
	{
		
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
		
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	
	/*here to handle the source list*/
	
	clientEntry->igmpVersion=IGMP_V3;
	
	if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
	{
		clientEntry->groupFilterTimer=rtl_sysUpSeconds;
		rtl_deleteSourceList(clientEntry);
		/*link the new source list*/
		for(j=0; j<numOfSrc; j++)
		{

			newSourceEntry=rtl_allocateSourceEntry();
			if(newSourceEntry==NULL)
			{
				rtl_gluePrintf("run out of source entry!\n");
				return FAILED;
			}

			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
				
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
				newSourceEntry->sourceAddr[1]=sourceAddr[1];
				newSourceEntry->sourceAddr[2]=sourceAddr[2];
				newSourceEntry->sourceAddr[3]=sourceAddr[3];

				sourceAddr=sourceAddr+4;
			}
#endif
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		}
			
	}
	else
	{
		
		while(sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			}
			sourceEntry=nextSourceEntry;
		}
		
		/*add  new source list*/
		for(j=0; j<numOfSrc; j++)
		{
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr, clientEntry);
			if(sourceEntry!=NULL)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			else
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if(newSourceEntry==NULL)
				{
					rtl_gluePrintf("run out of source entry!\n");
					return FAILED;
				}
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			
			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
				
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
				newSourceEntry->sourceAddr[1]=sourceAddr[1];
				newSourceEntry->sourceAddr[2]=sourceAddr[2];
				newSourceEntry->sourceAddr[3]=sourceAddr[3];

				sourceAddr=sourceAddr+4;
			}
#endif
		}
		
		if(clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
		{
			clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
		}
		
	}	
		
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
			rtl_handle_igmpgroup_change(&reportEventContext);
		}
		
	}
	#endif

return SUCCESS;
	
}

static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
	
	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif


		newGroupEntry->ipVersion=ipVersion;
		assert(newGroupEntry->clientList==NULL);
			
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}
	
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif	
		
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	/*flush the old source list*/
	rtl_deleteSourceList( clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
	
	/*link the new source list*/
	for(j=0; j<numOfSrc; j++)
	{
		newSourceEntry=rtl_allocateSourceEntry();
		if(newSourceEntry==NULL)
		{
			rtl_gluePrintf("run out of source entry!\n");
			return FAILED;
		}

		if(ipVersion==IP_VERSION4)
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];

			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{	
			newSourceEntry->sourceAddr[0]=sourceAddr[0];
			newSourceEntry->sourceAddr[1]=sourceAddr[1];
			newSourceEntry->sourceAddr[2]=sourceAddr[2];
			newSourceEntry->sourceAddr[3]=sourceAddr[3];

			sourceAddr=sourceAddr+4;
		}
#endif						
		/*time out the sources included in the MODE_IS_EXCLUDE report*/
		newSourceEntry->portTimer=rtl_sysUpSeconds;
		rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
	}

	

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
		
	}
	#endif

	return SUCCESS;
}

static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif	

	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif


		newGroupEntry->ipVersion=ipVersion;

		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif

		groupEntry=newGroupEntry;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->portNum=portNum;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->groupFilterTimer=rtl_sysUpSeconds;
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
	
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	
	clientEntry->igmpVersion=IGMP_V3;
	
	/*here to handle the source list*/
	for(j=0; j<numOfSrc; j++)
	{
		sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,clientEntry);
	
		if(sourceEntry==NULL)
		{
			newSourceEntry=rtl_allocateSourceEntry();
			if(newSourceEntry==NULL)
			{
				rtl_gluePrintf("run out of source entry!\n");
				return FAILED;
			}

		
			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				newSourceEntry->sourceAddr[0]=sourceAddr[0];
				newSourceEntry->sourceAddr[1]=sourceAddr[1];
				newSourceEntry->sourceAddr[2]=sourceAddr[2];
				newSourceEntry->sourceAddr[3]=sourceAddr[3];
			}
#endif						
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
		}
		else
		{		
			/*just update source timer*/
			sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;		
		}
			
		if(ipVersion==IP_VERSION4)
		{	
			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			sourceAddr=sourceAddr+4;
		}
#endif				
	}


	
	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
		
	}
	#endif

	return SUCCESS;
}

static int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};

	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	//struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif

	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)
	{
		goto out;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);	
	if(clientEntry==NULL)
	{
		goto out;
	}

	clientEntry->igmpVersion=IGMP_V3;
	
	if(clientEntry->groupFilterTimer>rtl_sysUpSeconds) /*means exclude mode*/
	{

		for(j=0; j<numOfSrc; j++)
		{
			
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,clientEntry);
		
			if(sourceEntry==NULL)
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if(newSourceEntry==NULL)
				{
					rtl_gluePrintf("run out of source entry!\n");
					return FAILED;
				}

			
				if(ipVersion==IP_VERSION4)
				{	
					newSourceEntry->sourceAddr[0]=sourceAddr[0];
				
				}
#ifdef CONFIG_RTL_MLD_SNOOPING
				else
				{	
					newSourceEntry->sourceAddr[0]=sourceAddr[0];
					newSourceEntry->sourceAddr[1]=sourceAddr[1];
					newSourceEntry->sourceAddr[2]=sourceAddr[2];
					newSourceEntry->sourceAddr[3]=sourceAddr[3];
				}
#endif                          
				newSourceEntry->portTimer=rtl_sysUpSeconds;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			else
			{
				if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;	
				}
				else
				{
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
				}	
			}

		
			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{
				sourceAddr=sourceAddr+4;
			}
#endif					
		}
                    
	}
	else           /*means include mode*/
	{

		for(j=0; j<numOfSrc; j++)
          	{
		       sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,clientEntry);
			if(sourceEntry!=NULL)
			{
				if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;
				}
				else
				{
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
							
				}
				
			}	

			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{
				sourceAddr=sourceAddr+4;
			}
#endif					
 		}

	}

out:
	
	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
			rtl_handle_igmpgroup_change(&reportEventContext);
		}
		
	}
	#endif
	
	return SUCCESS;

}


static uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 i=0;
	uint16 numOfRecords=0;
	uint8 *groupRecords=NULL;
	uint8 recordType=0xff;
	uint16 numOfSrc=0;
	int32 returnVal=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		numOfRecords=ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
		if(numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
		}
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{	
		numOfRecords=ntohs(((struct mldv2Report *)pktBuf)->numOfRecords);
		if(numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct mldv2Report *)pktBuf)->recordList));
		}
	}
#endif
	
	for(i=0; i<numOfRecords; i++)
	{
		if(ipVersion==IP_VERSION4)
		{
			recordType=((struct groupRecord *)groupRecords)->type;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		else
		{
			recordType=((struct mCastAddrRecord *)groupRecords)->type;
		}
#endif		
	
		switch(recordType)
		{
			case MODE_IS_INCLUDE:
				returnVal=rtl_processIsInclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			break;
			
			case MODE_IS_EXCLUDE:
				returnVal=rtl_processIsExclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			break;
			
			case CHANGE_TO_INCLUDE_MODE:
				returnVal=rtl_processToInclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			break;
			
			case CHANGE_TO_EXCLUDE_MODE:
				returnVal=rtl_processToExclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			break;
			
			case ALLOW_NEW_SOURCES:
				returnVal=rtl_processAllow(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			break;
			
			case BLOCK_OLD_SOURCES:
				returnVal=rtl_processBlock(moduleIndex, ipVersion, portNum, clientAddr ,groupRecords);
			break;
			
			default:break;
			
		}

		if(ipVersion==IP_VERSION4)
		{
			numOfSrc=ntohs(((struct groupRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+8+numOfSrc*4+(((struct groupRecord *)(groupRecords))->auxLen)*4;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		else
		{
			numOfSrc=ntohs(((struct mCastAddrRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+20+numOfSrc*16+(((struct mCastAddrRecord *)(groupRecords))->auxLen)*4;
		}
#endif		
	}

	return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8* pktBuf, uint32 pktLen)
{	
	uint32 fwdPortMask=0;

	reportEventContext.moduleIndex=moduleIndex;

	switch(pktBuf[0])
	{
		case IGMP_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case IGMPV1_REPORT:
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr,pktBuf);
		break;
			
		case IGMPV2_REPORT:	
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr, pktBuf);
		break;
			
		case IGMPV2_LEAVE:
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, clientAddr,pktBuf);
		break;

		case IGMPV3_REPORT:
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		break;

		case MLD_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case MLDV1_REPORT:
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		break;
			
		case MLDV1_DONE:	
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		break;
			
		case MLDV2_REPORT:
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		break;

		default:
			fwdPortMask=((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
		break;
	}						
	
	return fwdPortMask;
			
}



static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.dvmrpRouterAgingTime; /*update timer*/
	}

	if(ipVersion==IP_VERSION4)
	{

		reportEventContext.ipVersion=ipVersion;
		reportEventContext.groupAddr[0]=0;
		reportEventContext.groupAddr[1]=0;
		reportEventContext.groupAddr[2]=0;
		reportEventContext.groupAddr[3]=0;

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
		#endif
	}

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));

}

static uint32 rtl_processMospf(uint32 moduleIndex,uint32 ipVersion,uint32 portNum, uint8* pktBuf, uint32 pktLen)
{ 
	struct ipv4MospfHdr *ipv4MospfHeader=(struct ipv4MospfHdr*)pktBuf;
	struct ipv4MospfHello *ipv4HelloPkt=(struct ipv4MospfHello*)pktBuf;
	
#ifdef CONFIG_RTL_MLD_SNOOPING		
	struct ipv6MospfHdr *ipv6MospfHeader=(struct ipv6MospfHdr*)pktBuf;
	struct ipv6MospfHello *ipv6HelloPkt=(struct ipv6MospfHello*)pktBuf;
#endif


	if(ipVersion==IP_VERSION4)
	{	
		/*mospf is built based on ospfv2*/
		if((ipv4MospfHeader->version==2) && (ipv4MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv4HelloPkt->options & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
			}
		}
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{	
		if((ipv6MospfHeader->version==3) && (ipv6MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv6HelloPkt->options[2] & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
			
			}
		}
	}
#endif

	

	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif
	
	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	if(ipVersion==IP_VERSION4)
	{	
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
	}
#endif



	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}



/*********************************************
				External Function
  *********************************************/


//External called function by high level program

int32 rtl_registerIgmpSnoopingModule(uint32 *moduleIndex)
{
	int32 i=0;
	uint32 index=0xFFFFFFFF;
	
	*moduleIndex=0xFFFFFFFF;

	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if(rtl_mCastModuleArray[i].enableSnooping==FALSE)
		{
			index=i;
			break;
		}
	}

	if(i>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	

	if(rtl_mCastModuleArray[index].enableSnooping==FALSE)
	{
	      /*initialize multicast Routers information*/
	      for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	      {
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.mospfRouter.portTimer[i]=0;
			
#ifdef CONFIG_RTL_MLD_SNOOPING		
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.mospfRouter.portTimer[i]=0;
#endif			
	      }
	
	      /*initialize hash table*/
	      rtl_initHashTable(index, rtl_hashTableSize);
	
	      if((rtl_mCastModuleArray[index].rtl_ipv4HashTable==NULL) )
	      {
		      return FAILED;
	      }
		  
#ifdef CONFIG_RTL_MLD_SNOOPING
		if(rtl_mCastModuleArray[index].rtl_ipv6HashTable==NULL)
		{
			return FAILED;
		}
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[index].deviceInfo,0, sizeof(rtl_multicastDeviceInfo_t));
#endif
      		for(i=0; i<6; i++)
      		{
	      		rtl_mCastModuleArray[index].rtl_gatewayMac[i]=0;
      		}
	
		rtl_mCastModuleArray[index]. rtl_gatewayIpv4Addr=0;

#ifdef CONFIG_RTL_MLD_SNOOPING		
		 for(i=0; i<4; i++)
		{
			rtl_mCastModuleArray[index].rtl_gatewayIpv6Addr[i]=0;
		}
#endif
		rtl_mCastModuleArray[index].enableFastLeave=FALSE;
		rtl_mCastModuleArray[index].enableSnooping=TRUE;
		rtl_mCastModuleArray[index].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[index].staticRouterPortMask=0;
#ifdef CONFIG_PROC_FS
		rtl_mCastModuleArray[index].expireEventCnt=0;
#endif
		*moduleIndex=index;

		return SUCCESS;
	}
	else
	{
	       return FAILED;
	}

	*moduleIndex=index;
	*moduleIndex=index;
	return SUCCESS;
}



int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex)
{
	uint32 i=0;
	struct rtl_groupEntry *groupEntryPtr=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntryPtr=NULL;
	#endif
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
       if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
       {
	
		 rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=0;
		 
          	 for(i=0; i<6; i++)
     		{
	   		 rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[i]=0;
      		}
			
#ifdef CONFIG_RTL_MLD_SNOOPING					 
		for(i=0;i<4;i++)
		{
			rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[i]=0;
		}
#endif	

	 	 /*delete ipv4 multicast entry*/
        	for(i=0;i<rtl_hashTableSize;i++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
				
			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
			}
	       }
		rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters), 0, sizeof(struct rtl_multicastRouters));
		
#ifdef CONFIG_RTL_MLD_SNOOPING		
		/*delete ipv6 multicast entry*/
		for(i=0; i<rtl_hashTableSize; i++)
		{
		
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			}
		}
		rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters), 0, sizeof(struct rtl_multicastRouters));
#endif

#ifdef CONFIG_RECORD_MCAST_FLOW
		/*delete multicast flow entry*/
        	for(i=0;i<rtl_hashTableSize;i++)
	     	{
			mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
				
			while(mcastFlowEntryPtr!=NULL)
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntryPtr, rtl_mCastModuleArray[moduleIndex].flowHashTable);
				mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
			}
	       }
		rtl_glueFree(rtl_mCastModuleArray[moduleIndex].flowHashTable);
		rtl_mCastModuleArray[moduleIndex].flowHashTable=NULL;
#endif
		rtl_mCastModuleArray[moduleIndex].enableSnooping=FALSE;
		rtl_mCastModuleArray[moduleIndex].enableFastLeave=FALSE;
		rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=0;

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[moduleIndex].deviceInfo,0,sizeof(rtl_multicastDeviceInfo_t));
#endif
	       return SUCCESS;
       }
	   
	 return SUCCESS;
	
}

static void _rtl865x_configIgmpSnoopingExpire(int32 disableExpire)
{
	uint32 maxTime=0xffffffff;
	
	if((rtl_mCastTimerParas.disableExpire==FALSE) && (disableExpire==TRUE))
	{
		rtl_mCastTimerParas.disableExpire=TRUE;
	}
		
	if((rtl_mCastTimerParas.disableExpire==TRUE) && (disableExpire==FALSE) )
	{
#if defined(__linux__) && defined(__KERNEL__)
		struct timeval currentTimeVector; 
		do_gettimeofday(&currentTimeVector);
		/*reset start time*/
		if(currentTimeVector.tv_sec>=rtl_sysUpSeconds)
		{
			rtl_startTime=(uint32)(currentTimeVector.tv_sec)-rtl_sysUpSeconds;
		}
		else
		{
			/*avoid timer wrap back*/
			rtl_startTime=maxTime-rtl_sysUpSeconds+(uint32)(currentTimeVector.tv_sec)+1;
		}
#endif
		rtl_mCastTimerParas.disableExpire=FALSE;
	}
	

	return;
}

//External called function by high level program
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters)
{
	_rtl865x_configIgmpSnoopingExpire(mCastTimerParameters.disableExpire);

	if(mCastTimerParameters.groupMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.groupMemberAgingTime= mCastTimerParameters.groupMemberAgingTime;
	}
	
	if(mCastTimerParameters.lastMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.lastMemberAgingTime= mCastTimerParameters.lastMemberAgingTime;
	}

	if(mCastTimerParameters.querierPresentInterval!=0)
	{
	
		rtl_mCastTimerParas.querierPresentInterval=mCastTimerParameters.querierPresentInterval;
	}


	if(mCastTimerParameters.dvmrpRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastTimerParameters.dvmrpRouterAgingTime;
	}

	if(mCastTimerParameters.mospfRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.mospfRouterAgingTime=mCastTimerParameters.mospfRouterAgingTime;
	}

	if(mCastTimerParameters.pimRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.pimRouterAgingTime=mCastTimerParameters.pimRouterAgingTime;
	}
	
	return;
}


int32 rtl_configIgmpSnoopingModule(uint32 moduleIndex, struct rtl_mCastSnoopingLocalConfig *mCastSnoopingLocalConfig)
{

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(mCastSnoopingLocalConfig==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	rtl_mCastModuleArray[moduleIndex].enableFastLeave=mCastSnoopingLocalConfig->enableFastLeave;
	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=mCastSnoopingLocalConfig->unknownMcastFloodMap;
	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=mCastSnoopingLocalConfig->staticRouterPortMask;
		
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[0]=mCastSnoopingLocalConfig->gatewayMac[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[1]=mCastSnoopingLocalConfig->gatewayMac[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[2]=mCastSnoopingLocalConfig->gatewayMac[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[3]=mCastSnoopingLocalConfig->gatewayMac[3];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[4]=mCastSnoopingLocalConfig->gatewayMac[4];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[5]=mCastSnoopingLocalConfig->gatewayMac[5];



	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=mCastSnoopingLocalConfig->gatewayIpv4Addr;
	
#ifdef CONFIG_RTL_MLD_SNOOPING		
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[0]=mCastSnoopingLocalConfig->gatewayIpv6Addr[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[1]=mCastSnoopingLocalConfig->gatewayIpv6Addr[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[2]=mCastSnoopingLocalConfig->gatewayIpv6Addr[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[3]=mCastSnoopingLocalConfig->gatewayIpv6Addr[3];
#endif
	return SUCCESS;
}

/*
@func int32	| rtl_maintainMulticastSnoopingTimerList	|   Multicast snooping timer list maintenance function.
@parm  uint32	| currentSystemTime	|The current system time (unit: seconds).
@rvalue SUCCESS	|Always return SUCCESS.
@comm 
 This function should be called once a second to maintain multicast timer list.
*/
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime)
{
	/* maintain current time */
	uint32 i=0;
	uint32 maxTime=0xffffffff;

	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			if((currentSystemTime%DEFAULT_MCAST_FLOW_EXPIRE_TIME)==0)
			{
				rtl_doMcastFlowRecycle(moduleIndex, BOTH_IPV4_IPV6);
			}
			
		}
	}
	#endif	
	
	if(rtl_mCastTimerParas.disableExpire==TRUE)
	{
		return SUCCESS;
	}
	
	/*handle timer conter overflow*/
	if(currentSystemTime>rtl_startTime)
	{
		rtl_sysUpSeconds=currentSystemTime-rtl_startTime;
	}
	else
	{
		rtl_sysUpSeconds=(maxTime-rtl_startTime)+currentSystemTime+1;
	}

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
			strcpy(timerEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
			timerEventContext.moduleIndex=moduleIndex;
			#endif

			/*maintain ipv4 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				 if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				 {
				 	timerEventContext.ipVersion=IP_VERSION4;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=0;
						timerEventContext.groupAddr[2]=0;
						timerEventContext.groupAddr[3]=0;
						rtl_checkGroupEntryTimer(groupEntryPtr,  rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				 }
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING		
			/*maintain ipv6 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
				{
					timerEventContext.ipVersion=IP_VERSION6;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
						timerEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
						timerEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}



int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 * macFrame,  uint32 portNum, uint32 *fwdPortMask)
{

	struct rtl_macFrameInfo macFrameInfo;

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	reportEventContext.portMask=1<<portNum;
	#endif

	*fwdPortMask=(~(1<<portNum)) & 0xFFFFFFFF;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
		
	//rtl_parseMacFrame(moduleIndex, macFrame, TRUE, &macFrameInfo);
	rtl_parseMacFrame(moduleIndex, macFrame, FALSE, &macFrameInfo);
	if(  rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		if(macFrameInfo.ipBuf==NULL)
		{
			return FAILED;
		}
		
		if((macFrameInfo.ipVersion!=IP_VERSION4) && (macFrameInfo.ipVersion!=IP_VERSION6))
		{
		
			return FAILED;
		}
		
#ifndef CONFIG_RTL_MLD_SNOOPING	
		if (macFrameInfo.ipVersion==IP_VERSION6)
		{
			return FAILED;
		}
#endif
		/*port num starts from 0*/
		if(portNum>=MAX_SUPPORT_PORT_NUMBER)
		{
			return FAILED;
		}

		if(macFrameInfo.checksumFlag!=SUCCESS)
		{
			return FAILED;
		}
		
		switch(macFrameInfo.l3Protocol)
		{

			case IGMP_PROTOCOL:
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case ICMP_PROTOCOL:
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;


			case DVMRP_PROTOCOL:
				*fwdPortMask=rtl_processDvmrp(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case MOSPF_PROTOCOL:
				*fwdPortMask=rtl_processMospf(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;
				
			case PIM_PROTOCOL:
				*fwdPortMask=rtl_processPim(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			default: break;
		}
		
	}
	
	return SUCCESS;
}

#ifdef CONFIG_RECORD_MCAST_FLOW
static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo)
{
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 

	if(multicastFwdInfo==NULL)
	{
		return FAILED;
	}
	
	mcastFlowEntry=rtl_searchMcastFlowEntry(moduleIndex, ipVersion, sourceIpAddr, groupAddr);

	if(mcastFlowEntry==NULL)
	{

		mcastFlowEntry=rtl_allocateMcastFlowEntry();
		if(mcastFlowEntry==NULL)
		{
			rtl_doMcastFlowRecycle(moduleIndex, ipVersion);
			
			mcastFlowEntry=rtl_allocateMcastFlowEntry();
			if(mcastFlowEntry==NULL)
			{
				rtl_gluePrintf("run out of multicast flow entry!\n");
				return FAILED;
			}
		}
	
		if(ipVersion==IP_VERSION4)
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->groupAddr[0]=groupAddr[0];
		
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->serverAddr[1]=sourceIpAddr[1];
			mcastFlowEntry->serverAddr[2]=sourceIpAddr[2];
			mcastFlowEntry->serverAddr[3]=sourceIpAddr[3];
			
			mcastFlowEntry->groupAddr[0]=groupAddr[0];
			mcastFlowEntry->groupAddr[1]=groupAddr[1];
			mcastFlowEntry->groupAddr[2]=groupAddr[2];
			mcastFlowEntry->groupAddr[3]=groupAddr[3];
		}
#endif		

		mcastFlowEntry->ipVersion=ipVersion;

		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));

		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;
		
		rtl_linkMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
		
		return SUCCESS;
			
	}
	else
	{
		/*update forward port mask information */
		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));
		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;
		return SUCCESS;
	}

	return SUCCESS;
}

static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr)
{
	uint32 hashIndex;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
	
	if(NULL==groupAddr)
	{
		return ;
	}
	

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
	
	while (mcastFlowEntry!=NULL)
	{	
		nextMcastFlowEntry=mcastFlowEntry->next;

		if(ipVersion==mcastFlowEntry->ipVersion)		
		{
		
				
#ifdef CONFIG_RTL_MLD_SNOOPING	
			if((groupAddr[0]==0)&&(groupAddr[1]==0)&&(groupAddr[2]==0)&&(groupAddr[3]==0))	
#else
			if(groupAddr[0]==0)
#endif
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
			}
			else
			{
#ifdef CONFIG_RTL_MLD_SNOOPING	
				if (	(mcastFlowEntry->groupAddr[0]==groupAddr[0])&&(mcastFlowEntry->groupAddr[1]==groupAddr[1])&&
					(mcastFlowEntry->groupAddr[2]==groupAddr[2])&&(mcastFlowEntry->groupAddr[3]==groupAddr[3])	)	
#else
				if(mcastFlowEntry->groupAddr[0] == groupAddr[0])
#endif	
				{
					rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
				}
			}


					
		}
		
		mcastFlowEntry = nextMcastFlowEntry;
	}
	
	return ;

}

static void rtl_doMcastFlowRecycle(uint32 moduleIndex, uint32 ipVersion)
{
	uint32 i;
	uint32 freeCnt=0;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* oldestMcastFlowEntry = NULL;


	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
		
		if(oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}
		
		while (mcastFlowEntry!=NULL)
		{	
			nextMcastFlowEntry=mcastFlowEntry->next;
			/*keep the most recently used entry*/
			if((mcastFlowEntry->refreshTime+DEFAULT_MCAST_FLOW_EXPIRE_TIME) < rtl_sysUpSeconds)
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
				freeCnt++;
			}
			mcastFlowEntry=nextMcastFlowEntry;
			
		}
	}

	if(freeCnt>0)
	{
		return;
	}

	/*if too many concurrent flow,we have to do LRU*/
	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
		
		if(oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}
		
		while (mcastFlowEntry!=NULL)
		{	
			nextMcastFlowEntry=mcastFlowEntry->next;
			if(mcastFlowEntry->refreshTime < oldestMcastFlowEntry->refreshTime)
			{
				oldestMcastFlowEntry=mcastFlowEntry;
			}
			
			mcastFlowEntry=nextMcastFlowEntry;
			
		}
	}

	if(oldestMcastFlowEntry!=NULL)
	{
		rtl_deleteMcastFlowEntry(oldestMcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);

	}
					
	return ;

}

#endif

int32 rtl_getMulticastDataFwdPortMask(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, uint32 *fwdPortMask)
{
	int32 retVal=FAILED;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
		
	if(multicastDataInfo==NULL)
	{
		return FAILED;
	}

	if(fwdPortMask==NULL)
	{
		return FAILED;
	}
	
	retVal=rtl_getMulticastDataFwdInfo( moduleIndex, multicastDataInfo, &multicastFwdInfo);

	*fwdPortMask=multicastFwdInfo.fwdPortMask;

	if(retVal==SUCCESS)
	{
		if(multicastFwdInfo.unknownMCast==TRUE)
		{
			return FAILED;
		}
		else
		{
			return SUCCESS;
		}
	}
	
	return FAILED;
	
}

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo)
{
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 
	#endif
	struct rtl_groupEntry * groupEntry=NULL;
	uint32 multicastRouterPortMask=0;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
		
	if(multicastDataInfo==NULL)
	{
		return FAILED;
	}

	if(multicastFwdInfo==NULL)
	{
		return FAILED;
	}

	memset(multicastFwdInfo, 0, sizeof(struct rtl_multicastFwdInfo));

	if(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR1)
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		multicastFwdInfo->cpuFlag=TRUE;
		
		return FAILED;
	}
	#if 0
	if(IN_MULTICAST_RESV1(multicastDataInfo->groupAddr[0]) )
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		multicastFwdInfo->cpuFlag=TRUE;
		
		return FAILED;
	}
	#endif
	#ifdef CONFIG_RECORD_MCAST_FLOW
	mcastFlowEntry=rtl_searchMcastFlowEntry( moduleIndex, multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr);
	if(mcastFlowEntry!=NULL)
	{
		memcpy(multicastFwdInfo, &mcastFlowEntry->multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo));
		return SUCCESS;
	}
	#endif
	
	groupEntry=rtl_searchGroupEntry(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->groupAddr); 

	if(groupEntry==NULL)
	{
		multicastFwdInfo->unknownMCast=TRUE;
		multicastFwdInfo->fwdPortMask= rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		return FAILED;
	}
	else
	{
		/*here to get multicast router port mask and forward port mask*/
		//multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, multicastDataInfo->ipVersion, rtl_sysUpSeconds);
		multicastFwdInfo->fwdPortMask=rtl_getGroupSourceFwdPortMask(groupEntry, multicastDataInfo->sourceIp, rtl_sysUpSeconds);
		multicastFwdInfo->fwdPortMask=(multicastFwdInfo->fwdPortMask|multicastRouterPortMask);

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_recordMcastFlow(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr, multicastFwdInfo);
		#endif
		
		return SUCCESS;
	
	}
	return FAILED;

}
#if defined(__linux__) && defined(__KERNEL__)

static void rtl_multicastSysTimerExpired(uint32 expireDada)
{
	struct timeval currentTimeVector; 
	
	do_gettimeofday(&currentTimeVector);
	rtl_maintainMulticastSnoopingTimerList((uint32)(currentTimeVector.tv_sec));
	mod_timer(&igmpSysTimer, jiffies+HZ);
	
}

static void rtl_multicastSysTimerInit(void)
{
	struct timeval startTimeVector; 
	do_gettimeofday(&startTimeVector);
	rtl_startTime=(uint32)(startTimeVector.tv_sec);
	rtl_sysUpSeconds=0;  
	
	init_timer(&igmpSysTimer);
	igmpSysTimer.data=igmpSysTimer.expires;
	igmpSysTimer.expires=jiffies+HZ;
	igmpSysTimer.function=(void*)rtl_multicastSysTimerExpired;
	add_timer(&igmpSysTimer);
}

static void rtl_multicastSysTimerDestroy(void)
{
	del_timer(&igmpSysTimer);
}

#endif

int32 rtl_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	int i;
	*moduleIndex=0xFFFFFFFF;
	if(devInfo==NULL)
	{
		return FAILED;
	}
	
	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if(rtl_mCastModuleArray[i].enableSnooping==TRUE)
		{
			if(strcmp(rtl_mCastModuleArray[i].deviceInfo.devName, devInfo->devName)==0)
			{
				*moduleIndex=i;
				return SUCCESS;
			}
		}
	}
	
	return FAILED;
}

int32 rtl865x_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	return rtl_getDeviceIgmpSnoopingModuleIndex(devInfo,moduleIndex);
}

int32 rtl_setIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(devInfo==NULL)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	memcpy(&rtl_mCastModuleArray[moduleIndex].deviceInfo,devInfo, sizeof(rtl_multicastDeviceInfo_t));
	
	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(devInfo==NULL)
	{
		return FAILED;
	}
	memset(devInfo,0,sizeof(rtl_multicastDeviceInfo_t));
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	memcpy(devInfo,&rtl_mCastModuleArray[moduleIndex].deviceInfo, sizeof(rtl_multicastDeviceInfo_t));
	return SUCCESS;
}

int32 rtl_setIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 staticRouterPortMask)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=staticRouterPortMask;
	
	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 *staticRouterPortMask)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(staticRouterPortMask==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	*staticRouterPortMask=rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;
	
	return SUCCESS;
}


int32 rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=unknownMCastFloodMap;
	
	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(unknownMCastFloodMap==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	*unknownMCastFloodMap=rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;
	
	return SUCCESS;
}

#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v)
{
	int32 moduleIndex;
	int32 hashIndex,groupCnt,clientCnt;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntryPtr;
	#ifdef CONFIG_RECORD_MCAST_FLOW	
	int32 flowCnt;
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 
	#endif

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			seq_printf(s, "-------------------------------------------------------------------------\n");
			seq_printf(s, "module index:%d, ",moduleIndex);
			#ifdef CONFIG_RTL_HARDWARE_MULTICAST
			seq_printf(s, "device:%s, portMask:0x%x\n\n",rtl_mCastModuleArray[moduleIndex].deviceInfo.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.portMask);
			#else
			seq_printf(s, "\n\n");
			#endif
			seq_printf(s,"igmp list:\n");
			groupCnt=0;	
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{
					groupCnt++;
					seq_printf(s, "    [%d] Group address:%d.%d.%d.%d\n",groupCnt,//hashIndex
					groupEntryPtr->groupAddr[0]>>24, (groupEntryPtr->groupAddr[0]&0x00ff0000)>>16,
					(groupEntryPtr->groupAddr[0]&0x0000ff00)>>8, (groupEntryPtr->groupAddr[0]&0xff));
					
					clientEntry=groupEntryPtr->clientList;
					
					clientCnt=0;
					while (clientEntry!=NULL)
					{	
						
						clientCnt++;
						seq_printf(s, "        <%d>%d.%d.%d.%d\\port %d\\IGMPv%d\\",clientCnt,
							clientEntry->clientAddr[0]>>24, (clientEntry->clientAddr[0]&0x00ff0000)>>16,
							(clientEntry->clientAddr[0]&0x0000ff00)>>8, clientEntry->clientAddr[0]&0xff,clientEntry->portNum, clientEntry->igmpVersion);
						
						seq_printf(s, "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
						if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
						{
							seq_printf(s, ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
						}
						else
						{
							seq_printf(s, ":0s");
						}
						
						sourceEntryPtr=clientEntry->sourceList;
						if(sourceEntryPtr!=NULL)
						{
							seq_printf(s, "\\source list:");
						}

						while(sourceEntryPtr!=NULL)
						{
							seq_printf(s, "%d.%d.%d.%d:",
									sourceEntryPtr->sourceAddr[0]>>24, (sourceEntryPtr->sourceAddr[0]&0x00ff0000)>>16,
									(sourceEntryPtr->sourceAddr[0]&0x0000ff00)>>8, (sourceEntryPtr->sourceAddr[0]&0xff));
					
							if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
							{
								seq_printf(s, "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
							}
							else
							{
								seq_printf(s, "0s");
							}

							if(sourceEntryPtr->next!=NULL)
							{
								seq_printf(s, ", ");
							}
							
							sourceEntryPtr=sourceEntryPtr->next;
						}

						
						seq_printf(s, "\n");
						clientEntry = clientEntry->next;
					}
					
					seq_printf(s, "\n");	
					groupEntryPtr=groupEntryPtr->next;	
				}
				
		       }
			if(groupCnt==0)
			{
				seq_printf(s,"\tnone\n");
			}
		
#if defined (CONFIG_RTL_MLD_SNOOPING)			
			seq_printf(s, "\n\n");
			seq_printf(s, "mld list:\n");
			groupCnt=0;	
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{
					groupCnt++;	
					seq_printf(s, "    [%d] Group address:%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x\n",groupCnt,
					(groupEntryPtr->groupAddr[0])>>28,(groupEntryPtr->groupAddr[0]<<4)>>28, (groupEntryPtr->groupAddr[0]<<8)>>28,(groupEntryPtr->groupAddr[0]<<12)>>28, 
					(groupEntryPtr->groupAddr[0]<<16)>>28,(groupEntryPtr->groupAddr[0]<<20)>>28,(groupEntryPtr->groupAddr[0]<<24)>>28, (groupEntryPtr->groupAddr[0]<<28)>>28, 
					(groupEntryPtr->groupAddr[1])>>28,(groupEntryPtr->groupAddr[1]<<4)>>28, (groupEntryPtr->groupAddr[1]<<8)>>28,(groupEntryPtr->groupAddr[1]<<12)>>28, 
					(groupEntryPtr->groupAddr[1]<<16)>>28,(groupEntryPtr->groupAddr[1]<<20)>>28,(groupEntryPtr->groupAddr[1]<<24)>>28, (groupEntryPtr->groupAddr[1]<<28)>>28, 
					(groupEntryPtr->groupAddr[2])>>28,(groupEntryPtr->groupAddr[2]<<4)>>28, (groupEntryPtr->groupAddr[2]<<8)>>28,(groupEntryPtr->groupAddr[2]<<12)>>28, 
					(groupEntryPtr->groupAddr[2]<<16)>>28,(groupEntryPtr->groupAddr[2]<<20)>>28,(groupEntryPtr->groupAddr[2]<<24)>>28, (groupEntryPtr->groupAddr[2]<<28)>>28, 
					(groupEntryPtr->groupAddr[3])>>28,(groupEntryPtr->groupAddr[3]<<4)>>28, (groupEntryPtr->groupAddr[3]<<8)>>28,(groupEntryPtr->groupAddr[3]<<12)>>28, 
					(groupEntryPtr->groupAddr[3]<<16)>>28,(groupEntryPtr->groupAddr[3]<<20)>>28,(groupEntryPtr->groupAddr[3]<<24)>>28, (groupEntryPtr->groupAddr[3]<<28)>>28);
					
					clientEntry=groupEntryPtr->clientList;
					
					clientCnt=0;
					while (clientEntry!=NULL)
					{	
						
						clientCnt++;
						seq_printf(s, "        <%d>%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x\\port %d\\MLDv%d\\",clientCnt,
							(clientEntry->clientAddr[0])>>28,(clientEntry->clientAddr[0]<<4)>>28, (clientEntry->clientAddr[0]<<8)>>28,(clientEntry->clientAddr[0]<<12)>>28, 
							(clientEntry->clientAddr[0]<<16)>>28,(clientEntry->clientAddr[0]<<20)>>28,(clientEntry->clientAddr[0]<<24)>>28, (clientEntry->clientAddr[0]<<28)>>28, 
							(clientEntry->clientAddr[1])>>28,(clientEntry->clientAddr[1]<<4)>>28, (clientEntry->clientAddr[1]<<8)>>28,(clientEntry->clientAddr[1]<<12)>>28, 
							(clientEntry->clientAddr[1]<<16)>>28,(clientEntry->clientAddr[1]<<20)>>28,(clientEntry->clientAddr[1]<<24)>>28, (clientEntry->clientAddr[1]<<28)>>28, 
							(clientEntry->clientAddr[2])>>28,(clientEntry->clientAddr[2]<<4)>>28, (clientEntry->clientAddr[2]<<8)>>28,(clientEntry->clientAddr[2]<<12)>>28, 
							(clientEntry->clientAddr[2]<<16)>>28,(clientEntry->clientAddr[2]<<20)>>28,(clientEntry->clientAddr[2]<<24)>>28, (clientEntry->clientAddr[2]<<28)>>28, 
							(clientEntry->clientAddr[3])>>28,(clientEntry->clientAddr[3]<<4)>>28, (clientEntry->clientAddr[3]<<8)>>28,(clientEntry->clientAddr[3]<<12)>>28, 
							(clientEntry->clientAddr[3]<<16)>>28,(clientEntry->clientAddr[3]<<20)>>28,(clientEntry->clientAddr[3]<<24)>>28, (clientEntry->clientAddr[3]<<28)>>28, 
							clientEntry->portNum, clientEntry->igmpVersion);
						
						seq_printf(s, "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
						if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
						{
							seq_printf(s, ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
						}
						else
						{
							seq_printf(s, ":0s");
						}
						
						sourceEntryPtr=clientEntry->sourceList;
						if(sourceEntryPtr!=NULL)
						{
							seq_printf(s, "\\source list:");
						}

						while(sourceEntryPtr!=NULL)
						{
							seq_printf(s, "%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x:",
								(sourceEntryPtr->sourceAddr[0])>>28,(sourceEntryPtr->sourceAddr[0]<<4)>>28, (sourceEntryPtr->sourceAddr[0]<<8)>>28,(sourceEntryPtr->sourceAddr[0]<<12)>>28, 
								(sourceEntryPtr->sourceAddr[0]<<16)>>28,(sourceEntryPtr->sourceAddr[0]<<20)>>28,(sourceEntryPtr->sourceAddr[0]<<24)>>28, (sourceEntryPtr->sourceAddr[0]<<28)>>28, 
								(sourceEntryPtr->sourceAddr[1])>>28,(sourceEntryPtr->sourceAddr[1]<<4)>>28, (sourceEntryPtr->sourceAddr[1]<<8)>>28,(sourceEntryPtr->sourceAddr[1]<<12)>>28, 
								(sourceEntryPtr->sourceAddr[1]<<16)>>28,(sourceEntryPtr->sourceAddr[1]<<20)>>28,(sourceEntryPtr->sourceAddr[1]<<24)>>28, (sourceEntryPtr->sourceAddr[1]<<28)>>28, 
								(sourceEntryPtr->sourceAddr[2])>>28,(sourceEntryPtr->sourceAddr[2]<<4)>>28, (sourceEntryPtr->sourceAddr[2]<<8)>>28,(sourceEntryPtr->sourceAddr[2]<<12)>>28, 
								(sourceEntryPtr->sourceAddr[2]<<16)>>28,(sourceEntryPtr->sourceAddr[2]<<20)>>28,(sourceEntryPtr->sourceAddr[2]<<24)>>28, (sourceEntryPtr->sourceAddr[2]<<28)>>28, 
								(sourceEntryPtr->sourceAddr[3])>>28,(sourceEntryPtr->sourceAddr[3]<<4)>>28, (sourceEntryPtr->sourceAddr[3]<<8)>>28,(sourceEntryPtr->sourceAddr[3]<<12)>>28, 
								(sourceEntryPtr->sourceAddr[3]<<16)>>28,(sourceEntryPtr->sourceAddr[3]<<20)>>28,(sourceEntryPtr->sourceAddr[3]<<24)>>28, (sourceEntryPtr->sourceAddr[3]<<28)>>28);
					
							if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
							{
								seq_printf(s, "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
							}
							else
							{
								seq_printf(s, "0s");
							}

							if(sourceEntryPtr->next!=NULL)
							{
								seq_printf(s, ", ");
							}
							
							sourceEntryPtr=sourceEntryPtr->next;
						}

						seq_printf(s, "\n");
						clientEntry = clientEntry->next;
					}
					
					seq_printf(s, "\n");	
					groupEntryPtr=groupEntryPtr->next;	
				}
				
		       }
			if(groupCnt==0)
			{
				seq_printf(s,"\tnone\n");
			}
#endif			
#ifdef CONFIG_RECORD_MCAST_FLOW	
			seq_printf(s,"ipv4 flow list:\n");
			flowCnt=1;
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{

				/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
				
				while(mcastFlowEntry!=NULL)
				{
					if(mcastFlowEntry->ipVersion==IP_VERSION4)
					{
						seq_printf(s, "    [%d] %d.%d.%d.%d-->",flowCnt,
						mcastFlowEntry->serverAddr[0]>>24, (mcastFlowEntry->serverAddr[0]&0x00ff0000)>>16,
						(mcastFlowEntry->serverAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->serverAddr[0]&0xff));
					
						seq_printf(s, "%d.%d.%d.%d-->",
						mcastFlowEntry->groupAddr[0]>>24, (mcastFlowEntry->groupAddr[0]&0x00ff0000)>>16,
						(mcastFlowEntry->groupAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->groupAddr[0]&0xff));
		
						seq_printf(s, "port mask:0x%x\n",mcastFlowEntry->multicastFwdInfo.fwdPortMask);
					}

					flowCnt++;
					mcastFlowEntry=mcastFlowEntry->next;
				}

			}
			seq_printf(s, "ipv6 flow list:\n");
			flowCnt=1;
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{

				/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
				
				while(mcastFlowEntry!=NULL)
				{
					if(mcastFlowEntry->ipVersion==IP_VERSION6)
					{
						seq_printf(s, "    [%d] %x-%x-%x-%x-->",flowCnt,
						mcastFlowEntry->serverAddr[0], mcastFlowEntry->serverAddr[1],
						mcastFlowEntry->serverAddr[2], (mcastFlowEntry->serverAddr[3]);
						
						seq_printf(s, "%x-%x-%x-%x-->",
						mcastFlowEntry->groupAddr[1], mcastFlowEntry->groupAddr[1],
						mcastFlowEntry->groupAddr[2], mcastFlowEntry->groupAddr[3];
			
						seq_printf(s, "port mask:0x%x\n",mcastFlowEntry->multicastFwdInfo.fwdPortMask);
					}

					flowCnt++;
					mcastFlowEntry=mcastFlowEntry->next;
				}

			}
#endif
		}
	}

	seq_printf(s, "------------------------------------------------------------------\n");
	return SUCCESS;
}
#endif

void rtl865x_igmpLinkStatusChangeCallback(uint32 moduleIndex, rtl_igmpPortInfo_t * portInfo)
{
	int32 hashIndex;
	int32 clearFlag=FALSE;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* nextClientEntry=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntry, *nextMcastFlowEntry;
	#endif
	
	if(portInfo==NULL)
	{
		return ;
	}
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return ;
	}
	
#ifdef CONFIG_RECORD_MCAST_FLOW			
	for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
     	{

     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

		while(mcastFlowEntry!=NULL)
		{
			nextMcastFlowEntry=mcastFlowEntry->next;
			
			/*clear multicast forward flow cache*/
			 rtl_deleteMcastFlowEntry( mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
			
			mcastFlowEntry=nextMcastFlowEntry;
		}

	}
#endif


	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
	
		for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
			while(groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
				while (clientEntry!=NULL)
				{	
					/*save next client entry first*/
					nextClientEntry=clientEntry->next;
					if(((1<<clientEntry->portNum) & portInfo->linkPortMask)==0)
					{
						rtl_deleteClientEntry(groupEntryPtr,clientEntry);
						clearFlag=TRUE;
					}
					
		
					clientEntry = nextClientEntry;
				}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				if(clearFlag==TRUE)
				{
					strcpy(linkEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
					linkEventContext.moduleIndex=moduleIndex;
					
					linkEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
					linkEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
					linkEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
					linkEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
					
					linkEventContext.sourceAddr[0]=0;
					linkEventContext.sourceAddr[1]=0;
					linkEventContext.sourceAddr[2]=0;
					linkEventContext.sourceAddr[3]=0;
					
					//rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &linkEventContext);
					rtl_handle_igmpgroup_change(&linkEventContext);
				}
#endif
				groupEntryPtr=groupEntryPtr->next;	
			}
			
	       }
		
#if defined (CONFIG_RTL_MLD_SNOOPING)
		for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
			while(groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
				while (clientEntry!=NULL)
				{	
					/*save next client entry first*/
					nextClientEntry=clientEntry->next;
					if(((1<<clientEntry->portNum) & portInfo->linkPortMask)==0)
					{
						rtl_deleteClientEntry(groupEntryPtr,clientEntry);
					}
					
					clientEntry = nextClientEntry;
				}
				groupEntryPtr=groupEntryPtr->next;	
			}
			
	       }
#endif		

	}
	


	return ;
}


int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo * groupInfo)
{
	int32 moduleIndex;
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr;
	
	if(groupInfo==NULL)
	{
		return FAILED;
	}

	memset(groupInfo, 0 , sizeof(struct rtl_groupInfo));
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			hashIndex=rtl_hashMask&groupAddr;
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				
			while(groupEntryPtr!=NULL)
			{
				if(groupEntryPtr->groupAddr[0]==groupAddr)
				{
					groupInfo->ownerMask |= (1<<moduleIndex);
					break;
				}
				groupEntryPtr=groupEntryPtr->next;
			}
		      
		}
	}

	return SUCCESS;
}

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int rtl_handle_igmpgroup_change(rtl_multicastEventContext_t* param)	
{
	if(strlen(param->devName)==0)
	{
		return FAILED;
	}

	/*case 1:this is multicast event from bridge(br0) module */
	if(memcmp(param->devName,RTL_BR_NAME,3)==0)
	{
		struct rtl_multicastFwdInfo multicastFwdInfo;
		struct rtl_multicastDataInfo multicastDataInfo;
		struct rtl_multicastDeviceInfo_s igmp_snooping_module;
		int retVal;
		multicastDataInfo.ipVersion=4;
		multicastDataInfo.sourceIp[0]=  param->sourceAddr[0];
		multicastDataInfo.groupAddr[0]= param->groupAddr[0];
		retVal= rtl_getMulticastDataFwdInfo(param->moduleIndex, &multicastDataInfo, &multicastFwdInfo);
		if(retVal!=SUCCESS)
		{
			return FAILED;
		}

		retVal = rtl_getIgmpSnoopingModuleDevInfo(param->moduleIndex,&igmp_snooping_module);
		if(retVal!=SUCCESS)
		{
			return FAILED;
		}

		/* if there are netif under br0 but not nic port (ex.wlan0,vc0) , cancel multicast-acc */
		if( multicastFwdInfo.fwdPortMask & igmp_snooping_module.swPortMask )
		{
			rtl865x_delMulticastEntry(param->groupAddr[0]);
		}	

		return SUCCESS;
	}


	/*case 2:this is multicast event from ethernet(eth*) module */
	if(memcmp(param->devName,"eth*",4)==0)
	{
		struct rtl_multicastFwdInfo multicastFwdInfo;
		struct rtl_multicastDataInfo multicastDataInfo;
		struct rtl_multicastDeviceInfo_s igmp_snooping_module;
		int retVal;
		multicastDataInfo.ipVersion=4;
		multicastDataInfo.sourceIp[0]=  param->sourceAddr[0];
		multicastDataInfo.groupAddr[0]= param->groupAddr[0];
		retVal= rtl_getMulticastDataFwdInfo(param->moduleIndex, &multicastDataInfo, &multicastFwdInfo);
		if(retVal!=SUCCESS)
		{
			return FAILED;
		}

		retVal = rtl_getIgmpSnoopingModuleDevInfo(param->moduleIndex,&igmp_snooping_module);
		if(retVal!=SUCCESS)
		{
			return FAILED;
		}

		rtl865x_resetMulticastEntry(param->groupAddr[0],multicastFwdInfo.fwdPortMask);
		return SUCCESS;
	}
	return FAILED;	

}
#endif
