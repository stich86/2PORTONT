/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * $Revision: 63020 $
 * $Date: 2015-10-30 11:17:28 +0800 (Fri, 30 Oct 2015) $
 *
 * Purpose : GMac Driver FSM Processor
 *
 * Feature : GMac Driver FSM Processor
 *
 */

#include <module/gpon/gpon_defs.h>
#include <module/gpon/gpon_fsm.h>
#include <module/gpon/gpon_res.h>
#include <module/gpon/gpon_debug.h>
#include <rtk/ponmac.h>
#include <hal/common/halctrl.h>

static int32 gLosClrO6 = 0;

typedef void (*GMac_fsm_event_handler_t)(gpon_dev_obj_t *obj);

GMac_fsm_event_handler_t g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O7][GPON_FSM_EVENT_MAX];

static void gmac_fsm_o1_los_clear(gpon_dev_obj_t *obj)
{
	int32 ret;
    rtk_port_macAbility_t mac_ability;

	obj->status = RTK_GPONMAC_FSM_STATE_O2;

	if((ret = rtk_gpon_onuState_set(obj->status))!=RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
		return ;
	}
#if 0 /* do re lock tx PLL in ponmac_mode_set */
    /* re lock tx PLL. MP2099 pull RX_SD by power on in high optic (> -12dbm), it casuse tx Pll lock wrong clock rate */
    if((ret = rtk_ponmac_txPll_relock())!=RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
		return ;
	}
#endif
#if 1 /* keep pon port link down until pon clock is steady */
    if((ret = rtk_port_macForceAbility_get(HAL_GET_PON_PORT(), &mac_ability)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
        return ;
    }
    if(PORT_LINKDOWN == mac_ability.linkStatus)
    {
        mac_ability.linkStatus = PORT_LINKUP;
        if((ret = rtk_port_macForceAbility_set(HAL_GET_PON_PORT(), mac_ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
            return ;
        }
    }
#endif
}

static void gmac_fsm_o2_upstream(gpon_dev_obj_t *obj)
{
    int32 ret;
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->timer = GPON_OS_StartTimer(obj->parameter.onu.to1_timer,FALSE,0,gpon_fsm_to1_expire);
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"start TO1 timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->status = RTK_GPONMAC_FSM_STATE_O3;

    if((ret = rtk_gpon_onuState_set(obj->status))!=RT_ERR_OK)
    {
    		RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
		return;
    }
}

static void gmac_fsm_o2_los_detect(gpon_dev_obj_t *obj)
{
    obj->status = RTK_GPONMAC_FSM_STATE_O1;

    rtk_gpon_onuState_set(obj->status);
}

static void gmac_fsm_o2_disable(gpon_dev_obj_t *obj)
{
    /* shut down optical transmit power immediately. */
    //TBD: use gpio to control laser off

    obj->status = RTK_GPONMAC_FSM_STATE_O7;

    rtk_gpon_onuState_set(obj->status);

    /* set TX disable when OTL set ONT to disable */
    gpon_dev_rogueOntDisTx_set(obj, DISABLED);
}

static void gmac_fsm_o3_onuid(gpon_dev_obj_t *obj)
{
    obj->status = RTK_GPONMAC_FSM_STATE_O4;

    rtk_gpon_onuState_set(obj->status);
}

static void gmac_fsm_o3_to1_expire(gpon_dev_obj_t *obj)
{
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"timer TO1 expire %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->timer = 0;
    obj->status = RTK_GPONMAC_FSM_STATE_O2;

    rtk_gpon_onuState_set(obj->status);
}

static void gmac_fsm_o3_los_detect(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O1;

    rtk_gpon_onuState_set(obj->status);
}

static void gmac_fsm_o3_disable(gpon_dev_obj_t *obj)
{
    /* shut down optical transmit power immediately. */
    //TBD: use gpio to control laser off

    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O7;

    rtk_gpon_onuState_set(obj->status);

    /* set TX disable when OTL set ONT to disable */
    gpon_dev_rogueOntDisTx_set(obj, DISABLED);
}

static void gmac_fsm_o4_eqd(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O5;

    gpon_dev_burstHead_ranged_set(obj);
    rtk_gpon_usPloamBuf_flush();
    rtk_gpon_onuState_set(obj->status);
#if 0 /* this function is for power saving, it is not verified yet. */
    rtk_gpon_autoDisTx_set(ENABLED);
#endif
	/*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKUP);
}

static void gmac_fsm_o4_to1_expire(gpon_dev_obj_t *obj)
{
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"timer TO1 expire %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->timer = 0;
    obj->status = RTK_GPONMAC_FSM_STATE_O2;

    rtk_gpon_onuState_set(obj->status);
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);
}

static void gmac_fsm_o4_deactivate(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O2;

    rtk_gpon_onuState_set(obj->status);
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);
}

static void gmac_fsm_o4_los_detect(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O1;

    rtk_gpon_onuState_set(obj->status);
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);
}

static void gmac_fsm_o4_disable(gpon_dev_obj_t *obj)
{
    /* shut down optical transmit power immediately. */
    //TBD: use gpio to control laser off

    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O7;

    rtk_gpon_onuState_set(obj->status);
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);

    /* set TX disable when OTL set ONT to disable */
    gpon_dev_rogueOntDisTx_set(obj, DISABLED);
}

static void gmac_fsm_o5_deactivate(gpon_dev_obj_t *obj)
{
    obj->status = RTK_GPONMAC_FSM_STATE_O2;

  	/*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKDOWN);

#if 0 /* this function is for power saving, it is not verified yet. */
    rtk_gpon_autoDisTx_set(DISABLED);
#endif
    rtk_gpon_onuState_set(obj->status);
    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
		 obj->ber_timer = 0;
    }

	if(obj->signal_callback)
	{
		(*obj->signal_callback)(GPON_SIGNAL_DEACTIVE);
	}
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);

	/*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();
}

static void gmac_fsm_o5_los_detect(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->timer = GPON_OS_StartTimer(obj->parameter.onu.to2_timer,FALSE,0,gpon_fsm_to2_expire);
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"start TO2 timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->status = RTK_GPONMAC_FSM_STATE_O6;
#if 0 /* this function is for power saving, it is not verified yet. */
    rtk_gpon_autoDisTx_set(DISABLED);
#endif

    rtk_gpon_onuState_set(obj->status);

#if 0 /* move to O6 handle */
	/*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();
#endif
}

static void gmac_fsm_o5_disable(gpon_dev_obj_t *obj)
{
  	/*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKDOWN);

    /* shut down optical transmit power immediately. */
    //TBD: use gpio to control laser off

    obj->status = RTK_GPONMAC_FSM_STATE_O7;

#if 0 /* this function is for power saving, it is not verified yet. */
    rtk_gpon_autoDisTx_set(DISABLED);
#endif
    rtk_gpon_onuState_set(obj->status);
    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
		 obj->ber_timer = 0;
    }
	if(obj->signal_callback)
	{
		(*obj->signal_callback)(GPON_SIGNAL_DEACTIVE);
	}
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);

	/*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();

    /* set TX disable when OTL set ONT to disable */
    gpon_dev_rogueOntDisTx_set(obj, DISABLED);
}

