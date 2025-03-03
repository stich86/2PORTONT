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


/*
#if defined(RTL8188E_FOR_TEST_CHIP) && (RTL8188E_FOR_TEST_CHIP > 1)
    #define READ_AND_CONFIG(ic, txt) do {\
                                            if (pDM_Odm->bIsMPChip)\
                                    		    READ_AND_CONFIG_MP(ic,txt);\
                                            else\
                                                READ_AND_CONFIG_TC(ic,txt);\
                                     } while(0)
#elif defined(RTL8188E_FOR_TEST_CHIP) && (RTL8188E_FOR_TEST_CHIP == 1)
    #define READ_AND_CONFIG     READ_AND_CONFIG_TC
#else
    #define READ_AND_CONFIG     READ_AND_CONFIG_MP
#endif

#define READ_AND_CONFIG_MP(ic, txt) (ODM_ReadAndConfig_##ic##txt(pDM_Odm))
#define READ_AND_CONFIG_TC(ic, txt) (ODM_ReadAndConfig_TC_##ic##txt(pDM_Odm))
*/
#define GET_VERSION_MP(ic, txt) 		(ODM_GetVersion_MP_##ic##txt())
#define GET_VERSION_TC(ic, txt) 		(ODM_GetVersion_TC_##ic##txt())
#define GET_VERSION(ic, txt) (pDM_Odm->bIsMPChip?GET_VERSION_MP(ic,txt):GET_VERSION_TC(ic,txt))


#define READ_AND_CONFIG_MP(ic, txt) (ODM_ReadAndConfig_MP_##ic##txt(pDM_Odm))
#define READ_AND_CONFIG_TC(ic, txt) (ODM_ReadAndConfig_TC_##ic##txt(pDM_Odm))

#define READ_AND_CONFIG(ic, txt) do {\
                                            if (pDM_Odm->bIsMPChip)\
                                    		    READ_AND_CONFIG_MP(ic,txt);\
                                            else\
                                                READ_AND_CONFIG_TC(ic,txt);\
                                    } while(0)

#define READ_FIRMWARE(ic, txt) 		(ODM_ReadFirmware_##ic##txt(pDM_Odm, pFirmware, pSize))
u1Byte
odm_QueryRxPwrPercentage(
	IN		s1Byte		AntPower
	)
{
	if ((AntPower <= -100) || (AntPower >= 20))
	{
		return	0;
	}
	else if (AntPower >= 0)
	{
		return	100;
	}
	else
	{
		return	(100+AntPower);
	}
	
}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
//
// 2012/01/12 MH MOve some signal strength smooth method to MP HAL layer.
// IF other SW team do not support the feature, remove this section.??
//
s4Byte
odm_SignalScaleMapping_92CSeries_patch_RT_CID_819x_Lenovo(	
	IN OUT PDM_ODM_T pDM_Odm,
	s4Byte CurrSig 
)
{	
	s4Byte RetSig=0;
	//if(pDM_Odm->SupportInterface  == ODM_ITRF_PCIE) 
	{
		// Step 1. Scale mapping.
		// 20100611 Joseph: Re-tunning RSSI presentation for Lenovo.
		// 20100426 Joseph: Modify Signal strength mapping.
		// This modification makes the RSSI indication similar to Intel solution.
		// 20100414 Joseph: Tunning RSSI for Lenovo according to RTL8191SE.
		if(CurrSig >= 54 && CurrSig <= 100)
		{
			RetSig = 100;
		}
		else if(CurrSig>=42 && CurrSig <= 53 )
		{
			RetSig = 95;
		}
		else if(CurrSig>=36 && CurrSig <= 41 )
		{
			RetSig = 74 + ((CurrSig - 36) *20)/6;
		}
		else if(CurrSig>=33 && CurrSig <= 35 )
		{
			RetSig = 65 + ((CurrSig - 33) *8)/2;
		}
		else if(CurrSig>=18 && CurrSig <= 32 )
		{
			RetSig = 62 + ((CurrSig - 18) *2)/15;
		}
		else if(CurrSig>=15 && CurrSig <= 17 )
		{
			RetSig = 33 + ((CurrSig - 15) *28)/2;
		}
		else if(CurrSig>=10 && CurrSig <= 14 )
		{
			RetSig = 39;
		}
		else if(CurrSig>=8 && CurrSig <= 9 )
		{
			RetSig = 33;
		}
		else if(CurrSig <= 8 )
		{
			RetSig = 19;
		}
	}
	return RetSig;
}

s4Byte
odm_SignalScaleMapping_92CSeries_patch_RT_CID_819x_Netcore(	
	IN OUT PDM_ODM_T pDM_Odm,
	s4Byte CurrSig 
)
{
	s4Byte RetSig=0;
	//if(pDM_Odm->SupportInterface  == ODM_ITRF_USB)
	{
		// Netcore request this modification because 2009.04.13 SU driver use it. 
		if(CurrSig >= 31 && CurrSig <= 100)
		{
			RetSig = 100;
		}	
		else if(CurrSig >= 21 && CurrSig <= 30)
		{
			RetSig = 90 + ((CurrSig - 20) / 1);
		}
		else if(CurrSig >= 11 && CurrSig <= 20)
		{
			RetSig = 80 + ((CurrSig - 10) / 1);
		}
		else if(CurrSig >= 7 && CurrSig <= 10)
		{
			RetSig = 69 + (CurrSig - 7);
		}
		else if(CurrSig == 6)
		{
			RetSig = 54;
		}
		else if(CurrSig == 5)
		{
			RetSig = 45;
		}
		else if(CurrSig == 4)
		{
			RetSig = 36;
		}
		else if(CurrSig == 3)
		{
			RetSig = 27;
		}
		else if(CurrSig == 2)
		{
			RetSig = 18;
		}
		else if(CurrSig == 1)
		{
			RetSig = 9;
		}
		else
		{
			RetSig = CurrSig;
		}
	}
	return RetSig;
}
#endif //ENDIF (DM_ODM_SUPPORT_TYPE == ODM_WIN)


#if (DM_ODM_SUPPORT_TYPE != ODM_WIN)
s4Byte
odm_SignalScaleMapping_92CSeries(	
	IN OUT PDM_ODM_T pDM_Odm,
	IN s4Byte CurrSig 
)
{
	s4Byte RetSig=0;
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE) 
	if(pDM_Odm->SupportInterface  == ODM_ITRF_PCIE) 
	{
		// Step 1. Scale mapping.
		if(CurrSig >= 61 && CurrSig <= 100)
		{
			RetSig = 90 + ((CurrSig - 60) / 4);
		}
		else if(CurrSig >= 41 && CurrSig <= 60)
		{
			RetSig = 78 + ((CurrSig - 40) / 2);
		}
		else if(CurrSig >= 31 && CurrSig <= 40)
		{
			RetSig = 66 + (CurrSig - 30);
		}
		else if(CurrSig >= 21 && CurrSig <= 30)
		{
			RetSig = 54 + (CurrSig - 20);
		}
		else if(CurrSig >= 5 && CurrSig <= 20)
		{
			RetSig = 42 + (((CurrSig - 5) * 2) / 3);
		}
		else if(CurrSig == 4)
		{
			RetSig = 36;
		}
		else if(CurrSig == 3)
		{
			RetSig = 27;
		}
		else if(CurrSig == 2)
		{
			RetSig = 18;
		}
		else if(CurrSig == 1)
		{
			RetSig = 9;
		}
		else
		{
			RetSig = CurrSig;
		}
	}
#endif

#if ((DEV_BUS_TYPE == RT_USB_INTERFACE) ||(DEV_BUS_TYPE == RT_SDIO_INTERFACE))
	if((pDM_Odm->SupportInterface  == ODM_ITRF_USB) || (pDM_Odm->SupportInterface  == ODM_ITRF_SDIO))
	{
		if(CurrSig >= 51 && CurrSig <= 100)
		{
			RetSig = 100;
		}
		else if(CurrSig >= 41 && CurrSig <= 50)
		{
			RetSig = 80 + ((CurrSig - 40)*2);
		}
		else if(CurrSig >= 31 && CurrSig <= 40)
		{
			RetSig = 66 + (CurrSig - 30);
		}
		else if(CurrSig >= 21 && CurrSig <= 30)
		{
			RetSig = 54 + (CurrSig - 20);
		}
		else if(CurrSig >= 10 && CurrSig <= 20)
		{
			RetSig = 42 + (((CurrSig - 10) * 2) / 3);
		}
		else if(CurrSig >= 5 && CurrSig <= 9)
		{
			RetSig = 22 + (((CurrSig - 5) * 3) / 2);
		}
		else if(CurrSig >= 1 && CurrSig <= 4)
		{
			RetSig = 6 + (((CurrSig - 1) * 3) / 2);
		}
		else
		{
			RetSig = CurrSig;
		}
	}

