//============================================================
//
// File Name: HalDMOutSrc_AP.c 
//
// Description:
//
// This file is for common outsource dynamic mechanism for partner.
//
//
//============================================================

#ifndef _HALDM_COMMON_C_
#define _HALDM_COMMON_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#endif

#include "8192cd_cfg.h"
#include "8192cd.h"
#include "8192cd_hw.h"
#include "8192cd_headers.h"
#include "8192cd_debug.h"

#ifdef USE_OUT_SRC
#include "./OUTSRC/odm_precomp.h"
#else
#define TX_POWER_NEAR_FIELD_THRESH_AP	HP_LOWER
#endif

#if defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#include <bsp/bspchip.h>
#endif

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#ifndef CONFIG_RTL_8198B
#ifndef BSP_WDTCNR
        #define BSP_WDTCNR 0xB800311C
#endif
#endif
#endif

#ifdef HW_ANT_SWITCH
#define RXDVY_A_EN		((HW_DIV_ENABLE && !priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#define RXDVY_B_EN		((HW_DIV_ENABLE &&  priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#endif


//3 ============================================================
//3 DIG related functions
//3 ============================================================

int getIGIFor1RCCA(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}

#if 1

#if defined(DETECT_STA_EXISTANCE)  && defined(CONFIG_WLAN_HAL)
// Check for STA existance. If STA disappears, disconnect it. Added by Annie, 2010-08-10.
void DetectSTAExistance88XX(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat)
{
	//unsigned char tmpbuf[16];

	// Parameters
	const unsigned int		maxTxFailCnt = 300;		// MAX Tx fail packet count
	const unsigned int		minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	const unsigned int		txFailSecThr=  3;			// threshold of Tx Fail Time (in second)
	const unsigned int		NoTXOKSecThr=  5;			// threshold of NO Tx ok Time (in second)
	const unsigned int		NoRXOKSecThr=  5;			// threshold of NO Tx ok Time (in second)	

	// Temporarily change Retry Limit when TxFail. (tfrl: TxFailRetryLimit)
	const unsigned char	TFRL = 7;				// New Retry Limit value
	const unsigned char	TFRL_FailCnt = 2;		// Tx Fail Count threshold to set Retry Limit
	const unsigned char	TFRL_SetTime = 2;		// Time to set Retry Limit (in second)
	const unsigned char	TFRL_RcvTime = 3;		// Time to recover Retry Limit (in second)

    const unsigned short Back_retty_value = 0x10;
    
    if( report->txok != 0 )
    { // Reset Counter
            pstat->tx_conti_fail_cnt = 0;
            pstat->tx_last_good_time = priv->up_time;
            pstat->leave = 0;
    }
    else if( report->txfail != 0 )
    {
        pstat->tx_conti_fail_cnt += report->txfail;
        printk( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", report->txfail, pstat->tx_conti_fail_cnt );

        //RTL_W16(RL, (TFRL&SRL_Mask)<<SRL_SHIFT|(TFRL&LRL_Mask)<<LRL_SHIFT);

        #if 0 // Not to modify retry limit
        if( priv->up_time >= (pstat->tx_last_good_time+TFRL_SetTime) &&
            pstat->tx_conti_fail_cnt >= TFRL_FailCnt && 
	#if defined(CONFIG_RTL8672) || defined (NOT_RTK_BSP) 
            !pstat->ht_cap_len && // legacy rate only
	#endif
            !priv->pshare->bRLShortened )
        { // Shorten retry limit, because AP spending too much time to send out g mode STA pending packets in HW queue.
            
            priv->pshare->bRLShortened = TRUE;
            printk( "== Shorten RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
        }
        #endif 

        if(     (pstat->tx_conti_fail_cnt >= maxTxFailCnt) ||
            (pstat->tx_conti_fail_cnt >= minTxFailCnt && priv->up_time >= (pstat->tx_last_good_time+txFailSecThr) )
            )
        { 
            // This STA is considered as disappeared, so delete it.
            DEBUG_WARN( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", pstat->tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
            DEBUG_WARN( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)pstat->tx_last_good_time, (int)priv->up_time, txFailSecThr );
            DEBUG_WARN( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5] );

            // del_sta(priv, tmpbuf);
            ++(pstat->leave);
            
            GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat->aid, DECREASE);
            //printk("Drop client packet, AID = %x TxFailCnt= %x \n",pstat->aid,pstat->tx_conti_fail_cnt);
            //RTL_W16(RL, (Back_retty_value&SRL_Mask)<<SRL_SHIFT|(Back_retty_value&LRL_Mask)<<LRL_SHIFT);
            // send H2C drop packet

            // Reset Counter
            pstat->tx_conti_fail_cnt = 0;
            pstat->tx_last_good_time = priv->up_time;

        }
    }
    else
    {
#if 0        
        //printk("AID[%x](pstat->leave)=%x priv->up_time=%x (pstat->tx_last_good_time+NoTXOKSecThr)=%x \n",pstat->aid,(pstat->leave),priv->up_time,(pstat->tx_last_good_time+NoTXOKSecThr));
        if (!(pstat->leave) && (priv->up_time  > (pstat->tx_last_good_time+NoTXOKSecThr)) 
                && (priv->up_time  > (pstat->rx_last_good_time+NoRXOKSecThr)) ) 
        {
            DEBUG_WARN("%s %d expire timeout, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
            GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat->aid, DECREASE);
            printk("Client no TRX over 10 Secs , Drop client packet, AID = %x \n",pstat->aid);
            ++(pstat->leave);
	    }           
#endif
    }
}
#endif
#if defined(DETECT_STA_EXISTANCE) && !defined(CONFIG_RTL_92C_SUPPORT) &&  !defined(CONFIG_RTL_92D_SUPPORT)

