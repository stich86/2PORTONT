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


#ifndef	__HALHWOUTSRC_H__
#define __HALHWOUTSRC_H__

//============================================================
// 	C Series Rate
//============================================================
//
//-----------------------------------------------------------
// CCK Rates, TxHT = 0
#define DESC92C_RATE1M					0x00
#define DESC92C_RATE2M					0x01
#define DESC92C_RATE5_5M				0x02
#define DESC92C_RATE11M				0x03

// OFDM Rates, TxHT = 0
#define DESC92C_RATE6M					0x04
#define DESC92C_RATE9M					0x05
#define DESC92C_RATE12M				0x06
#define DESC92C_RATE18M				0x07
#define DESC92C_RATE24M				0x08
#define DESC92C_RATE36M				0x09
#define DESC92C_RATE48M				0x0a
#define DESC92C_RATE54M				0x0b

// MCS Rates, TxHT = 1
#define DESC92C_RATEMCS0				0x0c
#define DESC92C_RATEMCS1				0x0d
#define DESC92C_RATEMCS2				0x0e
#define DESC92C_RATEMCS3				0x0f
#define DESC92C_RATEMCS4				0x10
#define DESC92C_RATEMCS5				0x11
#define DESC92C_RATEMCS6				0x12
#define DESC92C_RATEMCS7				0x13
#define DESC92C_RATEMCS8				0x14
#define DESC92C_RATEMCS9				0x15
#define DESC92C_RATEMCS10				0x16
#define DESC92C_RATEMCS11				0x17
#define DESC92C_RATEMCS12				0x18
#define DESC92C_RATEMCS13				0x19
#define DESC92C_RATEMCS14				0x1a
#define DESC92C_RATEMCS15				0x1b
#define DESC92C_RATEMCS15_SG			0x1c
#define DESC92C_RATEMCS32				0x20


//-----------------------------------------------------------
//
//	Jaguar Series Rate
//
//-----------------------------------------------------------
// CCK Rates, TxHT = 0
#define DESC8812_RATE1M				0x00
#define DESC8812_RATE2M				0x01
#define DESC8812_RATE5_5M				0x02
#define DESC8812_RATE11M				0x03

// OFDM Rates, TxHT = 0
#define DESC8812_RATE6M				0x04
#define DESC8812_RATE9M				0x05
#define DESC8812_RATE12M				0x06
#define DESC8812_RATE18M				0x07
#define DESC8812_RATE24M				0x08
#define DESC8812_RATE36M				0x09
#define DESC8812_RATE48M				0x0a
#define DESC8812_RATE54M				0x0b