#endif
	return RetSig;
}
s4Byte
odm_SignalScaleMapping(	
	IN OUT PDM_ODM_T pDM_Odm,
	IN	s4Byte CurrSig 
)
{	
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if(	(pDM_Odm->SupportPlatform == ODM_WIN) && 
		(pDM_Odm->SupportInterface  != ODM_ITRF_PCIE) && //USB & SDIO
		(pDM_Odm->PatchID==10))//pMgntInfo->CustomerID == RT_CID_819x_Netcore
	{
		return odm_SignalScaleMapping_92CSeries_patch_RT_CID_819x_Netcore(pDM_Odm,CurrSig);
	}
	else if(	(pDM_Odm->SupportPlatform == ODM_WIN) && 
			(pDM_Odm->SupportInterface  == ODM_ITRF_PCIE) &&
			(pDM_Odm->PatchID==19))//pMgntInfo->CustomerID == RT_CID_819x_Lenovo)
	{
		return odm_SignalScaleMapping_92CSeries_patch_RT_CID_819x_Lenovo(pDM_Odm, CurrSig);
	}else
#endif
	{
		return odm_SignalScaleMapping_92CSeries(pDM_Odm,CurrSig);
	}
	
}
#endif

//pMgntInfo->CustomerID == RT_CID_819x_Lenovo
static u1Byte odm_SQ_process_patch_RT_CID_819x_Lenovo(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte 		isCCKrate,
	IN u1Byte 		PWDB_ALL,
	IN u1Byte 		path,
	IN u1Byte 		RSSI
)
{
	u1Byte	SQ=0;
#if (DM_ODM_SUPPORT_TYPE &  ODM_WIN)			
	// mapping to 5 bars for vista signal strength
	// signal quality in driver will be displayed to signal strength
	if(isCCKrate){
		// in vista.
		if(PWDB_ALL >= 50)
			SQ = 100;
		else if(PWDB_ALL >= 35 && PWDB_ALL < 50)				
			SQ = 80;
		else if(PWDB_ALL >= 22 && PWDB_ALL < 35)
			SQ = 60;
		else if(PWDB_ALL >= 18 && PWDB_ALL < 22)
			SQ = 40;
		else
			SQ = 20;
	}
	else{//OFDM rate		
		
		// mapping to 5 bars for vista signal strength
		// signal quality in driver will be displayed to signal strength
		// in vista.
		if(RSSI >= 50)
			SQ = 100;
		else if(RSSI >= 35 && RSSI < 50)
			SQ = 80;
		else if(RSSI >= 22 && RSSI < 35)
			SQ = 60;
		else if(RSSI >= 18 && RSSI < 22)
			SQ = 40;
		else
			SQ = 20;			
	}
#endif
	return SQ;
}
			
static u1Byte 
odm_EVMdbToPercentage(
    IN		s1Byte Value
    )
{
	//
	// -33dB~0dB to 0%~99%
	//
	s1Byte ret_val;
    
	ret_val = Value;
	//ret_val /= 2;

	//ODM_RTPRINT(FRX, RX_PHY_SQ, ("EVMdbToPercentage92C Value=%d / %x \n", ret_val, ret_val));
	#ifdef ODM_EVM_ENHANCE_ANTDIV
	
		if(ret_val >= 0)
			ret_val = 0;
		if(ret_val <= -40)
			ret_val = -40;

		ret_val = 0 - ret_val;
		ret_val*=3;

	#else
		if(ret_val >= 0)
			ret_val = 0;
		if(ret_val <= -33)
			ret_val = -33;

		ret_val = 0 - ret_val;
		ret_val*=3;

		if(ret_val == 99)
			ret_val = 100;
	#endif

	return(ret_val);
}
			