// Timer callback function to recover hardware retry limit register. Added by Annie, 2010-08-10.
#ifdef __KERNEL__
void RetryLimitRecovery(unsigned long task_priv)
#elif defined(__ECOS)
void RetryLimitRecovery(void *task_priv)
#endif
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if( priv->pshare->bRLShortened )
	{
//		RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);
		RTL_W16(RL, priv->pshare->RL_setting);
		priv->pshare->bRLShortened = FALSE;
		DEBUG_WARN( "== Recover RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
	}
}


// Chack STA leaving status; per interface. Added by Annie, 2010-08-10.
unsigned char NoLeavingSTA(struct rtl8192cd_priv *priv)
{
	unsigned char bStaAllOK = TRUE;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	phead = &priv->asoc_list;
	if (!netif_running(priv->dev) || list_empty(phead))
		return bStaAllOK;

	plist = phead->next;
	while (plist != phead)  {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if( pstat->tx_conti_fail_cnt != 0 ) {
			bStaAllOK = FALSE;
			break;
		}
		plist = plist->next;
	}

	return bStaAllOK;
}


// Chack STA leaving status for all active interface and recover retry limit register value. Added by Annie, 2010-08-10.
void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv)
{
	unsigned char bIfAllOK = TRUE;
	static int AllOKTimes = 0;
#ifdef MBSSID
	int i;
#endif
	// Parameter
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if( !NoLeavingSTA(priv) )
		bIfAllOK = FALSE;

    printk("bIfAllOK = %x \n",bIfAllOK);

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) && GET_VXD_PRIV(priv) ) {
		if( !NoLeavingSTA(GET_VXD_PRIV(priv)) )
			bIfAllOK = FALSE;
	}
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					if( !NoLeavingSTA(priv->pvap_priv[i]) )
						bIfAllOK = FALSE;
				}
			}
		}
	}
#endif

	if( bIfAllOK ) {
		AllOKTimes ++;

		if( AllOKTimes >= TFRL_RcvTime )
#ifdef __KERNEL__
			RetryLimitRecovery((unsigned long)priv);
#elif defined(__ECOS)
			RetryLimitRecovery((void *)priv);
#endif
	}
	else {
		AllOKTimes = 0;
	}
}

#endif //#if defined(DETECT_STA_EXISTANCE) 

#endif 



void set_DIG_state(struct rtl8192cd_priv *priv, int state)
{
	int value_IGI;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || IS_HAL_CHIP(priv)) 
		return;
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
	if (state) {
		priv->pshare->DIG_on = 1;
		priv->pshare->restore = 0;
	}
	else {
		priv->pshare->DIG_on = 0;
		if (priv->pshare->restore == 0) {
			if (priv->pshare->rf_ft_var.use_ext_lna == 1)
				value_IGI = 0x30;
			else
				value_IGI = 0x20;

#if defined(HW_ANT_SWITCH)
			// wirte new initial gain index into regC50/C58
			if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, value_IGI | RXDVY_B_EN);
#endif
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, getIGIFor1RCCA(value_IGI) | RXDVY_B_EN);
#endif
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, value_IGI | RXDVY_B_EN);
#endif
			}
#else
			// Write IGI into HW
			if (priv->pshare->rf_ft_var.one_path_cca == 0) 	{
				RTL_W8(0xc50, value_IGI);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, value_IGI);
#endif
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, getIGIFor1RCCA(value_IGI));
#endif
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if (
#ifdef CONFIG_RTL_92C_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
					|| 
#endif
					(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
					)
					RTL_W8(0xc58, value_IGI);
#endif
			}
#endif
			priv->pshare->restore = 1;
		}
#ifdef INTERFERENCE_CONTROL
		priv->pshare->phw->signal_strength = 0;
#endif
	}
}
}

void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	unsigned int dig_on = 0;

	if (OPMODE & WIFI_SITE_MONITOR)
		return;

#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

	if ((rssi_strength > priv->pshare->rf_ft_var.digGoUpperLevel)
		&& (rssi_strength < TX_POWER_NEAR_FIELD_THRESH_AP+1) && (priv->pshare->phw->signal_strength != 2)) {
#ifndef CONFIG_RTL_92D_SUPPORT
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);
#endif

		if (priv->pshare->phw->signal_strength != 3)
			dig_on++;

		priv->pshare->phw->signal_strength = 2;
	}
	else if ((rssi_strength > TX_POWER_NEAR_FIELD_THRESH_AP+5) && (priv->pshare->phw->signal_strength != 3)) {
#ifndef CONFIG_RTL_92D_SUPPORT
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);
#endif

		if (priv->pshare->phw->signal_strength != 2)
			dig_on++;

		priv->pshare->phw->signal_strength = 3;
	}
	else if (((rssi_strength < priv->pshare->rf_ft_var.digGoLowerLevel)
		&& (priv->pshare->phw->signal_strength != 1)) || !priv->pshare->phw->signal_strength) {
		// DIG off
#if 0
		set_DIG_state(priv, 0);
#endif

#ifndef CONFIG_RTL_92D_SUPPORT
		if (priv->pshare->is_40m_bw)
			//RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);
#endif

		priv->pshare->phw->signal_strength = 1;
	}

	if (dig_on) {
		// DIG on
		set_DIG_state(priv, 1);
	}

	//check_DC_TH_by_rssi(priv, rssi_strength);
}


void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss)
{
	if (do_ss) {
		// DIG off
		set_DIG_state(priv, 0);
	}
	else {
		// DIG on
#ifndef INTERFERENCE_CONTROL
		if (priv->pshare->phw->signal_strength > 1) 
#endif
		{
			set_DIG_state(priv, 1);
		}
	}
}

 extern unsigned char CCKSwingTable_Ch14 [][8];
 extern unsigned char CCKSwingTable_Ch1_Ch13[][8];