// MCS Rates, TxHT = 1
#define DESC8812_RATEMCS0				0x0c
#define DESC8812_RATEMCS1				0x0d
#define DESC8812_RATEMCS2				0x0e
#define DESC8812_RATEMCS3				0x0f
#define DESC8812_RATEMCS4				0x10
#define DESC8812_RATEMCS5				0x11
#define DESC8812_RATEMCS6				0x12
#define DESC8812_RATEMCS7				0x13
#define DESC8812_RATEMCS8				0x14
#define DESC8812_RATEMCS9				0x15
#define DESC8812_RATEMCS10				0x16
#define DESC8812_RATEMCS11				0x17
#define DESC8812_RATEMCS12				0x18
#define DESC8812_RATEMCS13				0x19
#define DESC8812_RATEMCS14				0x1a
#define DESC8812_RATEMCS15				0x1b
#define DESC8812_RATEMCS16				0x1c
#define DESC8812_RATEMCS17				0x1d
#define DESC8812_RATEMCS18				0x1e
#define DESC8812_RATEMCS19				0x1f
#define DESC8812_RATEMCS20				0x20
#define DESC8812_RATEMCS21				0x21
#define DESC8812_RATEMCS22				0x22
#define DESC8812_RATEMCS23				0x23
#define DESC8812_RATEMCS24				0x24
#define DESC8812_RATEMCS25				0x25
#define DESC8812_RATEMCS26				0x26
#define DESC8812_RATEMCS27				0x27
#define DESC8812_RATEMCS28				0x28
#define DESC8812_RATEMCS29				0x29
#define DESC8812_RATEMCS30				0x2a
#define DESC8812_RATEMCS31				0x2b
#define DESC8812_RATEVHTSS1MCS0		0x2c
#define DESC8812_RATEVHTSS1MCS1		0x2d
#define DESC8812_RATEVHTSS1MCS2		0x2e
#define DESC8812_RATEVHTSS1MCS3		0x2f
#define DESC8812_RATEVHTSS1MCS4		0x30
#define DESC8812_RATEVHTSS1MCS5		0x31
#define DESC8812_RATEVHTSS1MCS6		0x32
#define DESC8812_RATEVHTSS1MCS7		0x33
#define DESC8812_RATEVHTSS1MCS8		0x34
#define DESC8812_RATEVHTSS1MCS9		0x35
#define DESC8812_RATEVHTSS2MCS0		0x36
#define DESC8812_RATEVHTSS2MCS1		0x37
#define DESC8812_RATEVHTSS2MCS2		0x38
#define DESC8812_RATEVHTSS2MCS3		0x39
#define DESC8812_RATEVHTSS2MCS4		0x3a
#define DESC8812_RATEVHTSS2MCS5		0x3b
#define DESC8812_RATEVHTSS2MCS6		0x3c
#define DESC8812_RATEVHTSS2MCS7		0x3d
#define DESC8812_RATEVHTSS2MCS8		0x3e
#define DESC8812_RATEVHTSS2MCS9		0x3f
#define DESC8812_RATEVHTSS3MCS0		0x40
#define DESC8812_RATEVHTSS3MCS1		0x41
#define DESC8812_RATEVHTSS3MCS2		0x42
#define DESC8812_RATEVHTSS3MCS3		0x43
#define DESC8812_RATEVHTSS3MCS4		0x44
#define DESC8812_RATEVHTSS3MCS5		0x45
#define DESC8812_RATEVHTSS3MCS6		0x46
#define DESC8812_RATEVHTSS3MCS7		0x47
#define DESC8812_RATEVHTSS3MCS8		0x48
#define DESC8812_RATEVHTSS3MCS9		0x49
#define DESC8812_RATEVHTSS4MCS0		0x4a
#define DESC8812_RATEVHTSS4MCS1		0x4b
#define DESC8812_RATEVHTSS4MCS2		0x4c
#define DESC8812_RATEVHTSS4MCS3		0x4d
#define DESC8812_RATEVHTSS4MCS4		0x4e
#define DESC8812_RATEVHTSS4MCS5		0x4f
#define DESC8812_RATEVHTSS4MCS6		0x50
#define DESC8812_RATEVHTSS4MCS7		0x51
#define DESC8812_RATEVHTSS4MCS8		0x52
#define DESC8812_RATEVHTSS4MCS9		0x53


/*--------------------------Define -------------------------------------------*/
/* BIT 7 HT Rate*/
// TxHT = 0
#define	MGN_1M				0x02
#define	MGN_2M				0x04
#define	MGN_5_5M			0x0b
#define	MGN_11M				0x16

#define	MGN_6M				0x0c
#define	MGN_9M				0x12
#define	MGN_12M				0x18
#define	MGN_18M				0x24
#define	MGN_24M				0x30
#define	MGN_36M				0x48
#define	MGN_48M				0x60
#define	MGN_54M				0x6c