static void gmac_fsm_o6_deactivate(gpon_dev_obj_t *obj)
{
    /*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKDOWN);

    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O2;

    rtk_gpon_onuState_set(obj->status);

    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
         obj->ber_timer = 0;
    }
	if(obj->signal_callback)
	{
		(*obj->signal_callback)(GPON_SIGNAL_DEACTIVE);
	}
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);

    /*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();
}

static void gmac_fsm_o6_broadcast_popup(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }

    /*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();

    obj->timer = GPON_OS_StartTimer(obj->parameter.onu.to1_timer,FALSE,0,gpon_fsm_to1_expire);
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"start TO1 timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->status = RTK_GPONMAC_FSM_STATE_O4;

    gpon_dev_burstHead_preRanged_set(obj);
    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
		 obj->ber_timer = 0;
    }
    /* set EQD to pre_eqd */
    rtk_gpon_eqd_set(obj->pre_eqd,obj->eqd_offset);
    rtk_gpon_onuState_set(obj->status);

    /*
    * O6 popup should not set this flag
    */
    gLosClrO6 = 0;
}

static void gmac_fsm_o6_directed_popup(gpon_dev_obj_t *obj)
{
    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O5;

    rtk_gpon_onuState_set(obj->status);

    /*
    * O6 popup should not set this flag
    */
    gLosClrO6 = 0;
}

