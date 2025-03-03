/*
 * Copyright (C) 2014 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * Purpose : Definition of OMCI alarm related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI alarm related define
 */

#ifndef __OMCI_ALARM_H__
#define __OMCI_ALARM_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "gos_type.h"


// define alarm properties
#define OMCI_ALARM_MAX_NUMBER	(224)
#define OMCI_ALARM_NUMBER_BITMASK_IN_BYTE	(OMCI_ALARM_MAX_NUMBER / 8)
#define OMCI_ALARM_REPORT_CTRL_INTERVAL_SCALE_FACTOR		(60)

// define alarm type
typedef enum {
	OMCI_ALM_TYPE_ONU_G							= 256,
	OMCI_ALM_TYPE_CARDHOLDER					= 5,
	OMCI_ALM_TYPE_CIRCUIT_PACK					= 6,
	OMCI_ALM_TYPE_EQUIPMENT_EXTENSION_PACKAGE	= 160,
	OMCI_ALM_TYPE_EQUIPMENT_PROTECTION_PROFILE	= 159,
	OMCI_ALM_TYPE_ONU_E							= 331,
	OMCI_ALM_TYPE_ANI_G							= 263,
	OMCI_ALM_TYPE_GEM_PORT_NETWORK_CTP			= 268,
	OMCI_ALM_TYPE_PRIORITY_QUEUE				= 277,
	OMCI_ALM_TYPE_MAC_BRIDGE_PORT_CONFIG_DATA	= 47,
	OMCI_ALM_TYPE_DOT1X_PORT_EXTENSION_PACKAGE	= 290,
	OMCI_ALM_TYPE_DOT1AG_MEP					= 302,
	OMCI_ALM_TYPE_MULTICAST_OPERATIONS_PROFILE	= 309,
	OMCI_ALM_TYPE_PPTP_ETHERNET_UNI				= 11,
	OMCI_ALM_TYPE_VEIP							= 329,
	OMCI_ALM_TYPE_PPTP_XDSL_UNI_PART_1			= 98,
	OMCI_ALM_TYPE_PPTP_CES_UNI					= 12,
	OMCI_ALM_TYPE_PSEUDOWIRE_TP					= 282,
	OMCI_ALM_TYPE_SIP_USER_DATA					= 153,
	OMCI_ALM_TYPE_SIP_AGENT_CONFIG_DATA			= 150,
	OMCI_ALM_TYPE_VOICE_SERVICE_PROFILE			= 58,
	OMCI_ALM_TYPE_MGC_CONFIG_DATA				= 155,
	OMCI_ALM_TYPE_VOIP_CONFIG_DATA				= 138,
	OMCI_ALM_TYPE_PPTP_MOCA_UNI					= 162,
	OMCI_ALM_TYPE_PPTP_VIDEO_UNI				= 82,
	OMCI_ALM_TYPE_PPTP_VIDEO_ANI				= 90,
	OMCI_ALM_TYPE_INTERWORKING_VCC_TP			= 14,
	OMCI_ALM_TYPE_VP_NETWORK_CTP				= 269,
	OMCI_ALM_TYPE_RE_ANI_G						= 313,
	OMCI_ALM_TYPE_PPTP_RE_UNI					= 314,
	OMCI_ALM_TYPE_RE_US_AMPLIFIER				= 315,
	OMCI_ALM_TYPE_RE_DS_AMPLIFIER				= 316,
	OMCI_ALM_TYPE_RE_COMMON_AMPLIFIER			= 328,
} omci_alm_type_t;

