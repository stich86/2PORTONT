/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================
#include "Mp_Precomp.h"
#include "phydm_precomp.h"


VOID
Phydm_CheckAdaptivity(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if(pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY)
	{
			if(pDM_Odm->DynamicLinkAdaptivity == TRUE)
			{
				if(pDM_Odm->bLinked && pDM_Odm->bCheck == FALSE)
				{
					Phydm_NHMCounterStatistics(pDM_Odm);
					Phydm_CheckEnvironment(pDM_Odm);
				}
				else if(!pDM_Odm->bLinked)
				{
				pDM_Odm->bCheck = FALSE;
				}
			}
			else
			{
			pDM_Odm->Adaptivity_enable = TRUE;
			
			if(pDM_Odm->SupportICType & ODM_RTL8814A)
				pDM_Odm->adaptivity_flag = FALSE;
			else
				pDM_Odm->adaptivity_flag = TRUE;
			}
		}
		else
		{
		pDM_Odm->Adaptivity_enable = FALSE;
			pDM_Odm->adaptivity_flag = FALSE;
		}
		
}

VOID
Phydm_NHMCounterStatisticsInit(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{
		//PHY parameters initialize for ac series
		ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TIMER_11AC+2, 0xC350);	//0x990[31:16]=0xC350	Time duration for NHM unit: us, 0xc350=200ms
		ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TH9_TH10_11AC+2, 0xffff);	//0x994[31:16]=0xffff	th_9, th_10
		//ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH3_TO_TH0_11AC, 0xffffff5c);	//0x998=0xffffff5c 		th_3, th_2, th_1, th_0
		ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH3_TO_TH0_11AC, 0xffffff50);	//0x998=0xffffff52 		th_3, th_2, th_1, th_0
		ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH7_TO_TH4_11AC, 0xffffffff);	//0x99c=0xffffffff		th_7, th_6, th_5, th_4
		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH8_11AC, bMaskByte0, 0xff);		//0x9a0[7:0]=0xff		th_8
		//ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11AC, BIT8|BIT9|BIT10, 0x7);	//0x994[9:8]=3			enable CCX
		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11AC, BIT8|BIT9|BIT10, 0x1);	//0x994[10:8]=1	ignoreCCA ignore PHYTXON	enable CCX
		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_9E8_11AC, BIT0, 0x1);		//0x9e8[7]=1			max power among all RX ants	
				
	}
	else if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		//PHY parameters initialize for n series
		ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TIMER_11N+2, 0xC350);	//0x894[31:16]=0x0xC350	Time duration for NHM unit: us, 0xc350=200ms
		//ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TIMER_11N+2, 0x4e20);	//0x894[31:16]=0x4e20	Time duration for NHM unit: 4us, 0x4e20=80ms
		ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N+2, 0xffff);	//0x890[31:16]=0xffff	th_9, th_10
		//ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH3_TO_TH0_11N, 0xffffff5c);	//0x898=0xffffff5c 		th_3, th_2, th_1, th_0
		ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH3_TO_TH0_11N, 0xffffff50);	//0x898=0xffffff52 		th_3, th_2, th_1, th_0
		ODM_Write4Byte(pDM_Odm, ODM_REG_NHM_TH7_TO_TH4_11N, 0xffffffff);	//0x89c=0xffffffff		th_7, th_6, th_5, th_4
		ODM_SetBBReg(pDM_Odm, ODM_REG_FPGA0_IQK_11N, bMaskByte0, 0xff);		//0xe28[7:0]=0xff		th_8
		//ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N, BIT10|BIT9|BIT8, 0x7);	//0x890[9:8]=3			enable CCX
		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N, BIT10|BIT9|BIT8, 0x1);	//0x890[10:8]=1		ignoreCCA ignore PHYTXON	enable CCX
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT7, 0x1);		//0xc0c[7]=1			max power among all RX ants				
	}
}

VOID
Phydm_NHMCounterStatistics(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if(!(pDM_Odm->SupportAbility & ODM_BB_NHM_CNT))
		return;

	// Get NHM report
	Phydm_GetNHMCounterStatistics(pDM_Odm);

	// Reset NHM counter
	Phydm_NHMCounterStatisticsReset(pDM_Odm);
}

