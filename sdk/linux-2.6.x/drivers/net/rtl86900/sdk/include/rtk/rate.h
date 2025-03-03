/*
 * Copyright (C) 2012 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 61949 $
 * $Date: 2015-09-15 20:10:29 +0800 (Tue, 15 Sep 2015) $
 *
 * Purpose : Definition of Port Bandwidth Control and Storm Control API 
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of Ingress Port Bandwidth Control [Ingress Rate Limit]
 *           (2) Configuration of Egress  Port Bandwidth Control [Egress  Rate Limit]
 *           (3) Configuration of Storm Control
 *           (3) Configuration of meter
 *
 */

#ifndef __RTK_RATE_H__
#define __RTK_RATE_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/* Group type of storm control */
typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_DHCP,
    STORM_GROUP_ARP,
    STORM_GROUP_IGMP_MLD,
    STORM_GROUP_END
} rtk_rate_storm_group_t;


typedef struct rtk_rate_storm_group_ctrl_s
{
    uint8 unknown_unicast_enable;
    uint8 unknown_multicast_enable;
    uint8 multicast_enable;
    uint8 broadcast_enable;
    uint8 dhcp_enable;
    uint8 arp_enable;
    uint8 igmp_mld_enable;
} rtk_rate_storm_group_ctrl_t;




typedef enum rtk_storm_bypass_e
{
    BYPASS_BRG_GROUP                          = 0x00,
    BYPASS_FD_PAUSE                           = 0x01,
    BYPASS_SP_MCAST                           = 0x02,
    BYPASS_1X_PAE                             = 0x03,
    BYPASS_UNDEF_BRG_04                       = 0x04,
    BYPASS_UNDEF_BRG_05                       = 0x05,
    BYPASS_UNDEF_BRG_06                       = 0x06,
    BYPASS_UNDEF_BRG_07                       = 0x07,
    BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS      = 0x08,
    BYPASS_UNDEF_BRG_09                       = 0x09,
    BYPASS_UNDEF_BRG_0A                       = 0x0A,
    BYPASS_UNDEF_BRG_0B                       = 0x0B,
    BYPASS_UNDEF_BRG_0C                       = 0x0C,
    BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS       = 0x0D,
    BYPASS_8021AB                             = 0x0E,
    BYPASS_UNDEF_BRG_0F                       = 0x0F,
    BYPASS_BRG_MNGEMENT                       = 0x10,
    BYPASS_UNDEFINED_11                       = 0x11,
    BYPASS_UNDEFINED_12                       = 0x12,
    BYPASS_UNDEFINED_13                       = 0x13,
    BYPASS_UNDEFINED_14                       = 0x14,
    BYPASS_UNDEFINED_15                       = 0x15,
    BYPASS_UNDEFINED_16                       = 0x16,
    BYPASS_UNDEFINED_17                       = 0x17,
    BYPASS_UNDEFINED_18                       = 0x18,
    BYPASS_UNDEFINED_19                       = 0x19,
    BYPASS_UNDEFINED_1A                       = 0x1A,
    BYPASS_UNDEFINED_1B                       = 0x1B,
    BYPASS_UNDEFINED_1C                       = 0x1C,
    BYPASS_UNDEFINED_1D                       = 0x1D,
    BYPASS_UNDEFINED_1E                       = 0x1E,
    BYPASS_UNDEFINED_1F                       = 0x1F,
    BYPASS_GMRP                               = 0x20,
    BYPASS_GVRP                               = 0x21,
    BYPASS_UNDEF_GARP_22                      = 0x22,
    BYPASS_UNDEF_GARP_23                      = 0x23,
    BYPASS_UNDEF_GARP_24                      = 0x24,
    BYPASS_UNDEF_GARP_25                      = 0x25,
    BYPASS_UNDEF_GARP_26                      = 0x26,
    BYPASS_UNDEF_GARP_27                      = 0x27,
    BYPASS_UNDEF_GARP_28                      = 0x28,
    BYPASS_UNDEF_GARP_29                      = 0x29,
    BYPASS_UNDEF_GARP_2A                      = 0x2A,
    BYPASS_UNDEF_GARP_2B                      = 0x2B,
    BYPASS_UNDEF_GARP_2C                      = 0x2C,
    BYPASS_UNDEF_GARP_2D                      = 0x2D,
    BYPASS_UNDEF_GARP_2E                      = 0x2E,
    BYPASS_UNDEF_GARP_2F                      = 0x2F,

    BYPASS_CSSTP                              = 0x1000,
    BYPASS_CDP                                = 0x1001,
    BYPASS_IGMP                               = 0xFFFF,
    BYPASS_END,
}rtk_storm_bypass_t;