// define alarm number
typedef enum {
	OMCI_ALM_NBR_ONUG_EQUIPMENT_ALM				= 0,
	OMCI_ALM_NBR_ONUG_MINIMUM					= OMCI_ALM_NBR_ONUG_EQUIPMENT_ALM,
	OMCI_ALM_NBR_ONUG_POWERING_ALM				= 1,
	OMCI_ALM_NBR_ONUG_BATTERY_MISSING			= 2,
	OMCI_ALM_NBR_ONUG_BATTERY_FAILURE			= 3,
	OMCI_ALM_NBR_ONUG_BATTERY_LOW				= 4,
	OMCI_ALM_NBR_ONUG_PHYSICAL_INSTRUSION		= 5,
	OMCI_ALM_NBR_ONUG_ONU_SELF_TEST_FAILURE		= 6,
	OMCI_ALM_NBR_ONUG_DYING_GASP				= 7,
	OMCI_ALM_NBR_ONUG_TEMPERATURE_YELLOW		= 8,
	OMCI_ALM_NBR_ONUG_TEMPERATURE_RED			= 9,
	OMCI_ALM_NBR_ONUG_VOLTAGE_YELLOW			= 10,
	OMCI_ALM_NBR_ONUG_VOLTAGE_RED				= 11,
	OMCI_ALM_NBR_ONUG_ONU_MANUAL_POWER_OFF		= 12,
	OMCI_ALM_NBR_ONUG_INV_IMAGE					= 13,
	OMCI_ALM_NBR_ONUG_PSE_OVERLOAD_YELLOW		= 14,
	OMCI_ALM_NBR_ONUG_PSE_OVERLOAD_RED			= 15,
	OMCI_ALM_NBR_ONUG_MAXIMUM					= OMCI_ALM_NBR_ONUG_PSE_OVERLOAD_RED,
} omci_alm_nbr_onug_t;

typedef enum {
	OMCI_ALM_NBR_CH_PLUG_IN_CIRCUIT_PACK_MISSING		= 0,
	OMCI_ALM_NBR_CH_MINIMUM								= OMCI_ALM_NBR_CH_PLUG_IN_CIRCUIT_PACK_MISSING,
	OMCI_ALM_NBR_CH_PLUG_IN_TYPE_MISMATCH_ALM			= 1,
	OMCI_ALM_NBR_CH_IMPROPER_CARD_REMOVAL				= 2,
	OMCI_ALM_NBR_CH_PLUG_IN_EQUIPMENT_ID_MISMATCH_ALM	= 3,
	OMCI_ALM_NBR_CH_PROTECTION_SWITCH					= 4,
	OMCI_ALM_NBR_CH_MAXIMUM								= OMCI_ALM_NBR_CH_PROTECTION_SWITCH,
} omci_alm_nbr_cardholder_t;

typedef enum {
	OMCI_ALM_NBR_CP_EQUIPMENT_ALM		= 0,
	OMCI_ALM_NBR_CP_MINIMUM				= OMCI_ALM_NBR_CP_EQUIPMENT_ALM,
	OMCI_ALM_NBR_CP_POWERING_ALM		= 1,
	OMCI_ALM_NBR_CP_SELF_TEST_FAILURE	= 2,
	OMCI_ALM_NBR_CP_LASER_EOL			= 3,
	OMCI_ALM_NBR_CP_TEMPERATURE_YELLOW	= 4,
	OMCI_ALM_NBR_CP_TEMPERATURE_RED		= 5,
	OMCI_ALM_NBR_CP_MAXIMUM				= OMCI_ALM_NBR_CP_TEMPERATURE_RED,
} omci_alm_nbr_circuit_pack_t;

typedef enum {
	OMCI_ALM_NBR_ANIG_LOW_RX_OPTICAL_PWR	= 0,
	OMCI_ALM_NBR_ANIG_MINIMUM				= OMCI_ALM_NBR_ANIG_LOW_RX_OPTICAL_PWR,
	OMCI_ALM_NBR_ANIG_HIGH_RX_OPTICAL_PWR	= 1,
	OMCI_ALM_NBR_ANIG_SF					= 2,
	OMCI_ALM_NBR_ANIG_SD					= 3,
	OMCI_ALM_NBR_ANIG_LOW_TX_OPTICAL_PWR	= 4,
	OMCI_ALM_NBR_ANIG_HIGH_TX_OPTICAL_PWR	= 5,
	OMCI_ALM_NBR_ANIG_LASER_BIAS_CURRENT	= 6,
	OMCI_ALM_NBR_ANIG_MAXIMUM				= OMCI_ALM_NBR_ANIG_LASER_BIAS_CURRENT,
} omci_alm_nbr_anig_t;