#if(ODM_IC_11N_SERIES_SUPPORT == 1)
VOID
odm_RxPhyStatus92CSeries_Parsing(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T			pPhyInfo,		
	IN 		pu1Byte						pPhyStatus,
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{							
	SWAT_T				*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u1Byte				i, Max_spatial_stream;
	s1Byte				rx_pwr[4], rx_pwr_all=0;
	u1Byte				EVM, PWDB_ALL = 0, PWDB_ALL_BT;
	u1Byte				RSSI, total_rssi=0;
	BOOLEAN				isCCKrate=FALSE;	
	u1Byte				rf_rx_num = 0;
	u1Byte				cck_highpwr = 0;
	u1Byte				LNA_idx, VGA_idx;
	
	PPHY_STATUS_RPT_8192CD_T pPhyStaRpt = (PPHY_STATUS_RPT_8192CD_T)pPhyStatus;	

	isCCKrate = ((pPktinfo->DataRate >= ODM_RATE1M ) && (pPktinfo->DataRate <= ODM_RATE11M ))?TRUE :FALSE;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = -1;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_B] = -1;


	if(isCCKrate)
	{
		u1Byte report;
		u1Byte cck_agc_rpt;
		
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusCCK++;
		// 
		// (1)Hardware does not provide RSSI for CCK
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//

		//if(pHalData->eRFPowerState == eRfOn)
			cck_highpwr = pDM_Odm->bCckHighPower;
		//else
		//	cck_highpwr = FALSE;

		cck_agc_rpt =  pPhyStaRpt->cck_agc_rpt_ofdm_cfosho_a ;
		
		//2011.11.28 LukeLee: 88E use different LNA & VGA gain table
		//The RSSI formula should be modified according to the gain table
		//In 88E, cck_highpwr is always set to 1
		if(pDM_Odm->SupportICType & (ODM_RTL8188E|ODM_RTL8192E|ODM_RTL8723B))
		{
			LNA_idx = ((cck_agc_rpt & 0xE0) >>5);
			VGA_idx = (cck_agc_rpt & 0x1F); 
			if(pDM_Odm->SupportICType & (ODM_RTL8188E|ODM_RTL8192E))
			{
		 		switch(LNA_idx)
				{
					case 7:
						if(VGA_idx <= 27)
							rx_pwr_all = -100 + 2*(27-VGA_idx); //VGA_idx = 27~2
						else
							rx_pwr_all = -100;
						break;
					case 6:
							rx_pwr_all = -48 + 2*(2-VGA_idx); //VGA_idx = 2~0
						break;
					case 5:
							rx_pwr_all = -42 + 2*(7-VGA_idx); //VGA_idx = 7~5
						break;
					case 4:
							rx_pwr_all = -36 + 2*(7-VGA_idx); //VGA_idx = 7~4
						break;
					case 3:
							//rx_pwr_all = -28 + 2*(7-VGA_idx); //VGA_idx = 7~0
							rx_pwr_all = -24 + 2*(7-VGA_idx); //VGA_idx = 7~0
						break;
					case 2:
						if(cck_highpwr)
							rx_pwr_all = -12 + 2*(5-VGA_idx); //VGA_idx = 5~0
						else
							rx_pwr_all = -6+ 2*(5-VGA_idx);
						break;
					case 1:
							rx_pwr_all = 8-2*VGA_idx;
						break;
					case 0:
							rx_pwr_all = 14-2*VGA_idx;
						break;
					default:
						//DbgPrint("CCK Exception default\n");
						break;
				}
				rx_pwr_all += 6;
				PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
				if(cck_highpwr == FALSE)
				{
					if(PWDB_ALL >= 80)
						PWDB_ALL = ((PWDB_ALL-80)<<1)+((PWDB_ALL-80)>>1)+80;
					else if((PWDB_ALL <= 78) && (PWDB_ALL >= 20))
						PWDB_ALL += 3;
					if(PWDB_ALL>100)
						PWDB_ALL = 100;
				}
			}
			else if(pDM_Odm->SupportICType & (ODM_RTL8723B))
			{
				#if (RTL8723B_SUPPORT == 1)			
				rx_pwr_all = odm_CCKRSSI_8723B(LNA_idx,VGA_idx);
				PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
				if(PWDB_ALL>100)
					PWDB_ALL = 100;	
				#endif				
			}
		}
		else
		{
			if(!cck_highpwr)
			{			
				report =( cck_agc_rpt & 0xc0 )>>6;
				switch(report)
				{
					// 03312009 modified by cosa
					// Modify the RF RNA gain value to -40, -20, -2, 14 by Jenyu's suggestion
					// Note: different RF with the different RNA gain.
					case 0x3:
						rx_pwr_all = -46 - (cck_agc_rpt & 0x3e);
						break;
					case 0x2:
						rx_pwr_all = -26 - (cck_agc_rpt & 0x3e);
						break;
					case 0x1:
						rx_pwr_all = -12 - (cck_agc_rpt & 0x3e);
						break;
					case 0x0:
						rx_pwr_all = 16 - (cck_agc_rpt & 0x3e);
						break;
				}
			}
			else
			{
				//report = pDrvInfo->cfosho[0] & 0x60;			
				//report = pPhyStaRpt->cck_agc_rpt_ofdm_cfosho_a& 0x60;
				
				report = (cck_agc_rpt & 0x60)>>5;
				switch(report)
				{
					case 0x3:
						rx_pwr_all = -46 - ((cck_agc_rpt & 0x1f)<<1) ;
						break;
					case 0x2:
						rx_pwr_all = -26 - ((cck_agc_rpt & 0x1f)<<1);
						break;
					case 0x1:
						rx_pwr_all = -12 - ((cck_agc_rpt & 0x1f)<<1) ;
						break;
					case 0x0:
						rx_pwr_all = 16 - ((cck_agc_rpt & 0x1f)<<1) ;
						break;
				}
			}

			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);

			//Modification for ext-LNA board
			if(pDM_Odm->BoardType & ODM_BOARD_EXT_LNA)
			{
				if((cck_agc_rpt>>7) == 0){
					PWDB_ALL = (PWDB_ALL>94)?100:(PWDB_ALL +6);
				}
				else	
	                   {
					if(PWDB_ALL > 38)
						PWDB_ALL -= 16;
					else
						PWDB_ALL = (PWDB_ALL<=16)?(PWDB_ALL>>2):(PWDB_ALL -12);
				}             

				//CCK modification
				if(PWDB_ALL > 25 && PWDB_ALL <= 60)
					PWDB_ALL += 6;
				//else if (PWDB_ALL <= 25)
				//	PWDB_ALL += 8;
			}
			else//Modification for int-LNA board
			{
				if(PWDB_ALL > 99)
				  	PWDB_ALL -= 8;
				else if(PWDB_ALL > 50 && PWDB_ALL <= 68)
					PWDB_ALL += 4;
			}
		}
	
		pPhyInfo->RxPWDBAll = PWDB_ALL;
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
#endif		
		//
		// (3) Get Signal Quality (EVM)
		//
		if(pPktinfo->bPacketMatchBSSID)
		{
			u1Byte	SQ,SQ_rpt;			
			
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			if((pDM_Odm->SupportPlatform == ODM_WIN) &&
				(pDM_Odm->PatchID==RT_CID_819x_Lenovo)){
				SQ = odm_SQ_process_patch_RT_CID_819x_Lenovo(pDM_Odm,isCCKrate,PWDB_ALL,0,0);
			}else if((pDM_Odm->SupportPlatform == ODM_WIN) &&
				(pDM_Odm->PatchID==RT_CID_819x_Acer))
			{
				SQ = odm_SQ_process_patch_RT_CID_819x_Acer(pDM_Odm,isCCKrate,PWDB_ALL,0,0);
			}else
#endif
			if(pPhyInfo->RxPWDBAll > 40 && !pDM_Odm->bInHctTest){
				SQ = 100;
			}
			else{						
				SQ_rpt = pPhyStaRpt->cck_sig_qual_ofdm_pwdb_all;
					
				if(SQ_rpt > 64)
					SQ = 0;
				else if (SQ_rpt < 20)
					SQ = 100;
				else
					SQ = ((64-SQ_rpt) * 100) / 44;
			
			}
			
			//DbgPrint("cck SQ = %d\n", SQ);
			pPhyInfo->SignalQuality = SQ;
			pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = SQ;
			pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_B] = -1;
		}

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++)   
		{
			if (i==0)
				pPhyInfo->RxMIMOSignalStrength[0]=PWDB_ALL;
			else
				pPhyInfo->RxMIMOSignalStrength[1]=0;
		}
	}
	else //2 is OFDM rate
	{
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusOFDM++;

		// 
		// (1)Get RSSI for HT rate
		//
		
       	 for(i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++)   
		{
			// 2008/01/30 MH we will judge RF RX path now.
			if (pDM_Odm->RFPathRxEnable & BIT(i))
				rf_rx_num++;
			//else
				//continue;

			rx_pwr[i] = ((pPhyStaRpt->path_agc[i].gain& 0x3F)*2) - 110;
			
		#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
			pPhyInfo->RxPwr[i] = rx_pwr[i];
		#endif	

			/* Translate DBM to percentage. */
			RSSI = odm_QueryRxPwrPercentage(rx_pwr[i]);	
			total_rssi += RSSI;
			//RTPRINT(FRX, RX_PHY_SS, ("RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI));
		if(pDM_Odm->SupportICType & ODM_RTL8192C)
		{
			//Modification for ext-LNA board
			if(pDM_Odm->BoardType & ODM_BOARD_EXT_LNA)
			{
				if((pPhyStaRpt->path_agc[i].trsw) == 1)
					RSSI = (RSSI>94)?100:(RSSI +6);
				else
					RSSI = (RSSI<=16)?(RSSI>>3):(RSSI -16);

				if((RSSI <= 34) && (RSSI >=4))
					RSSI -= 4;
			}
		}
		
			pPhyInfo->RxMIMOSignalStrength[i] =(u1Byte) RSSI;

		#if (DM_ODM_SUPPORT_TYPE &  (/*ODM_WIN|*/ODM_CE|ODM_AP|ODM_ADSL))
			//Get Rx snr value in DB		
			pPhyInfo->RxSNR[i] = pDM_Odm->PhyDbgInfo.RxSNRdB[i] = (s4Byte)(pPhyStaRpt->path_rxsnr[i]/2);
		#endif
		
			/* Record Signal Strength for next packet */
			if(pPktinfo->bPacketMatchBSSID)
			{				
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
				if((pDM_Odm->SupportPlatform == ODM_WIN) &&
					(pDM_Odm->PatchID==RT_CID_819x_Lenovo))
				{
					if(i==ODM_RF_PATH_A)
						pPhyInfo->SignalQuality = odm_SQ_process_patch_RT_CID_819x_Lenovo(pDM_Odm,isCCKrate,PWDB_ALL,i,RSSI);
				
				}		
				else if((pDM_Odm->SupportPlatform == ODM_WIN) &&
					(pDM_Odm->PatchID==RT_CID_819x_Acer))
				{
					pPhyInfo->SignalQuality = odm_SQ_process_patch_RT_CID_819x_Acer(pDM_Odm,isCCKrate,PWDB_ALL,0,RSSI);
				}	
#endif
			}
		}
		
		
		//
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//
		rx_pwr_all = (((pPhyStaRpt->cck_sig_qual_ofdm_pwdb_all) >> 1 )& 0x7f) -110;		
		
		PWDB_ALL_BT = PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);	
	
	
		pPhyInfo->RxPWDBAll = PWDB_ALL;
		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ODM OFDM RSSI=%d\n",pPhyInfo->RxPWDBAll));
	#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL_BT;
		pPhyInfo->RxPower = rx_pwr_all;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
	#endif
		
		if((pDM_Odm->SupportPlatform == ODM_WIN) &&(pDM_Odm->PatchID==19)){
			//do nothing	
		}
		else{//pMgntInfo->CustomerID != RT_CID_819x_Lenovo
			//
			// (3)EVM of HT rate
			//
			if(pPktinfo->DataRate >=ODM_RATEMCS8 && pPktinfo->DataRate <=ODM_RATEMCS15)
				Max_spatial_stream = 2; //both spatial stream make sense
			else
				Max_spatial_stream = 1; //only spatial stream 1 makes sense

			for(i=0; i<Max_spatial_stream; i++)
			{
				// Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment
				// fill most significant bit to "zero" when doing shifting operation which may change a negative 
				// value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore.			
				EVM = odm_EVMdbToPercentage( (pPhyStaRpt->stream_rxevm[i] ));	//dbm

				//GET_RX_STATUS_DESC_RX_MCS(pDesc), pDrvInfo->rxevm[i], "%", EVM));
				
				if(pPktinfo->bPacketMatchBSSID)
				{
					if(i==ODM_RF_PATH_A) // Fill value in RFD, Get the first spatial stream only
					{						
						pPhyInfo->SignalQuality = (u1Byte)(EVM & 0xff);
					}					
					pPhyInfo->RxMIMOSignalQuality[i] = (u1Byte)(EVM & 0xff);
				}
			}
		}

		ODM_ParsingCFO(pDM_Odm, pPktinfo, pPhyStaRpt->path_cfotail);

	}
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
	//UI BSS List signal strength(in percentage), make it good looking, from 0~100.
	//It is assigned to the BSS List in GetValueFromBeaconOrProbeRsp().
	if(isCCKrate)
	{		
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		// 2012/01/12 MH Use customeris signal strength from HalComRxdDesc.c/	
		if(pDM_Odm->PatchID == RT_CID_819x_Acer)
		{
			if(IS_HARDWARE_TYPE_8723BE(pDM_Odm->Adapter))
				pPhyInfo->SignalStrength = (u1Byte) (PWDB_ALL - 4);
			else if(IS_HARDWARE_TYPE_8188EE(pDM_Odm->Adapter))
				pPhyInfo->SignalStrength = (u1Byte) (PWDB_ALL + 6);
			else
				pPhyInfo->SignalStrength = (u1Byte) (PWDB_ALL - 4);
		}
		else
		pPhyInfo->SignalStrength = (u1Byte)(SignalScaleMapping(pDM_Odm->Adapter, PWDB_ALL));//PWDB_ALL;
#else
		pPhyInfo->SignalStrength = (u1Byte)(odm_SignalScaleMapping(pDM_Odm, PWDB_ALL));//PWDB_ALL;
#endif
	}
	else
	{	
		if (rf_rx_num != 0)
		{			
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			// 2012/01/12 MH Use customeris signal strength from HalComRxdDesc.c/	
			if(pDM_Odm->PatchID == RT_CID_819x_Acer)
				pPhyInfo->SignalStrength = (u1Byte) (total_rssi/=rf_rx_num);
			else
				pPhyInfo->SignalStrength = (u1Byte)(SignalScaleMapping(pDM_Odm->Adapter, total_rssi/=rf_rx_num));//PWDB
#else
			pPhyInfo->SignalStrength = (u1Byte)(odm_SignalScaleMapping(pDM_Odm, total_rssi/=rf_rx_num));
#endif
		}
	}