VOID
Phydm_GetNHMCounterStatistics(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte		value32 = 0;

	if (pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
		value32 = ODM_GetBBReg(pDM_Odm, ODM_REG_NHM_CNT_11AC, bMaskDWord);
	else if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		value32 = ODM_GetBBReg(pDM_Odm, ODM_REG_NHM_CNT_11N, bMaskDWord);

	pDM_Odm->NHM_cnt_0 = (u1Byte)(value32 & bMaskByte0);
	pDM_Odm->NHM_cnt_1 = (u1Byte)((value32 & bMaskByte1)>>8);

}

VOID
Phydm_NHMCounterStatisticsReset(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if (pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{           		
    		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11AC, BIT1, 0);
    		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11AC, BIT1, 1);
	}
	else if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
    		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N, BIT1, 0);
    		ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N, BIT1, 1);
	}
}

VOID
Phydm_SetEDCCAThreshold(
	IN	PVOID	pDM_VOID,
	IN	s1Byte	H2L,
	IN	s1Byte	L2H
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		{
		ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)L2H);
		ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)H2L);
		}
	else if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
		{
		ODM_SetBBReg(pDM_Odm, rFPGA0_XB_LSSIReadBack, bMaskByte0, (u1Byte)L2H);
		ODM_SetBBReg(pDM_Odm, rFPGA0_XB_LSSIReadBack, bMaskByte1, (u1Byte)H2L);
		}
}

