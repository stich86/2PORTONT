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

//======================================================
// when antenna test utility is on or some testing need to disable antenna diversity
// call this function to disable all ODM related mechanisms which will switch antenna.
//======================================================
VOID
ODM_StopAntennaSwitchDm(
	IN	PDM_ODM_T	pDM_Odm
	)
{
	// disable ODM antenna diversity
	pDM_Odm->SupportAbility &= ~ODM_BB_ANT_DIV;
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("STOP Antenna Diversity \n"));
}

VOID
ODM_SetAntConfig(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte		antSetting	// 0=A, 1=B, 2=C, ....
	)
{
	if(pDM_Odm->SupportICType == ODM_RTL8723B)
	{
		if(antSetting == 0)		// ant A
			ODM_SetBBReg(pDM_Odm, 0x948, bMaskDWord, 0x00000000);
		else if(antSetting == 1)
			ODM_SetBBReg(pDM_Odm, 0x948, bMaskDWord, 0x00000280);
	}
}

//======================================================


VOID
ODM_SwAntDivRestAfterLink(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte             i;

	if(pDM_Odm->SupportICType == ODM_RTL8723A)
	{
		pDM_SWAT_Table->RSSI_cnt_A = 0;
		pDM_SWAT_Table->RSSI_cnt_B = 0;
		pDM_Odm->RSSI_test = FALSE;
		pDM_SWAT_Table->try_flag = 0xff;
		pDM_SWAT_Table->RSSI_Trying = 0;
		pDM_SWAT_Table->SelectAntennaMap=0xAA;
	
	}
	else if(pDM_Odm->SupportICType & (ODM_RTL8723B|ODM_RTL8821))
	{
		pDM_Odm->RSSI_test = FALSE;
		pDM_SWAT_Table->try_flag = 0xff;
		pDM_SWAT_Table->RSSI_Trying = 0;
		pDM_SWAT_Table->Double_chk_flag= 0;

		pDM_FatTable->RxIdleAnt=MAIN_ANT;

		for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
		{
			pDM_FatTable->MainAnt_Sum[i] = 0;
			pDM_FatTable->AuxAnt_Sum[i] = 0;
			pDM_FatTable->MainAnt_Cnt[i] = 0;
			pDM_FatTable->AuxAnt_Cnt[i] = 0;
		}

	}
}


#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))
VOID
odm_AntDiv_on_off( IN PDM_ODM_T	pDM_Odm ,IN u1Byte swch)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	if(pDM_FatTable->AntDiv_OnOff != swch)
	{
		if(pDM_Odm->AntDivType==S0S1_SW_ANTDIV || pDM_Odm->AntDivType==CGCS_RX_SW_ANTDIV) 
			return;

		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) N-Series HW-AntDiv block\n",(swch==ANTDIV_ON)?"ON" : "OFF"));
			ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, swch); //OFDM AntDiv function block enable
			ODM_SetBBReg(pDM_Odm, 0xa00 , BIT15, swch); //CCK AntDiv function block enable
		}
		else if(pDM_Odm->SupportICType & ODM_AC_ANTDIV_SUPPORT)
		{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) AC-Series HW-AntDiv block\n",(swch==ANTDIV_ON)?"ON" : "OFF"));
			if(pDM_Odm->SupportICType == ODM_RTL8812)
			{
				ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, swch); //OFDM AntDiv function block enable
				ODM_SetBBReg(pDM_Odm, 0xa00 , BIT15, swch); //CCK AntDiv function block enable
			}
			else
			{
				ODM_SetBBReg(pDM_Odm, 0x8D4 , BIT24, swch); //OFDM AntDiv function block enable
					
					if( (pDM_Odm->CutVersion >= ODM_CUT_C) && (pDM_Odm->SupportICType == ODM_RTL8821) && ( pDM_Odm->AntDivType != S0S1_SW_ANTDIV))
					{
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) CCK HW-AntDiv block\n",(swch==ANTDIV_ON)?"ON" : "OFF"));
						ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, swch); 
						ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, swch); //CCK AntDiv function block enable
					}
			}
		}
	}
	pDM_FatTable->AntDiv_OnOff =swch;
	
}

VOID
odm_FastTraining_enable(
	IN		PDM_ODM_T		pDM_Odm , IN  u1Byte  swch
)
{
	u1Byte enable;
	
	if( swch== FAT_ON)
		enable=1;
	else
		enable=0;

	if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
           
            ODM_SetBBReg(pDM_Odm, 0xe08 , BIT16, enable);	//enable fast training
        }
        else if(pDM_Odm->SupportICType == ODM_RTL8192E)
        {
	    ODM_SetBBReg(pDM_Odm, 0xB34 , BIT28, enable);	//enable fast training (path-A)
	    //ODM_SetBBReg(pDM_Odm, 0xB34 , BIT29, enable);	//enable fast training (path-B)
        }
}

VOID
odm_Tx_By_TxDesc_or_Reg( IN PDM_ODM_T pDM_Odm , IN u1Byte swch)
{
	u1Byte enable;
	
	if( swch== TX_BY_DESC)
		enable=1;
	else
		enable=0;

	if(pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV)
	{
		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
				ODM_SetBBReg(pDM_Odm, 0x80c , BIT21, enable); 
		}	
		else if(pDM_Odm->SupportICType & ODM_AC_ANTDIV_SUPPORT)
		{
				ODM_SetBBReg(pDM_Odm, 0x900 , BIT18, enable); 
		}
	}
}

VOID
ODM_UpdateRxIdleAnt(IN PDM_ODM_T pDM_Odm, IN u1Byte Ant)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte	DefaultAnt, OptionalAnt,value32;
	
	//#if (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	//PADAPTER 		pAdapter = pDM_Odm->Adapter;
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	//#endif

	if(pDM_FatTable->RxIdleAnt != Ant)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] RxIdleAnt =%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));

		if(!(pDM_Odm->SupportICType & ODM_RTL8723B))
                	pDM_FatTable->RxIdleAnt = Ant;

		if(Ant == MAIN_ANT)
		{
			DefaultAnt   =  ANT1_2G; 
			OptionalAnt =  ANT2_2G; 
		}
		else
		{
			DefaultAnt  =   ANT2_2G;
			OptionalAnt =  ANT1_2G;
		}
	
		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
			if(pDM_Odm->SupportICType==ODM_RTL8192E)
			{
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT5|BIT4|BIT3, DefaultAnt); //Default RX
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT8|BIT7|BIT6, OptionalAnt);//Optional RX
				ODM_SetBBReg(pDM_Odm, 0x860, BIT14|BIT13|BIT12, DefaultAnt);//Default TX	
			}
                        #if (RTL8723B_SUPPORT == 1)
			else if(pDM_Odm->SupportICType==ODM_RTL8723B)
			{
				value32 = ODM_GetBBReg(pDM_Odm, 0x948, 0xFFF);
			
				if(value32 !=0x280)
					ODM_UpdateRxIdleAnt_8723B(pDM_Odm, Ant, DefaultAnt, OptionalAnt);
				else
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Fail to set RX antenna due to 0x948 = 0x280\n"));
			}
			#endif
			else //88E
			{
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT5|BIT4|BIT3, DefaultAnt);	//Default RX
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, OptionalAnt);	//Optional RX
				ODM_SetBBReg(pDM_Odm, 0x860, BIT14|BIT13|BIT12, DefaultAnt);	        //Default TX	
			}
		}
		else if(pDM_Odm->SupportICType & ODM_AC_ANTDIV_SUPPORT)
		{
			u2Byte	value16 = ODM_Read2Byte(pDM_Odm, ODM_REG_TRMUX_11AC+2);
			//
			// 2014/01/14 MH/Luke.Lee Add direct write for register 0xc0a to prevnt 
			// incorrect 0xc08 bit0-15 .We still not know why it is changed.
			//
			value16 &= ~(BIT11|BIT10|BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3);
			value16 |= ((u2Byte)DefaultAnt <<3);
			value16 |= ((u2Byte)OptionalAnt <<6);
			value16 |= ((u2Byte)DefaultAnt <<9);
			ODM_Write2Byte(pDM_Odm, ODM_REG_TRMUX_11AC+2, value16);
			/*
			ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC , BIT21|BIT20|BIT19, DefaultAnt);	 //Default RX
			ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC , BIT24|BIT23|BIT22, OptionalAnt);//Optional RX
			ODM_SetBBReg(pDM_Odm, ODM_REG_TRMUX_11AC , BIT27|BIT26|BIT25, DefaultAnt);	 //Default TX
			*/
		}

		if(pDM_Odm->SupportICType==ODM_RTL8188E)
		{		
			ODM_SetMACReg(pDM_Odm, 0x6D8 , BIT7|BIT6, DefaultAnt);	//PathA Resp Tx
		}
		else
		{
		ODM_SetMACReg(pDM_Odm, 0x6D8 , BIT10|BIT9|BIT8, DefaultAnt);	//PathA Resp Tx
		}		

	}
	else// pDM_FatTable->RxIdleAnt == Ant
        {
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Stay in Ori-Ant ]  RxIdleAnt =%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
	    pDM_FatTable->RxIdleAnt = Ant;
	}
}


VOID
odm_UpdateTxAnt(IN PDM_ODM_T pDM_Odm, IN u1Byte Ant, IN u4Byte MacId)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u1Byte	TxAnt;

	if (pDM_Odm->AntDivType==CG_TRX_SMART_ANTDIV)
	{
		TxAnt=Ant;
	}
	else
	{
		if(Ant == MAIN_ANT)
			TxAnt = ANT1_2G;
		else
			TxAnt = ANT2_2G;
	}
	
	pDM_FatTable->antsel_a[MacId] = TxAnt&BIT0;
	pDM_FatTable->antsel_b[MacId] = (TxAnt&BIT1)>>1;
	pDM_FatTable->antsel_c[MacId] = (TxAnt&BIT2)>>2;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Tx from TxInfo]: MacID:(( %d )),  TxAnt = (( %s ))\n", MacId,(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=(( 3'b%d%d%d ))\n",pDM_FatTable->antsel_c[MacId] , pDM_FatTable->antsel_b[MacId] , pDM_FatTable->antsel_a[MacId] ));
	
}

#ifdef BEAMFORMING_SUPPORT
#if(DM_ODM_SUPPORT_TYPE == ODM_AP)

VOID
odm_BDC_Init(IN PDM_ODM_T pDM_Odm)
{
	pBDC_T	pDM_BdcTable=&pDM_Odm->DM_BdcTable;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n[ BDC Initialization......] \n"));
	pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
	pDM_BdcTable->BDC_Mode=BDC_MODE_NULL;
	pDM_BdcTable->BDC_Try_flag=0;
	pDM_BdcTable->BDCcoexType_wBfer=0;
	pDM_Odm->bdc_holdstate=0xff;
	
	if(pDM_Odm->SupportICType == ODM_RTL8192E)
	{
		ODM_SetBBReg(pDM_Odm, 0xd7c , 0x0FFFFFFF, 0x1081008); 
		ODM_SetBBReg(pDM_Odm, 0xd80 , 0x0FFFFFFF, 0); 
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8812)
	{
		ODM_SetBBReg(pDM_Odm, 0x9b0 , 0x0FFFFFFF, 0x1081008);     //0x9b0[30:0] = 01081008
		ODM_SetBBReg(pDM_Odm, 0x9b4 , 0x0FFFFFFF, 0);                 //0x9b4[31:0] = 00000000
	}
	
}


VOID
odm_CSI_on_off(IN PDM_ODM_T pDM_Odm, IN u1Byte CSI_en)
{
	if(CSI_en==CSI_ON)
	{
		if(pDM_Odm->SupportICType == ODM_RTL8192E)
		{
			ODM_SetMACReg(pDM_Odm, 0xd84 , BIT11, 1);  //0xd84[11]=1
		}
		else if(pDM_Odm->SupportICType == ODM_RTL8812)
		{
			ODM_SetMACReg(pDM_Odm, 0x9b0 , BIT31, 1);  //0x9b0[31]=1
		}
	
	}
	else if(CSI_en==CSI_OFF)
	{
		if(pDM_Odm->SupportICType == ODM_RTL8192E)
		{
			ODM_SetMACReg(pDM_Odm, 0xd84 , BIT11, 0);  //0xd84[11]=0
		}
		else if(pDM_Odm->SupportICType == ODM_RTL8812)
		{
			ODM_SetMACReg(pDM_Odm, 0x9b0 , BIT31, 0);  //0x9b0[31]=0
		}
	}	
}

VOID
odm_BDCcoexType_withBferClient(IN PDM_ODM_T pDM_Odm, IN u1Byte swch)
{
	pBDC_T 	pDM_BdcTable = &pDM_Odm->DM_BdcTable;
	u1Byte     BDCcoexType_wBfer;
	
	if(swch==DIVON_CSIOFF)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[BDCcoexType: 1] {DIV,CSI} ={1,0} \n"));
		BDCcoexType_wBfer=1;

		if(BDCcoexType_wBfer != pDM_BdcTable->BDCcoexType_wBfer)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			odm_CSI_on_off(pDM_Odm,CSI_OFF);
			pDM_BdcTable->BDCcoexType_wBfer=1;
		}
	}
	else if(swch==DIVOFF_CSION)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[BDCcoexType: 2] {DIV,CSI} ={0,1}\n"));
		BDCcoexType_wBfer=2;

		if(BDCcoexType_wBfer != pDM_BdcTable->BDCcoexType_wBfer)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			odm_CSI_on_off(pDM_Odm,CSI_ON);
			pDM_BdcTable->BDCcoexType_wBfer=2;
		}
	}
}

VOID
odm_BF_AntDiv_ModeArbitration(IN PDM_ODM_T pDM_Odm)
{
		pBDC_T 			pDM_BdcTable = &pDM_Odm->DM_BdcTable;
		u1Byte			current_BDC_Mode;

	#if(DM_ODM_SUPPORT_TYPE  == ODM_AP)
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n"));
	
		//2 Mode 1
		if((pDM_BdcTable->num_Txbfee_Client !=0) && (pDM_BdcTable->num_Txbfer_Client == 0))
		{
			current_BDC_Mode=BDC_MODE_1;
			
			if(current_BDC_Mode != pDM_BdcTable->BDC_Mode)
			{
				pDM_BdcTable->BDC_Mode=BDC_MODE_1;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF);
				pDM_BdcTable->BDC_RxIdleUpdate_counter=1;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Change to (( Mode1 ))\n"));
			}

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Antdiv + BF coextance Mode] : (( Mode1 ))\n"));
		}
		//2 Mode 2
		else if((pDM_BdcTable->num_Txbfee_Client ==0) && (pDM_BdcTable->num_Txbfer_Client != 0))
		{
			current_BDC_Mode=BDC_MODE_2;
			
			if(current_BDC_Mode != pDM_BdcTable->BDC_Mode)
			{
				pDM_BdcTable->BDC_Mode=BDC_MODE_2;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				pDM_BdcTable->BDC_Try_flag=0;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Change to (( Mode2 ))\n"));
				
			}
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Antdiv + BF coextance Mode] : (( Mode2 ))\n"));
		}
		//2 Mode 3
		else if((pDM_BdcTable->num_Txbfee_Client !=0) && (pDM_BdcTable->num_Txbfer_Client != 0))
		{
			current_BDC_Mode=BDC_MODE_3;
			
			if(current_BDC_Mode != pDM_BdcTable->BDC_Mode)
			{
				pDM_BdcTable->BDC_Mode=BDC_MODE_3;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_RxIdleUpdate_counter=1;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Change to (( Mode3 ))\n"));
			}

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Antdiv + BF coextance Mode] : (( Mode3 ))\n"));
		}
		//2 Mode 4
		else if((pDM_BdcTable->num_Txbfee_Client ==0) && (pDM_BdcTable->num_Txbfer_Client == 0))
		{
			current_BDC_Mode=BDC_MODE_4;
			
			if(current_BDC_Mode != pDM_BdcTable->BDC_Mode)
			{
				pDM_BdcTable->BDC_Mode=BDC_MODE_4;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF);
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Change to (( Mode4 ))\n"));
			}

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Antdiv + BF coextance Mode] : (( Mode4 ))\n"));
		}
	#endif

}

VOID
odm_DivTrainState_setting( PDM_ODM_T pDM_Odm)
{
	pBDC_T	pDM_BdcTable=&pDM_Odm->DM_BdcTable;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n*****[S T A R T ]*****  [2-0. DIV_TRAIN_STATE] \n"));
	pDM_BdcTable->BDC_Try_counter =2;
	pDM_BdcTable->BDC_Try_flag=1; 
	pDM_BdcTable->BDC_state=BDC_BFer_TRAIN_STATE;					
	odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF);
}

VOID
odm_BDCcoex_BFeeRxDiv_Arbitration(
	IN		PDM_ODM_T		pDM_Odm
)
{

	pBDC_T    pDM_BdcTable = &pDM_Odm->DM_BdcTable;
	BOOLEAN StopBF_flag;
	u1Byte 	BDC_active_Mode;


	#if(DM_ODM_SUPPORT_TYPE  == ODM_AP)

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***{ num_BFee,  num_BFer , num_Client}  = (( %d  ,  %d  ,  %d))  \n",pDM_BdcTable->num_Txbfee_Client,pDM_BdcTable->num_Txbfer_Client,pDM_BdcTable->num_Client));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***{ num_BF_tars,  num_DIV_tars }  = ((  %d  ,  %d ))  \n",pDM_BdcTable->num_BfTar , pDM_BdcTable->num_DivTar ));

		//2 [ MIB control ]
		if (pDM_Odm->bdc_holdstate==2) 
		{
			odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION); 
			pDM_BdcTable->BDC_state=BDC_BF_HOLD_STATE;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Force in [ BF STATE] \n"));
			return;	
		}
		else if (pDM_Odm->bdc_holdstate==1) 
		{
			pDM_BdcTable->BDC_state=BDC_DIV_HOLD_STATE;
			odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF); 
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Force in [ DIV STATE] \n"));
			return;	
		}

		//------------------------------------------------------------


		
		//2 Mode 2 & 3
		if(pDM_BdcTable->BDC_Mode==BDC_MODE_2 ||pDM_BdcTable->BDC_Mode==BDC_MODE_3)
		{

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n{ Try_flag ,  Try_counter } = {  %d , %d  } \n",pDM_BdcTable->BDC_Try_flag,pDM_BdcTable->BDC_Try_counter));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BDCcoexType = (( %d ))  \n\n", pDM_BdcTable->BDCcoexType_wBfer));
			
                        // All Client have Bfer-Cap-------------------------------
			if(pDM_BdcTable->num_Txbfer_Client == pDM_BdcTable->num_Client) //BFer STA Only?: yes
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BFer STA only?  (( Yes ))\n"));
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION);
				return;
			}
			else
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BFer STA only?  (( No ))\n"));
			}
			//
			if(pDM_BdcTable->bAll_BFSta_Idle==FALSE && pDM_BdcTable->bAll_DivSta_Idle==TRUE)
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("All DIV-STA are idle, but BF-STA not\n"));
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_state=BDC_BFer_TRAIN_STATE;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION);
				return;
			}
			else if(pDM_BdcTable->bAll_BFSta_Idle==TRUE && pDM_BdcTable->bAll_DivSta_Idle==FALSE)
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("All BF-STA are idle, but DIV-STA not\n"));
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF);
				return;
			}

			//Select active mode--------------------------------------
			if(pDM_BdcTable->num_BfTar ==0) //  Selsect_1,  Selsect_2
			{
				if(pDM_BdcTable->num_DivTar ==0)  // Selsect_3
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Select active mode (( 1 )) \n"));
					pDM_BdcTable->BDC_active_Mode=1;
				}
				else
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Select active mode  (( 2 ))\n"));
					pDM_BdcTable->BDC_active_Mode=2;
				}
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF);
				return;
			}
			else // num_BfTar > 0
			{			
				if(pDM_BdcTable->num_DivTar ==0)  // Selsect_3
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Select active mode (( 3 ))\n"));		
					pDM_BdcTable->BDC_active_Mode=3;
					pDM_BdcTable->BDC_Try_flag=0;
					pDM_BdcTable->BDC_state=BDC_BFer_TRAIN_STATE;
					odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION);
					return;
				}
				else // Selsect_4
				{
					BDC_active_Mode=4;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Select active mode (( 4 ))\n"));	
					
					if(BDC_active_Mode!=pDM_BdcTable->BDC_active_Mode)
					{
						pDM_BdcTable->BDC_active_Mode=4;
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Change to active mode (( 4 ))  &  return!!! \n"));	
						return;
					}
				}
			}