// TxHT = 1
#define	MGN_MCS0			0x80
#define	MGN_MCS1			0x81
#define	MGN_MCS2			0x82
#define	MGN_MCS3			0x83
#define	MGN_MCS4			0x84
#define	MGN_MCS5			0x85
#define	MGN_MCS6			0x86
#define	MGN_MCS7			0x87
#define	MGN_MCS8			0x88
#define	MGN_MCS9			0x89
#define	MGN_MCS10			0x8a
#define	MGN_MCS11			0x8b
#define	MGN_MCS12			0x8c
#define	MGN_MCS13			0x8d
#define	MGN_MCS14			0x8e
#define	MGN_MCS15			0x8f
#define	MGN_VHT1SS_MCS0		0x90
#define	MGN_VHT1SS_MCS1		0x91
#define	MGN_VHT1SS_MCS2		0x92
#define	MGN_VHT1SS_MCS3		0x93
#define	MGN_VHT1SS_MCS4		0x94
#define	MGN_VHT1SS_MCS5		0x95
#define	MGN_VHT1SS_MCS6		0x96
#define	MGN_VHT1SS_MCS7		0x97
#define	MGN_VHT1SS_MCS8		0x98
#define	MGN_VHT1SS_MCS9		0x99
#define	MGN_VHT2SS_MCS0		0x9a
#define	MGN_VHT2SS_MCS1		0x9b
#define	MGN_VHT2SS_MCS2		0x9c
#define	MGN_VHT2SS_MCS3		0x9d
#define	MGN_VHT2SS_MCS4		0x9e
#define	MGN_VHT2SS_MCS5		0x9f
#define	MGN_VHT2SS_MCS6		0xa0
#define	MGN_VHT2SS_MCS7		0xa1
#define	MGN_VHT2SS_MCS8		0xa2
#define	MGN_VHT2SS_MCS9		0xa3

#define	MGN_MCS0_SG			0xc0
#define	MGN_MCS1_SG			0xc1
#define	MGN_MCS2_SG			0xc2
#define	MGN_MCS3_SG			0xc3
#define	MGN_MCS4_SG			0xc4
#define	MGN_MCS5_SG			0xc5
#define	MGN_MCS6_SG			0xc6
#define	MGN_MCS7_SG			0xc7
#define	MGN_MCS8_SG			0xc8
#define	MGN_MCS9_SG			0xc9
#define	MGN_MCS10_SG		0xca
#define	MGN_MCS11_SG		0xcb
#define	MGN_MCS12_SG		0xcc
#define	MGN_MCS13_SG		0xcd
#define	MGN_MCS14_SG		0xce
#define	MGN_MCS15_SG		0xcf

//============================================================
// structure and define
//============================================================

__PACK typedef struct _Phy_Rx_AGC_Info
{
	#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
		u1Byte	gain:7,trsw:1;			
	#else			
		u1Byte	trsw:1,gain:7;
	#endif
} __WLAN_ATTRIB_PACK__ PHY_RX_AGC_INFO_T,*pPHY_RX_AGC_INFO_T;

__PACK typedef struct _Phy_Status_Rpt_8192cd
{
	PHY_RX_AGC_INFO_T path_agc[2];
	u1Byte 	ch_corr[2];									
	u1Byte	cck_sig_qual_ofdm_pwdb_all;
	u1Byte	cck_agc_rpt_ofdm_cfosho_a;
	u1Byte	cck_rpt_b_ofdm_cfosho_b;
	u1Byte 	rsvd_1;//ch_corr_msb;
	u1Byte 	noise_power_db_msb;
	s1Byte	path_cfotail[2];	
	u1Byte	pcts_mask[2];	
	s1Byte	stream_rxevm[2];	
	u1Byte	path_rxsnr[2];
	u1Byte 	noise_power_db_lsb;
	u1Byte	rsvd_2[3];
	u1Byte 	stream_csi[2];
	u1Byte 	stream_target_csi[2];
	s1Byte 	sig_evm;
	u1Byte 	rsvd_3;	

#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	u1Byte 	antsel_rx_keep_2:1;	//ex_intf_flg:1;
	u1Byte 	sgi_en:1;
	u1Byte 	rxsc:2;	
	u1Byte 	idle_long:1;
	u1Byte 	r_ant_train_en:1;
	u1Byte 	ant_sel_b:1;
	u1Byte 	ant_sel:1;	
#else	// _BIG_ENDIAN_	
	u1Byte 	ant_sel:1;	
	u1Byte 	ant_sel_b:1;
	u1Byte 	r_ant_train_en:1;
	u1Byte 	idle_long:1;
	u1Byte 	rxsc:2;
	u1Byte 	sgi_en:1;
	u1Byte 	antsel_rx_keep_2:1;	//ex_intf_flg:1;
#endif
} __WLAN_ATTRIB_PACK__ PHY_STATUS_RPT_8192CD_T,*PPHY_STATUS_RPT_8192CD_T;