VOID
Phydm_SetLNA(
	IN	PVOID				pDM_VOID,
	IN	PhyDM_set_LNA	type
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if (pDM_Odm->SupportICType & (ODM_RTL8188E | ODM_RTL8192E)) {
		if (type == PhyDM_disable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0000f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0x37f82);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (pDM_Odm->RFType > ODM_1T1R) {
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x0000f);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0x37f82);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		} else if (type == PhyDM_enable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0000f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0x77f82);	/*back to normal*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (pDM_Odm->RFType > ODM_1T1R) {
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x0000f);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0x77f82);
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		}
	} else if (pDM_Odm->SupportICType & ODM_RTL8723B) {
		if (type == PhyDM_disable_LNA) {
			/*S0*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0001f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xe6137);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			/*S1*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x43, 0xfffff, 0x3008d);	/*select Rx mode and disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x0);
		} else if (type == PhyDM_enable_LNA) {
			/*S0*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0001f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xe6177);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			/*S1*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x43, 0xfffff, 0x300bd);	/*select Rx mode and disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x0);
		}
	
	} else if (pDM_Odm->SupportICType & ODM_RTL8812) {
		if (type == PhyDM_disable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
				if (pDM_Odm->RFType > ODM_1T1R) {
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
				}
		} else if (type == PhyDM_enable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
				if (pDM_Odm->RFType > ODM_1T1R) {
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
				}
		}
	} else if (pDM_Odm->SupportICType & (ODM_RTL8821 | ODM_RTL8881A)) {
		if (type == PhyDM_disable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0002f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xfb09b);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
		} else if (type == PhyDM_enable_LNA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0002f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xfb0bb);	/*disable LNA*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);	
		}
	}
}

VOID
Phydm_SetTRxMux(
	IN	PVOID				pDM_VOID,
	IN	PhyDM_Trx_MUX_Type	txMode,
	IN	PhyDM_Trx_MUX_Type	rxMode
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT3|BIT2|BIT1, txMode);	// set TXmod to standby mode to remove outside noise affect
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT22|BIT21|BIT20, rxMode);	// set RXmod to standby mode to remove outside noise affect
		if(pDM_Odm->RFType > ODM_1T1R)
		{
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT3|BIT2|BIT1, txMode);	// set TXmod to standby mode to remove outside noise affect
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT22|BIT21|BIT20, rxMode);	// set RXmod to standby mode to remove outside noise affect
		}
	}
	else if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{
		ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC, BIT11|BIT10|BIT9|BIT8, txMode);	// set TXmod to standby mode to remove outside noise affect
		ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC, BIT7|BIT6|BIT5|BIT4, rxMode);	// set RXmod to standby mode to remove outside noise affect
		if(pDM_Odm->RFType > ODM_1T1R)
		{
			ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC_B, BIT11|BIT10|BIT9|BIT8, txMode);	// set TXmod to standby mode to remove outside noise affect
			ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC_B, BIT7|BIT6|BIT5|BIT4, rxMode);	// set RXmod to standby mode to remove outside noise affect
		}
	}

}

VOID
Phydm_MACEDCCAState(
	IN	PVOID					pDM_VOID,
	IN	PhyDM_MACEDCCA_Type		State
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if(State == PhyDM_IGNORE_EDCCA)
	{
		ODM_SetMACReg(pDM_Odm, REG_TX_PTCL_CTRL, BIT15, 1);	//ignore EDCCA	reg520[15]=1
		ODM_SetMACReg(pDM_Odm, REG_RD_CTRL, BIT11, 0);		//reg524[11]=0
	}
	else		// don't set MAC ignore EDCCA signal
	{
		ODM_SetMACReg(pDM_Odm, REG_TX_PTCL_CTRL, BIT15, 0);	//don't ignore EDCCA	 reg520[15]=0
		ODM_SetMACReg(pDM_Odm, REG_RD_CTRL, BIT11, 1);	//reg524[11]=1	
	}

	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("EDCCA enable State = %d \n", State));

}

BOOLEAN
Phydm_CalNHMcnt(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u2Byte			Base = 0;

	Base = pDM_Odm->NHM_cnt_0 + pDM_Odm->NHM_cnt_1;

	if(Base != 0)
	{
		pDM_Odm->NHM_cnt_0 = ((pDM_Odm->NHM_cnt_0) << 8) / Base;
		pDM_Odm->NHM_cnt_1 = ((pDM_Odm->NHM_cnt_1) << 8) / Base;
	}
	if((pDM_Odm->NHM_cnt_0 - pDM_Odm->NHM_cnt_1) >= 100)
		return TRUE;			// clean environment
	else
		return FALSE;		//noisy environment

}


VOID
Phydm_CheckEnvironment(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN 	isCleanEnvironment = FALSE;
	u1Byte		clean = 0;

	if(pDM_Odm->bFirstLink == TRUE)
	{
		if(pDM_Odm->SupportICType & ODM_RTL8814A)
			pDM_Odm->adaptivity_flag = FALSE;
		else
		pDM_Odm->adaptivity_flag = TRUE;
		
		pDM_Odm->bFirstLink = FALSE;
		return;
	}
	else
	{
		if(pDM_Odm->NHMWait < 3)			// Start enter NHM after 4 NHMWait
		{
			pDM_Odm->NHMWait ++;
			Phydm_NHMCounterStatistics(pDM_Odm);
			return;
		}
		else
		{
			Phydm_NHMCounterStatistics(pDM_Odm);
			isCleanEnvironment = Phydm_CalNHMcnt(pDM_Odm);
			if(isCleanEnvironment == TRUE)
			{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
				pDM_Odm->TH_L2H_ini = pDM_Odm->TH_L2H_ini_backup;			//mode 1
				pDM_Odm->TH_EDCCA_HL_diff= pDM_Odm->TH_EDCCA_HL_diff_backup;
#endif
				pDM_Odm->Adaptivity_enable = TRUE;

				if(pDM_Odm->SupportICType & ODM_RTL8814A)
					pDM_Odm->adaptivity_flag = FALSE;
				else
				pDM_Odm->adaptivity_flag = TRUE;
			}
			else
			{
#if(DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
				Phydm_SetEDCCAThreshold(pDM_Odm, 0x7f, 0x7f);
#else
				pDM_Odm->TH_L2H_ini = pDM_Odm->TH_L2H_ini_mode2;			// for AP mode 2
				pDM_Odm->TH_EDCCA_HL_diff= pDM_Odm->TH_EDCCA_HL_diff_mode2;
#endif
				pDM_Odm->adaptivity_flag = FALSE;
				pDM_Odm->Adaptivity_enable = FALSE;
			}
			pDM_Odm->NHMWait = 0;
			pDM_Odm->bFirstLink = TRUE;
			pDM_Odm->bCheck = TRUE;
		}
	}
}

VOID
Phydm_SearchPwdBLowerBound(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte			value32 =0;
	u1Byte			cnt, IGI_Pause = 0x7f, IGI_Resume = 0x20, IGI = 0x50;	//IGI = 0x50 for cal EDCCA lower bound
	u1Byte			txEdcca1 = 0, txEdcca0 = 0;
	BOOLEAN			bAdjust = TRUE;
	s1Byte 			TH_L2H_dmc, TH_H2L_dmc, IGI_target = 0x32;
	s1Byte 			Diff;

	if (pDM_Odm->SupportICType & (ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8192E | ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8881A))
		Phydm_SetLNA(pDM_Odm, PhyDM_disable_LNA);
	else {
	Phydm_SetTRxMux(pDM_Odm, PhyDM_STANDBY_MODE, PhyDM_STANDBY_MODE);
	ODM_Write_DIG(pDM_Odm, IGI_Pause);
	}
	
	Diff = IGI_target -(s1Byte)IGI;
	TH_L2H_dmc = pDM_Odm->TH_L2H_ini + Diff;
		if(TH_L2H_dmc > 10) 	
			TH_L2H_dmc = 10;
	TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

	Phydm_SetEDCCAThreshold(pDM_Odm, TH_H2L_dmc, TH_L2H_dmc);			
	ODM_delay_ms(5);
		
	while (bAdjust) {
		for (cnt = 0; cnt < 20; cnt++) {
			if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
				value32 = ODM_GetBBReg(pDM_Odm,ODM_REG_RPT_11N, bMaskDWord);
			else if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
				value32 = ODM_GetBBReg(pDM_Odm,ODM_REG_RPT_11AC, bMaskDWord);
		
			if (value32 & BIT30 && (pDM_Odm->SupportICType & (ODM_RTL8723B | ODM_RTL8188E)))
				txEdcca1 = txEdcca1 + 1;
			else if(value32 & BIT29)
				txEdcca1 = txEdcca1 + 1;
			else
				txEdcca0 = txEdcca0 + 1;
		}
			
		if (txEdcca1 > 1) {
			IGI = IGI -1;
			TH_L2H_dmc = TH_L2H_dmc + 1;
				if(TH_L2H_dmc > 10)
					TH_L2H_dmc = 10;
			TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

			Phydm_SetEDCCAThreshold(pDM_Odm, TH_H2L_dmc, TH_L2H_dmc);
			if (TH_L2H_dmc == 10) {
				bAdjust = FALSE;
				pDM_Odm->H2L_lb = TH_H2L_dmc;
				pDM_Odm->L2H_lb = TH_L2H_dmc;
				pDM_Odm->Adaptivity_IGI_upper = IGI;
			}

			txEdcca1 = 0;
			txEdcca0 = 0;

		} else {
			bAdjust = FALSE;
			pDM_Odm->H2L_lb = TH_H2L_dmc;
			pDM_Odm->L2H_lb = TH_L2H_dmc;	
			pDM_Odm->Adaptivity_IGI_upper = IGI;
			txEdcca1 = 0;
			txEdcca0 = 0;
		}
	}
							
	pDM_Odm->Adaptivity_IGI_upper = pDM_Odm->Adaptivity_IGI_upper - pDM_Odm->DCbackoff;
	pDM_Odm->H2L_lb = pDM_Odm->H2L_lb + pDM_Odm->DCbackoff;
	pDM_Odm->L2H_lb = pDM_Odm->L2H_lb + pDM_Odm->DCbackoff;
							
	if (pDM_Odm->SupportICType & (ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8192E | ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8881A))
		Phydm_SetLNA(pDM_Odm, PhyDM_enable_LNA);
	else {							
	Phydm_SetTRxMux(pDM_Odm, PhyDM_TX_MODE, PhyDM_RX_MODE);
	ODM_Write_DIG(pDM_Odm, IGI_Resume);
	}

	Phydm_SetEDCCAThreshold(pDM_Odm, 0x7f, 0x7f); // resume to no link state
}

BOOLEAN
phydm_reSearchCondition(
	IN	PVOID				pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	/*PADAPTIVITY_STATISTICS	Adaptivity = (PADAPTIVITY_STATISTICS)PhyDM_Get_Structure(pDM_Odm, PHYDM_ADAPTIVITY);*/
	u1Byte			Adaptivity_IGI_upper;
	/*s1Byte		TH_L2H_dmc, IGI_target = 0x32;*/
	/*s1Byte		Diff;*/

	Adaptivity_IGI_upper = pDM_Odm->Adaptivity_IGI_upper + pDM_Odm->DCbackoff;
	
	/*TH_L2H_dmc = 10;*/

	/*Diff = TH_L2H_dmc - pDM_Odm->TH_L2H_ini;*/
	/*lowest_IGI_upper = IGI_target - Diff;*/

	/*if ((Adaptivity_IGI_upper - lowest_IGI_upper) <= 5)*/
	if (Adaptivity_IGI_upper <= 0x26)
		return TRUE;
	else
		return FALSE;
	
}