#if 1
		if (pDM_Odm->bdc_holdstate==0xff) 
		{
			pDM_BdcTable->BDC_state=BDC_DIV_HOLD_STATE;
			odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF); 
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Force in [ DIV STATE] \n"));	
			return;
		}
#endif

			// Does Client number changed ? -------------------------------
			if(pDM_BdcTable->num_Client !=pDM_BdcTable->pre_num_Client)
			{ 
				pDM_BdcTable->BDC_Try_flag=0;
				pDM_BdcTable->BDC_state=BDC_DIV_TRAIN_STATE;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[  The number of client has been changed !!!]   return to (( BDC_DIV_TRAIN_STATE )) \n"));	
			}
			pDM_BdcTable->pre_num_Client=pDM_BdcTable->num_Client;

			if( pDM_BdcTable->BDC_Try_flag==0)
			{
				//2 DIV_TRAIN_STATE (Mode 2-0)
				if(pDM_BdcTable->BDC_state==BDC_DIV_TRAIN_STATE)
				{
					odm_DivTrainState_setting( pDM_Odm);
				}
				//2 BFer_TRAIN_STATE (Mode 2-1)
				else if(pDM_BdcTable->BDC_state==BDC_BFer_TRAIN_STATE) 
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*****[2-1. BFer_TRAIN_STATE ]*****  \n"));
					
					//if(pDM_BdcTable->num_BfTar==0) 
					//{
					//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BF_tars exist?  : (( No )),   [ BDC_BFer_TRAIN_STATE ] >> [BDC_DIV_TRAIN_STATE] \n"));
					//	odm_DivTrainState_setting( pDM_Odm);
					//}
					//else //num_BfTar != 0
					//{
						pDM_BdcTable->BDC_Try_counter=2;
						pDM_BdcTable->BDC_Try_flag=1;
						pDM_BdcTable->BDC_state=BDC_DECISION_STATE;
						odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION); 
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BF_tars exist?  : (( Yes )),   [ BDC_BFer_TRAIN_STATE ] >> [BDC_DECISION_STATE] \n"));
					//}
				}
				//2 DECISION_STATE (Mode 2-2)
				else if(pDM_BdcTable->BDC_state==BDC_DECISION_STATE)
				{			
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*****[2-2. DECISION_STATE]***** \n"));
					//if(pDM_BdcTable->num_BfTar==0) 
					//{
					//	ODM_AntDiv_Printk(("BF_tars exist?  : (( No )),   [ DECISION_STATE ] >> [BDC_DIV_TRAIN_STATE] \n"));
					//	odm_DivTrainState_setting( pDM_Odm);
					//}
					//else //num_BfTar != 0
					//{
						if(pDM_BdcTable->BF_pass==FALSE || pDM_BdcTable->DIV_pass == FALSE)
							StopBF_flag=TRUE;
						else
							StopBF_flag=FALSE;
						
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BF_tars exist?  : (( Yes )),  {BF_pass, DIV_pass, StopBF_flag }  = { %d, %d, %d } \n" ,pDM_BdcTable->BF_pass,pDM_BdcTable->DIV_pass,StopBF_flag));
					
						if(StopBF_flag==TRUE) //DIV_en
						{
							pDM_BdcTable->BDC_Hold_counter=10; //20
							odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF); 
							pDM_BdcTable->BDC_state=BDC_DIV_HOLD_STATE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ StopBF_flag= ((TRUE)),   BDC_DECISION_STATE ] >> [BDC_DIV_HOLD_STATE] \n"));
						}
						else //BF_en
						{
							pDM_BdcTable->BDC_Hold_counter=10; //20
							odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION); 
							pDM_BdcTable->BDC_state=BDC_BF_HOLD_STATE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[StopBF_flag= ((FALSE)),   BDC_DECISION_STATE ] >> [BDC_BF_HOLD_STATE] \n"));
						}
					//}
				}
				//2 BF-HOLD_STATE (Mode 2-3)
				else if(pDM_BdcTable->BDC_state==BDC_BF_HOLD_STATE)
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*****[2-3. BF_HOLD_STATE ]*****\n"));	

					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BDC_Hold_counter = (( %d )) \n",pDM_BdcTable->BDC_Hold_counter ));	

					if(pDM_BdcTable->BDC_Hold_counter==1)
					{
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ BDC_BF_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE] \n"));	
						odm_DivTrainState_setting( pDM_Odm);
					}
					else
					{
						pDM_BdcTable->BDC_Hold_counter--;
						
						//if(pDM_BdcTable->num_BfTar==0) 
						//{
						//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BF_tars exist?  : (( No )),   [ BDC_BF_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE] \n"));	
						//	odm_DivTrainState_setting( pDM_Odm);
						//}
						//else //num_BfTar != 0
						//{
							//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BF_tars exist?  : (( Yes ))\n"));	
							pDM_BdcTable->BDC_state=BDC_BF_HOLD_STATE;
							odm_BDCcoexType_withBferClient( pDM_Odm, DIVOFF_CSION);
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ BDC_BF_HOLD_STATE ] >> [BDC_BF_HOLD_STATE] \n"));	
						//}
					}
				
				}
				//2 DIV-HOLD_STATE (Mode 2-4)
				else if(pDM_BdcTable->BDC_state==BDC_DIV_HOLD_STATE)
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*****[2-4. DIV_HOLD_STATE ]*****\n"));	
					
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("BDC_Hold_counter = (( %d )) \n",pDM_BdcTable->BDC_Hold_counter ));
					
					if(pDM_BdcTable->BDC_Hold_counter==1)
					{
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ BDC_DIV_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE] \n"));	
						odm_DivTrainState_setting( pDM_Odm);
					}
					else
					{
						pDM_BdcTable->BDC_Hold_counter--;
						pDM_BdcTable->BDC_state=BDC_DIV_HOLD_STATE;
						odm_BDCcoexType_withBferClient( pDM_Odm, DIVON_CSIOFF); 
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ BDC_DIV_HOLD_STATE ] >> [BDC_DIV_HOLD_STATE] \n"));	
					}
				
				}
				
			}
			else if( pDM_BdcTable->BDC_Try_flag==1)
			{
				//2 Set Training Counter
				if(pDM_BdcTable->BDC_Try_counter >1)
				{
					pDM_BdcTable->BDC_Try_counter--;
					if(pDM_BdcTable->BDC_Try_counter ==1)
						pDM_BdcTable->BDC_Try_flag=0; 
						
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training !!\n"));
					//return ;
				}
				
			}
			
		}

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n[end]\n"));

	#endif //#if(DM_ODM_SUPPORT_TYPE  == ODM_AP)




	

}

#endif
#endif //#ifdef BEAMFORMING_SUPPORT


#if (RTL8188E_SUPPORT == 1)


VOID
odm_RX_HWAntDiv_Init_88E(
	IN		PDM_ODM_T		pDM_Odm
)
{
	u4Byte	value32;

	if(pDM_Odm->mp_mode == TRUE)
	{
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 0); // disable HW AntDiv 
		ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);  // 1:CG, 0:CS
        	return;
	}
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  AntDivType=[CGCS_RX_HW_ANTDIV]\n"));
	
	//MAC Setting
	value32 = ODM_GetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord, value32|(BIT23|BIT25)); //Reg4C[25]=1, Reg4C[23]=1 for pin output
	//Pin Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_PIN_CTRL_11N , BIT9|BIT8, 0);//Reg870[8]=1'b0, Reg870[9]=1'b0 		//antsel antselb by HW
	ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT10, 0);	//Reg864[10]=1'b0 	//antsel2 by HW
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT22, 1);	//Regb2c[22]=1'b0 	//disable CS/CG switch
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);	//Regb2c[31]=1'b1	//output at CG only
	//OFDM Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N , bMaskDWord, 0x000000a0);
	//CCK Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_BB_PWR_SAV4_11N , BIT7, 1); //Fix CCK PHY status report issue
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA2_11N , BIT4, 1); //CCK complete HW AntDiv within 64 samples	
	
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANT_MAPPING1_11N , 0xFFFF, 0x0001);	//antenna mapping table
}

VOID
odm_TRX_HWAntDiv_Init_88E(
	IN		PDM_ODM_T		pDM_Odm
)
{
	u4Byte	value32;
	
	if(pDM_Odm->mp_mode == TRUE)
	{
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 0); // disable HW AntDiv 
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT5|BIT4|BIT3, 0); //Default RX   (0/1)
		return;
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  AntDivType=[CG_TRX_HW_ANTDIV (SPDT)]\n"));
	
	//MAC Setting
	value32 = ODM_GetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord, value32|(BIT23|BIT25)); //Reg4C[25]=1, Reg4C[23]=1 for pin output
	//Pin Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_PIN_CTRL_11N , BIT9|BIT8, 0);//Reg870[8]=1'b0, Reg870[9]=1'b0 		//antsel antselb by HW
	ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT10, 0);	//Reg864[10]=1'b0 	//antsel2 by HW
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT22, 0);	//Regb2c[22]=1'b0 	//disable CS/CG switch
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);	//Regb2c[31]=1'b1	//output at CG only
	//OFDM Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N , bMaskDWord, 0x000000a0);
	//CCK Settings
	ODM_SetBBReg(pDM_Odm, ODM_REG_BB_PWR_SAV4_11N , BIT7, 1); //Fix CCK PHY status report issue
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA2_11N , BIT4, 1); //CCK complete HW AntDiv within 64 samples

	//antenna mapping table
	if(!pDM_Odm->bIsMPChip) //testchip
	{
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DEFUALT_A_11N , BIT10|BIT9|BIT8, 1);	//Reg858[10:8]=3'b001
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DEFUALT_A_11N , BIT13|BIT12|BIT11, 2);	//Reg858[13:11]=3'b010
	}
	else //MPchip
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANT_MAPPING1_11N , bMaskDWord, 0x0001);	//Reg914=3'b010, Reg915=3'b001
}


#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
VOID
odm_Smart_HWAntDiv_Init_88E(
	IN		PDM_ODM_T		pDM_Odm
)
{
	u4Byte	value32, i;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  AntDivType=[CG_TRX_SMART_ANTDIV]\n"));
    
	if(pDM_Odm->mp_mode == TRUE)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("pDM_Odm->AntDivType: %d\n", pDM_Odm->AntDivType));
		return;
	}

	pDM_FatTable->TrainIdx = 0;
	pDM_FatTable->FAT_State = FAT_NORMAL_STATE;
	
	pDM_Odm->fat_comb_a=5;
	pDM_Odm->antdiv_intvl = 0x64; // 100ms

	for(i=0; i<6; i++)
	{
		pDM_FatTable->Bssid[i] = 0;
	}
	for(i=0; i< (pDM_Odm->fat_comb_a) ; i++)
	{
		pDM_FatTable->antSumRSSI[i] = 0;
		pDM_FatTable->antRSSIcnt[i] = 0;
		pDM_FatTable->antAveRSSI[i] = 0;	
	}
		
	//MAC Setting
	value32 = ODM_GetMACReg(pDM_Odm, 0x4c, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, 0x4c, bMaskDWord, value32|(BIT23|BIT25)); //Reg4C[25]=1, Reg4C[23]=1 for pin output
	value32 = ODM_GetMACReg(pDM_Odm,  0x7B4, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, 0x7b4, bMaskDWord, value32|(BIT16|BIT17)); //Reg7B4[16]=1 enable antenna training, Reg7B4[17]=1 enable A2 match
	//value32 = PlatformEFIORead4Byte(Adapter, 0x7B4);
	//PlatformEFIOWrite4Byte(Adapter, 0x7b4, value32|BIT18);	//append MACID in reponse packet

	//Match MAC ADDR
	ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, 0);
	ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, 0);
	
	ODM_SetBBReg(pDM_Odm, 0x870 , BIT9|BIT8, 0);//Reg870[8]=1'b0, Reg870[9]=1'b0 		//antsel antselb by HW
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT10, 0);	//Reg864[10]=1'b0 	//antsel2 by HW
	ODM_SetBBReg(pDM_Odm, 0xb2c , BIT22, 0);	//Regb2c[22]=1'b0 	//disable CS/CG switch
	ODM_SetBBReg(pDM_Odm, 0xb2c , BIT31, 0);	//Regb2c[31]=1'b1	//output at CS only
	ODM_SetBBReg(pDM_Odm, 0xca4 , bMaskDWord, 0x000000a0);
	
	//antenna mapping table
	if(pDM_Odm->fat_comb_a == 2)
	{
		if(!pDM_Odm->bIsMPChip) //testchip
		{
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT10|BIT9|BIT8, 1);	//Reg858[10:8]=3'b001
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT13|BIT12|BIT11, 2);	//Reg858[13:11]=3'b010
		}
		else //MPchip
		{
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 1);
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 2);
		}
	}
	else
	{
		if(!pDM_Odm->bIsMPChip) //testchip
		{
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT10|BIT9|BIT8, 0);	//Reg858[10:8]=3'b000
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT13|BIT12|BIT11, 1);	//Reg858[13:11]=3'b001
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT16, 0);
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT15|BIT14, 2);	//(Reg878[0],Reg858[14:15])=3'b010
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT19|BIT18|BIT17, 3);//Reg878[3:1]=3b'011
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT22|BIT21|BIT20, 4);//Reg878[6:4]=3b'100
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT25|BIT24|BIT23, 5);//Reg878[9:7]=3b'101 
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT28|BIT27|BIT26, 6);//Reg878[12:10]=3b'110 
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT31|BIT30|BIT29, 7);//Reg878[15:13]=3b'111
		}
		else //MPchip
		{
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 4);     // 0: 3b'000
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 2);     // 1: 3b'001	
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte2, 0);     // 2: 3b'010
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte3, 1);     // 3: 3b'011
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte0, 3);     // 4: 3b'100
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte1, 5);     // 5: 3b'101
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte2, 6);     // 6: 3b'110
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte3, 255); // 7: 3b'111
		}
	}

	//Default Ant Setting when no fast training
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT5|BIT4|BIT3, 0);	//Default RX
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, 1);	//Optional RX
	ODM_SetBBReg(pDM_Odm, 0x860 , BIT14|BIT13|BIT12, 0);//Default TX

	//Enter Traing state
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT2|BIT1|BIT0, (pDM_Odm->fat_comb_a-1));	//Reg864[2:0]=3'd6	//ant combination=reg864[2:0]+1
		
	//SW Control
	//PHY_SetBBReg(Adapter, 0x864 , BIT10, 1);
	//PHY_SetBBReg(Adapter, 0x870 , BIT9, 1);
	//PHY_SetBBReg(Adapter, 0x870 , BIT8, 1);
	//PHY_SetBBReg(Adapter, 0x864 , BIT11, 1);
	//PHY_SetBBReg(Adapter, 0x860 , BIT9, 0);
	//PHY_SetBBReg(Adapter, 0x860 , BIT8, 0);
}
#endif

#endif //#if (RTL8188E_SUPPORT == 1)


#if (RTL8192E_SUPPORT == 1)
VOID
odm_RX_HWAntDiv_Init_92E(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	if(pDM_Odm->mp_mode == TRUE)
	{
		//pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		ODM_SetBBReg(pDM_Odm, 0xc50 , BIT8, 0); //r_rxdiv_enable_anta  Regc50[8]=1'b0  0: control by c50[9]
		ODM_SetBBReg(pDM_Odm, 0xc50 , BIT9, 1);  // 1:CG, 0:CS
		return;
	}
	
	 ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8192E AntDiv_Init =>  AntDivType=[CGCS_RX_HW_ANTDIV]\n"));
	
	 //Pin Settings
	 ODM_SetBBReg(pDM_Odm, 0x870 , BIT8, 0);//Reg870[8]=1'b0,    // "antsel" is controled by HWs
	 ODM_SetBBReg(pDM_Odm, 0xc50 , BIT8, 1); //Regc50[8]=1'b1  //" CS/CG switching" is controled by HWs

	 //Mapping table
	 ODM_SetBBReg(pDM_Odm, 0x914 , 0xFFFF, 0x0100); //antenna mapping table
	  
	 //OFDM Settings
	 ODM_SetBBReg(pDM_Odm, 0xca4 , 0x7FF, 0xA0); //thershold
	 ODM_SetBBReg(pDM_Odm, 0xca4 , 0x7FF000, 0x0); //bias
	 
	 //CCK Settings
	 ODM_SetBBReg(pDM_Odm, 0xa04 , 0xF000000, 0); //Select which path to receive for CCK_1 & CCK_2
	 ODM_SetBBReg(pDM_Odm, 0xb34 , BIT30, 0); //(92E) ANTSEL_CCK_opt = r_en_antsel_cck? ANTSEL_CCK: 1'b0
	 ODM_SetBBReg(pDM_Odm, 0xa74 , BIT7, 1); //Fix CCK PHY status report issue
	 ODM_SetBBReg(pDM_Odm, 0xa0c , BIT4, 1); //CCK complete HW AntDiv within 64 samples
	 
	 #ifdef ODM_EVM_ENHANCE_ANTDIV
	//EVM enhance AntDiv method init----------------------------------------------------------------------
	pDM_FatTable->EVM_method_enable=0;
	pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
	pDM_Odm->antdiv_intvl = 0x64; 
	ODM_SetBBReg(pDM_Odm, 0x910 , 0x3f, 0xf );	   
	pDM_Odm->antdiv_evm_en=1;
	//pDM_Odm->antdiv_period=1;

	#endif
	 
}