#endif

	//DbgPrint("pPhyInfo->RxPWDBAll=%d\n", pPhyInfo->RxPWDBAll);
	//DbgPrint("pPhyInfo->SignalStrength=%d\n", pPhyInfo->SignalStrength);
	//DbgPrint("isCCKrate = %d, pPhyInfo->RxPWDBAll = %d, pPhyStaRpt->cck_agc_rpt_ofdm_cfosho_a = 0x%x\n", 
		//isCCKrate, pPhyInfo->RxPWDBAll, pPhyStaRpt->cck_agc_rpt_ofdm_cfosho_a);

	//For 92C/92D HW (Hybrid) Antenna Diversity
#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))	
	pDM_SWAT_Table->antsel = pPhyStaRpt->ant_sel;
	//For 88E HW Antenna Diversity
	pDM_Odm->DM_FatTable.antsel_rx_keep_0 = pPhyStaRpt->ant_sel;
	pDM_Odm->DM_FatTable.antsel_rx_keep_1 = pPhyStaRpt->ant_sel_b;
	pDM_Odm->DM_FatTable.antsel_rx_keep_2 = pPhyStaRpt->antsel_rx_keep_2;
#endif
}
#endif

#if	ODM_IC_11AC_SERIES_SUPPORT

VOID
odm_RxPhyBWJaguarSeries_Parsing(
	OUT		PODM_PHY_INFO_T			pPhyInfo,
	IN		PODM_PACKET_INFO_T			pPktinfo,
	IN		PPHY_STATUS_RPT_8812_T 		pPhyStaRpt
	)
{

	if(pPktinfo->DataRate <= ODM_RATE54M)
	{
		switch(pPhyStaRpt->r_RFMOD){
			case 1:
				if(pPhyStaRpt->sub_chnl == 0)
					pPhyInfo->BandWidth = 1;
				else
					pPhyInfo->BandWidth = 0;
				break;

			case 2:
				if(pPhyStaRpt->sub_chnl == 0)
					pPhyInfo->BandWidth = 2;
				else if(pPhyStaRpt->sub_chnl == 9 || pPhyStaRpt->sub_chnl == 10)
					pPhyInfo->BandWidth = 1;
				else 
					pPhyInfo->BandWidth = 0;
				break;

			default:	case 0:
				pPhyInfo->BandWidth = 0;
				break;			
		}	
	}

}