typedef struct _Phy_Status_Rpt_8812
{
	//2 DWORD 0
	u1Byte			gain_trsw[2];							// path-A and path-B {TRSW, gain[6:0] }
	u1Byte			chl_num_LSB;							// channel number[7:0]
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	u1Byte			chl_num_MSB:2;							// channel number[9:8]
	u1Byte			sub_chnl:4;								// sub-channel location[3:0]
	u1Byte			r_RFMOD:2;								// RF mode[1:0]
#else	// _BIG_ENDIAN_	
	u1Byte			r_RFMOD:2;
	u1Byte			sub_chnl:4;
	u1Byte			chl_num_MSB:2;
#endif

	//2 DWORD 1
	u1Byte			pwdb_all;								// CCK signal quality / OFDM pwdb all
	s1Byte			cfosho[2];		   // DW1 byte 1 DW1 byte2	//CCK AGC report and CCK_BB_Power / OFDM Path-A and Path-B short CFO
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	// this should be checked again because the definition of 8812 and 8814 is different
	//u1Byte			r_cck_rx_enable_pathc:2;					// cck rx enable pathc[1:0]
	//u1Byte			cck_rx_path:4;							// cck rx path[3:0]
	u1Byte			resvd_0:6;								
	u1Byte			bt_RF_ch_MSB:2;						// 8812A:2'b0			8814A: bt rf channel keep[7:6]
#else	// _BIG_ENDIAN_
	u1Byte			bt_RF_ch_MSB:2;
	u1Byte			resvd_0:6;
#endif

	//2 DWORD 2
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	u1Byte			ant_div_sw_a:1;							// 8812A: ant_div_sw_a    8814A: 1'b0
	u1Byte			ant_div_sw_b:1;							// 8812A: ant_div_sw_b    8814A: 1'b0
	u1Byte			bt_RF_ch_LSB:6;							// 8812A: 6'b0                   8814A: bt rf channel keep[5:0]
#else	// _BIG_ENDIAN_	
	u1Byte			bt_RF_ch_LSB:6;
	u1Byte			ant_div_sw_b:1;
	u1Byte			ant_div_sw_a:1;
#endif
	s1Byte			cfotail[2];		   // DW2 byte 1 DW2 byte 2	// path-A and path-B CFO tail
	u1Byte			PCTS_MSK_RPT_0;  						// PCTS mask report[7:0]
	u1Byte			PCTS_MSK_RPT_1;						// PCTS mask report[15:8]

	//2 DWORD 3
	s1Byte			rxevm[2];	         // DW3 byte 1 DW3 byte 2	// stream 1 and stream 2 RX EVM
	s1Byte			rxsnr[2];	         // DW3 byte 3 DW4 byte 0	// path-A and path-B RX SNR

	//2 DWORD 4
	u1Byte			PCTS_MSK_RPT_2;						// PCTS mask report[23:16]
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	u1Byte			PCTS_MSK_RPT_3:6;						// PCTS mask report[29:24]
	u1Byte			pcts_rpt_valid:1;							// pcts_rpt_valid
	u1Byte			resvd_1:1;								// 1'b0
#else	// _BIG_ENDIAN_
	u1Byte			resvd_1:1;
	u1Byte			pcts_rpt_valid:1;
	u1Byte			PCTS_MSK_RPT_3:6;
#endif
	s1Byte			rxevm_cd[2];	   //	DW 4 byte 3 DW5 byte 0  // 8812A: 16'b0	8814A: stream 3 and stream 4 RX EVM

	//2 DWORD 5
	u1Byte			csi_current[2];	   // DW5 byte 1 DW5 byte 2	// 8812A: stream 1 and 2 CSI	8814A:  path-C and path-D RX SNR
	u1Byte			gain_trsw_cd[2];	   // DW5 byte 3 DW6 byte 0	// path-C and path-D {TRSW, gain[6:0] }

	//2 DWORD 6
	s1Byte			sigevm;									// signal field EVM
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)		
	u1Byte			antidx_antc:3;							// 8812A: 3'b0		8814A: antidx_antc[2:0]
	u1Byte			antidx_antd:3;							// 8812A: 3'b0		8814A: antidx_antd[2:0]
	u1Byte			dpdt_ctrl_keep:1;						// 8812A: 1'b0		8814A: dpdt_ctrl_keep
	u1Byte			GNT_BT_keep:1;							// 8812A: 1'b0		8814A: GNT_BT_keep