#ifndef CCK_TABLE_SIZE
#define CCK_TABLE_SIZE	33
#endif

 int get_CCK_swing_index(struct rtl8192cd_priv *priv)
 {
	 int TempCCk, index=12, i;
	 short channel;
#ifdef MP_TEST
	 if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		 channel=priv->pshare->working_channel;
	 else
#endif
		 channel = (priv->pmib->dot11RFEntry.dot11channel);
 
	 //Query CCK default setting From 0xa24
	 TempCCk = PHY_QueryBBReg(priv, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
	 TempCCk = cpu_to_le32(TempCCk);
	 for(i=0 ; i<CCK_TABLE_SIZE ; i++)		 {
		 if(channel==14) {
			 if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0) {
				 index = i;
				 break;
			 }
		 } else {
			 if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0) {
				 index = i;
				 break;
			 }
		 }
	 }
	 DEBUG_INFO("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch %d\n",
							 rCCK0_TxFilter2, TempCCk, index, channel);
	 return index;
 }

 void set_CCK_swing_index(struct rtl8192cd_priv *priv, short CCK_index)
{
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

#ifdef USE_OUT_SRC	
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {	
			if (channel !=14) {
				RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13_New[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13_New[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13_New[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13_New[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13_New[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13_New[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13_New[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13_New[CCK_index][7]);
			}
			else{
				RTL_W8( 0xa22, CCKSwingTable_Ch14_New[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch14_New[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch14_New[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch14_New[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch14_New[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch14_New[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch14_New[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch14_New[CCK_index][7]);
			}		
		} else
#endif		
#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E) {	
			if (channel !=14) {
				RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13_92E[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13_92E[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13_92E[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13_92E[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13_92E[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13_92E[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13_92E[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13_92E[CCK_index][7]);
			}
			else{
				RTL_W8( 0xa22, CCKSwingTable_Ch14_92E[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch14_92E[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch14_92E[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch14_92E[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch14_92E[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch14_92E[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch14_92E[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch14_92E[CCK_index][7]);
			}		
		}	 else
#endif	
#endif
		{
			if(channel !=14) {
				RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13[CCK_index][7]);
			}
			else{
				RTL_W8( 0xa22, CCKSwingTable_Ch14[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch14[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch14[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch14[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch14[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch14[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch14[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch14[CCK_index][7]);
			}				
		}
}
 
#ifdef WIFI_WMM
void check_NAV_prot_len(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int disassoc)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif


	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat

		&& pstat->ht_cap_len && (pstat->IOTPeer==HT_IOT_PEER_INTEL)) 

	{
		if (!disassoc && (pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)) {
#ifdef STA_EXT
			if (pstat->aid <= FW_NUM_STAT)
				priv->pshare->mimo_ps_dynamic_sta |= BIT(pstat->aid - 1);
			else
				priv->pshare->mimo_ps_dynamic_sta_ext |= BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				if (pstat->aid <= 32)
					priv->pshare->mimo_ps_dynamic_sta |= BIT(pstat->aid - 1);
				else
					priv->pshare->mimo_ps_dynamic_sta_88e_hw_ext |= BIT(pstat->aid - 1 - 32);
			} else
#endif
			{
				priv->pshare->mimo_ps_dynamic_sta |= BIT(pstat->aid -1);
			}
#endif
		} else {
#ifdef STA_EXT
			if (pstat->aid <= FW_NUM_STAT)
				priv->pshare->mimo_ps_dynamic_sta &= ~BIT(pstat->aid - 1);
			else
				priv->pshare->mimo_ps_dynamic_sta_ext &= ~BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				if (pstat->aid <= 32)
					priv->pshare->mimo_ps_dynamic_sta &= ~BIT(pstat->aid - 1);
				else
					priv->pshare->mimo_ps_dynamic_sta_88e_hw_ext &= ~BIT(pstat->aid - 1 - 32);
			} else
#endif
			{
				priv->pshare->mimo_ps_dynamic_sta &= ~BIT(pstat->aid -1);
			}
#endif
		}

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) != VERSION_8188E)
#endif
		{
			if (priv->pshare->mimo_ps_dynamic_sta
#ifdef STA_EXT
				|| priv->pshare->mimo_ps_dynamic_sta_ext
#endif
				) {
				RTL_W8(NAV_PROT_LEN, 0x40);
			} else {
				RTL_W8(NAV_PROT_LEN, 0x20);
			}
		}
	}
}
#endif


//3 ============================================================
//3 FA statistic functions
//3 ============================================================

#if !defined(CONFIG_RTL_NEW_AUTOCH)
static
#endif
void reset_FA_reg(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

#if !defined(CONFIG_RTL_NEW_AUTOCH)
	unsigned char value8;

	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 | 0x08);	// regD00[27]=1 to reset these OFDM FA counters
	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 & 0xF7);	// regD00[27]=0 to start counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 & 0x3F);	// regA2D[7:6]=00 to disable counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x80);	// regA2D[7:6]=10 to enable counting
#else
	/* cck CCA */
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 2);
	/* cck FA*/
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 2);
	/* ofdm */
	PHY_SetBBReg(priv, 0xd00, BIT(27), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(27), 0);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		PHY_SetBBReg(priv, 0xf14, BIT(16),1);
		PHY_SetBBReg(priv, 0xf14, BIT(16),0);
		RTL_W32(RXERR_RPT, RTL_R32(RXERR_RPT)|BIT(27));
		RTL_W32(RXERR_RPT, RTL_R32(RXERR_RPT)&(~BIT(27)));
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 1);
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 0);
	}
#endif
}

#if defined(CONFIG_RTL_NEW_AUTOCH)
void hold_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

	/* hold cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 1);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 1);

	/* hold ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 1);
}

void release_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

	/* release cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 0);

	/* release ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 0);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 0);

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 1);
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 0);
	}
#endif
}


void _FA_statistic(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	priv->pshare->cck_FA_cnt = (RTL_R8(0xa5b) << 8) + RTL_R8(0xa5c);

#ifdef INTERFERENCE_CONTROL
	priv->pshare->ofdm_FA_total_cnt = (unsigned int) priv->pshare->ofdm_FA_cnt1 +
			priv->pshare->ofdm_FA_cnt2 + priv->pshare->ofdm_FA_cnt3 +
			priv->pshare->ofdm_FA_cnt4 + RTL_R16(0xcf0) + RTL_R16(0xcf2);
	
	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_total_cnt + priv->pshare->cck_FA_cnt;
#else
	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);
#endif
}
#endif

void FA_statistic(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))        
		return;
#endif


#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
//		priv->pshare->F90_cnt = PHY_QueryBBReg(priv, 0xf90, bMaskHWord);
		priv->pshare->F94_cnt = PHY_QueryBBReg(priv, 0xf94, bMaskHWord);
		priv->pshare->F94_cntOK = PHY_QueryBBReg(priv, 0xf94, bMaskLWord);
		RTL_W32(RXERR_RPT,(RTL_R32(RXERR_RPT)&0x0fffffff)|0x70000000);
		priv->pshare->Reg664_cnt = RTL_R32(RXERR_RPT) & 0xfffff;
		RTL_W32(RXERR_RPT,(RTL_R32(RXERR_RPT)&0x0fffffff)|0x60000000);
		priv->pshare->Reg664_cntOK = RTL_R32(RXERR_RPT) & 0xfffff;
	}
#endif

#if !defined(CONFIG_RTL_NEW_AUTOCH)
	signed char value8;

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	// read the CCK FA counters
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x40);	// regA2D[6]=1 to hold and read the CCK FA counters
	priv->pshare->cck_FA_cnt = RTL_R8(0xa5b);
	priv->pshare->cck_FA_cnt = priv->pshare->cck_FA_cnt << 8;
	priv->pshare->cck_FA_cnt += RTL_R8(0xa5c);

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);
#else
	hold_CCA_FA_counter(priv);
	_FA_statistic(priv);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);

	release_CCA_FA_counter(priv);
#endif

	reset_FA_reg(priv);

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G  && !(OPMODE & WIFI_SITE_MONITOR)) {
			if (priv->pshare->DNC_on == 0){
				//if ((priv->pshare->F94_cnt + priv->pshare->F90_cnt)> 3000){
				/* Reg 664: x > y && x > 1000
				    Reg F94: x > 0.75*y && x > 1000 */
				if (((priv->pshare->Reg664_cnt>priv->pshare->Reg664_cntOK) && (priv->pshare->Reg664_cnt > 1000))||
					((priv->pshare->F94_cnt > ((priv->pshare->Reg664_cntOK*3)>>2)) && (priv->pshare->F94_cnt > 1000))) {
					priv->ext_stats.tp_average_pre = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
					priv->pshare->DNC_on = 1;
					priv->pshare->DNC_chk_cnt = 1;
					priv->pshare->DNC_chk = 2; // 0: don't check, 1; check, 2: just entering DNC
					//PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0x00a00000);
					PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07600760);
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, 0x20);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, 0x20);
					//printk("Dynamic Noise Control ON\n");
				}
			} else {
				if ((priv->pshare->DNC_chk_cnt % 5)==0){ // check every 5*2=10 seconds
					unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
					priv->pshare->DNC_chk_cnt = 0;

 					if ((priv->pshare->DNC_chk == 2) && (tp_now < priv->ext_stats.tp_average_pre+5)){
						//no advantage, leave DNC state
						priv->pshare->DNC_on = 0;
						priv->pshare->DNC_chk = 0;
						//PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0);
						PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000700);
					}
					else
					{
						priv->pshare->DNC_chk = 0;

						/* If TP < 20M or TP varies more than 5M. Start Checking...*/
						if ((tp_now < 20) || ((tp_now < (priv->ext_stats.tp_average_pre-5))|| (tp_now > (priv->ext_stats.tp_average_pre+5)))){
							priv->pshare->DNC_chk = 1;
							//PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0);
							PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000700);
							if (!timer_pending(&priv->dnc_timer)) {
								//printk("... Start Check Noise ...\n");
								mod_timer(&priv->dnc_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));	// 100 ms
							}
						}
					}

					priv->ext_stats.tp_average_pre = tp_now;

				} else if ((priv->pshare->DNC_chk_cnt % 5)==1 && priv->pshare->DNC_chk == 1) {
					priv->pshare->DNC_chk = 0;
					//if ((priv->pshare->F94_cnt + priv->pshare->F90_cnt) < 120) {
					if ((priv->pshare->F94_cnt + priv->pshare->Reg664_cnt) < 120) {
						priv->pshare->DNC_on = 0;
						//PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0);
						PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000700);
						//printk("Dynamic Noise Control OFF\n");
					}
				}
				priv->pshare->DNC_chk_cnt++;
			}
		}
	}
#endif
}


//3 ============================================================
//3 Rate Adaptive
//3 ============================================================

void check_RA_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int level = 0;

	switch (pstat->rssi_level) {
		case 1:
			if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
				level = 1;
			else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		case 2:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi < priv->pshare->rf_ft_var.raGoDown40MLower) ||
				((!pstat->ht_cap_len || !priv->pshare->is_40m_bw ||
				!(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) &&
				(pstat->rssi < priv->pshare->rf_ft_var.raGoDown20MLower)))
				level = 3;
			else
				level = 2;
			break;
		case 3:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi > priv->pshare->rf_ft_var.raGoUp20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi > priv->pshare->rf_ft_var.raGoUp40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		default:
			if (isErpSta(pstat))
				DEBUG_ERR("wrong rssi level setting\n");
			break;
	}

	if (level != pstat->rssi_level && CHIP_VER_92X_SERIES(priv)) {
		pstat->rssi_level = level;
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
			add_RATid(priv, pstat);
#endif
		} else
#endif
		{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)		
			add_update_RATid(priv, pstat);
#endif
		}
	}
}

void add_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char limit=16;
	int i;
	unsigned long flags;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	unsigned int update_reg=0;
#endif

	SAVE_INT_AND_CLI(flags);

	pstat->tx_ra_bitmap = 0;

	for (i=0; i<32; i++) {
		if (pstat->bssrateset[i])
			pstat->tx_ra_bitmap |= get_bit_value_from_ieee_value(pstat->bssrateset[i]&0x7f);
	}

	if (pstat->ht_cap_len) {
		if ((pstat->MIMO_ps & _HT_MIMO_PS_STATIC_) ||
			(get_rf_mimo_mode(priv)== MIMO_1T2R) ||
			(get_rf_mimo_mode(priv)== MIMO_1T1R))
			limit=8;

		for (i=0; i<limit; i++) {
			if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8))
				pstat->tx_ra_bitmap |= BIT(i+12);
		}
	}

	if (pstat->ht_cap_len) {
		unsigned int set_sgi = 0;
		if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
			&& !((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11nConfigEntry.dot11nCoexist &&
			(priv->bg_ap_timeout || priv->force_20_sta || priv->switch_20_sta
#ifdef CONFIG_RTL_88E_SUPPORT
			|| ((GET_CHIP_VER(priv) == VERSION_8188E)?(priv->force_20_sta_88e_hw_ext || priv->switch_20_sta_88e_hw_ext):0)
#endif
#ifdef STA_EXT
			|| priv->force_20_sta_ext || priv->switch_20_sta_ext
#endif
			))
#endif
			) {
			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)
				&& priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				set_sgi++;
		} else if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
			priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M) {
			set_sgi++;
		}

		if (set_sgi) {
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv)==VERSION_8188E)
#ifdef RATEADAPTIVE_BY_ODM
				ODMPTR->RAInfo[pstat->aid].SGIEnable = 1;