VOID
odm_TRX_HWAntDiv_Init_92E(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	if(pDM_Odm->mp_mode == TRUE)
	{
		//pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		ODM_SetBBReg(pDM_Odm, 0xc50 , BIT8, 0); //r_rxdiv_enable_anta  Regc50[8]=1'b0  0: control by c50[9]
		ODM_SetBBReg(pDM_Odm, 0xc50 , BIT9, 1);  // 1:CG, 0:CS
		return;
	}

	 ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8192E AntDiv_Init =>  AntDivType=[ Only for DIR605, CG_TRX_HW_ANTDIV]\n"));
	
	//3 --RFE pin setting---------
	//[MAC]
	ODM_SetMACReg(pDM_Odm, 0x38, BIT11, 1);            //DBG PAD Driving control (GPIO 8)
	ODM_SetMACReg(pDM_Odm, 0x4c, BIT23, 0);            //path-A , RFE_CTRL_3 
	ODM_SetMACReg(pDM_Odm, 0x4c, BIT29, 1);            //path-A , RFE_CTRL_8
	//[BB]
	ODM_SetBBReg(pDM_Odm, 0x944 , BIT3, 1);              //RFE_buffer
	ODM_SetBBReg(pDM_Odm, 0x944 , BIT8, 1);	
	ODM_SetBBReg(pDM_Odm, 0x940 , BIT7|BIT6, 0x0); // r_rfe_path_sel_   (RFE_CTRL_3)
	ODM_SetBBReg(pDM_Odm, 0x940 , BIT17|BIT16, 0x0); // r_rfe_path_sel_   (RFE_CTRL_8)
	ODM_SetBBReg(pDM_Odm, 0x944 , BIT31, 0);     //RFE_buffer
	ODM_SetBBReg(pDM_Odm, 0x92C , BIT3, 0);     //rfe_inv  (RFE_CTRL_3)
	ODM_SetBBReg(pDM_Odm, 0x92C , BIT8, 1);     //rfe_inv  (RFE_CTRL_8)
	ODM_SetBBReg(pDM_Odm, 0x930 , 0xF000, 0x8);           //path-A , RFE_CTRL_3 
	ODM_SetBBReg(pDM_Odm, 0x934 , 0xF, 0x8);           //path-A , RFE_CTRL_8
	//3 -------------------------
	
	 //Pin Settings
	ODM_SetBBReg(pDM_Odm, 0xC50 , BIT8, 0);	   //path-A   	//disable CS/CG switch

/* Let it follows PHY_REG for bit9 setting
	if(pDM_Odm->priv->pshare->rf_ft_var.use_ext_pa || pDM_Odm->priv->pshare->rf_ft_var.use_ext_lna)
		ODM_SetBBReg(pDM_Odm, 0xC50 , BIT9, 1);//path-A 	//output at CS
	else
		ODM_SetBBReg(pDM_Odm, 0xC50 , BIT9, 0);    //path-A 	//output at CG ->normal power
*/

	ODM_SetBBReg(pDM_Odm, 0x870 , BIT9|BIT8, 0);  //path-A  	//antsel antselb by HW
	ODM_SetBBReg(pDM_Odm, 0xB38 , BIT10, 0);	   //path-A    	//antsel2 by HW	
 
	//Mapping table
	 ODM_SetBBReg(pDM_Odm, 0x914 , 0xFFFF, 0x0100); //antenna mapping table
	  
	 //OFDM Settings
	 ODM_SetBBReg(pDM_Odm, 0xca4 , 0x7FF, 0xA0); //thershold
	 ODM_SetBBReg(pDM_Odm, 0xca4 , 0x7FF000, 0x0); //bias
	 
	 //CCK Settings
	 ODM_SetBBReg(pDM_Odm, 0xa04 , 0xF000000, 0); //Select which path to receive for CCK_1 & CCK_2
	 ODM_SetBBReg(pDM_Odm, 0xb34 , BIT30, 0); //(92E) ANTSEL_CCK_opt = r_en_antsel_cck? ANTSEL_CCK: 1'b0
	 ODM_SetBBReg(pDM_Odm, 0xa74 , BIT7, 1); //Fix CCK PHY status report issue
	 ODM_SetBBReg(pDM_Odm, 0xa0c , BIT4, 1); //CCK complete HW AntDiv within 64 samples 	 

	 //Timming issue
	 ODM_SetBBReg(pDM_Odm, 0xE20 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)

	#ifdef ODM_EVM_ENHANCE_ANTDIV
	//EVM enhance AntDiv method init----------------------------------------------------------------------
	pDM_FatTable->EVM_method_enable=0;
	pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
	pDM_Odm->antdiv_intvl = 0x64; 
	ODM_SetBBReg(pDM_Odm, 0x910 , 0x3f, 0xf );	   
	pDM_Odm->antdiv_evm_en=1;
	//pDM_Odm->antdiv_period=1;
	#endif
}

#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
VOID
odm_Smart_HWAntDiv_Init_92E(
	IN		PDM_ODM_T		pDM_Odm
)
{
    ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  AntDivType=[CG_TRX_SMART_ANTDIV]\n"));
}
#endif

#endif //#if (RTL8192E_SUPPORT == 1)


#if (RTL8723B_SUPPORT == 1)
VOID
odm_TRX_HWAntDiv_Init_8723B(
	IN		PDM_ODM_T		pDM_Odm
)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8723B AntDiv_Init =>  AntDivType=[CG_TRX_HW_ANTDIV(DPDT)]\n"));
      
	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 1);
	
	//OFDM HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xCA4 , 0x7FF, 0xa0); //thershold
	ODM_SetBBReg(pDM_Odm, 0xCA4 , 0x7FF000, 0x00); //bias
		
	//CCK HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xA74 , BIT7, 1); //patch for clk from 88M to 80M
	ODM_SetBBReg(pDM_Odm, 0xA0C , BIT4, 1); //do 64 samples
	
	//BT Coexistence
	ODM_SetBBReg(pDM_Odm, 0x864, BIT12, 0); //keep antsel_map when GNT_BT = 1
	ODM_SetBBReg(pDM_Odm, 0x874 , BIT23, 0); //Disable hw antsw & fast_train.antsw when GNT_BT=1

        //Output Pin Settings
	ODM_SetBBReg(pDM_Odm, 0x870 , BIT8, 0); //
		
	ODM_SetBBReg(pDM_Odm, 0x948 , BIT6, 0); //WL_BB_SEL_BTG_TRXG_anta,  (1: HW CTRL  0: SW CTRL)
	ODM_SetBBReg(pDM_Odm, 0x948 , BIT7, 0);
		
	ODM_SetMACReg(pDM_Odm, 0x40 , BIT3, 1);
	ODM_SetMACReg(pDM_Odm, 0x38 , BIT11, 1);
	ODM_SetMACReg(pDM_Odm, 0x4C ,  BIT24|BIT23, 2); //select DPDT_P and DPDT_N as output pin
		
	ODM_SetBBReg(pDM_Odm, 0x944 , BIT0|BIT1, 3); //in/out
	ODM_SetBBReg(pDM_Odm, 0x944 , BIT31, 0); //

	ODM_SetBBReg(pDM_Odm, 0x92C , BIT1, 0); //DPDT_P non-inverse
	ODM_SetBBReg(pDM_Odm, 0x92C , BIT0, 1); //DPDT_N inverse

	ODM_SetBBReg(pDM_Odm, 0x930 , 0xF0, 8); // DPDT_P = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0x930 , 0xF, 8); // DPDT_N = ANTSEL[0]

	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0xE20 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)

	//2 [--For HW Bug Setting]
	if(pDM_Odm->AntType == ODM_AUTO_ANT)
		ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, 0); //CCK AntDiv function block enable

}

	

VOID
odm_S0S1_SWAntDiv_Init_8723B(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8723B AntDiv_Init => AntDivType=[ S0S1_SW_AntDiv] \n"));

	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 1);
	
	//Output Pin Settings
	//ODM_SetBBReg(pDM_Odm, 0x948 , BIT6, 0x1); 
	ODM_SetBBReg(pDM_Odm, 0x870 , BIT9|BIT8, 0); 

	pDM_FatTable->bBecomeLinked  =FALSE;
	pDM_SWAT_Table->try_flag = 0xff;	
	pDM_SWAT_Table->Double_chk_flag = 0;
	pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
	
	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0xE20 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)
	
	//2 [--For HW Bug Setting]
	ODM_SetBBReg(pDM_Odm, 0x80C , BIT21, 0); //TX Ant  by Reg

}

VOID
odm_S0S1_SWAntDiv_Reset_8723B(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pSWAT_T		pDM_SWAT_Table 	= &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable 	= &pDM_Odm->DM_FatTable;
    
	pDM_FatTable->bBecomeLinked  =FALSE;
	pDM_SWAT_Table->try_flag = 0xff;	
	pDM_SWAT_Table->Double_chk_flag = 0;
	pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SWAntDiv_Reset_8723B(): pDM_FatTable->bBecomeLinked = %d\n", pDM_FatTable->bBecomeLinked));
}

VOID
ODM_UpdateRxIdleAnt_8723B(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Ant,
	IN		u4Byte			DefaultAnt, 
	IN		u4Byte			OptionalAnt
)
{
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	PADAPTER 		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte			count=0;
	u1Byte			u1Temp;
	u1Byte			H2C_Parameter;
	u4Byte			value32;
	
	if(!pDM_Odm->bLinked)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Fail to set RX antenna due to no link\n"));
		return;
	}

	// Send H2C command to FW
	// Enable wifi calibration
	H2C_Parameter = TRUE;
	ODM_FillH2CCmd(pDM_Odm, ODM_H2C_WIFI_CALIBRATION, 1, &H2C_Parameter);

	// Check if H2C command sucess or not (0x1e6)
	u1Temp = ODM_Read1Byte(pDM_Odm, 0x1e6);
	while((u1Temp != 0x1) && (count < 100))
	{
		ODM_delay_us(10);	
		u1Temp = ODM_Read1Byte(pDM_Odm, 0x1e6);
		count++;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: H2C command status = %d, count = %d\n", u1Temp, count));

	if(u1Temp == 0x1)
	{
		// Check if BT is doing IQK (0x1e7)
		count = 0;
		u1Temp = ODM_Read1Byte(pDM_Odm, 0x1e7);
		while((!(u1Temp & BIT0))  && (count < 100))
		{
			ODM_delay_us(50);	
			u1Temp = ODM_Read1Byte(pDM_Odm, 0x1e7);
			count++;
		}
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: BT IQK status = %d, count = %d\n", u1Temp, count));

		if(u1Temp & BIT0)
		{
			ODM_SetBBReg(pDM_Odm, 0x948 , BIT6, 0x1);
			ODM_SetBBReg(pDM_Odm, 0x948 , BIT9, DefaultAnt);	
			ODM_SetBBReg(pDM_Odm, 0x864 , BIT5|BIT4|BIT3, DefaultAnt);	//Default RX
			ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, OptionalAnt);	//Optional RX
			ODM_SetBBReg(pDM_Odm, 0x860, BIT14|BIT13|BIT12, DefaultAnt); //Default TX	
			pDM_FatTable->RxIdleAnt = Ant;

			// Set TX AGC by S0/S1
			// Need to consider Linux driver
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			 pAdapter->HalFunc.SetTxPowerLevelHandler(pAdapter, pHalData->CurrentChannel);
#elif(DM_ODM_SUPPORT_TYPE == ODM_CE)
			rtw_hal_set_tx_power_level(pAdapter, pHalData->CurrentChannel);
#endif

			// Set IQC by S0/S1
			ODM_SetIQCbyRFpath(pDM_Odm,DefaultAnt);
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Sucess to set RX antenna\n"));
		}
		else
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Fail to set RX antenna due to BT IQK\n"));
	}
	else
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Fail to set RX antenna due to H2C command fail\n"));

	// Send H2C command to FW
	// Disable wifi calibration
	H2C_Parameter = FALSE;
	ODM_FillH2CCmd(pDM_Odm, ODM_H2C_WIFI_CALIBRATION, 1, &H2C_Parameter);

}

#endif //#if (RTL8723B_SUPPORT == 1)

#if (RTL8821A_SUPPORT == 1)
VOID
odm_TRX_HWAntDiv_Init_8821A(
	IN		PDM_ODM_T		pDM_Odm
)
{

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8821A AntDiv_Init => AntDivType=[ CG_TRX_HW_ANTDIV (DPDT)] \n"));

	//Output Pin Settings
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT25, 0);

	ODM_SetMACReg(pDM_Odm, 0x64 , BIT29, 1); //PAPE by WLAN control
	ODM_SetMACReg(pDM_Odm, 0x64 , BIT28, 1); //LNAON by WLAN control

	ODM_SetBBReg(pDM_Odm, 0xCB0 , bMaskDWord, 0x77775745);
	ODM_SetBBReg(pDM_Odm, 0xCB8 , BIT16, 0);
	
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT23, 0); //select DPDT_P and DPDT_N as output pin
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT24, 1); //by WLAN control
	ODM_SetBBReg(pDM_Odm, 0xCB4 , 0xF, 8); // DPDT_P = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB4 , 0xF0, 8); // DPDT_N = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT29, 0); //DPDT_P non-inverse
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT28, 1); //DPDT_N inverse

	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte1, 1);

	//OFDM HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF, 0xA0); //thershold
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF000, 0x10); //bias
		
	//CCK HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xA74 , BIT7, 1); //patch for clk from 88M to 80M
	ODM_SetBBReg(pDM_Odm, 0xA0C , BIT4, 1); //do 64 samples

	ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, 0); //ANTSEL_CCK sent to the smart_antenna circuit
	ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, 0); //CCK AntDiv function block enable

	//BT Coexistence
	ODM_SetBBReg(pDM_Odm, 0xCAC , BIT9, 1); //keep antsel_map when GNT_BT = 1
	ODM_SetBBReg(pDM_Odm, 0x804 , BIT4, 1); //Disable hw antsw & fast_train.antsw when GNT_BT=1

	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0x818 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)
	ODM_SetBBReg(pDM_Odm, 0x8CC , BIT20|BIT19|BIT18, 3); //settling time of antdiv by RF LNA = 100ns

	//response TX ant by RX ant
	ODM_SetMACReg(pDM_Odm, 0x668 , BIT3, 1);
	
}

VOID
odm_S0S1_SWAntDiv_Init_8821A(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8821A AntDiv_Init => AntDivType=[ S0S1_SW_AntDiv] \n"));

	//Output Pin Settings
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT25, 0);

	ODM_SetMACReg(pDM_Odm, 0x64 , BIT29, 1); //PAPE by WLAN control
	ODM_SetMACReg(pDM_Odm, 0x64 , BIT28, 1); //LNAON by WLAN control

	ODM_SetBBReg(pDM_Odm, 0xCB0 , bMaskDWord, 0x77775745);
	ODM_SetBBReg(pDM_Odm, 0xCB8 , BIT16, 0);
	
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT23, 0); //select DPDT_P and DPDT_N as output pin
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT24, 1); //by WLAN control
	ODM_SetBBReg(pDM_Odm, 0xCB4 , 0xF, 8); // DPDT_P = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB4 , 0xF0, 8); // DPDT_N = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT29, 0); //DPDT_P non-inverse
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT28, 1); //DPDT_N inverse

	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte1, 1);

	//OFDM HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF, 0xA0); //thershold
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF000, 0x10); //bias
	
	//CCK HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xA74 , BIT7, 1); //patch for clk from 88M to 80M
	ODM_SetBBReg(pDM_Odm, 0xA0C , BIT4, 1); //do 64 samples

	ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, 0); //ANTSEL_CCK sent to the smart_antenna circuit
	ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, 0); //CCK AntDiv function block enable

	//BT Coexistence
	ODM_SetBBReg(pDM_Odm, 0xCAC , BIT9, 1); //keep antsel_map when GNT_BT = 1
	ODM_SetBBReg(pDM_Odm, 0x804 , BIT4, 1); //Disable hw antsw & fast_train.antsw when GNT_BT=1

	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0x818 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)
	ODM_SetBBReg(pDM_Odm, 0x8CC , BIT20|BIT19|BIT18, 3); //settling time of antdiv by RF LNA = 100ns

	//response TX ant by RX ant
	ODM_SetMACReg(pDM_Odm, 0x668 , BIT3, 1);
	
		
	ODM_SetBBReg(pDM_Odm, 0x900 , BIT18, 0); 
	
	pDM_SWAT_Table->try_flag = 0xff;	
	pDM_SWAT_Table->Double_chk_flag = 0;
	pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
	pDM_SWAT_Table->CurAntenna = MAIN_ANT;
	pDM_SWAT_Table->PreAntenna = MAIN_ANT;
	pDM_SWAT_Table->SWAS_NoLink_State = 0;

}
#endif //#if (RTL8821A_SUPPORT == 1)

#if (RTL8881A_SUPPORT == 1)
VOID
odm_RX_HWAntDiv_Init_8881A(
	IN		PDM_ODM_T		pDM_Odm
)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8881A AntDiv_Init => AntDivType=[ CGCS_RX_HW_ANTDIV] \n"));

}

VOID
odm_TRX_HWAntDiv_Init_8881A(
	IN		PDM_ODM_T		pDM_Odm
)
{

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8881A AntDiv_Init => AntDivType=[ CG_TRX_HW_ANTDIV (SPDT)] \n"));

	//Output Pin Settings
	// [SPDT related]
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT25, 0);
	ODM_SetMACReg(pDM_Odm, 0x4C , BIT26, 0);
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT31, 0); //delay buffer
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT22, 0); 
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT24, 1);
	ODM_SetBBReg(pDM_Odm, 0xCB0 , 0xF00, 8); // DPDT_P = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB0 , 0xF0000, 8); // DPDT_N = ANTSEL[0]	
	
	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte1, 1);

	//OFDM HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF, 0xA0); //thershold
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF000, 0x0); //bias
	ODM_SetBBReg(pDM_Odm, 0x8CC , BIT20|BIT19|BIT18, 3); //settling time of antdiv by RF LNA = 100ns
	
	//CCK HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xA74 , BIT7, 1); //patch for clk from 88M to 80M
	ODM_SetBBReg(pDM_Odm, 0xA0C , BIT4, 1); //do 64 samples

	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0x818 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)

	//2 [--For HW Bug Setting]

	ODM_SetBBReg(pDM_Odm, 0x900 , BIT18, 0); //TX Ant  by Reg //  A-cut bug
}

#endif //#if (RTL8881A_SUPPORT == 1)