VOID
Phydm_AdaptivityInit(
	IN 	PVOID	 	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if(DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	PMGNT_INFO		pMgntInfo = &(pAdapter->MgntInfo);
	pDM_Odm->Carrier_Sense_enable = (BOOLEAN)pMgntInfo->RegEnableCarrierSense;
	pDM_Odm->DynamicLinkAdaptivity = (BOOLEAN)pMgntInfo->RegDmLinkAdaptivity;
#elif(DM_ODM_SUPPORT_TYPE == ODM_CE)
	pDM_Odm->Carrier_Sense_enable = (pDM_Odm->Adapter->registrypriv.adaptivity_mode!=0)?TRUE:FALSE;
	pDM_Odm->DynamicLinkAdaptivity = FALSE; // Jeff please add this
#endif

#if(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))

	if(pDM_Odm->Carrier_Sense_enable == FALSE)
	{
#if(DM_ODM_SUPPORT_TYPE == ODM_WIN)
		if( pMgntInfo->RegL2HForAdaptivity != 0 )
			pDM_Odm->TH_L2H_ini = pMgntInfo->RegL2HForAdaptivity;
		else
#endif
			pDM_Odm->TH_L2H_ini = 0xf5; // -7
	}
	else
	{
#if(DM_ODM_SUPPORT_TYPE == ODM_WIN)
		if( pMgntInfo->RegL2HForAdaptivity != 0 )
			pDM_Odm->TH_L2H_ini = pMgntInfo->RegL2HForAdaptivity;
		else
#endif
		pDM_Odm->TH_L2H_ini = 0xa; 
	}

#if(DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if( pMgntInfo->RegHLDiffForAdaptivity != 0 )
		pDM_Odm->TH_EDCCA_HL_diff = pMgntInfo->RegHLDiffForAdaptivity;
	else
#endif
	pDM_Odm->TH_EDCCA_HL_diff = 7;

	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("TH_L2H_ini = 0x%x, TH_EDCCA_HL_diff = 0x%x\n", pDM_Odm->TH_L2H_ini, pDM_Odm->TH_EDCCA_HL_diff));

#elif (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv				priv = pDM_Odm->priv;

	if(pDM_Odm->Carrier_Sense_enable){
		pDM_Odm->TH_L2H_ini = 10;
		pDM_Odm->TH_EDCCA_HL_diff = 7;		
	}
	else
	{
		pDM_Odm->TH_L2H_ini = pDM_Odm->TH_L2H_ini_backup;	//set by mib
		pDM_Odm->TH_EDCCA_HL_diff = 7;
	}

	pDM_Odm->TH_L2H_ini_mode2 = 20;
	pDM_Odm->TH_EDCCA_HL_diff_mode2 = 8;
	//pDM_Odm->TH_L2H_ini_backup = pDM_Odm->TH_L2H_ini;
	pDM_Odm->TH_EDCCA_HL_diff_backup = pDM_Odm->TH_EDCCA_HL_diff ;
	if(priv->pshare->rf_ft_var.adaptivity_enable == 2)
		pDM_Odm->DynamicLinkAdaptivity = TRUE;
	else
		pDM_Odm->DynamicLinkAdaptivity = FALSE;

#endif

	pDM_Odm->IGI_Base = 0x32;	
	pDM_Odm->IGI_target = 0x1c;
	pDM_Odm->FABound = 6000;
	pDM_Odm->H2L_lb= 0;
	pDM_Odm->L2H_lb= 0;
	pDM_Odm->Adaptivity_IGI_upper = 0;
	pDM_Odm->NHMWait = 0;
	pDM_Odm->bCheck = FALSE;
	pDM_Odm->bFirstLink = TRUE;
	pDM_Odm->Adaptivity_enable = FALSE;	// use this flag to judge enable or disable

	Phydm_MACEDCCAState(pDM_Odm, PhyDM_DONT_IGNORE_EDCCA);

	//Search pwdB lower bound
	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		ODM_SetBBReg(pDM_Odm,ODM_REG_DBG_RPT_11N, bMaskDWord, 0x208);
	else if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
		ODM_SetBBReg(pDM_Odm,ODM_REG_DBG_RPT_11AC, bMaskDWord, 0x209);

	if(pDM_Odm->SupportICType & ODM_RTL8814A)	// 8814a no need to find pwdB lower bound, maybe
	{
		ODM_SetBBReg(pDM_Odm,ODM_REG_EDCCA_DOWN_OPT, BIT30|BIT29|BIT28, 0x7); 	// interfernce need  > 2^x us, and then EDCCA will be 1
		ODM_SetBBReg(pDM_Odm,ODM_REG_EDCCA_POWER_CAL, BIT5, 1);					// 0: mean, 1:max pwdB
		ODM_SetBBReg(pDM_Odm,ODM_REG_ACBB_EDCCA_ENHANCE, BIT29|BIT28, 0x1);		// 0 : rx_dfir, 1: dcnf_out, 2 :rx_iq, 3: rx_nbi_nf_out
	}
	else
	{
		Phydm_SearchPwdBLowerBound(pDM_Odm);
		if (phydm_reSearchCondition(pDM_Odm))
		Phydm_SearchPwdBLowerBound(pDM_Odm);
	}

}