typedef enum rtk_rate_metet_mode_e
{
    METER_MODE_BIT_RATE = 0,
    METER_MODE_PACKET_RATE,
    METER_MODE_END
} rtk_rate_metet_mode_t;


/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_rate_init
 * Description:
 *      Initial the rate module.
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
rtk_rate_init(void);

/* Module Name    : Rate                                            */
/* Sub-module Name: Configuration of ingress port bandwidth control */

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      port  - port id
 * Output:
 *      pRate - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in apollo is 8Kbps.
 */
extern int32
rtk_rate_portIgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      port - port id
 *      rate - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in apollo is 8Kbps.
 */
extern int32
rtk_rate_portIgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate);

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      port  - port id
 * Output:
 *      pIfgInclude - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(rtk_port_t port, rtk_enable_t *pIfgInclude);

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      port  - port id
 *      ifgInclude - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifgInclude:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(rtk_port_t port, rtk_enable_t ifgInclude);



/* Module Name    : Rate                                           */
/* Sub-module Name: Configuration of egress port bandwidth control */

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      port  - port id
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
extern int32
rtk_rate_portEgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      port - port id
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
extern int32
rtk_rate_portEgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of egress bandwidth control includes IFG or not.
 * Input:
 *      None
 * Output:
 *      pIfgInclude - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_egrBandwidthCtrlIncludeIfg_get(rtk_enable_t *pIfgInclude);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      ifgInclude - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_egrBandwidthCtrlIncludeIfg_set(rtk_enable_t ifgInclude);



/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Per port get the status of egress bandwidth control includes IFG or not.
 * Input:
 *      None
 * Output:
 *      pIfgInclude - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(rtk_port_t port,rtk_enable_t *pIfgInclude);

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Per port set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      port    - port id
 *      ifgInclude - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(rtk_port_t port,rtk_enable_t ifgInclude);