#else
				priv->pshare->RaInfo[pstat->aid].SGIEnable = 1;
#endif
			else
#endif
				pstat->tx_ra_bitmap |= BIT(28);
		}
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
		else {
			if (GET_CHIP_VER(priv)==VERSION_8188E)
#ifdef RATEADAPTIVE_BY_ODM
				ODMPTR->RAInfo[pstat->aid].SGIEnable = 0;
#else				
				priv->pshare->RaInfo[pstat->aid].SGIEnable = 0;
#endif
		}
#endif
	}
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	else {
		if (GET_CHIP_VER(priv)==VERSION_8188E)
#ifdef RATEADAPTIVE_BY_ODM
			ODMPTR->RAInfo[pstat->aid].SGIEnable = 0;
#else							
			priv->pshare->RaInfo[pstat->aid].SGIEnable = 0;
#endif
	}
#endif

	if ((pstat->rssi_level < 1) || (pstat->rssi_level > 3)) {
		if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
			pstat->rssi_level = 1;
		else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
			((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
			(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
			(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
			pstat->rssi_level = 2;
		else
			pstat->rssi_level = 3;
	}

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
		((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)))
		pstat->tx_ra_bitmap &= 0xfffffff0; //disable cck rate

#ifdef P2P_SUPPORT
	if(pstat->is_p2p_client){ 
		pstat->tx_ra_bitmap &= 0xfffffff0; //disable cck rate
	}
#endif

	// rate adaptive by rssi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
		if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x100f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x100ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x100ff005;
					else
						pstat->tx_ra_bitmap &= 0x100ff001;
					break;
			}
		} else {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x1f8f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x1f8ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x010ff005;
					else
						pstat->tx_ra_bitmap &= 0x010ff001;
					break;
			}

			// Don't need to mask high rates due to new rate adaptive parameters
			//if (pstat->is_broadcom_sta)		// use MCS12 as the highest rate vs. Broadcom sta
			//	pstat->tx_ra_bitmap &= 0x81ffffff;

			// NIC driver will report not supporting MCS15 and MCS14 in asoc req
			//if (pstat->is_rtl8190_sta && !pstat->is_2t_mimo_sta)
			//	pstat->tx_ra_bitmap &= 0x83ffffff;		// if Realtek 1x2 sta, don't use MCS15 and MCS14
		}
	}
	else if (((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat)) ||
			((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
			((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))))
	{
		switch (pstat->rssi_level) {
			case 1:
				pstat->tx_ra_bitmap &= 0x00000f00;
				break;
			case 2:
				pstat->tx_ra_bitmap &= 0x00000ff0;
				break;
			case 3:
				pstat->tx_ra_bitmap &= 0x00000ff5;
				break;
		}
	} else {
		pstat->tx_ra_bitmap &= 0x0000000d;
	}

// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE) {

		if((pstat->IOTPeer!=HT_IOT_PEER_REALTEK_92SE) && pstat->is_realtek_sta && pstat->is_legacy_encrpt)

			pstat->tx_ra_bitmap &= 0x0001ffff;					// up to MCS4
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) && defined (USB_POWER_SUPPORT)
	if ((GET_CHIP_VER(priv)==VERSION_8192D) &&	(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
		pstat->tx_ra_bitmap &= USB_RA_MASK;
#endif

#ifdef STA_EXT
	//update STA_map
	{
		int remapped_aid = 0;
		remapped_aid = find_reampped_aid(priv, pstat->aid);
		if(remapped_aid == 0) {
			/*WARNING:  THIS SHOULD NOT HAPPEN*/
			printk("add AID fail!!\n");
			BUG();
		}
#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)==VERSION_8812E && remapped_aid < RTL8812_NUM_STAT - 1) {
			if(priv->pshare->remapped_aidarray[remapped_aid]  == 0) {
				pstat->remapped_aid = remapped_aid;
				priv->pshare->remapped_aidarray[remapped_aid] = pstat->aid;
				pstat->sta_in_firmware = 1; // this value will updated in expire_timer
				priv->pshare->fw_free_space --;
			}

		}
		else if(GET_CHIP_VER(priv)==VERSION_8812E && remapped_aid >= RTL8812_NUM_STAT - 1) {
			pstat->remapped_aid = RTL8812_NUM_STAT-1;
			pstat->sta_in_firmware = 0; // this value will updated in expire_timer
		}
		else
#endif // CONFIG_RTL_8812_SUPPORT
		{
			if(remapped_aid >= (FW_NUM_STAT-1)){// no room for the STA
	//			priv->STA_map |= (1<< pstat->aid) ;
				pstat->remapped_aid = FW_NUM_STAT-1;
				pstat->sta_in_firmware = 0; // this value will updated in expire_timer
			} else if(priv->pshare->remapped_aidarray[remapped_aid]  == 0) { // if not 0, it should have been added before
				//we got a room
				//clear STA_map
	//			priv->STA_map &= ~(BIT(pstat->aid));
				pstat->remapped_aid = remapped_aid;
				priv->pshare->remapped_aidarray[remapped_aid] = pstat->aid;
				pstat->sta_in_firmware = 1; // this value will updated in expire_timer
				priv->pshare->fw_free_space --;
			} else {// added before
				pstat->sta_in_firmware = 1;
			}
		}
	}
#endif// STA_EXT

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifndef	RATEADAPTIVE_BY_ODM
		if (pstat->tx_ra_bitmap & 0xff000) {
			if (priv->pshare->is_40m_bw)
				priv->pshare->RaInfo[pstat->aid].RateID = ARFR_1T_40M;
			else
				priv->pshare->RaInfo[pstat->aid].RateID = ARFR_1T_20M;
		} else if (pstat->tx_ra_bitmap & 0xff0) {
			priv->pshare->RaInfo[pstat->aid].RateID = ARFR_BG_MIX;
		} else {
			priv->pshare->RaInfo[pstat->aid].RateID = ARFR_B_ONLY;
		}

		priv->pshare->RaInfo[pstat->aid].RateMask = pstat->tx_ra_bitmap;
		ARFBRefresh(priv, &priv->pshare->RaInfo[pstat->aid]);
#else
		PODM_RA_INFO_T pRAInfo = &(ODMPTR->RAInfo[pstat->aid]);
		if (pstat->tx_ra_bitmap & 0xff000) {
			if (priv->pshare->is_40m_bw)
				pRAInfo->RateID = ARFR_1T_40M;
			else
				pRAInfo->RateID = ARFR_1T_20M;
		} else if (pstat->tx_ra_bitmap & 0xff0) {
			pRAInfo->RateID = ARFR_BG_MIX;
		} else {
			pRAInfo->RateID = ARFR_B_ONLY;
		}
		ODM_RA_UpdateRateInfo_8188E(ODMPTR, pstat->aid, pRAInfo->RateID, pstat->tx_ra_bitmap, pRAInfo->SGIEnable);
#endif		
	} else
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	if(CHIP_VER_92X_SERIES(priv))
	{
#ifdef STA_EXT
		if (REMAP_AID(pstat) < FW_NUM_STAT-1)
#else
		if (REMAP_AID(pstat) < 32)
#endif
		{
#ifdef CONFIG_RTL_92D_SUPPORT
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) 
			{
				pstat->tx_ra_bitmap &= 0xfffffff0;
				if (pstat->tx_ra_bitmap & 0xff00000) {
					if (priv->pshare->is_40m_bw)
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_Band_A_40M, pstat->tx_ra_bitmap);
					else
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_Band_A_20M, pstat->tx_ra_bitmap);
					update_reg++;
				} else if (pstat->tx_ra_bitmap & 0xff000) {
					if (priv->pshare->is_40m_bw)
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_Band_A_40M, pstat->tx_ra_bitmap);
					else
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_Band_A_20M, pstat->tx_ra_bitmap);
				} else if (pstat->tx_ra_bitmap & 0xff0) {
					set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_Band_A_BMC, pstat->tx_ra_bitmap);
				} else {
					set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_Band_A_BMC, pstat->tx_ra_bitmap);
				}
			} else 