#if (RTL8812A_SUPPORT == 1)
VOID
odm_TRX_HWAntDiv_Init_8812A(
	IN		PDM_ODM_T		pDM_Odm
)
{
	 ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8812A AntDiv_Init => AntDivType=[ CG_TRX_HW_ANTDIV (SPDT)] \n"));

	//3 //3 --RFE pin setting---------
	//[BB]
	ODM_SetBBReg(pDM_Odm, 0x900 , BIT10|BIT9|BIT8, 0x0);	  //disable SW switch
	ODM_SetBBReg(pDM_Odm, 0x900 , BIT17|BIT16, 0x0);	 
	ODM_SetBBReg(pDM_Odm, 0x974 , BIT7|BIT6, 0x3);     // in/out
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT31, 0); //delay buffer
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT26, 0); 
	ODM_SetBBReg(pDM_Odm, 0xCB4 , BIT27, 1);
	ODM_SetBBReg(pDM_Odm, 0xCB0 , 0xF000000, 8); // DPDT_P = ANTSEL[0]
	ODM_SetBBReg(pDM_Odm, 0xCB0 , 0xF0000000, 8); // DPDT_N = ANTSEL[0]
	//3 -------------------------

	//Mapping Table
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte0, 0);
	ODM_SetBBReg(pDM_Odm, 0xCA4 , bMaskByte1, 1);

	//OFDM HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF, 0xA0); //thershold
	ODM_SetBBReg(pDM_Odm, 0x8D4 , 0x7FF000, 0x0); //bias
	ODM_SetBBReg(pDM_Odm, 0x8CC , BIT20|BIT19|BIT18, 3); //settling time of antdiv by RF LNA = 100ns
	
	//CCK HW AntDiv Parameters
	ODM_SetBBReg(pDM_Odm, 0xA74 , BIT7, 1); //patch for clk from 88M to 80M
	ODM_SetBBReg(pDM_Odm, 0xA0C , BIT4, 1); //do 64 samples

	//Timming issue
	ODM_SetBBReg(pDM_Odm, 0x818 , BIT23|BIT22|BIT21|BIT20, 8); //keep antidx after tx for ACK ( unit x 32 mu sec)

	//2 [--For HW Bug Setting]

	ODM_SetBBReg(pDM_Odm, 0x900 , BIT18, 0); //TX Ant  by Reg //  A-cut bug
	
}

#endif //#if (RTL8812A_SUPPORT == 1)




#ifdef ODM_EVM_ENHANCE_ANTDIV



VOID
odm_EVM_FastAnt_Reset(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
		
	pDM_FatTable->EVM_method_enable=0;
	odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
	pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
	pDM_Odm->antdiv_period=0;
	ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 0);
}


VOID
odm_EVM_Enhance_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
)
{
	u4Byte	Main_RSSI, Aux_RSSI ;
	u4Byte	Main_CRC_utility=0,Aux_CRC_utility=0,utility_ratio=1;
	u4Byte	Main_EVM, Aux_EVM,Diff_RSSI=0,diff_EVM=0;	
	u1Byte	score_EVM=0,score_CRC=0;
	u1Byte	rssi_larger_ant = 0;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte	value32, i;
	BOOLEAN Main_above1=FALSE,Aux_above1=FALSE;
	BOOLEAN Force_antenna=FALSE;
	PSTA_INFO_T   	pEntry;
	pDM_FatTable->TargetAnt_enhance=0xFF;
	
	
	if((pDM_Odm->SupportICType & ODM_EVM_ENHANCE_ANTDIV_SUPPORT_IC))
	{
		if(pDM_Odm->bOneEntryOnly)
		{
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[One Client only] \n"));
			i = pDM_Odm->OneEntry_MACID;

			Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
			Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;

			if((Main_RSSI==0 && Aux_RSSI !=0 && Aux_RSSI>=FORCE_RSSI_DIFF) || (Main_RSSI!=0 && Aux_RSSI==0 && Main_RSSI>=FORCE_RSSI_DIFF))
			{
				Diff_RSSI=FORCE_RSSI_DIFF;
			}
			else if(Main_RSSI!=0 && Aux_RSSI !=0)
			{
				Diff_RSSI = (Main_RSSI>=Aux_RSSI)?(Main_RSSI-Aux_RSSI):(Aux_RSSI-Main_RSSI); 
			}
			
			if (Main_RSSI >= Aux_RSSI)
				rssi_larger_ant = MAIN_ANT;
			else
				rssi_larger_ant = AUX_ANT;

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n", pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n" , pDM_FatTable->AuxAnt_Cnt[i] , Aux_RSSI));
						
			if(  ((Main_RSSI>=Evm_RSSI_TH_High||Aux_RSSI>=Evm_RSSI_TH_High )|| (pDM_FatTable->EVM_method_enable==1)  )
				//&& (Diff_RSSI <= FORCE_RSSI_DIFF + 1)
                                    )
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_H || EVM_method_enable==1]  && "));
				
				if(((Main_RSSI>=Evm_RSSI_TH_Low)||(Aux_RSSI>=Evm_RSSI_TH_Low) ))
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_L ] \n"));

					//2 [ Normal state Main]
					if(pDM_FatTable->FAT_State == NORMAL_STATE_MIAN)
					{

						pDM_FatTable->EVM_method_enable=1;
						odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
						pDM_Odm->antdiv_period=3;

						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: MIAN] \n"));
						pDM_FatTable->MainAntEVM_Sum[i] = 0;
						pDM_FatTable->AuxAntEVM_Sum[i] = 0;
						pDM_FatTable->MainAntEVM_Cnt[i] = 0;
						pDM_FatTable->AuxAntEVM_Cnt[i] = 0;

						pDM_FatTable->FAT_State = NORMAL_STATE_AUX;
						ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 1); //Accept CRC32 Error packets.
						ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
						
						pDM_FatTable->CRC32_Ok_Cnt=0;
						pDM_FatTable->CRC32_Fail_Cnt=0;
						ODM_SetTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //m
					}
					//2 [ Normal state Aux ]
					else if(pDM_FatTable->FAT_State == NORMAL_STATE_AUX)
					{
						pDM_FatTable->MainCRC32_Ok_Cnt=pDM_FatTable->CRC32_Ok_Cnt;
						pDM_FatTable->MainCRC32_Fail_Cnt=pDM_FatTable->CRC32_Fail_Cnt;
						
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: AUX] \n"));
						pDM_FatTable->FAT_State = TRAINING_STATE;
						ODM_UpdateRxIdleAnt(pDM_Odm, AUX_ANT);

						pDM_FatTable->CRC32_Ok_Cnt=0;
						pDM_FatTable->CRC32_Fail_Cnt=0;
						ODM_SetTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //ms
					}					
					else if(pDM_FatTable->FAT_State == TRAINING_STATE)
					{
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Training state ] \n"));
						pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
						
						//3 [CRC32 statistic]
						pDM_FatTable->AuxCRC32_Ok_Cnt=pDM_FatTable->CRC32_Ok_Cnt;
						pDM_FatTable->AuxCRC32_Fail_Cnt=pDM_FatTable->CRC32_Fail_Cnt;

						if( (pDM_FatTable->MainCRC32_Ok_Cnt  >= ((pDM_FatTable->AuxCRC32_Ok_Cnt)<<1)) || ((Diff_RSSI>=18) && (rssi_larger_ant == MAIN_ANT)))
						{
							pDM_FatTable->TargetAnt_CRC32=MAIN_ANT;
							Force_antenna=TRUE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Main \n"));
						}
						else if((pDM_FatTable->AuxCRC32_Ok_Cnt  >= ((pDM_FatTable->MainCRC32_Ok_Cnt)<<1)) || ((Diff_RSSI>=18) && (rssi_larger_ant == AUX_ANT)))
						{
							pDM_FatTable->TargetAnt_CRC32=AUX_ANT;
							Force_antenna=TRUE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Aux \n"));
						}
						else
						{
							if(pDM_FatTable->MainCRC32_Fail_Cnt<=5)
								pDM_FatTable->MainCRC32_Fail_Cnt=5;
							
							if(pDM_FatTable->AuxCRC32_Fail_Cnt<=5)
								pDM_FatTable->AuxCRC32_Fail_Cnt=5;
						
							if(pDM_FatTable->MainCRC32_Ok_Cnt >pDM_FatTable->MainCRC32_Fail_Cnt )
								Main_above1=TRUE;
						
							if(pDM_FatTable->AuxCRC32_Ok_Cnt >pDM_FatTable->AuxCRC32_Fail_Cnt )
								Aux_above1=TRUE;

							if(Main_above1==TRUE && Aux_above1==FALSE)
							{
								Force_antenna=TRUE;
								pDM_FatTable->TargetAnt_CRC32=MAIN_ANT;
							}
							else if(Main_above1==FALSE && Aux_above1==TRUE)
							{
								Force_antenna=TRUE;
								pDM_FatTable->TargetAnt_CRC32=AUX_ANT;
							}
							else if(Main_above1==TRUE && Aux_above1==TRUE)
							{
								Main_CRC_utility=((pDM_FatTable->MainCRC32_Ok_Cnt)<<7)/pDM_FatTable->MainCRC32_Fail_Cnt;
								Aux_CRC_utility=((pDM_FatTable->AuxCRC32_Ok_Cnt)<<7)/pDM_FatTable->AuxCRC32_Fail_Cnt;
								pDM_FatTable->TargetAnt_CRC32 = (Main_CRC_utility==Aux_CRC_utility)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_CRC_utility>=Aux_CRC_utility)?MAIN_ANT:AUX_ANT);
								
								if(Main_CRC_utility!=0 && Aux_CRC_utility!=0)
								{
									if(Main_CRC_utility>=Aux_CRC_utility)
										utility_ratio=(Main_CRC_utility<<1)/Aux_CRC_utility;
									else
										utility_ratio=(Aux_CRC_utility<<1)/Main_CRC_utility;
								}
							}
							else if(Main_above1==FALSE && Aux_above1==FALSE)
							{
								if(pDM_FatTable->MainCRC32_Ok_Cnt==0)
									pDM_FatTable->MainCRC32_Ok_Cnt=1;
								if(pDM_FatTable->AuxCRC32_Ok_Cnt==0)
									pDM_FatTable->AuxCRC32_Ok_Cnt=1;
								
								Main_CRC_utility=((pDM_FatTable->MainCRC32_Fail_Cnt)<<7)/pDM_FatTable->MainCRC32_Ok_Cnt;
								Aux_CRC_utility=((pDM_FatTable->AuxCRC32_Fail_Cnt)<<7)/pDM_FatTable->AuxCRC32_Ok_Cnt;
								pDM_FatTable->TargetAnt_CRC32 = (Main_CRC_utility==Aux_CRC_utility)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_CRC_utility<=Aux_CRC_utility)?MAIN_ANT:AUX_ANT);	

								if(Main_CRC_utility!=0 && Aux_CRC_utility!=0)
								{
									if(Main_CRC_utility>=Aux_CRC_utility)
										utility_ratio=(Main_CRC_utility<<1)/(Aux_CRC_utility);
									else
										utility_ratio=(Aux_CRC_utility<<1)/(Main_CRC_utility);
								}
							}
						}
						ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 0);//NOT Accept CRC32 Error packets.

						//3 [EVM statistic]			
						Main_EVM = (pDM_FatTable->MainAntEVM_Cnt[i]!=0)?(pDM_FatTable->MainAntEVM_Sum[i]/pDM_FatTable->MainAntEVM_Cnt[i]):0;
						Aux_EVM = (pDM_FatTable->AuxAntEVM_Cnt[i]!=0)?(pDM_FatTable->AuxAntEVM_Sum[i]/pDM_FatTable->AuxAntEVM_Cnt[i]):0;
						pDM_FatTable->TargetAnt_EVM = (Main_EVM==Aux_EVM)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_EVM>=Aux_EVM)?MAIN_ANT:AUX_ANT);

						if((Main_EVM==0 || Aux_EVM==0))
							diff_EVM=0;
						else if(Main_EVM>=Aux_EVM)
							diff_EVM=Main_EVM-Aux_EVM;
						else
							diff_EVM=Aux_EVM-Main_EVM;

						//2 [ Decision state ]					
						if(pDM_FatTable->TargetAnt_EVM ==pDM_FatTable->TargetAnt_CRC32 )
						{
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision Type 1, CRC_utility = ((%d)), EVM_diff = ((%d))\n",utility_ratio, diff_EVM));
							
							if( (utility_ratio<2 && Force_antenna==FALSE)  && diff_EVM<=30)
								pDM_FatTable->TargetAnt_enhance=pDM_FatTable->pre_TargetAnt_enhance;
							else
								pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
						}
						else if((diff_EVM<=50 && (utility_ratio > 4 && Force_antenna==FALSE)) || (Force_antenna==TRUE)) 
						{
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision Type 2, CRC_utility = ((%d)), EVM_diff = ((%d))\n",utility_ratio, diff_EVM));
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
						}
						else if(diff_EVM>=100) // 
						{
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision Type 3, CRC_utility = ((%d)), EVM_diff = ((%d))\n",utility_ratio, diff_EVM));
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
						}
						else if(utility_ratio>=6 && Force_antenna==FALSE) // utility_ratio>3
						{
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision Type 4, CRC_utility = ((%d)), EVM_diff = ((%d))\n",utility_ratio, diff_EVM));
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
						}
						else
						{
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision Type 5, CRC_utility = ((%d)), EVM_diff = ((%d))\n",utility_ratio, diff_EVM));
						
							if(Force_antenna==TRUE)
								score_CRC=3;
							else if(utility_ratio>=3) //>0.5
								score_CRC=2;
							else if(utility_ratio>=2) //>1
						score_CRC=1;
					else
						score_CRC=0;
					
					if(diff_EVM>=100)
						score_EVM=2;
					else if(diff_EVM>=50)
						score_EVM=1;
					else
						score_EVM=0;

					if(score_CRC>score_EVM)
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
					else if(score_CRC<score_EVM)
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
					else
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->pre_TargetAnt_enhance;
				}
				pDM_FatTable->pre_TargetAnt_enhance=pDM_FatTable->TargetAnt_enhance;

						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : MainEVM_Cnt = (( %d ))  , Main_EVM= ((  %d )) \n",i, pDM_FatTable->MainAntEVM_Cnt[i], Main_EVM));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : AuxEVM_Cnt   = (( %d ))  , Aux_EVM = ((  %d )) \n" ,i, pDM_FatTable->AuxAntEVM_Cnt[i] , Aux_EVM));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** TargetAnt_EVM = (( %s ))\n", ( pDM_FatTable->TargetAnt_EVM  ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("M_CRC_Ok = (( %d ))  , M_CRC_Fail = ((  %d )), Main_CRC_utility = (( %d )) \n" , pDM_FatTable->MainCRC32_Ok_Cnt, pDM_FatTable->MainCRC32_Fail_Cnt,Main_CRC_utility));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("A_CRC_Ok  = (( %d ))  , A_CRC_Fail = ((  %d )), Aux_CRC_utility   = ((  %d )) \n" , pDM_FatTable->AuxCRC32_Ok_Cnt, pDM_FatTable->AuxCRC32_Fail_Cnt,Aux_CRC_utility));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** TargetAnt_CRC32 = (( %s ))\n", ( pDM_FatTable->TargetAnt_CRC32 ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("****** TargetAnt_enhance = (( %s ))******\n", ( pDM_FatTable->TargetAnt_enhance ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
				
					
					}
				}
				else // RSSI< = Evm_RSSI_TH_Low
				{ 
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ <TH_L: escape from > TH_L ] \n"));
					odm_EVM_FastAnt_Reset(pDM_Odm);
				}
			}
			else 
			{ 
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[escape from> TH_H || EVM_method_enable==1] \n"));
				odm_EVM_FastAnt_Reset(pDM_Odm);
			}
		}
		else
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[multi-Client] \n"));
			odm_EVM_FastAnt_Reset(pDM_Odm);
		}			
	}
}

VOID
odm_EVM_FastAntTrainingCallback(
	IN		PDM_ODM_T		pDM_Odm
)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_EVM_FastAntTrainingCallback****** \n"));
	odm_HW_AntDiv(pDM_Odm);
}
#endif