/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_egrQueueBwCtrlEnable_get(
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtk_rate_egrQueueBwCtrlEnable_set(
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlMeterIdx_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pMeterIndex - meter index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in Apollo is 8Kbps.
 */
extern int32
rtk_rate_egrQueueBwCtrlMeterIdx_get(
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pMeterIndex);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlMeterIdx_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      meterIndex  - meter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in Apollo is 8Kbps.
 */
extern int32
rtk_rate_egrQueueBwCtrlMeterIdx_set(
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      meterIndex);

/* Module Name    : Rate                           */
/* Sub-module Name: Configuration of storm control */

/* Function Name:
 *      rtk_rate_stormControlRate_get
 * Description:
 *      Get the storm control meter index.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 * Output:
 *      pIndex     - storm control meter index.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_ENTRY_NOTFOUND    - The global strom group is not enable for this group
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - Before call this API must make sure the global strom gruop for given group is enabled,
 *      otherwise this API will return RT_ERR_ENTRY_NOTFOUND
 */
extern int32
rtk_rate_stormControlMeterIdx_get(
    rtk_port_t              port,
    rtk_rate_storm_group_t  stormType,
    uint32                  *pIndex);

/* Function Name:
 *      rtk_rate_stormControlMeterIdx_set
 * Description:
 *      Set the storm control meter index.
 * Input:
 *      port       - port id
 *      storm_type - storm group type
 *      index       - storm control meter index.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_ENTRY_NOTFOUND    - The global strom group is not enable for this group
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE    - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - Before call this API must make sure the global strom gruop for given group is enabled,
 *      otherwise this API will return RT_ERR_ENTRY_NOTFOUND
 */
extern int32
rtk_rate_stormControlMeterIdx_set(
    rtk_port_t              port,
    rtk_rate_storm_group_t  stormType,
    uint32                  index);

/* Function Name:
 *      rtk_rate_stormControlEnable_get
 * Description:
 *      Get enable status of storm control on specified port.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 * Output:
 *      pEnable    - pointer to enable status of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - When global strom gruop for given strom type is disabled,
 *      API will return DISABLED  
 */
extern int32
rtk_rate_stormControlPortEnable_get(
    rtk_port_t              port,
    rtk_rate_storm_group_t  stormType,
    rtk_enable_t            *pEnable);

/* Function Name:
 *      rtk_rate_stormControlPortEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 *      enable     - enable status of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_ENTRY_NOTFOUND    - The global strom group is not enable for this group
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - Before call this API must make sure the global strom gruop for given group is enabled,
 *      otherwise this API will return RT_ERR_ENTRY_NOTFOUND  
 */
extern int32
rtk_rate_stormControlPortEnable_set(
    rtk_port_t              port,
    rtk_rate_storm_group_t  stormType,
    rtk_enable_t            enable);


/* Function Name:
 *      rtk_rate_stormControlEnable_get
 * Description:
 *      Get enable status of storm control on specified port.
 * Input:
 *      rtk_rate_storm_group_ctrl_t  - storm group type enable control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
*      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 */
extern int32
rtk_rate_stormControlEnable_get(rtk_rate_storm_group_ctrl_t  *stormCtrl);

/* Function Name:
 *      rtk_rate_stormControlEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      None
 * Output:
 *      rtk_rate_storm_group_ctrl_t  - storm group type enable control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    total 4 storm type can be enabled.
 *      - if total enable group exceed 4 system will return RT_ERR_ENTRY_FULL
 *
 *      - when global storm type set to disable the per port setting for this 
 *        storm type will also set to disable for all port.   
 */
extern int32
rtk_rate_stormControlEnable_set(rtk_rate_storm_group_ctrl_t  *stormCtrl);




/* Function Name:
 *      rtk_rate_stormBypass_set
 * Description:
 *      Set bypass storm filter control configuration.
 * Input:
 *      type    - Bypass storm filter control type.
 *      enable  - Bypass status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_ENABLE 		- Invalid IFG parameter
 * Note:
 *
 *      This API can set per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 */
extern int32 
rtk_rate_stormBypass_set(rtk_storm_bypass_t type, rtk_enable_t enable);


/* Function Name:
 *      rtk_rate_stormBypass_get
 * Description:
 *      Get bypass storm filter control configuration.
 * Input:
 *      type - Bypass storm filter control type.
 * Output:
 *      pEnable - Bypass status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 */
extern int32
rtk_rate_stormBypass_get(rtk_storm_bypass_t type, rtk_enable_t *pEnable);





/* Module Name    : Rate                                            */
/* Sub-module Name: Configuration of meter */

/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - shared meter index
 *      rate        - rate of share meter
 *      ifgInclude  - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE             - Invalid rate
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      The API can set shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k.
 *      The granularity of rate is 8 kbps. The ifg_include parameter is used
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
extern int32 
rtk_rate_shareMeter_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);


/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - shared meter index
 * Output:
 *      pRate        - pointer of rate of share meter
 *      pIfgInclude  - include IFG or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble
 */
extern int32 
rtk_rate_shareMeter_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);

/* Function Name:
 *      rtk_rate_shareMeterBucket_set
 * Description:
 *      Set meter Bucket Size
 * Input:
 *      index        - shared meter index
 *      bucketSize   - Bucket Size
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can set shared meter bucket size.
 */
extern int32 
rtk_rate_shareMeterBucket_set(uint32 index, uint32 bucketSize);


/* Function Name:
 *      rtk_rate_shareMeterBucket_get
 * Description:
 *      Get meter Bucket Size
 * Input:
 *      index        - shared meter index
 * Output:
 *      pBucketSize - Bucket Size
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter bucket size.
 */
extern int32 
rtk_rate_shareMeterBucket_get(uint32 index, uint32 *pBucketSize);



/* Function Name:
 *      rtk_rate_shareMeterExceed_get
 * Description:
 *      Get exceed meter status.
 * Input:
 *      index        - shared meter index
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - rate is more than configured rate.
 *      - FALSE     - rate is not over then configured rate.
 */