VOID
odm_RxPhyStatusJaguarSeries_Parsing(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T			pPhyInfo,		
	IN 		pu1Byte						pPhyStatus,
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{							
	SWAT_T					*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u1Byte					i, Max_spatial_stream;
	s1Byte					rx_pwr[4], rx_pwr_all=0;
	u1Byte					EVM, EVMdbm, PWDB_ALL = 0, PWDB_ALL_BT;
	u1Byte					RSSI, total_rssi=0;
	u1Byte					isCCKrate=0;	
	u1Byte					rf_rx_num = 0;
	u1Byte					cck_highpwr = 0;
	u1Byte					LNA_idx, VGA_idx;
	PPHY_STATUS_RPT_8812_T pPhyStaRpt = (PPHY_STATUS_RPT_8812_T)pPhyStatus;

	if(pDM_Odm->SupportICType == ODM_RTL8812 || pDM_Odm->SupportICType == ODM_RTL8821)
		odm_RxPhyBWJaguarSeries_Parsing(pPhyInfo, pPktinfo, pPhyStaRpt);

	if((pPktinfo->DataRate >= ODM_RATE1M ) &&	(pPktinfo->DataRate <= ODM_RATE11M ))
		isCCKrate = TRUE;
	else
		isCCKrate = FALSE;
	
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = -1;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_B] = -1;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_C] = -1;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_D] = -1;

	if(isCCKrate)
	{
		u1Byte cck_agc_rpt;
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusCCK++;
		// 
		// (1)Hardware does not provide RSSI for CCK
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//

		//if(pHalData->eRFPowerState == eRfOn)
			cck_highpwr = pDM_Odm->bCckHighPower;
		//else
		//	cck_highpwr = FALSE;

		cck_agc_rpt =  pPhyStaRpt->cfosho[0] ;
		LNA_idx = ((cck_agc_rpt & 0xE0) >>5);
		VGA_idx = (cck_agc_rpt & 0x1F); 
		
		if(pDM_Odm->SupportICType == ODM_RTL8812)
		{
			switch(LNA_idx)
			{
				case 7:
					if(VGA_idx <= 27)
						rx_pwr_all = -100 + 2*(27-VGA_idx); //VGA_idx = 27~2
					else
						rx_pwr_all = -100;
					break;
				case 6:
						rx_pwr_all = -48 + 2*(2-VGA_idx); //VGA_idx = 2~0
					break;
				case 5:
						rx_pwr_all = -42 + 2*(7-VGA_idx); //VGA_idx = 7~5
					break;
				case 4:
						rx_pwr_all = -36 + 2*(7-VGA_idx); //VGA_idx = 7~4
					break;
				case 3:
						//rx_pwr_all = -28 + 2*(7-VGA_idx); //VGA_idx = 7~0
						rx_pwr_all = -24 + 2*(7-VGA_idx); //VGA_idx = 7~0
					break;
				case 2:
					if(cck_highpwr)
						rx_pwr_all = -12 + 2*(5-VGA_idx); //VGA_idx = 5~0
					else
						rx_pwr_all = -6+ 2*(5-VGA_idx);
					break;
				case 1:
						rx_pwr_all = 8-2*VGA_idx;
					break;
				case 0:
						rx_pwr_all = 14-2*VGA_idx;
					break;
				default:
					//DbgPrint("CCK Exception default\n");
					break;
			}
			rx_pwr_all += 6;
			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
			
			if(cck_highpwr == FALSE)
			{
				if(PWDB_ALL >= 80)
					PWDB_ALL = ((PWDB_ALL-80)<<1)+((PWDB_ALL-80)>>1)+80;
				else if((PWDB_ALL <= 78) && (PWDB_ALL >= 20))
					PWDB_ALL += 3;
				if(PWDB_ALL>100)
					PWDB_ALL = 100;
			}
		}
		else if(pDM_Odm->SupportICType & (ODM_RTL8821|ODM_RTL8881A))
		{
			s1Byte Pout = -6;

			switch(LNA_idx)
				{
				case 5:
					rx_pwr_all = Pout -32 -(2*VGA_idx);
						break;
				case 4:
					rx_pwr_all = Pout -24 -(2*VGA_idx);
						break;
				case 2:
					rx_pwr_all = Pout -11 -(2*VGA_idx);
						break;
				case 1:
					rx_pwr_all = Pout + 5 -(2*VGA_idx);
						break;
				case 0:
					rx_pwr_all = Pout + 21 -(2*VGA_idx);
						break;
				}
			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
		}
		else if(pDM_Odm->SupportICType == ODM_RTL8814A)
		{	
			s1Byte Pout = -6;
			
			switch(LNA_idx)
			{
				// CCK only use LNA: 2, 3, 5, 7
				case 7:
					rx_pwr_all = Pout -32 - (2*VGA_idx);
					break;
				case 5:
					rx_pwr_all = Pout -22 - (2*VGA_idx);
					break;
				case 3:
					rx_pwr_all = Pout - 2 - (2*VGA_idx);
					break;
				case 2:
					rx_pwr_all = Pout + 5 - (2*VGA_idx);	
					break;
				//case 6:
				//	rx_pwr_all = Pout -26 - (2*VGA_idx);
				//	break;
				//case 4:
				//	rx_pwr_all = Pout - 8 - (2*VGA_idx);
				//	break;
				//case 1:
				//	rx_pwr_all = Pout + 21 - (2*VGA_idx);
				//	break;
				//case 0:
				//	rx_pwr_all = Pout + 10 - (2*VGA_idx);
				//	break;
				default:
					//DbgPrint("CCK Exception default\n");
					break;
			}
			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
		}
	
		pPhyInfo->RxPWDBAll = PWDB_ALL;
		//if(pPktinfo->StationID == 0)
		//{
		//	DbgPrint("CCK: LNA_idx = %d, VGA_idx = %d, pPhyInfo->RxPWDBAll = %d\n", 
		//		LNA_idx, VGA_idx, pPhyInfo->RxPWDBAll);
		//}
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
#endif		
		//
		// (3) Get Signal Quality (EVM)
		//
		if(pPktinfo->bPacketMatchBSSID)
		{
			u1Byte	SQ,SQ_rpt;			
			
			if((pDM_Odm->SupportPlatform == ODM_WIN) &&
                                 (pDM_Odm->PatchID==RT_CID_819x_Lenovo))
			{
				SQ = odm_SQ_process_patch_RT_CID_819x_Lenovo(pDM_Odm,isCCKrate,PWDB_ALL,0,0);
			}
			else if(pPhyInfo->RxPWDBAll > 40 && !pDM_Odm->bInHctTest)
			{
				SQ = 100;
			}
			else
			{						
				SQ_rpt = pPhyStaRpt->pwdb_all;
					
				if(SQ_rpt > 64)
					SQ = 0;
				else if (SQ_rpt < 20)
					SQ = 100;
				else
					SQ = ((64-SQ_rpt) * 100) / 44;
			}
			
			//DbgPrint("cck SQ = %d\n", SQ);
			pPhyInfo->SignalQuality = SQ;
			pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = SQ;
		}

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++)   
		{
			if (i==0)
				pPhyInfo->RxMIMOSignalStrength[0]=PWDB_ALL;
			else
				pPhyInfo->RxMIMOSignalStrength[i]=0;
		}
	}
	else //is OFDM rate
	{
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusOFDM++;

		// 
		// (1)Get RSSI for OFDM rate
		//
		
		for(i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++)   
		{
			// 2008/01/30 MH we will judge RF RX path now.
			//DbgPrint("pDM_Odm->RFPathRxEnable = %x\n", pDM_Odm->RFPathRxEnable);
			if (pDM_Odm->RFPathRxEnable & BIT(i))
			{				
				rf_rx_num++;
			}
			//else
				//continue;
			//2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip
			//if((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8821)) && (!pDM_Odm->bIsMPChip))
			if(i < ODM_RF_PATH_C)
				rx_pwr[i] = (pPhyStaRpt->gain_trsw[i] & 0x7F) - 110;
			else
				rx_pwr[i] = (pPhyStaRpt->gain_trsw_cd[i - 2] & 0x7F) - 110;
			//else
			//	rx_pwr[i] = ((pPhyStaRpt->gain_trsw[i]& 0x3F)*2) - 110;  //OLD FORMULA

		#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
			pPhyInfo->RxPwr[i] = rx_pwr[i];
		#endif	

			/* Translate DBM to percentage. */
			RSSI = odm_QueryRxPwrPercentage(rx_pwr[i]);	
		
			total_rssi += RSSI;
			//RTPRINT(FRX, RX_PHY_SS, ("RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI));

			pPhyInfo->RxMIMOSignalStrength[i] =(u1Byte) RSSI;

		#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE|ODM_AP|ODM_ADSL))
			//Get Rx snr value in DB		
			if(i < ODM_RF_PATH_C)
				pPhyInfo->RxSNR[i] = pDM_Odm->PhyDbgInfo.RxSNRdB[i] = pPhyStaRpt->rxsnr[i]/2;
			else if(pDM_Odm->SupportICType & ODM_RTL8814A)
				pPhyInfo->RxSNR[i] = pDM_Odm->PhyDbgInfo.RxSNRdB[i] = pPhyStaRpt->csi_current[i - 2]/2;
		#endif
		
			/* Record Signal Strength for next packet */
			if(pPktinfo->bPacketMatchBSSID)
			{				
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
				if((pDM_Odm->SupportPlatform == ODM_WIN) &&
					(pDM_Odm->PatchID==RT_CID_819x_Lenovo))
				{
					if(i==ODM_RF_PATH_A)
						pPhyInfo->SignalQuality = odm_SQ_process_patch_RT_CID_819x_Lenovo(pDM_Odm,isCCKrate,PWDB_ALL,i,RSSI);
				
				}		
#endif
			}
		}
		
		
		//
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//
		//2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip
		if((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8821|ODM_RTL8881A)) && (!pDM_Odm->bIsMPChip))
			rx_pwr_all = (pPhyStaRpt->pwdb_all& 0x7f) -110;
		else
			rx_pwr_all = (((pPhyStaRpt->pwdb_all) >> 1 )& 0x7f) -110;	 //OLD FORMULA

		PWDB_ALL_BT = PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);	
			
		pPhyInfo->RxPWDBAll = PWDB_ALL;
		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ODM OFDM RSSI=%d\n",pPhyInfo->RxPWDBAll));
	#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL_BT;
		pPhyInfo->RxPower = rx_pwr_all;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
	#endif
		
		if((pDM_Odm->SupportPlatform == ODM_WIN) &&(pDM_Odm->PatchID==19))
		{
			//do nothing	
		}
		else
		{//pMgntInfo->CustomerID != RT_CID_819x_Lenovo
			//
			// (3)EVM of HT rate
			//
			if(	(pPktinfo->DataRate >= ODM_RATEMCS8) &&
		 		(pPktinfo->DataRate <= ODM_RATEMCS15))
		 		Max_spatial_stream = 2;
			else if(	(pPktinfo->DataRate >= ODM_RATEVHTSS2MCS0) &&
		 		(pPktinfo->DataRate <= ODM_RATEVHTSS2MCS9))
		 		Max_spatial_stream = 2;
			else if(	(pPktinfo->DataRate >= ODM_RATEMCS16) &&
				(pPktinfo->DataRate <= ODM_RATEMCS23))
				Max_spatial_stream = 3;
			else if(	(pPktinfo->DataRate >= ODM_RATEVHTSS3MCS0) &&
				(pPktinfo->DataRate <= ODM_RATEVHTSS3MCS9))
				Max_spatial_stream = 3;
			else
				Max_spatial_stream = 1;

			for(i=0; i<Max_spatial_stream; i++)
			{
				// Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment
				// fill most significant bit to "zero" when doing shifting operation which may change a negative 
				// value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore.			
				if(i < ODM_RF_PATH_C)
					EVM = odm_EVMdbToPercentage( (pPhyStaRpt->rxevm[i] ));	//dbm
				else
					EVM = odm_EVMdbToPercentage( (pPhyStaRpt->rxevm_cd[i - 2] ));	//dbm

				//RTPRINT(FRX, RX_PHY_SQ, ("RXRATE=%x RXEVM=%x EVM=%s%d\n", 
				//GET_RX_STATUS_DESC_RX_MCS(pDesc), pDrvInfo->rxevm[i], "%", EVM));
				
				if(pPktinfo->bPacketMatchBSSID)
				{
					if(i==ODM_RF_PATH_A) // Fill value in RFD, Get the first spatial stream only
					{						
						pPhyInfo->SignalQuality = (u1Byte)(EVM & 0xff);
					}					
					pPhyInfo->RxMIMOSignalQuality[i] = (u1Byte)(EVM & 0xff);
				}
			}
		}

		ODM_ParsingCFO(pDM_Odm, pPktinfo, pPhyStaRpt->cfotail);

	}
	//DbgPrint("isCCKrate= %d, pPhyInfo->SignalStrength=%d % PWDB_AL=%d rf_rx_num=%d\n", isCCKrate, pPhyInfo->SignalStrength, PWDB_ALL, rf_rx_num);
	
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
	//UI BSS List signal strength(in percentage), make it good looking, from 0~100.
	//It is assigned to the BSS List in GetValueFromBeaconOrProbeRsp().
	if(isCCKrate)
	{		
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		// 2012/01/12 MH Use customeris signal strength from HalComRxdDesc.c/	
		pPhyInfo->SignalStrength = (u1Byte)(SignalScaleMapping(pDM_Odm->Adapter, PWDB_ALL));//PWDB_ALL;
#else
		pPhyInfo->SignalStrength = (u1Byte)(odm_SignalScaleMapping(pDM_Odm, PWDB_ALL));//PWDB_ALL;
#endif
	}
	else
	{	
		if (rf_rx_num != 0)
		{			
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			// 2012/01/12 MH Use customeris signal strength from HalComRxdDesc.c/	
			pPhyInfo->SignalStrength = (u1Byte)(SignalScaleMapping(pDM_Odm->Adapter, total_rssi/=rf_rx_num));//PWDB_ALL;
#else
			pPhyInfo->SignalStrength = (u1Byte)(odm_SignalScaleMapping(pDM_Odm, total_rssi/=rf_rx_num));
#endif
		}
	}