VOID
Phydm_Adaptivity(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			IGI
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	s1Byte TH_L2H_dmc, TH_H2L_dmc;
	s1Byte Diff, IGI_target;
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	BOOLEAN		bFwCurrentInPSMode=FALSE;	
	PMGNT_INFO				pMgntInfo = &(pAdapter->MgntInfo);
	
	pAdapter->HalFunc.GetHwRegHandler(pAdapter, HW_VAR_FW_PSMODE_STATUS, (pu1Byte)(&bFwCurrentInPSMode));	

	// Disable EDCCA mode while under LPS mode, added by Roger, 2012.09.14.
	if(bFwCurrentInPSMode)
		return;
#endif

	if (pDM_Odm->EDCCA_enable == FALSE) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Disable EDCCA!!!\n"));
		return;
	}

	if(!(pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY))
	{
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("adaptivity disable, enable EDCCA mode!!!\n"));
		pDM_Odm->TH_L2H_ini = pDM_Odm->TH_L2H_ini_mode2;
		pDM_Odm->TH_EDCCA_HL_diff = pDM_Odm->TH_EDCCA_HL_diff_mode2;
	}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if(pMgntInfo->RegEnableAdaptivity== 2)
	{
		if(pDM_Odm->Carrier_Sense_enable == FALSE)		// check domain Code for Adaptivity or CarrierSense
		{
			if ((*pDM_Odm->pBandType == ODM_BAND_5G) && 
				!(pDM_Odm->odm_Regulation5G == REGULATION_ETSI || pDM_Odm->odm_Regulation5G == REGULATION_WW))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Adaptivity skip 5G domain code : %d \n", pDM_Odm->odm_Regulation5G));
				return;
			}

			else if((*pDM_Odm->pBandType == ODM_BAND_2_4G) &&
				!(pDM_Odm->odm_Regulation2_4G == REGULATION_ETSI || pDM_Odm->odm_Regulation2_4G == REGULATION_WW))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Adaptivity skip 2.4G domain code : %d \n", pDM_Odm->odm_Regulation2_4G));
				return;
			
			}
			else if ((*pDM_Odm->pBandType != ODM_BAND_2_4G) && (*pDM_Odm->pBandType != ODM_BAND_5G))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Adaptivity neither 2G nor 5G band, return\n"));
				return;
			}
		}
		else
		{
			if ((*pDM_Odm->pBandType == ODM_BAND_5G) && 
				!(pDM_Odm->odm_Regulation5G == REGULATION_MKK || pDM_Odm->odm_Regulation5G == REGULATION_WW))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("CarrierSense skip 5G domain code : %d\n", pDM_Odm->odm_Regulation5G));
				return;
			}

			else if((*pDM_Odm->pBandType == ODM_BAND_2_4G) &&
				!(pDM_Odm->odm_Regulation2_4G == REGULATION_MKK  || pDM_Odm->odm_Regulation2_4G == REGULATION_WW))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("CarrierSense skip 2.4G domain code : %d\n", pDM_Odm->odm_Regulation2_4G));
				return;
			
			}
			else if ((*pDM_Odm->pBandType != ODM_BAND_2_4G) && (*pDM_Odm->pBandType != ODM_BAND_5G))
			{
				ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("CarrierSense neither 2G nor 5G band, return\n"));
				return;
			}
		}
	}