static void gmac_fsm_o6_to2_expire(gpon_dev_obj_t *obj)
{
    /*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKDOWN);

    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"timer TO2 expire %p [baseaddr:%p]",obj->timer,obj->base_addr);
    obj->timer = 0;
    obj->status = RTK_GPONMAC_FSM_STATE_O1;

    rtk_gpon_onuState_set(obj->status);
	if(obj->signal_callback)
	{
		(*obj->signal_callback)(GPON_SIGNAL_DEACTIVE);
	}
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);
    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
	 obj->ber_timer = 0;
    }
    /*
    * to2 set to 10s
    * plug fiber quickly,pon miss los intr in o6
    * when to2 expire,ont in o1 state,can't range
    */
    if(gLosClrO6 == 1)
    {
        osal_time_mdelay(10);
        gmac_fsm_o1_los_clear(obj);
	    gLosClrO6 = 0;
    }

    /*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();
}

static void gmac_fsm_o6_disable(gpon_dev_obj_t *obj)
{
    /* shut down optical transmit power immediately. */
    //TBD: use gpio to control laser off

    /*set pon port to mac fore mode*/
	gpon_dev_portMacForceMode_set(obj, PORT_LINKDOWN);

    if(obj->timer)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"stop timer %p [baseaddr:%p]",obj->timer,obj->base_addr);
        GPON_OS_StopTimer(obj->timer);
        obj->timer = 0;
    }
    obj->status = RTK_GPONMAC_FSM_STATE_O7;

    rtk_gpon_onuState_set(obj->status);

    if(obj->ber_timer)
    {
    	 GPON_OS_StopTimer(obj->ber_timer);
		 obj->ber_timer = 0;
    }
	if(obj->signal_callback)
	{
		(*obj->signal_callback)(GPON_SIGNAL_DEACTIVE);
	}
    gpon_dev_onuid_set(obj,GPON_DEV_DEFAULT_ONU_ID);

    /* set TX disable when OTL set ONT to disable */
    gpon_dev_rogueOntDisTx_set(obj, DISABLED);

    /*drain-out default queue first*/
	rtk_gpon_drainOutDefaultQueue_set();
}

static void gmac_fsm_o6_los_clear(gpon_dev_obj_t *obj)
{
    if(obj) ;

    gLosClrO6 = 1;
}

static void gmac_fsm_o7_enable(gpon_dev_obj_t *obj)
{
    /* turn on optical transmit power immediately. */
    //TBD: use gpio to control laser on

    obj->status = RTK_GPONMAC_FSM_STATE_O2;

    rtk_gpon_onuState_set(obj->status);

    /* set TX enable when OTL set ONT to enable */
    gpon_dev_rogueOntDisTx_set(obj, ENABLED);
}