VOID
odm_HW_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
)
{
	u4Byte	i,MinMaxRSSI=0xFF, AntDivMaxRSSI=0, MaxRSSI=0,  LocalMaxRSSI;
	u4Byte	Main_RSSI, Aux_RSSI;
	u1Byte	RxIdleAnt=0, TargetAnt=7;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	PSTA_INFO_T   	pEntry;
	
	#ifdef BEAMFORMING_SUPPORT
	#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
	pBDC_T    pDM_BdcTable = &pDM_Odm->DM_BdcTable;
	u4Byte	TH1=500000;
	u4Byte	TH2=10000000; 
	u4Byte	MA_rx_Temp, degrade_TP_temp, improve_TP_temp;
	u1Byte	Monitor_RSSI_threshold=30;

	pDM_BdcTable->BF_pass=TRUE;
	pDM_BdcTable->DIV_pass=TRUE;
	pDM_BdcTable->bAll_DivSta_Idle=TRUE;
	pDM_BdcTable->bAll_BFSta_Idle=TRUE;
	pDM_BdcTable->num_BfTar=0 ;
	pDM_BdcTable->num_DivTar=0;
	pDM_BdcTable->num_Client=0;	
	#endif
	#endif

	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));

		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , REG);
			pDM_Odm->antdiv_period=0;

			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}	
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , TX_BY_DESC);
			
			//if(pDM_Odm->SupportICType == ODM_RTL8821 )
				//ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, 0); //CCK AntDiv function disable
				
			//#if(DM_ODM_SUPPORT_TYPE  == ODM_AP)
			//else if(pDM_Odm->SupportICType == ODM_RTL8881A)
			//	ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, 0); //CCK AntDiv function disable
			//#endif
			
			//else if(pDM_Odm->SupportICType == ODM_RTL8723B ||pDM_Odm->SupportICType == ODM_RTL8812)
				//ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, 0); //CCK AntDiv function disable
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;

			if(pDM_Odm->SupportICType==ODM_RTL8723B && pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
			{
				ODM_SetBBReg(pDM_Odm, 0x930 , 0xF0, 8); // DPDT_P = ANTSEL[0]   // for 8723B AntDiv function patch.  BB  Dino  130412	
				ODM_SetBBReg(pDM_Odm, 0x930 , 0xF, 8); // DPDT_N = ANTSEL[0]
			}
			
			//2 BDC Init
			#ifdef BEAMFORMING_SUPPORT
			#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
				odm_BDC_Init(pDM_Odm);
			#endif
			#endif
			
			#ifdef ODM_EVM_ENHANCE_ANTDIV
				odm_EVM_FastAnt_Reset(pDM_Odm);
			#endif
		}
	}
	
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n AntDiv Start =>\n"));

	#ifdef ODM_EVM_ENHANCE_ANTDIV
	if(pDM_Odm->antdiv_evm_en==1)
	{
		odm_EVM_Enhance_AntDiv(pDM_Odm);
		if(pDM_FatTable->FAT_State !=NORMAL_STATE_MIAN)
			return;
	}
	else
	{
		odm_EVM_FastAnt_Reset(pDM_Odm);
	}
	#endif
	
	//2 BDC Mode Arbitration
	#ifdef BEAMFORMING_SUPPORT
	#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
	if(pDM_Odm->antdiv_evm_en == 0 ||pDM_FatTable->EVM_method_enable==0)
	{
		odm_BF_AntDiv_ModeArbitration(pDM_Odm);
	}
	#endif
	#endif

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pEntry))
		{
			//2 Caculate RSSI per Antenna
			Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
			Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;
			TargetAnt = (Main_RSSI==Aux_RSSI)?pDM_FatTable->RxIdleAnt:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);

			//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** SupportICType=[%d] \n",pDM_Odm->SupportICType));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n",i, pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n" ,i, pDM_FatTable->AuxAnt_Cnt[i] , Aux_RSSI));
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , TargetAnt = (( %s )) \n", i ,( TargetAnt ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
			//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** Phy_AntSel_A=[ %d, %d, %d] \n",((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT2)>>2,
			//	                                                                              ((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT1) >>1, ((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT0)));

			LocalMaxRSSI = (Main_RSSI>Aux_RSSI)?Main_RSSI:Aux_RSSI;
			//2 Select MaxRSSI for DIG
			if((LocalMaxRSSI > AntDivMaxRSSI) && (LocalMaxRSSI < 40))
				AntDivMaxRSSI = LocalMaxRSSI;
			if(LocalMaxRSSI > MaxRSSI)
				MaxRSSI = LocalMaxRSSI;

			//2 Select RX Idle Antenna
			if ( (LocalMaxRSSI != 0) &&  (LocalMaxRSSI < MinMaxRSSI) )
			{
				RxIdleAnt = TargetAnt;
				MinMaxRSSI = LocalMaxRSSI;
			}
			
			#ifdef ODM_EVM_ENHANCE_ANTDIV
			if(pDM_Odm->antdiv_evm_en==1)
			{
				if(pDM_FatTable->TargetAnt_enhance!=0xFF)
				{
					TargetAnt=pDM_FatTable->TargetAnt_enhance;
					RxIdleAnt = pDM_FatTable->TargetAnt_enhance;
				}
			}
			#endif

			//2 Select TX Antenna
			if(pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV)
			{
				#ifdef BEAMFORMING_SUPPORT
				#if(DM_ODM_SUPPORT_TYPE==ODM_AP)
					if(pDM_BdcTable->w_BFee_Client[i]==0)
				#endif	
				#endif
					{
						odm_UpdateTxAnt(pDM_Odm, TargetAnt, i);
					}
			}

			//------------------------------------------------------------

			#ifdef BEAMFORMING_SUPPORT
			#if(DM_ODM_SUPPORT_TYPE  == ODM_AP) 

			pDM_BdcTable->num_Client++;

			if(pDM_BdcTable->BDC_Mode==BDC_MODE_2 ||pDM_BdcTable->BDC_Mode==BDC_MODE_3)
			{
				//2 Byte Counter

				MA_rx_Temp=  (pEntry->rx_byte_cnt_LowMAW)<<3 ; //  RX  TP   ( bit /sec)
				
				if(pDM_BdcTable->BDC_state==BDC_BFer_TRAIN_STATE)
				{
					pDM_BdcTable->MA_rx_TP_DIV[i]=  MA_rx_Temp ;
				}
				else
				{
					pDM_BdcTable->MA_rx_TP[i] =MA_rx_Temp ;
				}

				if( (MA_rx_Temp < TH2)   &&  (MA_rx_Temp > TH1) && (LocalMaxRSSI<=Monitor_RSSI_threshold))
				{
					if(pDM_BdcTable->w_BFer_Client[i]==1) // Bfer_Target
					{
						pDM_BdcTable->num_BfTar++;
						
						if(pDM_BdcTable->BDC_state==BDC_DECISION_STATE && pDM_BdcTable->BDC_Try_flag==0)
						{
						improve_TP_temp = (pDM_BdcTable->MA_rx_TP_DIV[i] * 9)>>3 ; //* 1.125
						pDM_BdcTable->BF_pass = (pDM_BdcTable->MA_rx_TP[i] > improve_TP_temp)?TRUE:FALSE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] :  { MA_rx_TP,improve_TP_temp , MA_rx_TP_DIV,  BF_pass}={ %d,  %d, %d , %d }  \n" ,i,pDM_BdcTable->MA_rx_TP[i],improve_TP_temp,pDM_BdcTable->MA_rx_TP_DIV[i], pDM_BdcTable->BF_pass ));
						}		
					}		
					else// DIV_Target
					{
						pDM_BdcTable->num_DivTar++;
						
						if(pDM_BdcTable->BDC_state==BDC_DECISION_STATE && pDM_BdcTable->BDC_Try_flag==0)
						{
						degrade_TP_temp=(pDM_BdcTable->MA_rx_TP_DIV[i]*5)>>3;//* 0.625
						pDM_BdcTable->DIV_pass = (pDM_BdcTable->MA_rx_TP[i] >degrade_TP_temp)?TRUE:FALSE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] :  { MA_rx_TP, degrade_TP_temp , MA_rx_TP_DIV,  DIV_pass}=\n{ %d,  %d, %d , %d }  \n" ,i,pDM_BdcTable->MA_rx_TP[i],degrade_TP_temp,pDM_BdcTable->MA_rx_TP_DIV[i], pDM_BdcTable->DIV_pass ));
						}							
					}
				}

				if(MA_rx_Temp > TH1)
				{
					if(pDM_BdcTable->w_BFer_Client[i]==1) // Bfer_Target
					{
						pDM_BdcTable->bAll_BFSta_Idle=FALSE;
					}		
					else// DIV_Target
					{
						pDM_BdcTable->bAll_DivSta_Idle=FALSE;
					}
				}
		
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] :  { BFmeeCap , BFmerCap}  = { %d , %d } \n" ,i, pDM_BdcTable->w_BFee_Client[i] , pDM_BdcTable->w_BFer_Client[i]));

				if(pDM_BdcTable->BDC_state==BDC_BFer_TRAIN_STATE)
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] :    MA_rx_TP_DIV = (( %d ))  \n",i,pDM_BdcTable->MA_rx_TP_DIV[i]  ));
					
				}
				else
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] :    MA_rx_TP = (( %d ))  \n",i,pDM_BdcTable->MA_rx_TP[i]  ));
				}
			
			}
			#endif
			#endif

		}

		#ifdef BEAMFORMING_SUPPORT
		#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
		if(pDM_BdcTable->BDC_Try_flag==0)
		#endif	
		#endif	
		{
			pDM_FatTable->MainAnt_Sum[i] = 0;
			pDM_FatTable->AuxAnt_Sum[i] = 0;
			pDM_FatTable->MainAnt_Cnt[i] = 0;
			pDM_FatTable->AuxAnt_Cnt[i] = 0;
		}		
	}


	
	//2 Set RX Idle Antenna & TX Antenna(Because of HW Bug )	
	#if(DM_ODM_SUPPORT_TYPE  == ODM_AP ) 
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** RxIdleAnt = (( %s ))\n\n", ( RxIdleAnt ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
		
		#ifdef BEAMFORMING_SUPPORT
		#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
			if(pDM_BdcTable->BDC_Mode==BDC_MODE_1 ||pDM_BdcTable->BDC_Mode==BDC_MODE_3)
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** BDC_RxIdleUpdate_counter = (( %d ))\n", pDM_BdcTable->BDC_RxIdleUpdate_counter));
			
				if(pDM_BdcTable->BDC_RxIdleUpdate_counter==1)
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***Update RxIdle Antenna!!! \n"));
					pDM_BdcTable->BDC_RxIdleUpdate_counter=30;
					ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);
				}
				else
				{
					pDM_BdcTable->BDC_RxIdleUpdate_counter--;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***NOT update RxIdle Antenna because of BF  ( need to fix TX-ant)\n"));
				}
			}
			else
		#endif	
		#endif	
				ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);
	#else
	
		ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);
	
	#endif//#if(DM_ODM_SUPPORT_TYPE  == ODM_AP)



	//2 BDC Main Algorithm
	#ifdef BEAMFORMING_SUPPORT
	#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
	if(pDM_Odm->antdiv_evm_en ==0 ||pDM_FatTable->EVM_method_enable==0)
	{
		odm_BDCcoex_BFeeRxDiv_Arbitration(pDM_Odm);
	}
	#endif
	#endif

	if(AntDivMaxRSSI == 0)
		pDM_DigTable->AntDiv_RSSI_max = pDM_Odm->RSSI_Min;
	else
		pDM_DigTable->AntDiv_RSSI_max = AntDivMaxRSSI;
	
	pDM_DigTable->RSSI_max = MaxRSSI;
}



#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
VOID
odm_S0S1_SwAntDiv(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Step
	)
{
	u4Byte			i,MinMaxRSSI=0xFF, LocalMaxRSSI,LocalMinRSSI;
	u4Byte			Main_RSSI, Aux_RSSI;
	u1Byte			reset_period=10, SWAntDiv_threshold=35;
	u1Byte			HighTraffic_TrainTime_U=0x32,HighTraffic_TrainTime_L=0,Train_time_temp;
	u1Byte			LowTraffic_TrainTime_U=200,LowTraffic_TrainTime_L=0;
	pSWAT_T			pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;	
	u1Byte			RxIdleAnt = pDM_SWAT_Table->PreAntenna, TargetAnt, nextAnt=0;
	PSTA_INFO_T		pEntry=NULL;
	//static u1Byte		reset_idx;
	u4Byte			value32;
	PADAPTER		Adapter	 =  pDM_Odm->Adapter;
	u8Byte			curTxOkCnt=0, curRxOkCnt=0,TxCntOffset, RxCntOffset;
	
	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set REG 948[9:6]=0x0 \n"));
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
			if(pDM_Odm->SupportICType == ODM_RTL8723B)
				ODM_SetBBReg(pDM_Odm, 0x948 , BIT9|BIT8|BIT7|BIT6, 0x0); 
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			
			if(pDM_Odm->SupportICType == ODM_RTL8723B)
			{
				value32 = ODM_GetBBReg(pDM_Odm, 0x864, BIT5|BIT4|BIT3);
				
				if (value32==0x0)
					ODM_UpdateRxIdleAnt_8723B(pDM_Odm, MAIN_ANT, ANT1_2G, ANT2_2G);
				else if (value32==0x1)
					ODM_UpdateRxIdleAnt_8723B(pDM_Odm, AUX_ANT, ANT2_2G, ANT1_2G);
				
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("8723B: First link! Force antenna to  %s\n",(value32 == 0x0?"MAIN":"AUX") ));
			}

			pDM_SWAT_Table->lastTxOkCnt = 0; 
			pDM_SWAT_Table->lastRxOkCnt =0; 
			TxCntOffset =  *(pDM_Odm->pNumTxBytesUnicast);
			RxCntOffset =  *(pDM_Odm->pNumRxBytesUnicast);
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		else
		{
			TxCntOffset = 0;
			RxCntOffset = 0;
		}
	}
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[%d] { try_flag=(( %d )), Step=(( %d )), Double_chk_flag = (( %d )) }\n",
		__LINE__,pDM_SWAT_Table->try_flag,Step,pDM_SWAT_Table->Double_chk_flag));

	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if(	Step != pDM_SWAT_Table->try_flag	)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Step != try_flag]    Need to Reset After Link\n"));
		ODM_SwAntDivRestAfterLink(pDM_Odm);
	}

	if(pDM_SWAT_Table->try_flag == 0xff) 
	{	
		pDM_SWAT_Table->try_flag = 0;
		pDM_SWAT_Table->Train_time_flag=0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[set try_flag = 0]  Prepare for peak!\n\n"));
		return;
	}	
	else//if( try_flag != 0xff ) 
	{
		//1 Normal State (Begin Trying)
		if(pDM_SWAT_Table->try_flag == 0) 
		{
		
			//---trafic decision---
			curTxOkCnt =  *(pDM_Odm->pNumTxBytesUnicast) - pDM_SWAT_Table->lastTxOkCnt - TxCntOffset;
			curRxOkCnt =  *(pDM_Odm->pNumRxBytesUnicast) - pDM_SWAT_Table->lastRxOkCnt - RxCntOffset;
			pDM_SWAT_Table->lastTxOkCnt =  *(pDM_Odm->pNumTxBytesUnicast);
			pDM_SWAT_Table->lastRxOkCnt =  *(pDM_Odm->pNumRxBytesUnicast);
			
			if (curTxOkCnt > 1875000 || curRxOkCnt > 1875000)//if(PlatformDivision64(curTxOkCnt+curRxOkCnt, 2) > 1875000)  ( 1.875M * 8bit ) / 2= 7.5M bits /sec )
			{
				pDM_SWAT_Table->TrafficLoad = TRAFFIC_HIGH;
				Train_time_temp=pDM_SWAT_Table->Train_time ;
				
				if(pDM_SWAT_Table->Train_time_flag==3)
				{
					HighTraffic_TrainTime_L=0xa;
					
					if(Train_time_temp<=16)
						Train_time_temp=HighTraffic_TrainTime_L;
					else
						Train_time_temp-=16;
					
				}				
				else if(pDM_SWAT_Table->Train_time_flag==2)
				{
					Train_time_temp-=8;
					HighTraffic_TrainTime_L=0xf;
				}	
				else if(pDM_SWAT_Table->Train_time_flag==1)
				{
					Train_time_temp-=4;
					HighTraffic_TrainTime_L=0x1e;
				}
				else if(pDM_SWAT_Table->Train_time_flag==0)
				{
					Train_time_temp+=8;
					HighTraffic_TrainTime_L=0x28;
				}

				
				//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Train_time_temp = ((%d))\n",Train_time_temp));

				//--
				if(Train_time_temp > HighTraffic_TrainTime_U)
					Train_time_temp=HighTraffic_TrainTime_U;
				
				else if(Train_time_temp < HighTraffic_TrainTime_L)
					Train_time_temp=HighTraffic_TrainTime_L;

				pDM_SWAT_Table->Train_time = Train_time_temp; //50ms~10ms
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("  Train_time_flag=((%d)) , Train_time=((%d)) \n",pDM_SWAT_Table->Train_time_flag, pDM_SWAT_Table->Train_time));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("  [HIGH Traffic]  \n" ));
			}
			else if (curTxOkCnt > 125000 || curRxOkCnt > 125000) // ( 0.125M * 8bit ) / 2 =  0.5M bits /sec )
			{
				pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
				Train_time_temp=pDM_SWAT_Table->Train_time ;
				
				if(pDM_SWAT_Table->Train_time_flag==3)
				{
					LowTraffic_TrainTime_L=10;
					if(Train_time_temp<50)
						Train_time_temp=LowTraffic_TrainTime_L;
					else
						Train_time_temp-=50;
				}				
				else if(pDM_SWAT_Table->Train_time_flag==2)
				{
					Train_time_temp-=30;
					LowTraffic_TrainTime_L=36;
				}	
				else if(pDM_SWAT_Table->Train_time_flag==1)
				{
					Train_time_temp-=10;
					LowTraffic_TrainTime_L=40;
				}
				else
					Train_time_temp+=10;	
				
				//--
				if(Train_time_temp >= LowTraffic_TrainTime_U)
					Train_time_temp=LowTraffic_TrainTime_U;
				
				else if(Train_time_temp <= LowTraffic_TrainTime_L)
					Train_time_temp=LowTraffic_TrainTime_L;

				pDM_SWAT_Table->Train_time = Train_time_temp; //50ms~20ms
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("  Train_time_flag=((%d)) , Train_time=((%d)) \n",pDM_SWAT_Table->Train_time_flag, pDM_SWAT_Table->Train_time));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("  [Low Traffic]  \n" ));
			}
			else
			{
				pDM_SWAT_Table->TrafficLoad = TRAFFIC_UltraLOW;
				pDM_SWAT_Table->Train_time = 0xc8; //200ms
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("  [Ultra-Low Traffic]  \n" ));
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("TxOkCnt=(( %llu )), RxOkCnt=(( %llu )) \n", 
				curTxOkCnt ,curRxOkCnt ));
				
			//-----------------
		
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,(" Current MinMaxRSSI is ((%d)) \n",pDM_FatTable->MinMaxRSSI));

                        //---reset index---
			if(pDM_SWAT_Table->reset_idx>=reset_period)
			{
				pDM_FatTable->MinMaxRSSI=0; //
				pDM_SWAT_Table->reset_idx=0;
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reset_idx = (( %d )) \n",pDM_SWAT_Table->reset_idx ));
			//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("reset_idx=%d\n",pDM_SWAT_Table->reset_idx));
			pDM_SWAT_Table->reset_idx++;

			//---double check flag---
			if(pDM_FatTable->MinMaxRSSI > SWAntDiv_threshold && pDM_SWAT_Table->Double_chk_flag== 0)
			{			
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,(" MinMaxRSSI is ((%d)), and > %d \n",
					pDM_FatTable->MinMaxRSSI,SWAntDiv_threshold));

				pDM_SWAT_Table->Double_chk_flag =1;
				pDM_SWAT_Table->try_flag = 1; 
			        pDM_SWAT_Table->RSSI_Trying = 0;

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Test the current Ant for (( %d )) ms again \n", pDM_SWAT_Table->Train_time));
				ODM_UpdateRxIdleAnt(pDM_Odm, pDM_FatTable->RxIdleAnt);	
				ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer_8723B, pDM_SWAT_Table->Train_time ); //ms	
				return;
			}
			
			nextAnt = (pDM_FatTable->RxIdleAnt == MAIN_ANT)? AUX_ANT : MAIN_ANT;

			pDM_SWAT_Table->try_flag = 1;
			
			if(pDM_SWAT_Table->reset_idx<=1)
				pDM_SWAT_Table->RSSI_Trying = 2;
			else
				pDM_SWAT_Table->RSSI_Trying = 1;
			
			odm_S0S1_SwAntDivByCtrlFrame(pDM_Odm, SWAW_STEP_PEAK);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[set try_flag=1]  Normal State:  Begin Trying!! \n"));
		}
	
		else if(pDM_SWAT_Table->try_flag == 1 && pDM_SWAT_Table->Double_chk_flag== 0)
		{	
			nextAnt = (pDM_FatTable->RxIdleAnt  == MAIN_ANT)? AUX_ANT : MAIN_ANT;		
			pDM_SWAT_Table->RSSI_Trying--;
		}
		
		//1 Decision State
		if((pDM_SWAT_Table->try_flag == 1)&&(pDM_SWAT_Table->RSSI_Trying == 0) )
		{
			BOOLEAN bByCtrlFrame = FALSE;
			u8Byte	pkt_cnt_total = 0;
			
			for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
			{
				pEntry = pDM_Odm->pODM_StaInfo[i];
				if(IS_STA_VALID(pEntry))
				{
					//2 Caculate RSSI per Antenna
					Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
					Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;
					
					if(pDM_FatTable->MainAnt_Cnt[i]<=1 && pDM_FatTable->CCK_counter_main>=1)
						Main_RSSI=0;	
					
					if(pDM_FatTable->AuxAnt_Cnt[i]<=1 && pDM_FatTable->CCK_counter_aux>=1)
						Aux_RSSI=0;

					TargetAnt = (Main_RSSI==Aux_RSSI)?pDM_SWAT_Table->PreAntenna:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);
					LocalMaxRSSI = (Main_RSSI>=Aux_RSSI) ? Main_RSSI : Aux_RSSI;
					LocalMinRSSI = (Main_RSSI>=Aux_RSSI) ? Aux_RSSI : Main_RSSI;
					
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  CCK_counter_main = (( %d ))  , CCK_counter_aux= ((  %d )) \n", pDM_FatTable->CCK_counter_main, pDM_FatTable->CCK_counter_aux));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  OFDM_counter_main = (( %d ))  , OFDM_counter_aux= ((  %d )) \n", pDM_FatTable->OFDM_counter_main, pDM_FatTable->OFDM_counter_aux));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n", pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n", pDM_FatTable->AuxAnt_Cnt[i]  , Aux_RSSI ));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , TargetAnt = (( %s )) \n", i ,( TargetAnt ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
					
					//2 Select RX Idle Antenna
					
					if (LocalMaxRSSI != 0 && LocalMaxRSSI < MinMaxRSSI)
					{
							RxIdleAnt = TargetAnt;
							MinMaxRSSI = LocalMaxRSSI;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** LocalMaxRSSI-LocalMinRSSI = ((%d))\n",(LocalMaxRSSI-LocalMinRSSI)));
					
							if((LocalMaxRSSI-LocalMinRSSI)>8)
							{
								if(LocalMinRSSI != 0)
									pDM_SWAT_Table->Train_time_flag=3;
								else
								{
									if(MinMaxRSSI > SWAntDiv_threshold)
										pDM_SWAT_Table->Train_time_flag=0;
									else
										pDM_SWAT_Table->Train_time_flag=3;
								}
							}
							else if((LocalMaxRSSI-LocalMinRSSI)>5)
								pDM_SWAT_Table->Train_time_flag=2;
							else if((LocalMaxRSSI-LocalMinRSSI)>2)
								pDM_SWAT_Table->Train_time_flag=1;
							else
								pDM_SWAT_Table->Train_time_flag=0;
							
					}
					
					//2 Select TX Antenna
					if(TargetAnt == MAIN_ANT)
						pDM_FatTable->antsel_a[i] =ANT1_2G;
					else
						pDM_FatTable->antsel_a[i] = ANT2_2G;
					
				}
					pDM_FatTable->MainAnt_Sum[i] = 0;
					pDM_FatTable->AuxAnt_Sum[i] = 0;
					pDM_FatTable->MainAnt_Cnt[i] = 0;
					pDM_FatTable->AuxAnt_Cnt[i] = 0;
			}

			if(pDM_SWAT_Table->bSWAntDivByCtrlFrame)
			{
				odm_S0S1_SwAntDivByCtrlFrame(pDM_Odm, SWAW_STEP_DETERMINE);
				bByCtrlFrame = TRUE;
			}

			pkt_cnt_total = pDM_FatTable->CCK_counter_main + pDM_FatTable->CCK_counter_aux + 
			pDM_FatTable->OFDM_counter_main + pDM_FatTable->OFDM_counter_aux;
					pDM_FatTable->CCK_counter_main=0;
					pDM_FatTable->CCK_counter_aux=0;
					pDM_FatTable->OFDM_counter_main=0;
					pDM_FatTable->OFDM_counter_aux=0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Control frame packet counter = %d, Data frame packet counter = %llu\n", 
				pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame, pkt_cnt_total));
			
			if(MinMaxRSSI == 0xff || ((pkt_cnt_total < (pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame >> 1)) && pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 2))
			{	
				MinMaxRSSI = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Check RSSI of control frame because MinMaxRSSI == 0xff\n"));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("bByCtrlFrame = %d\n", bByCtrlFrame));
				
				if(bByCtrlFrame)
				{
					Main_RSSI = (pDM_FatTable->MainAnt_CtrlFrame_Cnt!=0)?(pDM_FatTable->MainAnt_CtrlFrame_Sum/pDM_FatTable->MainAnt_CtrlFrame_Cnt):0;
					Aux_RSSI = (pDM_FatTable->AuxAnt_CtrlFrame_Cnt!=0)?(pDM_FatTable->AuxAnt_CtrlFrame_Sum/pDM_FatTable->AuxAnt_CtrlFrame_Cnt):0;
					
					if(pDM_FatTable->MainAnt_CtrlFrame_Cnt<=1 && pDM_FatTable->CCK_CtrlFrame_Cnt_main>=1)
						Main_RSSI=0;	
					
					if(pDM_FatTable->AuxAnt_CtrlFrame_Cnt<=1 && pDM_FatTable->CCK_CtrlFrame_Cnt_aux>=1)
						Aux_RSSI=0;

					if (Main_RSSI != 0 || Aux_RSSI != 0)
					{
						RxIdleAnt = (Main_RSSI==Aux_RSSI)?pDM_SWAT_Table->PreAntenna:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);
						LocalMaxRSSI = (Main_RSSI>=Aux_RSSI) ? Main_RSSI : Aux_RSSI;
						LocalMinRSSI = (Main_RSSI>=Aux_RSSI) ? Aux_RSSI : Main_RSSI;

						if((LocalMaxRSSI-LocalMinRSSI)>8)
							pDM_SWAT_Table->Train_time_flag=3;
						else if((LocalMaxRSSI-LocalMinRSSI)>5)
							pDM_SWAT_Table->Train_time_flag=2;
						else if((LocalMaxRSSI-LocalMinRSSI)>2)
							pDM_SWAT_Table->Train_time_flag=1;
						else
							pDM_SWAT_Table->Train_time_flag=0;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Control frame: Main_RSSI = %d, Aux_RSSI = %d\n", Main_RSSI, Aux_RSSI));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("RxIdleAnt decided by control frame = %s\n", (RxIdleAnt == MAIN_ANT?"MAIN":"AUX")));
					}
				}
			}
		
			pDM_FatTable->MinMaxRSSI=MinMaxRSSI;
			pDM_SWAT_Table->try_flag = 0;
						
			if( pDM_SWAT_Table->Double_chk_flag==1)
			{
				pDM_SWAT_Table->Double_chk_flag=0;
				if(pDM_FatTable->MinMaxRSSI > SWAntDiv_threshold)
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,(" [Double check] MinMaxRSSI ((%d)) > %d again!! \n",
						pDM_FatTable->MinMaxRSSI,SWAntDiv_threshold));
					
					ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);	
					
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[reset try_flag = 0] Training accomplished !!!] \n\n\n"));
					return;
				}
				else
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,(" [Double check] MinMaxRSSI ((%d)) <= %d !! \n",
						pDM_FatTable->MinMaxRSSI,SWAntDiv_threshold));

					nextAnt = (pDM_FatTable->RxIdleAnt  == MAIN_ANT)? AUX_ANT : MAIN_ANT;
					pDM_SWAT_Table->try_flag = 0; 
					pDM_SWAT_Table->reset_idx=reset_period;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[set try_flag=0]  Normal State:  Need to tryg again!! \n\n\n"));
					return;
				}
			}
			else
			{
				if(pDM_FatTable->MinMaxRSSI < SWAntDiv_threshold)
					pDM_SWAT_Table->reset_idx=reset_period;
				
				pDM_SWAT_Table->PreAntenna =RxIdleAnt;
				ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt );
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[reset try_flag = 0] Training accomplished !!!] \n\n\n"));
			        return;
			}
			
		}

	}

	//1 4.Change TRX antenna

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("RSSI_Trying = (( %d )),    Ant: (( %s )) >>> (( %s )) \n",
		pDM_SWAT_Table->RSSI_Trying, (pDM_FatTable->RxIdleAnt  == MAIN_ANT?"MAIN":"AUX"),(nextAnt == MAIN_ANT?"MAIN":"AUX")));
		
	ODM_UpdateRxIdleAnt(pDM_Odm, nextAnt);

	//1 5.Reset Statistics

	pDM_FatTable->RxIdleAnt  = nextAnt;

	//1 6.Set next timer   (Trying State)
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Test ((%s)) Ant for (( %d )) ms \n", (nextAnt == MAIN_ANT?"MAIN":"AUX"), pDM_SWAT_Table->Train_time));
	ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer_8723B, pDM_SWAT_Table->Train_time ); //ms
}