#endif

	
	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("odm_Adaptivity() =====> \n"));
	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("IGI_Base=0x%x, TH_L2H_ini = %d, TH_EDCCA_HL_diff = %d\n", 
		pDM_Odm->IGI_Base, pDM_Odm->TH_L2H_ini, pDM_Odm->TH_EDCCA_HL_diff));

	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{											// fix AC series when enable EDCCA hang issue
		ODM_SetBBReg(pDM_Odm, 0x800, BIT10, 1); //ADC_mask disable
		ODM_SetBBReg(pDM_Odm, 0x800, BIT10, 0); //ADC_mask enable
	}

	if(*pDM_Odm->pBandWidth == ODM_BW20M) //CHANNEL_WIDTH_20
		IGI_target = pDM_Odm->IGI_Base;
	else if(*pDM_Odm->pBandWidth == ODM_BW40M)
		IGI_target = pDM_Odm->IGI_Base + 2;
	else if(*pDM_Odm->pBandWidth == ODM_BW80M)
		IGI_target = pDM_Odm->IGI_Base + 2;
	else
		IGI_target = pDM_Odm->IGI_Base;
	pDM_Odm->IGI_target = (u1Byte) IGI_target;
	
	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("BandWidth=%s, IGI_target=0x%x, DynamicLinkAdaptivity = %d\n",
		(*pDM_Odm->pBandWidth==ODM_BW80M)?"80M":((*pDM_Odm->pBandWidth==ODM_BW40M)?"40M":"20M"), IGI_target, pDM_Odm->DynamicLinkAdaptivity));
	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("RSSI_min = %d, AdapIGIUpper= 0x%x, adaptivity_flag = %d, Adaptivity_enable = %d\n",
		 pDM_Odm->RSSI_Min, pDM_Odm->Adaptivity_IGI_upper, pDM_Odm->adaptivity_flag, pDM_Odm->Adaptivity_enable));

	if((pDM_Odm->DynamicLinkAdaptivity == TRUE) && (!pDM_Odm->bLinked) && (pDM_Odm->Adaptivity_enable == FALSE))
	{
		Phydm_SetEDCCAThreshold(pDM_Odm, 0x7f, 0x7f);
		ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("In DynamicLink mode(noisy) and No link, Turn off EDCCA!!\n"));
		return;
	}

	if(pDM_Odm->SupportICType & ODM_RTL8814A)
	{
		TH_L2H_dmc = (s1Byte)pDM_Odm->TH_L2H_ini + IGI_target;
		TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;
	}
	else
	{
		Diff = IGI_target -(s1Byte)IGI;
		TH_L2H_dmc = pDM_Odm->TH_L2H_ini + Diff;
		if(TH_L2H_dmc > 10 && (pDM_Odm->Adaptivity_enable == TRUE)) 	
			TH_L2H_dmc = 10;
				
		TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

		//replace lower bound to prevent EDCCA always equal 1
		if(TH_H2L_dmc < pDM_Odm->H2L_lb)				
			TH_H2L_dmc = pDM_Odm->H2L_lb;
		if(TH_L2H_dmc < pDM_Odm->L2H_lb)
			TH_L2H_dmc = pDM_Odm->L2H_lb;
	}
	ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("IGI=0x%x, TH_L2H_dmc = %d, TH_H2L_dmc = %d\n", IGI, TH_L2H_dmc, TH_H2L_dmc));
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Adaptivity_IGI_upper=0x%x, H2L_lb = 0x%x, L2H_lb = 0x%x\n", pDM_Odm->Adaptivity_IGI_upper, pDM_Odm->H2L_lb, pDM_Odm->L2H_lb));
	
	Phydm_SetEDCCAThreshold(pDM_Odm, TH_H2L_dmc, TH_L2H_dmc);
	return;
}