#endif
			{
				if (pstat->tx_ra_bitmap & 0xff00000) {
					if (priv->pshare->is_40m_bw)
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_40M, pstat->tx_ra_bitmap);
					else
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_20M, pstat->tx_ra_bitmap);
					update_reg++;
				} else if (pstat->tx_ra_bitmap & 0xff000) {
					if (priv->pshare->is_40m_bw)
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_1T_40M, pstat->tx_ra_bitmap);
					else
						set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_1T_20M, pstat->tx_ra_bitmap);
				} else if (pstat->tx_ra_bitmap & 0xff0) {
					set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_BG_MIX, pstat->tx_ra_bitmap);
				} else {
					set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_B_ONLY, pstat->tx_ra_bitmap);
				}
			}

			/*
			 * Rate adaptive algorithm.
			 * If the STA is 2R, we set the inti rate to MCS 15
			 */
			if (update_reg) {
				if (!pstat->check_init_tx_rate && (pstat->rssi > 55)) {
					RTL_W8(INIDATA_RATE_SEL + REMAP_AID(pstat), 0x1b);
					pstat->check_init_tx_rate = 1;
				}
			}
			DEBUG_INFO("Add id %d val %08x to ratr\n", pstat->aid, pstat->tx_ra_bitmap);
		} else {
#ifdef STA_EXT
#ifdef CONFIG_RTL_92D_SUPPORT
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				if (priv->pshare->is_40m_bw)
					set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_Band_A_40M, 0x1ffffff0);
				else
					set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_Band_A_20M, 0x1ffffff0);
			} else