#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
VOID
ODM_SW_AntDiv_Callback(
	PRT_TIMER		pTimer
)
{
	PADAPTER		Adapter = (PADAPTER)pTimer->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T			pDM_SWAT_Table = &pHalData->DM_OutSrc.DM_SWAT_Table;

	#if DEV_BUS_TYPE==RT_PCI_INTERFACE
		#if USE_WORKITEM
			ODM_ScheduleWorkItem(&pDM_SWAT_Table->SwAntennaSwitchWorkitem_8723B);
		#else
			{
			//DbgPrint("SW_antdiv_Callback");
			odm_S0S1_SwAntDiv(&pHalData->DM_OutSrc, SWAW_STEP_DETERMINE);
			}
		#endif
	#else
	ODM_ScheduleWorkItem(&pDM_SWAT_Table->SwAntennaSwitchWorkitem_8723B);
	#endif
}
VOID
ODM_SW_AntDiv_WorkitemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER		pAdapter = (PADAPTER)pContext;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	
	//DbgPrint("SW_antdiv_Workitem_Callback");
	odm_S0S1_SwAntDiv(&pHalData->DM_OutSrc, SWAW_STEP_DETERMINE);
}

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)

VOID
ODM_SW_AntDiv_Callback(void *FunctionContext)
{
	PDM_ODM_T	pDM_Odm= (PDM_ODM_T)FunctionContext;
	PADAPTER	padapter = pDM_Odm->Adapter;
	if(padapter->net_closed == _TRUE)
	    return;
	odm_S0S1_SwAntDiv(pDM_Odm, SWAW_STEP_DETERMINE);
}


#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
VOID
odm_S0S1_SwAntDivByCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Step
	)
{
	pSWAT_T	pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	switch(Step)
	{
		case SWAW_STEP_PEAK:
			pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame = 0;
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = TRUE;
			pDM_FatTable->MainAnt_CtrlFrame_Cnt = 0;
			pDM_FatTable->AuxAnt_CtrlFrame_Cnt = 0;
			pDM_FatTable->MainAnt_CtrlFrame_Sum = 0;
			pDM_FatTable->AuxAnt_CtrlFrame_Sum = 0;
			pDM_FatTable->CCK_CtrlFrame_Cnt_main = 0;
			pDM_FatTable->CCK_CtrlFrame_Cnt_aux = 0;
			pDM_FatTable->OFDM_CtrlFrame_Cnt_main = 0;
			pDM_FatTable->OFDM_CtrlFrame_Cnt_aux = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("odm_S0S1_SwAntDivForAPMode(): Start peak and reset counter\n"));
			break;
		case SWAW_STEP_DETERMINE:
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = FALSE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("odm_S0S1_SwAntDivForAPMode(): Stop peak\n"));
			break;
		default:
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = FALSE;
			break;
	}			
}

VOID
odm_AntselStatisticsOfCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			RxPWDBAll
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	if(antsel_tr_mux == ANT1_2G)
	{
		pDM_FatTable->MainAnt_CtrlFrame_Sum+=RxPWDBAll;
		pDM_FatTable->MainAnt_CtrlFrame_Cnt++;
	}
	else
	{
		pDM_FatTable->AuxAnt_CtrlFrame_Sum+=RxPWDBAll;
		pDM_FatTable->AuxAnt_CtrlFrame_Cnt++;
	}
}

VOID
odm_S0S1_SwAntDivByCtrlFrame_ProcessRSSI(
	IN		PDM_ODM_T				pDM_Odm,
	IN		PODM_PHY_INFO_T		pPhyInfo,
	IN		PODM_PACKET_INFO_T		pPktinfo
	)
{
	pSWAT_T	pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	BOOLEAN		isCCKrate;

	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if(pDM_Odm->AntDivType != S0S1_SW_ANTDIV)
		return;

	// In try state
	if(!pDM_SWAT_Table->bSWAntDivByCtrlFrame)
		return;

	// No HW error and match receiver address
	if(!pPktinfo->bToSelf)
		return;
	
	pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame++;
	isCCKrate = ((pPktinfo->DataRate >= DESC_RATE1M ) && (pPktinfo->DataRate <= DESC_RATE11M ))?TRUE :FALSE;

	if(isCCKrate)
	{
	 	pDM_FatTable->antsel_rx_keep_0 = (pDM_FatTable->RxIdleAnt == MAIN_ANT) ? ANT1_2G : ANT2_2G;

		if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
			pDM_FatTable->CCK_CtrlFrame_Cnt_main++;
		else
			pDM_FatTable->CCK_CtrlFrame_Cnt_aux++;

		odm_AntselStatisticsOfCtrlFrame(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A]);
	}
	else
	{
		if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
			pDM_FatTable->OFDM_CtrlFrame_Cnt_main++;
		else
			pDM_FatTable->OFDM_CtrlFrame_Cnt_aux++;

		odm_AntselStatisticsOfCtrlFrame(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPhyInfo->RxPWDBAll);
	}
}
#endif  //#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)


#endif //#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)


#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )

VOID
odm_SetNextMACAddrTarget(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	PSTA_INFO_T   	pEntry;
	//u1Byte	Bssid[6];
	u4Byte	value32, i;

	//
	//2012.03.26 LukeLee: The MAC address is changed according to MACID in turn
	//
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_SetNextMACAddrTarget() ==>\n"));
	if(pDM_Odm->bLinked)
	{
		for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
		{
			if((pDM_FatTable->TrainIdx+1) == ODM_ASSOCIATE_ENTRY_NUM)
				pDM_FatTable->TrainIdx = 0;
			else
				pDM_FatTable->TrainIdx++;
			
			pEntry = pDM_Odm->pODM_StaInfo[pDM_FatTable->TrainIdx];
			if(IS_STA_VALID(pEntry))
			{
				//Match MAC ADDR
				#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
					value32 = (pEntry->hwaddr[5]<<8)|pEntry->hwaddr[4];
				#else
					value32 = (pEntry->MacAddr[5]<<8)|pEntry->MacAddr[4];
				#endif
				
				ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, value32);//0x7b4~0x7b5
				
				#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
					value32 = (pEntry->hwaddr[3]<<24)|(pEntry->hwaddr[2]<<16) |(pEntry->hwaddr[1]<<8) |pEntry->hwaddr[0];
				#else
					value32 = (pEntry->MacAddr[3]<<24)|(pEntry->MacAddr[2]<<16) |(pEntry->MacAddr[1]<<8) |pEntry->MacAddr[0];
				#endif
				ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, value32); //0x7b0~0x7b3

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pDM_FatTable->TrainIdx=%d\n",pDM_FatTable->TrainIdx));
				
				#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC Addr = %x:%x:%x:%x:%x:%x\n",
						pEntry->hwaddr[5],pEntry->hwaddr[4],pEntry->hwaddr[3],pEntry->hwaddr[2],pEntry->hwaddr[1],pEntry->hwaddr[0]));
				#else
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC Addr = %x:%x:%x:%x:%x:%x\n",
						pEntry->MacAddr[5],pEntry->MacAddr[4],pEntry->MacAddr[3],pEntry->MacAddr[2],pEntry->MacAddr[1],pEntry->MacAddr[0]));
				#endif

				break;
			}
		}
		
	}

#if 0
	//
	//2012.03.26 LukeLee: This should be removed later, the MAC address is changed according to MACID in turn
	//
	#if( DM_ODM_SUPPORT_TYPE & ODM_WIN)
	{		
		PADAPTER	Adapter =  pDM_Odm->Adapter;
		PMGNT_INFO	pMgntInfo = &Adapter->MgntInfo;

		for (i=0; i<6; i++)
		{
			Bssid[i] = pMgntInfo->Bssid[i];
			//DbgPrint("Bssid[%d]=%x\n", i, Bssid[i]);
		}
	}
	#endif

	//odm_SetNextMACAddrTarget(pDM_Odm);
	
	//1 Select MAC Address Filter
	for (i=0; i<6; i++)
	{
		if(Bssid[i] != pDM_FatTable->Bssid[i])
		{
			bMatchBSSID = FALSE;
			break;
		}
	}
	if(bMatchBSSID == FALSE)
	{
		//Match MAC ADDR
		value32 = (Bssid[5]<<8)|Bssid[4];
		ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, value32);
		value32 = (Bssid[3]<<24)|(Bssid[2]<<16) |(Bssid[1]<<8) |Bssid[0];
		ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, value32);
	}

	return bMatchBSSID;
#endif
				
}