void gpon_fsm_init(void)
{
    uint8 i,j;

    /* clear the table */
    for(i=0;i<RTK_GPONMAC_FSM_STATE_O7;i++)
    {
        for(j=0;j<GPON_FSM_EVENT_MAX;j++)
        {
            g_gmac_fsm_eventhandler[i][j] = NULL;
        }
    }

    /* initialize the table */
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O1-1][GPON_FSM_EVENT_LOS_CLEAR] = gmac_fsm_o1_los_clear;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O2-1][GPON_FSM_EVENT_RX_UPSTREAM] = gmac_fsm_o2_upstream;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O2-1][GPON_FSM_EVENT_LOS_DETECT] = gmac_fsm_o2_los_detect;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O2-1][GPON_FSM_EVENT_RX_DISABLE] = gmac_fsm_o2_disable;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O3-1][GPON_FSM_EVENT_RX_ONUID] = gmac_fsm_o3_onuid;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O3-1][GPON_FSM_EVENT_TO1_EXPIRE] = gmac_fsm_o3_to1_expire;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O3-1][GPON_FSM_EVENT_LOS_DETECT] = gmac_fsm_o3_los_detect;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O3-1][GPON_FSM_EVENT_RX_DISABLE] = gmac_fsm_o3_disable;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O4-1][GPON_FSM_EVENT_RX_EQD] = gmac_fsm_o4_eqd;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O4-1][GPON_FSM_EVENT_TO1_EXPIRE] = gmac_fsm_o4_to1_expire;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O4-1][GPON_FSM_EVENT_RX_DEACTIVATE] = gmac_fsm_o4_deactivate;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O4-1][GPON_FSM_EVENT_LOS_DETECT] = gmac_fsm_o4_los_detect;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O4-1][GPON_FSM_EVENT_RX_DISABLE] = gmac_fsm_o4_disable;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O5-1][GPON_FSM_EVENT_RX_DEACTIVATE] = gmac_fsm_o5_deactivate;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O5-1][GPON_FSM_EVENT_LOS_DETECT] = gmac_fsm_o5_los_detect;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O5-1][GPON_FSM_EVENT_RX_DISABLE] = gmac_fsm_o5_disable;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_RX_DEACTIVATE] = gmac_fsm_o6_deactivate;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_RX_BC_POPUP] = gmac_fsm_o6_broadcast_popup;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_RX_DIRECT_POPUP] = gmac_fsm_o6_directed_popup;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_TO2_EXPIRE] = gmac_fsm_o6_to2_expire;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_RX_DISABLE] = gmac_fsm_o6_disable;
    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O6-1][GPON_FSM_EVENT_LOS_CLEAR] = gmac_fsm_o6_los_clear;

    g_gmac_fsm_eventhandler[RTK_GPONMAC_FSM_STATE_O7-1][GPON_FSM_EVENT_RX_ENABLE] = gmac_fsm_o7_enable;
}

void gpon_fsm_event(gpon_dev_obj_t *obj, rtk_gpon_fsm_event_t event)
{
    rtk_gpon_fsm_status_t oldstate = obj->status;

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"FSM State %s receive event %s [baseaddr:%p]",gpon_dbg_fsm_status_str(obj->status),gpon_dbg_fsm_event_str(event),obj->base_addr);
    if(g_gmac_fsm_eventhandler[obj->status-1][event])
    {
        (*g_gmac_fsm_eventhandler[obj->status-1][event])(obj);
    }

    if(obj->status!=oldstate)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"FSM State changed: from %s to %s [baseaddr:%p]",gpon_dbg_fsm_status_str(oldstate),gpon_dbg_fsm_status_str(obj->status),obj->base_addr);
        if(obj->state_change_callback)
        {
            (*obj->state_change_callback)(event,obj->status,oldstate);  // modified by haoship,reason:support func "rtk_handle_state_change()"in rtk_drv_callback.c
        }
    }
}

extern gpon_drv_obj_t *g_gponmac_drv;
void gpon_fsm_to1_expire(uint32 data)
{
    /*for compiler warning*/
    if(data);


    GPON_OS_Lock(g_gponmac_drv->lock);
    gpon_fsm_event(g_gponmac_drv->dev,GPON_FSM_EVENT_TO1_EXPIRE);
    GPON_OS_Unlock(g_gponmac_drv->lock);
    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"FSM TO1 expire");
}

void gpon_fsm_to2_expire(uint32 data)
{
    /*for compiler warning*/
    if(data);


    GPON_OS_Lock(g_gponmac_drv->lock);
    gpon_fsm_event(g_gponmac_drv->dev,GPON_FSM_EVENT_TO2_EXPIRE);
    GPON_OS_Unlock(g_gponmac_drv->lock);
    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"FSM TO2 expire");
}