extern int32
rtk_rate_shareMeterExceed_get(
    uint32                  index,
    uint32                  *pIsExceed);



/* Function Name:
 *      rtk_rate_shareMeterExceed_clear
 * Description:
 *      Clear share meter exceed status.
 * Input:
 *      index        - shared meter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 * Note:
 */
extern int32
rtk_rate_shareMeterExceed_clear(
    uint32                  index);

/* Function Name:
 *      rtk_rate_shareMeterMode_set
 * Description:
 *      Set meter mode
 * Input:
 *      index     - shared meter index
 *      mode      - meter mode(bit rate mode or packet rate mode)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can set shared meter type.
 */
extern int32
rtk_rate_shareMeterMode_set(uint32 index, rtk_rate_metet_mode_t mode);

/* Function Name:
 *      rtk_rate_shareMeterMode_get
 * Description:
 *      Set meter mode
 * Input:
 *      index     - shared meter index
 *      pMode     - meter mode(bit rate mode or packet rate mode)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter mode.
 */
extern int32
rtk_rate_shareMeterMode_get(uint32 index, rtk_rate_metet_mode_t *pMode);

/* Function Name:
 *      rtk_rate_hostIgrBwCtrlState_get
 * Description:
 *      Get enable status of host ingress bandwidth control
 * Input:
 *      index	- host index
 * Output:
 *      pState	- Ingress bandwidth control state.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial 
 *      RT_ERR_INPUT            - Invalid input parameters
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostIgrBwCtrlState_get(
    uint32 index,
    rtk_enable_t    *pState);

/* Function Name:
 *      rtk_rate_hostIgrBwCtrlState_set
 * Description:
 *      Set state of host ingress bandwidth control
 * Input:
 *      index	- host index
 *      state	- Ingress bandwidth control state.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial 
 *      RT_ERR_INPUT            - Invalid input parameters
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostIgrBwCtrlState_set(
    uint32 index,
    rtk_enable_t    state);

/* Function Name:
 *      rtk_rate_hostEgrBwCtrlState_get
 * Description:
 *      Get enable status of host egress bandwidth control
 * Input:
 *      index	- host index
 * Output:
 *      pState	- Egress bandwidth control state.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial 
 *      RT_ERR_INPUT            - Invalid input parameters
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostEgrBwCtrlState_get(
    uint32 index,
    rtk_enable_t    *pState);

/* Function Name:
 *      rtk_rate_hostEgrBwCtrlState_set
 * Description:
 *      Set state of host egress bandwidth control
 * Input:
 *      index	- host index
 *      state	- Egress bandwidth control state.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial 
 *      RT_ERR_INPUT            - Invalid input parameters
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostEgrBwCtrlState_set(
    uint32 index,
    rtk_enable_t    state);

/* Function Name:
 *      rtk_rate_hostBwCtrlMeterIdx_get
 * Description:
 *      Get shared meter of host bandwith control.
 * Input:
 *      index	- host index
 * Output:
 *      pMeterIndex - meter index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostBwCtrlMeterIdx_get(
    uint32 index,
    uint32 *pMeterIndex);

/* Function Name:
 *      rtk_rate_hostBwCtrlMeterIdx_set
 * Description:
 *      Set shared meter of host bandwith control.
 * Input:
 *      index	- host index
 *      meterIndex - meter index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
rtk_rate_hostBwCtrlMeterIdx_set(
    uint32 index,
    uint32 meterIndex);

/* Function Name:
 *      rtk_rate_hostMacAddr_get
 * Description:
 *      Get MAC address of host bandwidth control.
 * Input:
 *      index	- host index
 * Output:
 *      pMac - pointer to MAC address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      None
 */
extern int32
rtk_rate_hostMacAddr_get(
    uint32 index,
	rtk_mac_t *pMac);

/* Function Name:
 *      rtk_rate_hostMacAddr_set
 * Description:
 *      Set MAC address of host bandwidth control.
 * Input:
 *      index	- host index
 *      pMac - MAC address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      None
 */
extern int32
rtk_rate_hostMacAddr_set(
    uint32 index,
	rtk_mac_t *pMac);

#endif /* __RTK_RATE_H__ */