typedef enum {
	OMCI_ALM_NBR_GPNC_E2E_LOSS_OF_CONTINUITY	= 5,
	OMCI_ALM_NBR_GPNC_MINIMUM					= OMCI_ALM_NBR_GPNC_E2E_LOSS_OF_CONTINUITY,
	OMCI_ALM_NBR_GPNC_MAXIMUM					= OMCI_ALM_NBR_GPNC_E2E_LOSS_OF_CONTINUITY,
} omci_alm_nbr_gem_port_network_ctp_t;

typedef enum {
	OMCI_ALM_NBR_PQ_BLOCK_LOSS	= 0,
	OMCI_ALM_NBR_PQ_MINIMUM		= OMCI_ALM_NBR_PQ_BLOCK_LOSS,
	OMCI_ALM_NBR_PQ_MAXIMUM		= OMCI_ALM_NBR_PQ_BLOCK_LOSS,
} omci_alm_nbr_priority_queue_t;

typedef enum {
	OMCI_ALM_NBR_MBPCD_PORT_BLOCKING	= 0,
	OMCI_ALM_NBR_MBPCD_MINIMUM			= OMCI_ALM_NBR_MBPCD_PORT_BLOCKING,
	OMCI_ALM_NBR_MBPCD_MAXIMUM			= OMCI_ALM_NBR_MBPCD_PORT_BLOCKING,
} omci_alm_nbr_mac_bridge_port_config_data_t;

typedef enum {
	OMCI_ALM_NBR_MOP_LOST_MCAST_GROUP	= 0,
	OMCI_ALM_NBR_MOP_MINIMUM			= OMCI_ALM_NBR_MOP_LOST_MCAST_GROUP,
	OMCI_ALM_NBR_MOP_MAXIMUM			= OMCI_ALM_NBR_MOP_LOST_MCAST_GROUP,
} omci_alm_nbr_mcast_operations_profile_t;

typedef enum {
	OMCI_ALM_NBR_ETHERNET_LAN_LOS	= 0,
	OMCI_ALM_NBR_ETHERNET_MINIMUM	= OMCI_ALM_NBR_ETHERNET_LAN_LOS,
	OMCI_ALM_NBR_ETHERNET_MAXIMUM	= OMCI_ALM_NBR_ETHERNET_LAN_LOS,
} omci_alm_nbr_pptp_ethernet_uni_t;

typedef enum {
	OMCI_ALM_NBR_VEIP_CONNECTION_FUNCTION_FAIL	= 0,
	OMCI_ALM_NBR_VEIP_MINIMUM					= OMCI_ALM_NBR_VEIP_CONNECTION_FUNCTION_FAIL,
	OMCI_ALM_NBR_VEIP_MAXIMUM					= OMCI_ALM_NBR_VEIP_CONNECTION_FUNCTION_FAIL,
} omci_alm_nbr_veip_t;

typedef enum {
	OMCI_ALM_STS_CLEAR		= 0,
	OMCI_ALM_STS_DECLARE	= 1,
} omci_alm_status_t;

// define alarm macro
#define m_omci_is_alm_nbr_valid(x)	(x < OMCI_ALARM_MAX_NUMBER)
#define m_omci_is_alm_sts_valid(x)	(OMCI_ALM_STS_CLEAR == x || OMCI_ALM_STS_DECLARE == x)
#define m_omci_alm_nbr_in_byte(x)	(x / 8)
#define m_omci_alm_nbr_in_msb(x)	(x % 8)
#define m_omci_alm_nbr_in_lsb(x)	(7 - m_omci_alm_nbr_in_msb(x))
#define m_omci_alm_nbr_in_bit(x)	(0x80 >> m_omci_alm_nbr_in_msb(x))
#define m_omci_get_alm_sts(a, x)	((a[m_omci_alm_nbr_in_byte(x)] >> m_omci_alm_nbr_in_lsb(x)) & 0x1)

// define alarm structure
typedef struct {
	UINT8	almNumber;
	UINT8	almStatus;
    UINT16	almDetail;
} omci_alm_data_t;

typedef struct {
    omci_alm_type_t		almType;
    omci_alm_data_t		almData;
} omci_alm_t;


#ifdef  __cplusplus
}
#endif

#endif 