#endif
	pDM_Odm->RxPWDBAve = pDM_Odm->RxPWDBAve + pPhyInfo->RxPWDBAll;

	pDM_Odm->DM_FatTable.antsel_rx_keep_0 = pPhyStaRpt->antidx_anta;
	pDM_Odm->DM_FatTable.antsel_rx_keep_1 = pPhyStaRpt->antidx_antb;
	pDM_Odm->DM_FatTable.antsel_rx_keep_2 = pPhyStaRpt->antidx_antc;
	pDM_Odm->DM_FatTable.antsel_rx_keep_3 = pPhyStaRpt->antidx_antd;
	//DbgPrint("----------------------------\n");
	//DbgPrint("pPktinfo->StationID=%d, pPktinfo->DataRate=0x%x\n",pPktinfo->StationID, pPktinfo->DataRate);
	//DbgPrint("pPhyStaRpt->gain_trsw[0]=0x%x, pPhyStaRpt->gain_trsw[1]=0x%x, pPhyStaRpt->pwdb_all=0x%x\n",
	//			pPhyStaRpt->gain_trsw[0],pPhyStaRpt->gain_trsw[1], pPhyStaRpt->pwdb_all);
	//DbgPrint("pPhyInfo->RxMIMOSignalStrength[0]=%d, pPhyInfo->RxMIMOSignalStrength[1]=%d, RxPWDBAll=%d\n",
	//			pPhyInfo->RxMIMOSignalStrength[0], pPhyInfo->RxMIMOSignalStrength[1], pPhyInfo->RxPWDBAll);

}

#endif

VOID
odm_Init_RSSIForDM(
	IN OUT	PDM_ODM_T	pDM_Odm
	)
{

}

VOID
odm_Process_RSSIForDM(	
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN		PODM_PHY_INFO_T			pPhyInfo,
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{
	
	s4Byte			UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK, UndecoratedSmoothedOFDM, RSSI_Ave;
	u1Byte			i, isCCKrate=0;	
	u1Byte			RSSI_max, RSSI_min;
	u4Byte			OFDM_pkt=0; 
	u4Byte			Weighting=0;
	PSTA_INFO_T           	pEntry;
	
	if(pPktinfo->StationID == 0xFF)
		return;

#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	odm_S0S1_SwAntDivByCtrlFrame_ProcessRSSI(pDM_Odm, pPhyInfo, pPktinfo);
#endif
#endif

	//
	// 2012/05/30 MH/Luke.Lee Add some description 
	// In windows driver: AP/IBSS mode STA
	//
	//if (pDM_Odm->SupportPlatform == ODM_WIN)
	//{
	//	pEntry = pDM_Odm->pODM_StaInfo[pDM_Odm->pAidMap[pPktinfo->StationID-1]];			
	//}
	//else
		pEntry = pDM_Odm->pODM_StaInfo[pPktinfo->StationID];							

	if(!IS_STA_VALID(pEntry) )
	{		
		return;
	}

	if((!pPktinfo->bPacketMatchBSSID) )
	{
		return;
	}


        isCCKrate = ((pPktinfo->DataRate >= ODM_RATE1M ) && (pPktinfo->DataRate <= ODM_RATE11M ))?TRUE :FALSE;

	//--------------Statistic for antenna/path diversity------------------
        if(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)
	{
		#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
			ODM_Process_RSSIForAntDiv(pDM_Odm,pPhyInfo,pPktinfo);
		#endif
	}
	#if(DM_ODM_SUPPORT_TYPE == ODM_WIN)
	else if(pDM_Odm->SupportAbility & ODM_BB_PATH_DIV)
	{
		#if (RTL8812A_SUPPORT == 1)
		if(pDM_Odm->SupportICType == ODM_RTL8812)
		{
			pPATHDIV_T	pDM_PathDiv = &pDM_Odm->DM_PathDiv;
			if(pPktinfo->bPacketToSelf || pPktinfo->bPacketMatchBSSID)
			{
				if(pPktinfo->DataRate > ODM_RATE11M)
					ODM_PathStatistics_8812A(pDM_Odm, pPktinfo->StationID, pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A], 
					                                                                                                      pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B]);
			}
		}
		#endif
	}
	#endif
	//-----------------Smart Antenna Debug Message------------------//
	
	UndecoratedSmoothedCCK =  pEntry->rssi_stat.UndecoratedSmoothedCCK;
	UndecoratedSmoothedOFDM = pEntry->rssi_stat.UndecoratedSmoothedOFDM;
	UndecoratedSmoothedPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;	
	
	if(pPktinfo->bPacketToSelf || pPktinfo->bPacketBeacon)
	{

		if(!isCCKrate)//ofdm rate
		{
			if(pDM_Odm->SupportICType & ODM_RTL8814A)
			{
				u1Byte RX_count = 0;
				u4Byte RSSI_linear = 0;

				// Find RSSI_min and RSSI_max
				// Calculate average RSSI
				for(i = 0; i < ODM_RF_PATH_MAX_JAGUAR; i++)
				{
					if(pPhyInfo->RxMIMOSignalStrength[i] == 0)
						continue;
				
					RX_count++;
					RSSI_linear += odm_ConvertTo_linear(pPhyInfo->RxMIMOSignalStrength[i]);
				}

				switch(RX_count)
				{
					case 2:
						RSSI_linear = (RSSI_linear >> 1);
						break;
					case 3:
						RSSI_linear = ((RSSI_linear) + (RSSI_linear << 1) + (RSSI_linear << 3)) >> 5;	// RSSI_linear/3 ~ RSSI_linear*11/32 
						break;
					case 4:
						RSSI_linear = (RSSI_linear >> 2);
						break;
				}			
				RSSI_Ave = odm_ConvertTo_dB(RSSI_linear);
			}
			else
			{
				if(pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B] == 0)
				{
					RSSI_Ave = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
				}
				else
				{
					//DbgPrint("pRfd->Status.RxMIMOSignalStrength[0] = %d, pRfd->Status.RxMIMOSignalStrength[1] = %d \n", 
						//pRfd->Status.RxMIMOSignalStrength[0], pRfd->Status.RxMIMOSignalStrength[1]);

				
					if(pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A] > pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B])
					{
						RSSI_max = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
						RSSI_min = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B];
					}
					else
					{
						RSSI_max = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B];
						RSSI_min = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
					}
					if((RSSI_max -RSSI_min) < 3)
						RSSI_Ave = RSSI_max;
					else if((RSSI_max -RSSI_min) < 6)
						RSSI_Ave = RSSI_max - 1;
					else if((RSSI_max -RSSI_min) < 10)
						RSSI_Ave = RSSI_max - 2;
					else
						RSSI_Ave = RSSI_max - 3;
				}
			}
					
			//1 Process OFDM RSSI
			if(UndecoratedSmoothedOFDM <= 0)	// initialize
			{
				UndecoratedSmoothedOFDM = pPhyInfo->RxPWDBAll;
			}
			else
			{
				if(pPhyInfo->RxPWDBAll > (u4Byte)UndecoratedSmoothedOFDM)
				{
					UndecoratedSmoothedOFDM = 	
							( ((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1)) + 
							(RSSI_Ave)) /(Rx_Smooth_Factor);
					UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM + 1;
				}
				else
				{
					UndecoratedSmoothedOFDM = 	
							( ((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1)) + 
							(RSSI_Ave)) /(Rx_Smooth_Factor);
				}
			}				
			if(pEntry->rssi_stat.OFDM_pkt != 64) {
				i = 63;
				pEntry->rssi_stat.OFDM_pkt -= (((pEntry->rssi_stat.PacketMap>>i)&BIT0)-1);
			}
			pEntry->rssi_stat.PacketMap = (pEntry->rssi_stat.PacketMap<<1) | BIT0;			
										
		}
		else
		{
			RSSI_Ave = pPhyInfo->RxPWDBAll;

			//1 Process CCK RSSI
			if(UndecoratedSmoothedCCK <= 0)	// initialize
			{
				UndecoratedSmoothedCCK = pPhyInfo->RxPWDBAll;
			}
			else
			{
				if(pPhyInfo->RxPWDBAll > (u4Byte)UndecoratedSmoothedCCK)
				{
					UndecoratedSmoothedCCK = 	
							( ((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1)) + 
							(pPhyInfo->RxPWDBAll)) /(Rx_Smooth_Factor);
					UndecoratedSmoothedCCK = UndecoratedSmoothedCCK + 1;
				}
				else
				{
					UndecoratedSmoothedCCK = 	
							( ((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1)) + 
							(pPhyInfo->RxPWDBAll)) /(Rx_Smooth_Factor);
				}
			}
			i = 63;
			pEntry->rssi_stat.OFDM_pkt -= ((pEntry->rssi_stat.PacketMap>>i)&BIT0);			
			pEntry->rssi_stat.PacketMap = pEntry->rssi_stat.PacketMap<<1;			
		}

		//if(pEntry)
		{
			//2011.07.28 LukeLee: modified to prevent unstable CCK RSSI
			if(pEntry->rssi_stat.OFDM_pkt == 64) { // speed up when all packets are OFDM
				UndecoratedSmoothedPWDB = UndecoratedSmoothedOFDM;
			}
			else {
				if(pEntry->rssi_stat.ValidBit < 64)
					pEntry->rssi_stat.ValidBit++;
					
				if(pEntry->rssi_stat.ValidBit == 64)
				{
					Weighting = ((pEntry->rssi_stat.OFDM_pkt<<4) > 64)?64:(pEntry->rssi_stat.OFDM_pkt<<4);
					UndecoratedSmoothedPWDB = (Weighting*UndecoratedSmoothedOFDM+(64-Weighting)*UndecoratedSmoothedCCK)>>6;
				}
				else
				{
					if(pEntry->rssi_stat.ValidBit != 0)
						UndecoratedSmoothedPWDB = (pEntry->rssi_stat.OFDM_pkt*UndecoratedSmoothedOFDM+(pEntry->rssi_stat.ValidBit-pEntry->rssi_stat.OFDM_pkt)*UndecoratedSmoothedCCK)/pEntry->rssi_stat.ValidBit;
					else
						UndecoratedSmoothedPWDB = 0;
				}
			}

			pEntry->rssi_stat.UndecoratedSmoothedCCK = UndecoratedSmoothedCCK;
			pEntry->rssi_stat.UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM;
			pEntry->rssi_stat.UndecoratedSmoothedPWDB = UndecoratedSmoothedPWDB;

			//DbgPrint("OFDM_pkt=%d, Weighting=%d\n", OFDM_pkt, Weighting);
			//DbgPrint("UndecoratedSmoothedOFDM=%d, UndecoratedSmoothedPWDB=%d, UndecoratedSmoothedCCK=%d\n", 
			//	UndecoratedSmoothedOFDM, UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK);
			
		}
	
	}
}