#else	// _BIG_ENDIAN_
	u1Byte			GNT_BT_keep:1;
	u1Byte			dpdt_ctrl_keep:1;
	u1Byte			antidx_antd:3;
	u1Byte			antidx_antc:3;
#endif
#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)	
	u1Byte			antidx_anta:3;							// antidx_anta[2:0]
	u1Byte			antidx_antb:3;							// antidx_antb[2:0]
	u1Byte			resvd_2:2;								// 1'b0
#else	// _BIG_ENDIAN_	
	u1Byte			resvd_2:2;
	u1Byte			antidx_antb:3;
	u1Byte			antidx_anta:3;
#endif
} PHY_STATUS_RPT_8812_T,*PPHY_STATUS_RPT_8812_T;

VOID
odm_Init_RSSIForDM(
	IN OUT	PDM_ODM_T	pDM_Odm
	);

VOID
ODM_PhyStatusQuery(
	IN OUT	PDM_ODM_T					pDM_Odm,
	OUT		PODM_PHY_INFO_T			pPhyInfo,
	IN 		pu1Byte						pPhyStatus,	
	IN		PODM_PACKET_INFO_T			pPktinfo
	);

VOID
ODM_MacStatusQuery(
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN 		pu1Byte						pMacStatus,
	IN		u1Byte						MacID,	
	IN		BOOLEAN						bPacketMatchBSSID,
	IN		BOOLEAN						bPacketToSelf,
	IN		BOOLEAN						bPacketBeacon
	);
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE|ODM_AP))

HAL_STATUS
ODM_ConfigRFWithTxPwrTrackHeaderFile(
	IN 	PDM_ODM_T	        	pDM_Odm
    );

HAL_STATUS
ODM_ConfigRFWithHeaderFile(
	IN 	PDM_ODM_T	        	pDM_Odm,
	IN 	ODM_RF_RADIO_PATH_E 	Content,
	IN 	ODM_RF_RADIO_PATH_E 	eRFPath
	);

HAL_STATUS
ODM_ConfigBBWithHeaderFile(
	IN  	PDM_ODM_T	                pDM_Odm,
	IN	ODM_BB_Config_Type		ConfigType
    );

HAL_STATUS
ODM_ConfigMACWithHeaderFile(
	IN  	PDM_ODM_T	pDM_Odm
    );

HAL_STATUS
ODM_ConfigFWWithHeaderFile(
	IN 	PDM_ODM_T			pDM_Odm,
	IN 	ODM_FW_Config_Type 	ConfigType,
	OUT u1Byte				*pFirmware,
	OUT u4Byte				*pSize
	);
VOID 
ODM_GetHWImgVersion(
	IN	PDM_ODM_T	pDM_Odm
	);

#endif

#endif