#endif
			{
				if (priv->pshare->is_40m_bw)
					set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_40M, 0x1fffffff);
				else
					set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_20M, 0x1fffffff);
			}
#else
			DEBUG_ERR("station aid %d exceed the max number\n", pstat->aid);
#endif
		}
	}
#endif
	RESTORE_INT(flags);
}


//3 ============================================================
//3 EDCCA
//3 ============================================================
#if 0
void check_EDCCA(struct rtl8192cd_priv *priv, short rssi)
{
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)    
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A))
		return;
#endif

	if ((priv->pshare->rf_ft_var.edcca_thd) && (priv->pmib->dot11RFEntry.dot11channel==14
		|| priv->pshare->is_40m_bw
#if defined(CONFIG_RTL_92D_SUPPORT)
		||  (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
#endif
	)) {
		if((rssi > priv->pshare->rf_ft_var.edcca_thd) && (priv->pshare->phw->EDCCA_on == 0)) {
			RTL_W32(rOFDM0_ECCAThreshold, 0xfc03fd);
			priv->pshare->phw->EDCCA_on =1;
		} else if( (rssi < priv->pshare->rf_ft_var.edcca_thd-5) && priv->pshare->phw->EDCCA_on) {
			RTL_W32(rOFDM0_ECCAThreshold, 0x7f037f);
			priv->pshare->phw->EDCCA_on =0;
		}
	}
	if ((!priv->pshare->rf_ft_var.edcca_thd) && priv->pshare->phw->EDCCA_on) {
		RTL_W32(0xc4c, 0x7f037f);
		priv->pshare->phw->EDCCA_on = 0;
	}	
}
#endif
void Dynamic_EDCCA(struct rtl8192cd_priv *priv, unsigned char IGI)
{
	char Diff,TH_L2H,TH_H2L;
	char TH_L2H_dmc, TH_H2L_dmc;
	int value32;
	int IGI_target;
	priv->pshare->rf_ft_var.IGI_target = 0x1c;
	PHY_SetBBReg(priv, 0x524, BIT11, 1); // stop counting if EDCCA is asserted

	if((OPMODE & WIFI_AP_STATE)?(!(priv->assoc_num)):(!OPMODE & WIFI_ASOC_STATE)
#ifdef UNIVERSAL_REPEATER
		&& !(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
#endif
	){
		PHY_SetBBReg(priv,rOFDM0_ECCAThreshold,0xFF, 0x7f);
		PHY_SetBBReg(priv,rOFDM0_ECCAThreshold,0xFF0000, 0x7f);
		return;
	}
	if(!priv->pshare->rf_ft_var.force_edcca){
		if(priv->pshare->rssi_min > priv->pshare->rf_ft_var.edcca_thd){
	        priv->pshare->phw->EDCCA_on =1;		
		}
	    else if(priv->pshare->rssi_min < priv->pshare->rf_ft_var.edcca_thd-5){
	        priv->pshare->phw->EDCCA_on =0;
		}
	}else
		priv->pshare->phw->EDCCA_on =1; 
	if(priv->pshare->CurrentChannelBW==HT_CHANNEL_WIDTH_20) 
		IGI_target = priv->pshare->rf_ft_var.IGI_base;
	else if(priv->pshare->CurrentChannelBW==HT_CHANNEL_WIDTH_20_40)
		IGI_target = priv->pshare->rf_ft_var.IGI_base + 2;
	else
		IGI_target = priv->pshare->rf_ft_var.IGI_base;

	priv->pshare->rf_ft_var.IGI_target = IGI_target;

	if( priv->pshare->phw->EDCCA_on == 1)
	{
		Diff = IGI_target -(int)IGI;
		TH_L2H_dmc = priv->pshare->rf_ft_var.TH_L2H_ini + Diff;
		if(TH_L2H_dmc >10)
			TH_L2H_dmc = 10;
		TH_H2L_dmc = TH_L2H_dmc - priv->pshare->rf_ft_var.TH_EDCCA_HL_diff;
	}
	else
	{
		TH_L2H_dmc = 0x7f;
		TH_H2L_dmc = 0x7f;
	}

	PHY_SetBBReg(priv,rOFDM0_ECCAThreshold,0xFF, 		TH_L2H_dmc);
	PHY_SetBBReg(priv,rOFDM0_ECCAThreshold,0xFF0000, 	TH_H2L_dmc);
}
void check_NBI_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if (priv->pshare->phw->nbi_filter_on) {
		if (rssi_strength < 20) 
			NBI_filter_off(priv);
		
	} else {	// NBI OFF previous
		if (rssi_strength > 25) 
			NBI_filter_on(priv);		
	}
}