#if(ODM_IC_11N_SERIES_SUPPORT ==1)
//
// Endianness before calling this API
//
VOID
ODM_PhyStatusQuery_92CSeries(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T				pPhyInfo,
	IN 		pu1Byte						pPhyStatus,	
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{

	odm_RxPhyStatus92CSeries_Parsing(
							pDM_Odm,
							pPhyInfo,
							pPhyStatus,
							pPktinfo);

	if( pDM_Odm->RSSI_test == TRUE)
	{
		// Select the packets to do RSSI checking for antenna switching.
		if(pPktinfo->bPacketToSelf || pPktinfo->bPacketBeacon )
		{
				/*
			#if 0//(DM_ODM_SUPPORT_TYPE == ODM_WIN)
			dm_SWAW_RSSI_Check(
				Adapter, 
				(tmppAdapter!=NULL)?(tmppAdapter==Adapter):TRUE,
				bPacketMatchBSSID,
				pEntry,
				pRfd);
			#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
			// Select the packets to do RSSI checking for antenna switching.
			//odm_SwAntDivRSSICheck8192C(padapter, precvframe->u.hdr.attrib.RxPWDBAll);
			#endif
				*/
#if (RTL8192C_SUPPORT == 1)
				ODM_SwAntDivChkPerPktRssi(pDM_Odm,pPktinfo->StationID,pPhyInfo);
#endif
		}	
	}
	else
	{
		odm_Process_RSSIForDM(pDM_Odm,pPhyInfo,pPktinfo);
	}
	
}
#endif


//
// Endianness before calling this API
//
#if	ODM_IC_11AC_SERIES_SUPPORT

VOID
ODM_PhyStatusQuery_JaguarSeries(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T			pPhyInfo,
	IN 		pu1Byte						pPhyStatus,	
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{
	odm_RxPhyStatusJaguarSeries_Parsing(
							pDM_Odm,
							pPhyInfo,
							pPhyStatus,
							pPktinfo);
	
	odm_Process_RSSIForDM(pDM_Odm,pPhyInfo,pPktinfo);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	//phydm_sbd_check(pDM_Odm);
#endif
}
#endif

VOID
ODM_PhyStatusQuery(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T				pPhyInfo,
	IN 		pu1Byte						pPhyStatus,	
	IN		PODM_PACKET_INFO_T			pPktinfo
	)
{

#if	ODM_IC_11AC_SERIES_SUPPORT
	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES )
		ODM_PhyStatusQuery_JaguarSeries(pDM_Odm,pPhyInfo,pPhyStatus,pPktinfo);
#endif		

#if	ODM_IC_11N_SERIES_SUPPORT
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES )
		ODM_PhyStatusQuery_92CSeries(pDM_Odm,pPhyInfo,pPhyStatus,pPktinfo);
#endif
}
	
// For future use.
VOID
ODM_MacStatusQuery(
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN 		pu1Byte						pMacStatus,
	IN		u1Byte						MacID,	
	IN		BOOLEAN						bPacketMatchBSSID,
	IN		BOOLEAN						bPacketToSelf,
	IN		BOOLEAN						bPacketBeacon
	)
{
	// 2011/10/19 Driver team will handle in the future.
	
}


//
// If you want to add a new IC, Please follow below template and generate a new one.
// 
//

HAL_STATUS
ODM_ConfigRFWithHeaderFile(
	IN 	PDM_ODM_T	        	pDM_Odm,
	IN 	ODM_RF_RADIO_PATH_E 	Content,
	IN 	ODM_RF_RADIO_PATH_E 	eRFPath
    )
{

#if (RTL8723A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8723A)
	{
		if(eRFPath == ODM_RF_PATH_A)
			READ_AND_CONFIG_MP(8723A,_RadioA_1T);
		else if(eRFPath == ODM_RF_PATH_B)
			READ_AND_CONFIG_MP(8723A,_RadioB_1T);
	}
#endif

#if (RTL8188E_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		if(eRFPath == ODM_RF_PATH_A){
			READ_AND_CONFIG_MP(8188E,_RadioA);
		}
	}
#endif

#if (RTL8814A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8814A)
	{
		if(eRFPath == ODM_RF_PATH_A)
			READ_AND_CONFIG(8814A,_RadioA);
		else if(eRFPath == ODM_RF_PATH_B)
			READ_AND_CONFIG(8814A,_RadioB);
		else if(eRFPath == ODM_RF_PATH_C)
			READ_AND_CONFIG(8814A,_RadioC);
		else if(eRFPath == ODM_RF_PATH_D)
			READ_AND_CONFIG(8814A,_RadioD);
	}