VOID
odm_FastAntTraining(
	IN		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	u4Byte	MaxRSSI_pathA=0, Pckcnt_pathA=0;
	u1Byte	i,TargetAnt_pathA=0;
	BOOLEAN	bPktFilterMacth_pathA = FALSE;
	#if(RTL8192E_SUPPORT == 1)
	u4Byte	MaxRSSI_pathB=0, Pckcnt_pathB=0;
	u1Byte	TargetAnt_pathB=0;
	BOOLEAN	bPktFilterMacth_pathB = FALSE;
	#endif


	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		
		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			odm_FastTraining_enable(pDM_Odm , FAT_OFF);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , REG);
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked!!!]\n"));
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , TX_BY_DESC);
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
	}

		
        if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
           ODM_SetBBReg(pDM_Odm, 0x864 , BIT2|BIT1|BIT0, ((pDM_Odm->fat_comb_a)-1));
        }
	#if(RTL8192E_SUPPORT == 1)
        else if(pDM_Odm->SupportICType == ODM_RTL8192E)
        {
           ODM_SetBBReg(pDM_Odm, 0xB38 , BIT2|BIT1|BIT0, ((pDM_Odm->fat_comb_a)-1) );	   //path-A  // ant combination=regB38[2:0]+1
	   ODM_SetBBReg(pDM_Odm, 0xB38 , BIT18|BIT17|BIT16, ((pDM_Odm->fat_comb_b)-1) );  //path-B  // ant combination=regB38[18:16]+1
        }
	#endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("==>odm_FastAntTraining()\n"));
	
	//1 TRAINING STATE
	if(pDM_FatTable->FAT_State == FAT_TRAINING_STATE)
	{
		//2 Caculate RSSI per Antenna

                //3 [path-A]---------------------------
		for (i=0; i<(pDM_Odm->fat_comb_a); i++) // i : antenna index
		{
			if(pDM_FatTable->antRSSIcnt[i] == 0)
				pDM_FatTable->antAveRSSI[i] = 0;
			else
			{
				pDM_FatTable->antAveRSSI[i] = pDM_FatTable->antSumRSSI[i] /pDM_FatTable->antRSSIcnt[i];
				bPktFilterMacth_pathA = TRUE;
			}
			
			if(pDM_FatTable->antAveRSSI[i] > MaxRSSI_pathA)
			{
				MaxRSSI_pathA = pDM_FatTable->antAveRSSI[i];
                                Pckcnt_pathA = pDM_FatTable ->antRSSIcnt[i];
				TargetAnt_pathA =  i ; 
			}
                        else if(pDM_FatTable->antAveRSSI[i] == MaxRSSI_pathA)
			{
				if(  (pDM_FatTable->antRSSIcnt[i] )   >   Pckcnt_pathA)
				{
					MaxRSSI_pathA = pDM_FatTable->antAveRSSI[i];
					Pckcnt_pathA = pDM_FatTable ->antRSSIcnt[i];
				        TargetAnt_pathA = i ;
			        }
			}
			
			ODM_RT_TRACE("*** Ant-Index : [ %d ],      Counter = (( %d )),     Avg RSSI = (( %d )) \n", i, pDM_FatTable->antRSSIcnt[i],  pDM_FatTable->antAveRSSI[i] );
		}


		/*
		#if(RTL8192E_SUPPORT == 1)
		//3 [path-B]---------------------------
		for (i=0; i<(pDM_Odm->fat_comb_b); i++)
		{
			if(pDM_FatTable->antRSSIcnt_pathB[i] == 0)
				pDM_FatTable->antAveRSSI_pathB[i] = 0;				
			else // (antRSSIcnt[i] != 0)
			{
				pDM_FatTable->antAveRSSI_pathB[i] = pDM_FatTable->antSumRSSI_pathB[i] /pDM_FatTable->antRSSIcnt_pathB[i];
				bPktFilterMacth_pathB = TRUE;
			}
			if(pDM_FatTable->antAveRSSI_pathB[i] > MaxRSSI_pathB)
			{
				MaxRSSI_pathB = pDM_FatTable->antAveRSSI_pathB[i];
                                Pckcnt_pathB = pDM_FatTable ->antRSSIcnt_pathB[i];
				TargetAnt_pathB = (u1Byte) i; 
			}
                        if(pDM_FatTable->antAveRSSI_pathB[i] == MaxRSSI_pathB)
			{
				if(pDM_FatTable ->antRSSIcnt_pathB > Pckcnt_pathB)
				{
					MaxRSSI_pathB = pDM_FatTable->antAveRSSI_pathB[i];
					TargetAnt_pathB = (u1Byte) i;
				} 
			}
			if (pDM_Odm->fat_print_rssi==1)
			{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***{Path-B}: Sum RSSI[%d] = (( %d )),      cnt RSSI [%d] = (( %d )),     Avg RSSI[%d] = (( %d )) \n",
				i, pDM_FatTable->antSumRSSI_pathB[i], i, pDM_FatTable->antRSSIcnt_pathB[i], i, pDM_FatTable->antAveRSSI_pathB[i]));
			}
		}
		#endif
		*/

	//1 DECISION STATE

		//2 Select TRX Antenna

	        odm_FastTraining_enable(pDM_Odm , FAT_OFF);

		//3 [path-A]---------------------------
		if(bPktFilterMacth_pathA  == FALSE)
		{
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{Path-A}: None Packet is matched\n"));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{Path-A}: None Packet is matched\n"));
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		}
		else
		{
			ODM_RT_TRACE("TargetAnt_pathA = (( %d )) , MaxRSSI_pathA = (( %d )) \n",TargetAnt_pathA,MaxRSSI_pathA);
			
			//3 [ update RX-optional ant ]        Default RX is Omni, Optional RX is the best decision by FAT
			if(pDM_Odm->SupportICType == ODM_RTL8188E)
			{
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, TargetAnt_pathA);	
			}
			else if(pDM_Odm->SupportICType == ODM_RTL8192E)
			{
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT8|BIT7|BIT6, TargetAnt_pathA);//Optional RX [pth-A]
			}
			//3 [ update TX ant ]
			odm_UpdateTxAnt(pDM_Odm, TargetAnt_pathA, (pDM_FatTable->TrainIdx)); 

			if(TargetAnt_pathA == 0)
				odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		}
		/*
		#if(RTL8192E_SUPPORT == 1)
		//3 [path-B]---------------------------
		if(bPktFilterMacth_pathB == FALSE)
		{
			if (pDM_Odm->fat_print_rssi==1)
			{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***[%d]{Path-B}: None Packet is matched\n\n\n",__LINE__));
			}
		}
		else
		{
			if (pDM_Odm->fat_print_rssi==1)
			{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, 
				(" ***TargetAnt_pathB = (( %d )) *** MaxRSSI = (( %d ))***\n\n\n",TargetAnt_pathB,MaxRSSI_pathB));
			}
			ODM_SetBBReg(pDM_Odm, 0xB38 , BIT21|BIT20|BIT19, TargetAnt_pathB);	//Default RX is Omni, Optional RX is the best decision by FAT		
			ODM_SetBBReg(pDM_Odm, 0x80c , BIT21, 1); //Reg80c[21]=1'b1		//from TX Info

			pDM_FatTable->antsel_pathB[pDM_FatTable->TrainIdx] = TargetAnt_pathB;
		}
		#endif
		*/
		
		//2 Reset Counter
		for(i=0; i<(pDM_Odm->fat_comb_a); i++)
		{
			pDM_FatTable->antSumRSSI[i] = 0;
			pDM_FatTable->antRSSIcnt[i] = 0;
		}
		/*
		#if(RTL8192E_SUPPORT == 1)
		for(i=0; i<=(pDM_Odm->fat_comb_b); i++)
		{
			pDM_FatTable->antSumRSSI_pathB[i] = 0;
			pDM_FatTable->antRSSIcnt_pathB[i] = 0;
		}
		#endif
		*/
		
		pDM_FatTable->FAT_State = FAT_NORMAL_STATE;
		return;
	}

	//1 NORMAL STATE
	if(pDM_FatTable->FAT_State == FAT_NORMAL_STATE)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Start  Normal State]\n"));
		
		odm_SetNextMACAddrTarget(pDM_Odm);

		//2 Prepare Training
		pDM_FatTable->FAT_State = FAT_TRAINING_STATE;
		odm_FastTraining_enable(pDM_Odm , FAT_ON);
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);		//enable HW AntDiv
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Start Training State]\n"));

		ODM_SetTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //ms
	}
		
}

VOID
odm_FastAntTrainingCallback(
	IN		PDM_ODM_T		pDM_Odm
)
{

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PADAPTER	padapter = pDM_Odm->Adapter;
	if(padapter->net_closed == _TRUE)
	    return;
	//if(*pDM_Odm->pbNet_closed == TRUE)
	   // return;
#endif

#if USE_WORKITEM
	ODM_ScheduleWorkItem(&pDM_Odm->FastAntTrainingWorkitem);
#else
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_FastAntTrainingCallback****** \n"));
	odm_FastAntTraining(pDM_Odm);
#endif
}

VOID
odm_FastAntTrainingWorkItemCallback(
	IN		PDM_ODM_T		pDM_Odm
)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_FastAntTrainingWorkItemCallback****** \n"));
	odm_FastAntTraining(pDM_Odm);
}

#endif

VOID
ODM_AntDivInit(
	IN PDM_ODM_T	pDM_Odm 
	)
{
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	pSWAT_T			pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;


	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}
        //---
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	if(pDM_FatTable->AntDiv_2G_5G == ODM_ANTDIV_2G)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[2G AntDiv Init]: Only Support 2G Antenna Diversity Function\n"));
		if(!(pDM_Odm->SupportICType & ODM_ANTDIV_2G_SUPPORT_IC))
			return;
	}
	else 	if(pDM_FatTable->AntDiv_2G_5G == ODM_ANTDIV_5G)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[5G AntDiv Init]: Only Support 5G Antenna Diversity Function\n"));
		if(!(pDM_Odm->SupportICType & ODM_ANTDIV_5G_SUPPORT_IC))
			return;
	}
	else 	if(pDM_FatTable->AntDiv_2G_5G == (ODM_ANTDIV_2G|ODM_ANTDIV_5G))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[2G & 5G AntDiv Init]:Support Both 2G & 5G Antenna Diversity Function\n"));
	}

#endif	
	//---
	
	//2 [--General---]
	pDM_Odm->antdiv_period=0;

	pDM_FatTable->bBecomeLinked =FALSE;
	pDM_FatTable->AntDiv_OnOff =0xff;

	//3       -   AP   -
	#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	
		#ifdef BEAMFORMING_SUPPORT
		#if(DM_ODM_SUPPORT_TYPE == ODM_AP)
		odm_BDC_Init(pDM_Odm);
		#endif
		#endif
		
	//3     -   WIN   -
	#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		pDM_SWAT_Table->Ant5G = MAIN_ANT;
		pDM_SWAT_Table->Ant2G = MAIN_ANT;
		pDM_FatTable->CCK_counter_main=0;
		pDM_FatTable->CCK_counter_aux=0;
		pDM_FatTable->OFDM_counter_main=0;
		pDM_FatTable->OFDM_counter_aux=0;
	#endif
	
	//2 [---Set MAIN_ANT as default antenna if Auto-Ant enable---]
	odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
	
	pDM_Odm->AntType = ODM_AUTO_ANT;
	ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);

	//2 [---Set TX Antenna---]
	odm_Tx_By_TxDesc_or_Reg(pDM_Odm , REG);
	
		
	//2 [--88E---]
	if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		#if (RTL8188E_SUPPORT == 1)
			//pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;

			if( (pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV)  && (pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV) && (pDM_Odm->AntDivType != CG_TRX_SMART_ANTDIV))
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]  88E Not Supprrt This AntDiv Type\n"));
			pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
			
			if(pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
				odm_RX_HWAntDiv_Init_88E(pDM_Odm);
			else if(pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				odm_TRX_HWAntDiv_Init_88E(pDM_Odm);
		        #if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			else if(pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV)
				odm_Smart_HWAntDiv_Init_88E(pDM_Odm);
                        #endif
		#endif
	}

	//2 [--92E---]
	#if (RTL8192E_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8192E)
	{
			//pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;

			if( (pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV) && (pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV)   && (pDM_Odm->AntDivType != CG_TRX_SMART_ANTDIV))
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]  8192E Not Supprrt This AntDiv Type\n"));
			pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
			
			if(pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
				odm_RX_HWAntDiv_Init_92E(pDM_Odm);
			else if(pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				odm_TRX_HWAntDiv_Init_92E(pDM_Odm);
	        	#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			else if(pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV)
				odm_Smart_HWAntDiv_Init_92E(pDM_Odm);
		        #endif
	
	}
		#endif
	
	//2 [--8723B---]
	#if (RTL8723B_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8723B)
	{
			//pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;

			if(pDM_Odm->AntDivType != S0S1_SW_ANTDIV && pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!] 8723B  Not Supprrt This AntDiv Type\n"));
			pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
				
			if( pDM_Odm->AntDivType==S0S1_SW_ANTDIV)
				odm_S0S1_SWAntDiv_Init_8723B(pDM_Odm);
			else if(pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV)
				odm_TRX_HWAntDiv_Init_8723B(pDM_Odm);
	}
		#endif
	
	//2 [--8811A 8821A---]
	#if (RTL8821A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8821)
	{
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
			pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
			
			if( pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV && pDM_Odm->AntDivType != S0S1_SW_ANTDIV)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!] 8821A & 8811A  Not Supprrt This AntDiv Type\n"));
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
		if(pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV)	
			odm_TRX_HWAntDiv_Init_8821A(pDM_Odm);
		else if( pDM_Odm->AntDivType==S0S1_SW_ANTDIV)
			odm_S0S1_SWAntDiv_Init_8821A(pDM_Odm);
	}
		#endif
	
	//2 [--8881A---]
	#if (RTL8881A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8881A)
	{
			//pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
			
			if(pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV && pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!] 8881A  Not Supprrt This AntDiv Type\n"));
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
			if(pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
				odm_RX_HWAntDiv_Init_8881A(pDM_Odm);
			else if(pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				odm_TRX_HWAntDiv_Init_8881A(pDM_Odm);
	}
		#endif
	
	//2 [--8812---]
	#if (RTL8812A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8812)
	{
			//pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
			
			if( pDM_Odm->AntDivType != CG_TRX_HW_ANTDIV)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!] 8812A  Not Supprrt This AntDiv Type\n"));
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
			odm_TRX_HWAntDiv_Init_8812A(pDM_Odm);
	}
		#endif
	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** SupportICType=[%lu] \n",pDM_Odm->SupportICType));
	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** AntDiv SupportAbility=[%lu] \n",(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)>>6));
	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** AntDiv Type=[%d] \n",pDM_Odm->AntDivType));

}

VOID
ODM_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
)
{	
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;

	if(*pDM_Odm->pBandType == ODM_BAND_5G )
	{
		if(pDM_FatTable->idx_AntDiv_counter_5G<  pDM_Odm->antdiv_period )
		{
			pDM_FatTable->idx_AntDiv_counter_5G++;
			return;
		}
		else
			pDM_FatTable->idx_AntDiv_counter_5G=0;
	}
	else 	if(*pDM_Odm->pBandType == ODM_BAND_2_4G )
	{
		if(pDM_FatTable->idx_AntDiv_counter_2G <  pDM_Odm->antdiv_period )
		{
			pDM_FatTable->idx_AntDiv_counter_2G++;
			return;
		}
		else
			pDM_FatTable->idx_AntDiv_counter_2G=0;
	}
	
	//----------
	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}

	//----------
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if(pAdapter->MgntInfo.AntennaTest)
	   return;

        {
	#if (BEAMFORMING_SUPPORT == 1)			
	        BEAMFORMING_CAP		BeamformCap = (pDM_Odm->BeamformingInfo.BeamformCap);

		if( BeamformCap & BEAMFORMEE_CAP ) //  BFmee On  &&   Div On ->  Div Off
		{	
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[ AntDiv : OFF ]   BFmee ==1 \n"));
			if(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)
			{
				odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
				return;
			}
		}
		else // BFmee Off   &&   Div Off ->  Div On
	#endif
		{
			if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)  &&  pDM_Odm->bLinked) 
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[ AntDiv : ON ]   BFmee ==0 \n"));
				if((pDM_Odm->AntDivType!=S0S1_SW_ANTDIV) )
					odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
				
				pDM_Odm->SupportAbility |= (ODM_BB_ANT_DIV);
			}
		}
        }
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
	//----------just for fool proof

	if(pDM_Odm->antdiv_rssi)
		pDM_Odm->DebugComponents |= ODM_COMP_ANT_DIV;
	else
		pDM_Odm->DebugComponents &= ~ODM_COMP_ANT_DIV;

	if(pDM_FatTable->AntDiv_2G_5G == ODM_ANTDIV_2G)
	{
		//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[ 2G AntDiv Running ]\n"));
		if(!(pDM_Odm->SupportICType & ODM_ANTDIV_2G_SUPPORT_IC))
			return;
	}
	else 	if(pDM_FatTable->AntDiv_2G_5G == ODM_ANTDIV_5G)
	{
		//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[ 5G AntDiv Running ]\n"));
		if(!(pDM_Odm->SupportICType & ODM_ANTDIV_5G_SUPPORT_IC))
			return;
	}
	//else 	if(pDM_FatTable->AntDiv_2G_5G == (ODM_ANTDIV_2G|ODM_ANTDIV_5G))
	//{
		//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[ 2G & 5G AntDiv Running ]\n"));
	//}
#endif

	//----------

	if (pDM_Odm->antdiv_select==1)
		pDM_Odm->AntType = ODM_FIX_MAIN_ANT;
	else if (pDM_Odm->antdiv_select==2)
		pDM_Odm->AntType = ODM_FIX_AUX_ANT;
	else  //if (pDM_Odm->antdiv_select==0)
		pDM_Odm->AntType = ODM_AUTO_ANT;

	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("AntType= (( %d )) , pre_AntType= (( %d ))  \n",pDM_Odm->AntType,pDM_Odm->pre_AntType));
			
	if(pDM_Odm->AntType != ODM_AUTO_ANT)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fix Antenna at (( %s ))\n",(pDM_Odm->AntType == ODM_FIX_MAIN_ANT)?"MAIN":"AUX"));
			
		if(pDM_Odm->AntType != pDM_Odm->pre_AntType)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , REG);
					
			if(pDM_Odm->AntType == ODM_FIX_MAIN_ANT)
				ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);		        
			else if(pDM_Odm->AntType == ODM_FIX_AUX_ANT)
				ODM_UpdateRxIdleAnt(pDM_Odm, AUX_ANT);
		}
		pDM_Odm->pre_AntType=pDM_Odm->AntType; 
		return;
	}
	else
	{
		if(pDM_Odm->AntType != pDM_Odm->pre_AntType)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm , TX_BY_DESC);
		}
		pDM_Odm->pre_AntType=pDM_Odm->AntType;
	}
	 
	
	//3 -----------------------------------------------------------------------------------------------------------
	//2 [--88E---]
	if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		#if (RTL8188E_SUPPORT == 1)
		if(pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV ||pDM_Odm->AntDivType==CGCS_RX_HW_ANTDIV)
			odm_HW_AntDiv(pDM_Odm);

		#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
		else if (pDM_Odm->AntDivType==CG_TRX_SMART_ANTDIV)
			odm_FastAntTraining(pDM_Odm);
		#endif
		
		#endif

	}
	//2 [--92E---]	
	#if (RTL8192E_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8192E)
	{
		if(pDM_Odm->AntDivType==CGCS_RX_HW_ANTDIV || pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV)
			odm_HW_AntDiv(pDM_Odm);
		
		#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
		else if (pDM_Odm->AntDivType==CG_TRX_SMART_ANTDIV)
			odm_FastAntTraining(pDM_Odm);	
		#endif
		
	}
	#endif

	#if (RTL8723B_SUPPORT == 1)	
	//2 [--8723B---]
	else if(pDM_Odm->SupportICType == ODM_RTL8723B)
	{
		if (pDM_Odm->AntDivType==S0S1_SW_ANTDIV)
			odm_S0S1_SwAntDiv(pDM_Odm, SWAW_STEP_PEAK);
		else if (pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV)
			odm_HW_AntDiv(pDM_Odm);
	}
	#endif
	
	//2 [--8821A---]
	#if (RTL8821A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8821)
	{
		if(!pDM_Odm->bBtEnabled)  //BT disabled
		{
			if(pDM_Odm->AntDivType == S0S1_SW_ANTDIV)
			{
			pDM_Odm->AntDivType=CG_TRX_HW_ANTDIV;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,(" [S0S1_SW_ANTDIV]  ->  [CG_TRX_HW_ANTDIV]\n"));
				//ODM_SetBBReg(pDM_Odm, 0x8D4 , BIT24, 1); 
				if(pDM_FatTable->bBecomeLinked ==TRUE)
					odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			}
		}	
		else //BT enabled
		{
			if(pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
			{
			pDM_Odm->AntDivType=S0S1_SW_ANTDIV;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,(" [CG_TRX_HW_ANTDIV]  ->  [S0S1_SW_ANTDIV]\n"));
				//ODM_SetBBReg(pDM_Odm, 0x8D4 , BIT24, 0);
				odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			}	
		}	
	
		if (pDM_Odm->AntDivType==S0S1_SW_ANTDIV)
			odm_S0S1_SwAntDiv(pDM_Odm, SWAW_STEP_PEAK);
		else if (pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV)
			odm_HW_AntDiv(pDM_Odm);
	}
	#endif
	
	//2 [--8881A---]
	#if (RTL8881A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8881A)
			odm_HW_AntDiv(pDM_Odm);
	#endif
	
	//2 [--8812A---]
	#if (RTL8812A_SUPPORT == 1)
	else if(pDM_Odm->SupportICType == ODM_RTL8812)
			odm_HW_AntDiv(pDM_Odm);
	#endif
	}


VOID
odm_AntselStatistics(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			MacId,
	IN		u4Byte			utility,
	IN            u1Byte			method
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	if(method==RSSI_METHOD)
	{
		if(antsel_tr_mux == ANT1_2G)
		{
			pDM_FatTable->MainAnt_Sum[MacId]+=utility;
			pDM_FatTable->MainAnt_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAnt_Sum[MacId]+=utility;
			pDM_FatTable->AuxAnt_Cnt[MacId]++;
		}
	}
	#ifdef ODM_EVM_ENHANCE_ANTDIV
	else if(method==EVM_METHOD)
	{
		if(antsel_tr_mux == ANT1_2G)
		{
			pDM_FatTable->MainAntEVM_Sum[MacId]+=(utility<<5);
			pDM_FatTable->MainAntEVM_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAntEVM_Sum[MacId]+=(utility<<5);
			pDM_FatTable->AuxAntEVM_Cnt[MacId]++;
		}
	}
	else if(method==CRC32_METHOD)
	{
		if(utility==0)
			pDM_FatTable->CRC32_Fail_Cnt++;
		else
			pDM_FatTable->CRC32_Ok_Cnt+=utility;
	}
	#endif
}