#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
VOID
Phydm_EnableEDCCA(
	IN		PVOID					pDM_VOID
)
{

	// This should be moved out of OUTSRC
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	// Enable EDCCA. The value is suggested by SD3 Wilson.

	//
	// Revised for ASUS 11b/g performance issues, suggested by BB Neil, 2012.04.13.
	//
	if((pDM_Odm->SupportICType == ODM_RTL8723A)&&(IS_WIRELESS_MODE_G(pAdapter)))
	{
		//PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold, 0x00);
		ODM_Write1Byte(pDM_Odm,rOFDM0_ECCAThreshold,0x00);
		ODM_Write1Byte(pDM_Odm,rOFDM0_ECCAThreshold+2,0xFD);
		
	}	
	else
	{
		//PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold, 0x03);
		ODM_Write1Byte(pDM_Odm,rOFDM0_ECCAThreshold,0x03);
		ODM_Write1Byte(pDM_Odm,rOFDM0_ECCAThreshold+2,0x00);
	}	
	
	//PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold+2, 0x00);
}

VOID
Phydm_DisableEDCCA(
	IN		PVOID					pDM_VOID
)
{	
	// Disable EDCCA..
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	ODM_Write1Byte(pDM_Odm, rOFDM0_ECCAThreshold, 0x7f);
	ODM_Write1Byte(pDM_Odm, rOFDM0_ECCAThreshold+2, 0x7f);
}