#endif
/*
#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812)
	{
		if(eRFPath == ODM_RF_PATH_A)
			READ_AND_CONFIG_MP(8812A,_RadioA);
		else if(eRFPath == ODM_RF_PATH_B)
			READ_AND_CONFIG_MP(8812A,_RadioB);
	}
#endif

#if (RTL8881A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8881A)
	{
		if(eRFPath == ODM_RF_PATH_A)
			READ_AND_CONFIG_MP(8881A,_RadioA);
	}
#endif

#if (RTL8192E_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8192E)
	{
		if(eRFPath == ODM_RF_PATH_A)
			READ_AND_CONFIG_MP(8192E,_RadioA);
		else if(eRFPath == ODM_RF_PATH_B)
			READ_AND_CONFIG_MP(8192E,_RadioB);
	}
#endif
*/
#if defined(RTL8821A_SUPPORT) && (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821)
	{
		//if(eRFPath == ODM_RF_PATH_A)
			//READ_AND_CONFIG(8821A,_RadioA);
		//else if(eRFPath == ODM_RF_PATH_B)
			//READ_AND_CONFIG(8821A,_RadioB);
	}
#endif

#if (RTL8192E_SUPPORT == 1)
    // TODO: Korden
#endif


	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("<===ODM_ConfigRFWithHeaderFile\n"));
	return HAL_STATUS_SUCCESS;
}

HAL_STATUS
ODM_ConfigRFWithTxPwrTrackHeaderFile(
	IN 	PDM_ODM_T	        	pDM_Odm
    )
{
   	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, 
		 		 ("===>ODM_ConfigRFWithTxPwrTrackHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
   	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, 
				 ("pDM_Odm->SupportPlatform: 0x%X, pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
				 pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface, pDM_Odm->BoardType));
	
#if RTL8814A_SUPPORT
	if(pDM_Odm->SupportICType == ODM_RTL8814A) 
	{	  
		if(pDM_Odm->RFEType == 0)
			READ_AND_CONFIG_MP(8814A,_TxPowerTrack_Type0);
		else if(pDM_Odm->RFEType == 2)
			READ_AND_CONFIG_MP(8814A,_TxPowerTrack_Type2);
		else		  
			READ_AND_CONFIG_MP(8814A,_TxPowerTrack);
	}
#endif	

	return HAL_STATUS_SUCCESS;
}

HAL_STATUS
ODM_ConfigBBWithHeaderFile(
	IN 	PDM_ODM_T	             	pDM_Odm,
	IN 	ODM_BB_Config_Type 		ConfigType
	)
{

#if (RTL8723A_SUPPORT == 1) 
    if(pDM_Odm->SupportICType == ODM_RTL8723A)
	{
		if(ConfigType == CONFIG_BB_PHY_REG)
		{
			READ_AND_CONFIG_MP(8723A,_PHY_REG_1T);
		}
		else if(ConfigType == CONFIG_BB_AGC_TAB)
		{
			READ_AND_CONFIG_MP(8723A,_AGC_TAB_1T);
		}
	}		
#endif

#if (RTL8188E_SUPPORT == 1)
    if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		if(ConfigType == CONFIG_BB_PHY_REG)
		{
			READ_AND_CONFIG_MP(8188E,_PHY_REG);
		}
		else if(ConfigType == CONFIG_BB_AGC_TAB)
		{
			READ_AND_CONFIG_MP(8188E,_AGC_TAB);
		}
	}
#endif

#if (RTL8814A_SUPPORT == 1)
    if(pDM_Odm->SupportICType == ODM_RTL8814A)
	{

		if(ConfigType == CONFIG_BB_PHY_REG)
		{
			READ_AND_CONFIG(8814A,_PHY_REG);
		}
		else if(ConfigType == CONFIG_BB_AGC_TAB)
		{
			READ_AND_CONFIG(8814A,_AGC_TAB);
		}
		else if(ConfigType == CONFIG_BB_PHY_REG_MP)
		{
			READ_AND_CONFIG(8814A,_PHY_REG_MP);
		}
	}
#endif

/*
#if (RTL8812A_SUPPORT == 1) 
	if(pDM_Odm->SupportICType == ODM_RTL8812)
	{
		if(ConfigType == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8812A,_PHY_REG);
		else if(ConfigType == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8812A,_AGC_TAB);
		else if(ConfigType == CONFIG_BB_PHY_REG_MP)
			READ_AND_CONFIG_MP(8812A,_PHY_REG_MP);
	}		
#endif

#if (RTL8881A_SUPPORT == 1)

	if(pDM_Odm->SupportICType == ODM_RTL8881A)
	{
		if(ConfigType == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8881A,_PHY_REG);
		else if(ConfigType == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8881A,_AGC_TAB);
	}

#endif

#if (RTL8192E_SUPPORT == 1) 
	if(pDM_Odm->SupportICType == ODM_RTL8192E)
	{
	
		if(ConfigType == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8192E,_PHY_REG);
		else if(ConfigType == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8192E,_AGC_TAB);
	}		
#endif
*/
#if defined(RTL8821A_SUPPORT) && (RTL8821A_SUPPORT == 1)
	if(pDM_Odm->SupportICType == ODM_RTL8812)
	{
		if(ConfigType == CONFIG_BB_PHY_REG)
		{
			//READ_AND_CONFIG(8821A,_PHY_REG);
		}
		else if(ConfigType == CONFIG_BB_AGC_TAB)
		{
			//READ_AND_CONFIG(8821A,_AGC_TAB);
		}
		else if(ConfigType == CONFIG_BB_PHY_REG_PG)
		{
			//READ_AND_CONFIG(8821A,_PHY_REG_PG);
		}
	}		
#endif

	return HAL_STATUS_SUCCESS; 
}                 

HAL_STATUS
ODM_ConfigMACWithHeaderFile(
	IN 	PDM_ODM_T	pDM_Odm
	)
{

#if (RTL8723A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8723A)
	{
		READ_AND_CONFIG_MP(8723A,_MAC_REG);
	}
#endif

#if (RTL8188E_SUPPORT == 1)  
	if (pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		READ_AND_CONFIG_MP(8188E,_MAC_REG);
	}
#endif

#if (RTL8814A_SUPPORT == 1)  
	if (pDM_Odm->SupportICType == ODM_RTL8814A)
	{
		READ_AND_CONFIG(8814A,_MAC_REG);
	}
#endif
/*
#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812)
	{
		READ_AND_CONFIG_MP(8812A,_MAC_REG);
	}
#endif
#if (RTL8881A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8881A)
	{
		READ_AND_CONFIG_MP(8881A,_MAC_REG);
	}	
#endif
#if (RTL8192E_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8192E)
	{
		READ_AND_CONFIG_MP(8192E,_MAC_REG);
	}
#endif
*/
#if defined(RTL8821A_SUPPORT) && (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821)
	{
		//READ_AND_CONFIG(8821A,_MAC_REG);
	}
#endif
	return HAL_STATUS_SUCCESS;    
} 

HAL_STATUS
ODM_ConfigFWWithHeaderFile(
	IN 	PDM_ODM_T			pDM_Odm,
	IN 	ODM_FW_Config_Type 	ConfigType,
	OUT u1Byte				*pFirmware,
	OUT u4Byte				*pSize
	)
{
#if (DM_ODM_SUPPORT_TYPE &  ODM_WIN)	
#if (RTL8188E_SUPPORT == 1)  
	if (pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		if (ConfigType == CONFIG_FW_NIC)
		{
			READ_FIRMWARE(8188E,_FW_NIC);
			ODM_ReadFirmware_8188E_FW_NIC(pDM_Odm, pFirmware, pSize);
		}
		else if (ConfigType == CONFIG_FW_WoWLAN)
		{
			READ_FIRMWARE(8188E,_FW_WoWLAN);
		}
	}
#endif
#endif
#if (RTL8812A_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if (pDM_Odm->SupportICType == ODM_RTL8812)
	{
		if (ConfigType == CONFIG_FW_NIC)
		{
			READ_FIRMWARE(8812A,_FW_NIC);
			ODM_ReadFirmware_8812A_FW_NIC(pDM_Odm, pFirmware, pSize);
		}
		else if (ConfigType == CONFIG_FW_WoWLAN)
		{
			//READ_FIRMWARE(8812A,_FW_WoWLAN);
		}
	}
#endif
#endif
#if defined(RTL8821A_SUPPORT) && (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821)
	{

	}
#endif
	return HAL_STATUS_SUCCESS;    
} 


u4Byte
ODM_GetHWImgVersion(
	IN	PDM_ODM_T	pDM_Odm
	)
{
	u4Byte  Version=0;

#if (RTL8188E_SUPPORT == 1)  
	if (pDM_Odm->SupportICType == ODM_RTL8188E)
		Version = GET_VERSION_MP(8188E,_MAC_REG);
#endif

#if (RTL8814A_SUPPORT == 1)  
	if (pDM_Odm->SupportICType == ODM_RTL8814A)
		Version = GET_VERSION(8814A,_MAC_REG);
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)	
{
	prtl8192cd_priv	priv = pDM_Odm->priv;
	priv->pshare->PhyVersion = Version;
}
#endif
}