void NBI_filter_on(struct rtl8192cd_priv *priv)
{
	priv->pshare->phw->nbi_filter_on = 1;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif
	
#ifdef RTK_AC_SUPPORT	
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		RTL_W16(rFPGA0_XCD_RFParam, RTL_R16(rFPGA0_XCD_RFParam) | BIT(13));		// NBI on
	else
#endif		
		RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) | BIT(9));		// NBI on

}

void NBI_filter_off(struct rtl8192cd_priv *priv)
{
	priv->pshare->phw->nbi_filter_on = 0;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif
	
#ifdef RTK_AC_SUPPORT	
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		RTL_W16(rFPGA0_XCD_RFParam, RTL_R16(rFPGA0_XCD_RFParam) & ~ BIT(13));		// NBI off
	else
#endif		
		RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) & ~ BIT(9));	// NBI off

}

#if defined(CONFIG_WLAN_HAL_8192EE)
void RRSR_power_control_11n(struct rtl8192cd_priv *priv, int lower)
{
	unsigned long x, pwrdiff= 0x0c;
	u1Byte pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[priv->pshare->working_channel-1];
	if ((pwrlevelHT40_1S_A == 0) )
		return;

	if (priv->pmib->dot11RFEntry.tx2path)
		pwrdiff= 0x16;
	pwrdiff = (priv->pshare->rf_ft_var.min_pwr_idex > pwrdiff) ? pwrdiff: priv->pshare->rf_ft_var.min_pwr_idex;	
	pwrdiff |= (pwrdiff<<24) | (pwrdiff<<16) | (pwrdiff<<8);
	
	if( lower && priv->pshare->phw->lower_tx_power== 0) {
		SAVE_INT_AND_CLI(x);
		priv->pshare->phw->power_backup[0x00] = RTL_R32(rTxAGC_A_Rate18_06);
		priv->pshare->phw->power_backup[0x01] = RTL_R32(rTxAGC_A_Rate54_24);
		priv->pshare->phw->power_backup[0x02] = RTL_R32(rTxAGC_B_Rate18_06);
		priv->pshare->phw->power_backup[0x03] = RTL_R32(rTxAGC_B_Rate54_24);
		priv->pshare->phw->power_backup[0x0c] = RTL_R32(rTxAGC_A_CCK11_2_B_CCK11);
		priv->pshare->phw->power_backup[0x0e] = RTL_R32(rTxAGC_B_CCK5_1_Mcs32);	
		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]-pwrdiff);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]-(pwrdiff & 0xff));
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]-pwrdiff);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]-(pwrdiff & 0xff));
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, priv->pshare->phw->power_backup[0x0c]-pwrdiff);		
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32, priv->pshare->phw->power_backup[0x0e]-(pwrdiff & 0xffff0000) );
		
		priv->pshare->phw->lower_tx_power = 1;
		RESTORE_INT(x);
	}
	else if( !lower && priv->pshare->phw->lower_tx_power) {
		SAVE_INT_AND_CLI(x);
		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]);
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]);		
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, priv->pshare->phw->power_backup[0x0c]);
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32,  priv->pshare->phw->power_backup[0x0e]); 		
		priv->pshare->phw->lower_tx_power = 0;
		RESTORE_INT(x);
	}
}
#endif