//
// Description: According to initial gain value to determine to enable or disable EDCCA.
//
// Suggested by SD3 Wilson. Added by tynli. 2011.11.25.
//
VOID
Phydm_DynamicEDCCA(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte			RegC50, RegC58;
	BOOLEAN			bEDCCAenable = FALSE;
	
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))	
	BOOLEAN			bFwCurrentInPSMode=FALSE;	

	pAdapter->HalFunc.GetHwRegHandler(pAdapter, HW_VAR_FW_PSMODE_STATUS, (pu1Byte)(&bFwCurrentInPSMode));	

	// Disable EDCCA mode while under LPS mode, added by Roger, 2012.09.14.
	if(bFwCurrentInPSMode)
		return;
#endif
	//
	// 2013/11/14 Ken According to BB team Jame's suggestion, we need to disable soft AP mode EDCCA.
	// 2014/01/08 MH For Miracst AP mode test. We need to disable EDCCA. Otherwise, we may stop
	// to send beacon in noisy environment or platform.
	//
	if(ACTING_AS_AP(pAdapter) || ACTING_AS_AP(GetFirstAPAdapter(pAdapter)))
	//if(ACTING_AS_AP(pAdapter))
	{
		ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("At least One Port as AP disable EDCCA\n"));
		Phydm_DisableEDCCA(pDM_Odm);
		if(pHalData->bPreEdccaEnable)
			Phydm_DisableEDCCA(pDM_Odm);
		pHalData->bPreEdccaEnable = FALSE;
		return;
	}
	
	RegC50 = (u1Byte)ODM_GetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskByte0);
	RegC58 = (u1Byte)ODM_GetBBReg(pDM_Odm, rOFDM0_XBAGCCore1, bMaskByte0);


 	if((RegC50 > 0x28 && RegC58 > 0x28) ||
  		((pDM_Odm->SupportICType == ODM_RTL8723A && IS_WIRELESS_MODE_G(pAdapter) && RegC50>0x26)) ||
  		(pDM_Odm->SupportICType == ODM_RTL8188E && RegC50 > 0x28))
	{
		if(!pHalData->bPreEdccaEnable)
		{
			Phydm_EnableEDCCA(pDM_Odm);
			pHalData->bPreEdccaEnable = TRUE;
		}
		
	}
	else if((RegC50 < 0x25 && RegC58 < 0x25) || (pDM_Odm->SupportICType == ODM_RTL8188E && RegC50 < 0x25))
	{
		if(pHalData->bPreEdccaEnable)
		{
			Phydm_DisableEDCCA(pDM_Odm);
			pHalData->bPreEdccaEnable = FALSE;
		}
	}
}

#endif