VOID
ODM_Process_RSSIForAntDiv(	
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN		PODM_PHY_INFO_T				pPhyInfo,
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{
u1Byte			isCCKrate=0,CCKMaxRate=ODM_RATE11M;
pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;

#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN))
	u4Byte			RxPower_Ant0, RxPower_Ant1;	
	u4Byte			RxEVM_Ant0, RxEVM_Ant1;
#else
	u1Byte			RxPower_Ant0, RxPower_Ant1;	
	u1Byte			RxEVM_Ant0, RxEVM_Ant1;
#endif

	CCKMaxRate=ODM_RATE11M;
	isCCKrate = (pPktinfo->DataRate <= CCKMaxRate)?TRUE:FALSE;

#if ((RTL8192C_SUPPORT == 1) ||(RTL8192D_SUPPORT == 1))
		if(pDM_Odm->SupportICType & ODM_RTL8192C|ODM_RTL8192D)
		{
				if(pPktinfo->bPacketToSelf || pPktinfo->bPacketBeacon)
				{
					ODM_AntselStatistics_88C(pDM_Odm, pPktinfo->StationID,  pPhyInfo->RxPWDBAll, isCCKrate);
				}
		}
#endif
		
	if(  (pDM_Odm->SupportICType == ODM_RTL8192E||pDM_Odm->SupportICType == ODM_RTL8812)   && (pPktinfo->DataRate > CCKMaxRate) )
	{
		RxPower_Ant0 = pPhyInfo->RxMIMOSignalStrength[0];
		RxPower_Ant1= pPhyInfo->RxMIMOSignalStrength[1];

		RxEVM_Ant0 =pPhyInfo->RxMIMOSignalQuality[0];
		RxEVM_Ant1 =pPhyInfo->RxMIMOSignalQuality[1];
	}
	else
		RxPower_Ant0=pPhyInfo->RxPWDBAll;
	
	if(pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV)
	{
		if( (pDM_Odm->SupportICType & ODM_SMART_ANT_SUPPORT) &&  (pPktinfo->bPacketToSelf)   && (pDM_FatTable->FAT_State == FAT_TRAINING_STATE) )//(pPktinfo->bPacketMatchBSSID && (!pPktinfo->bPacketBeacon))
		{
			u1Byte	antsel_tr_mux;
			antsel_tr_mux = (pDM_FatTable->antsel_rx_keep_2<<2) |(pDM_FatTable->antsel_rx_keep_1 <<1) |pDM_FatTable->antsel_rx_keep_0;
			pDM_FatTable->antSumRSSI[antsel_tr_mux] += RxPower_Ant0;
			pDM_FatTable->antRSSIcnt[antsel_tr_mux]++;
		}
	}
	else //AntDivType != CG_TRX_SMART_ANTDIV 
	{
		if(  ( pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT ) &&  (pPktinfo->bPacketToSelf || pPktinfo->bPacketMatchBSSID)  )
		{
			if(pDM_Odm->SupportICType == ODM_RTL8188E || pDM_Odm->SupportICType == ODM_RTL8192E)
			{
				odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID,RxPower_Ant0,RSSI_METHOD);

				#ifdef ODM_EVM_ENHANCE_ANTDIV
				if(!isCCKrate)
				{
					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID,RxEVM_Ant0,EVM_METHOD);
				}
				#endif
			}
			else// SupportICType == ODM_RTL8821 and ODM_RTL8723B and ODM_RTL8812)
			{
				if(isCCKrate && (pDM_Odm->AntDivType == S0S1_SW_ANTDIV))
				{
				 	pDM_FatTable->antsel_rx_keep_0 = (pDM_FatTable->RxIdleAnt == MAIN_ANT) ? ANT1_2G : ANT2_2G;


						if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
							pDM_FatTable->CCK_counter_main++;
						else// if(pDM_FatTable->antsel_rx_keep_0==ANT2_2G)
							pDM_FatTable->CCK_counter_aux++;

					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID, RxPower_Ant0,RSSI_METHOD);
				}
				else
				{
					if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
						pDM_FatTable->OFDM_counter_main++;
					else// if(pDM_FatTable->antsel_rx_keep_0==ANT2_2G)
						pDM_FatTable->OFDM_counter_aux++;
					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID, RxPower_Ant0,RSSI_METHOD);
			}
		}
	}
	}
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("isCCKrate=%d, PWDB_ALL=%d\n",isCCKrate, pPhyInfo->RxPWDBAll));
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=3'b%d%d%d\n",pDM_FatTable->antsel_rx_keep_2, pDM_FatTable->antsel_rx_keep_1, pDM_FatTable->antsel_rx_keep_0));
}

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
VOID
ODM_SetTxAntByTxInfo(
	IN		PDM_ODM_T		pDM_Odm,
	IN		pu1Byte			pDesc,
	IN		u1Byte			macId	
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if(pDM_Odm->AntDivType==CGCS_RX_HW_ANTDIV)
		return;


	if(pDM_Odm->SupportICType == ODM_RTL8723B)
	{
#if (RTL8723B_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8723B(pDesc, pDM_FatTable->antsel_a[macId]);
		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8723B] SetTxAntByTxInfo_WIN: MacID=%d, antsel_tr_mux=3'b%d%d%d\n", 
			//macId, pDM_FatTable->antsel_c[macId], pDM_FatTable->antsel_b[macId], pDM_FatTable->antsel_a[macId]));
#endif
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8821)
	{
#if (RTL8821A_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8812(pDesc, pDM_FatTable->antsel_a[macId]);
		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8821A] SetTxAntByTxInfo_WIN: MacID=%d, antsel_tr_mux=3'b%d%d%d\n", 
			//macId, pDM_FatTable->antsel_c[macId], pDM_FatTable->antsel_b[macId], pDM_FatTable->antsel_a[macId]));
#endif
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
#if (RTL8188E_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_88E(pDesc, pDM_FatTable->antsel_a[macId]);
		SET_TX_DESC_ANTSEL_B_88E(pDesc, pDM_FatTable->antsel_b[macId]);
		SET_TX_DESC_ANTSEL_C_88E(pDesc, pDM_FatTable->antsel_c[macId]);
		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8188E] SetTxAntByTxInfo_WIN: MacID=%d, antsel_tr_mux=3'b%d%d%d\n", 
			//macId, pDM_FatTable->antsel_c[macId], pDM_FatTable->antsel_b[macId], pDM_FatTable->antsel_a[macId]));
#endif
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8192E)
	{

	
	}
}
#elif(DM_ODM_SUPPORT_TYPE == ODM_AP)

VOID
ODM_SetTxAntByTxInfo(
	//IN		PDM_ODM_T		pDM_Odm,
	struct	rtl8192cd_priv		*priv,
	struct 	tx_desc			*pdesc,
	struct	tx_insn			*txcfg,
	unsigned short			aid	
)
{
	PDM_ODM_T	pDM_Odm = &(priv->pshare->_dmODM);
	pFAT_T		pDM_FatTable = &priv->pshare->_dmODM.DM_FatTable;
	u4Byte		SupportICType=priv->pshare->_dmODM.SupportICType;
	
	if (!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if (pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
		return;

	if(SupportICType == ODM_RTL8881A)
	{
		//panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8881E******   \n",__FUNCTION__,__LINE__);	
		pdesc->Dword6 &= set_desc(~ (BIT(18)|BIT(17)|BIT(16)));	
		pdesc->Dword6 |= set_desc(pDM_FatTable->antsel_a[aid]<<16);
	}
	else if(SupportICType == ODM_RTL8192E)
	{
		//panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8192E******   \n",__FUNCTION__,__LINE__);	
		pdesc->Dword6 &= set_desc(~ (BIT(18)|BIT(17)|BIT(16)));	
		pdesc->Dword6 |= set_desc(pDM_FatTable->antsel_a[aid]<<16);
	}
	else if(SupportICType == ODM_RTL8188E)
	{
		//panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8188E******   \n",__FUNCTION__,__LINE__);
		pdesc->Dword2 &= set_desc(~ BIT(24));
		pdesc->Dword2 &= set_desc(~ BIT(25));
		pdesc->Dword7 &= set_desc(~ BIT(29));
		if(txcfg->pstat)
		{
			pdesc->Dword2 |= set_desc(pDM_FatTable->antsel_a[aid]<<24);
			pdesc->Dword2 |= set_desc(pDM_FatTable->antsel_b[aid]<<25);
			pdesc->Dword7 |= set_desc(pDM_FatTable->antsel_c[aid]<<29);
		}
		
	}
	else if(SupportICType == ODM_RTL8812)
	{
		//3 [path-A]
		//panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8881E******   \n",__FUNCTION__,__LINE__);
			
		pdesc->Dword6 &= set_desc(~ BIT(16));
		pdesc->Dword6 &= set_desc(~ BIT(17));
		pdesc->Dword6 &= set_desc(~ BIT(18));
		if(txcfg->pstat)
		{
			pdesc->Dword6 |= set_desc(pDM_FatTable->antsel_a[aid]<<16);
			pdesc->Dword6 |= set_desc(pDM_FatTable->antsel_b[aid]<<17);
			pdesc->Dword6 |= set_desc(pDM_FatTable->antsel_c[aid]<<18);
		}
	}
}

#ifdef  CONFIG_WLAN_HAL
VOID
ODM_SetTxAntByTxInfo_HAL(
	//IN		PDM_ODM_T		pDM_Odm,
	struct	rtl8192cd_priv		*priv,
	PVOID      pdesc_data,
	struct	tx_insn			*txcfg,
	unsigned short			aid	
)
{
	PDM_ODM_T	pDM_Odm = &(priv->pshare->_dmODM);
	pFAT_T		pDM_FatTable = &priv->pshare->_dmODM.DM_FatTable;
	u4Byte		SupportICType=priv->pshare->_dmODM.SupportICType;
	PTX_DESC_DATA_88XX      pdescdata = (PTX_DESC_DATA_88XX)pdesc_data;

	if (!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if (pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
		return;
	
	if(SupportICType == ODM_RTL8881A || SupportICType == ODM_RTL8192E || SupportICType == ODM_RTL8814A)
	{
		//panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_HAL******   \n",__FUNCTION__,__LINE__);
		pdescdata->antSel  = 1;
		pdescdata->antSel_A = pDM_FatTable->antsel_a[aid];
	}
}
#endif	//#ifdef  CONFIG_WLAN_HAL
#endif


VOID
ODM_AntDiv_Config(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("WIN Config Antenna Diversity\n"));
		if(pDM_Odm->SupportICType==ODM_RTL8723B)
		{
			if((!pDM_Odm->DM_SWAT_Table.ANTA_ON || !pDM_Odm->DM_SWAT_Table.ANTB_ON))
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
		}
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE))

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CE Config Antenna Diversity\n"));
		if(pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT)
		{
			pDM_Odm->SupportAbility |= ODM_BB_ANT_DIV;	
		}
		
		if(pDM_Odm->SupportICType==ODM_RTL8723B)
		{
			pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
		}				

#elif (DM_ODM_SUPPORT_TYPE & (ODM_AP))

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("AP Config Antenna Diversity\n"));

		//2 [ NOT_SUPPORT_ANTDIV ]
	#if(defined(CONFIG_NOT_SUPPORT_ANTDIV)) 
		pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Disable AntDiv function] : Not Support 2.4G & 5G Antenna Diversity\n"));
		
		//2 [ 2G&5G_SUPPORT_ANTDIV ]
	#elif(defined(CONFIG_2G5G_SUPPORT_ANTDIV))
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Enable AntDiv function] : 2.4G & 5G Support Antenna Diversity Simultaneously \n"));
		pDM_FatTable->AntDiv_2G_5G = (ODM_ANTDIV_2G|ODM_ANTDIV_5G);

		if(pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT)
			pDM_Odm->SupportAbility |= ODM_BB_ANT_DIV;
		if(*pDM_Odm->pBandType == ODM_BAND_5G )
		{
				#if ( defined(CONFIG_5G_CGCS_RX_DIVERSITY) )
					pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV; 
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n"));
					panic_printk("[ 5G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n");
				#elif( defined(CONFIG_5G_CG_TRX_DIVERSITY)||defined(CONFIG_2G5G_CG_TRX_DIVERSITY_8881A))
					pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CG_TRX_HW_ANTDIV\n"));				
					panic_printk("[ 5G] : AntDiv Type = CG_TRX_HW_ANTDIV\n");
				#elif( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CG_SMART_ANTDIV\n"));
				#elif( defined(CONFIG_5G_S0S1_SW_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = S0S1_SW_ANTDIV\n"));
				#endif
		}		
		else if(*pDM_Odm->pBandType == ODM_BAND_2_4G )
		 {
				#if ( defined(CONFIG_2G_CGCS_RX_DIVERSITY) )
					pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n"));		
				#elif( defined(CONFIG_2G_CG_TRX_DIVERSITY) || defined(CONFIG_2G5G_CG_TRX_DIVERSITY_8881A))
					pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CG_TRX_HW_ANTDIV\n"));
				#elif( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CG_SMART_ANTDIV\n"));
				#elif( defined(CONFIG_2G_S0S1_SW_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = S0S1_SW_ANTDIV\n"));
				#endif
		}
		
		//2 [ 5G_SUPPORT_ANTDIV ]
	#elif(defined(CONFIG_5G_SUPPORT_ANTDIV))
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Enable AntDiv function] : Only 5G Support Antenna Diversity\n"));
		panic_printk("[ Enable AntDiv function] : Only 5G Support Antenna Diversity\n");
		pDM_FatTable->AntDiv_2G_5G = (ODM_ANTDIV_5G);
		if(*pDM_Odm->pBandType == ODM_BAND_5G )
		{
				if(pDM_Odm->SupportICType & ODM_ANTDIV_5G_SUPPORT_IC)
				pDM_Odm->SupportAbility |= ODM_BB_ANT_DIV;	
				#if ( defined(CONFIG_5G_CGCS_RX_DIVERSITY) )
					pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n"));
					panic_printk("[ 5G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n");
				#elif( defined(CONFIG_5G_CG_TRX_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
					panic_printk("[ 5G] : AntDiv Type = CG_TRX_HW_ANTDIV\n");
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CG_TRX_HW_ANTDIV\n"));
				#elif( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = CG_SMART_ANTDIV\n"));
				#elif( defined(CONFIG_5G_S0S1_SW_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 5G] : AntDiv Type = S0S1_SW_ANTDIV\n"));
				#endif
		}
		else if(*pDM_Odm->pBandType == ODM_BAND_2_4G )
		{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Not Support 2G AntDivType\n"));
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
		}
		
		//2 [ 2G_SUPPORT_ANTDIV ]
	#elif(defined(CONFIG_2G_SUPPORT_ANTDIV)) 
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Enable AntDiv function] : Only 2.4G Support Antenna Diversity\n"));
		pDM_FatTable->AntDiv_2G_5G = (ODM_ANTDIV_2G);
		if(*pDM_Odm->pBandType == ODM_BAND_2_4G )
		{
				if(pDM_Odm->SupportICType & ODM_ANTDIV_2G_SUPPORT_IC)
					pDM_Odm->SupportAbility |= ODM_BB_ANT_DIV;
				#if ( defined(CONFIG_2G_CGCS_RX_DIVERSITY) )
					pDM_Odm->AntDivType = CGCS_RX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CGCS_RX_HW_ANTDIV\n"));		
				#elif( defined(CONFIG_2G_CG_TRX_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CG_TRX_HW_ANTDIV\n"));
				#elif( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = CG_TRX_SMART_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = CG_SMART_ANTDIV\n"));
				#elif( defined(CONFIG_2G_S0S1_SW_ANT_DIVERSITY) )
					pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2.4G] : AntDiv Type = S0S1_SW_ANTDIV\n"));
				#endif
		}
		else if(*pDM_Odm->pBandType == ODM_BAND_5G )
		{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Not Support 5G AntDivType\n"));
				pDM_Odm->SupportAbility &= ~(ODM_BB_ANT_DIV);
		}
	#endif	
#endif	

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SupportAbility = (( %x ))\n", pDM_Odm->SupportAbility ));

}


VOID
ODM_AntDivTimers(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte			state
	)
{
	if(state==INIT_ANTDIV_TIMMER)
	{
		#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
			ODM_InitializeTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer_8723B,
			(RT_TIMER_CALL_BACK)ODM_SW_AntDiv_Callback, NULL, "SwAntennaSwitchTimer_8723B");
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_InitializeTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer,
			(RT_TIMER_CALL_BACK)odm_FastAntTrainingCallback, NULL, "FastAntTrainingTimer");
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
		ODM_InitializeTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer,
			(RT_TIMER_CALL_BACK)odm_EVM_FastAntTrainingCallback, NULL, "EVM_FastAntTrainingTimer");
		#endif
	}
	else if(state==CANCEL_ANTDIV_TIMMER)
	{
		#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
			ODM_CancelTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer_8723B);
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_CancelTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer);
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
			ODM_CancelTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer);
		#endif
	}
	else if(state==RELEASE_ANTDIV_TIMMER)
	{
		#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
			ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer_8723B);
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer);
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
			ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer);
		#endif
	}

}

#endif //#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))

VOID
ODM_AntDivReset(
	IN PDM_ODM_T	pDM_Odm 
	)
{
	//2 [--8723B---]
#if (RTL8723B_SUPPORT == 1)
	if(pDM_Odm->SupportICType == ODM_RTL8723B)
	{
		#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
		odm_S0S1_SWAntDiv_Reset_8723B(pDM_Odm);
		#endif
	}
#endif
}

VOID
odm_AntennaDiversityInit(
	IN 		PDM_ODM_T		pDM_Odm 
)
{
	if(pDM_Odm->mp_mode == TRUE)
		return;
	
	if(pDM_Odm->SupportICType & (ODM_OLD_IC_ANTDIV_SUPPORT))
	{
		#if (RTL8192C_SUPPORT==1) 
		#if (!(DM_ODM_SUPPORT_TYPE & (ODM_AP)))
		ODM_OldIC_AntDiv_Init(pDM_Odm);
		#endif
		#endif
	}
	else
	{
		#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
		ODM_AntDiv_Config(pDM_Odm);
		ODM_AntDivInit(pDM_Odm);
		#endif
	}
}

VOID
odm_AntennaDiversity(
	IN 		PDM_ODM_T		pDM_Odm 
)
{
	if(pDM_Odm->mp_mode == TRUE)
		return;

	if(pDM_Odm->SupportICType & (ODM_OLD_IC_ANTDIV_SUPPORT))
	{
		#if (RTL8192C_SUPPORT==1)
		#if (!(DM_ODM_SUPPORT_TYPE & (ODM_AP)))
		ODM_OldIC_AntDiv(pDM_Odm);
		#endif
		#endif
	}
	else
	{
		#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
		ODM_AntDiv(pDM_Odm);
		#endif
	}
}