//3 ============================================================
//3 PHY calibration
//3 ============================================================
#ifndef CALIBRATE_BY_ODM


void _PHY_SaveADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDABackup, unsigned int RegisterNum)
{
	unsigned int	i;
	for( i = 0 ; i < RegisterNum ; i++){
		ADDABackup[i] = PHY_QueryBBReg(priv, ADDAReg[i], bMaskDWord);
	}
}

void _PHY_SetADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDASettings, unsigned int RegisterNum)
{
	unsigned int	i;

	for( i = 0 ; i < RegisterNum ; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, ADDASettings[i]);
	}
}

void _PHY_SaveMACRegisters(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i;
	for( i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		MACBackup[i] = RTL_R8(MACReg[i]);
	}
	MACBackup[i] = RTL_R32( MACReg[i]);
}

void _PHY_ReloadADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, unsigned int*	ADDABackup, unsigned int RegiesterNum)
{
	unsigned int	i;
	for(i = 0 ; i < RegiesterNum; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, ADDABackup[i]);
	}
}

void _PHY_ReloadMACRegisters(struct rtl8192cd_priv *priv,unsigned int* MACReg, unsigned int*	 MACBackup)
{
	unsigned int	i;
	for(i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)MACBackup[i]);
	}
	RTL_W32( MACReg[i], MACBackup[i]);
}

void _PHY_MACSettingCalibration(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i = 0;
	RTL_W8(MACReg[i], 0x3F);
	for(i = 1 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(3))));
	}
	RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(5))));
}

void _PHY_PathADDAOn(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, char isPathAOn, char is2T)
{
	unsigned int	pathOn;
	unsigned int	i;

	pathOn = isPathAOn ? 0x04db25a4 : 0x0b1b25a4;
	if(FALSE == is2T){
		pathOn = 0x0bdb25a0;
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, 0x0b1b25a0);
	}
	else{
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, pathOn);
	}

	for( i = 1 ; i < IQK_ADDA_REG_NUM ; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, pathOn);
	}
}
#endif


void PHY_LCCalibrate(struct rtl8192cd_priv *priv)
{
	unsigned char tmpReg, value_IGI;
	unsigned int LC_Cal;
	int isNormal;

#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
	isNormal = (IS_TEST_CHIP(priv)? 0 : 1);
#else
	isNormal = 1;
#endif

	/* Check continuous TX and Packet TX */
	tmpReg = RTL_R8(0xd03);

	if ((tmpReg & 0x70) != 0)			/* Deal with contisuous TX case */
		RTL_W8(0xd03, tmpReg&0x8F);	/* disable all continuous TX */
	else								/* Deal with Packet TX case */
		RTL_W8(0x522, 0xFF);			/* block all queues */

	/* 2. Set RF mode = standby mode */
	if ((tmpReg & 0x70) != 0) {
		/* Path-A */
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x10000);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R)
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x00, bMask20Bits, 0x10000);
	}

	/* 3. Read RF reg18 */
	LC_Cal = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, 1);

	/* 4. Set LC calibration begin */
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, LC_Cal|0x08000);

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8192D)
		REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined (CONFIG_RTL_8198B) && defined(CONFIG_RTL_92D_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8192D)
		REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
#endif

	if (isNormal)
		delay_ms(100);
	else
		delay_ms(3);

	/* Restore original situation */
	if ((tmpReg & 0x70) != 0) {
		/* Deal with contisuous TX case */

		/* Path-A */
		RTL_W8(0xd03, tmpReg);

		/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
		value_IGI = (RTL_R8(0xc50) & 0x7F);
		RTL_W8(0xc50, ((value_IGI!=0x30)?0x30:0x31));
		RTL_W8(0xc50, value_IGI);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R) {
			/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
			value_IGI = (RTL_R8(0xc58) & 0x7F);
			RTL_W8(0xc58, ((value_IGI!=0x30)?0x30:0x31));
			RTL_W8(0xc58, value_IGI);
		}
	} else {
		/* Deal with Packet TX case */

		RTL_W8(0x522, 0x00);
	}
}

#endif

