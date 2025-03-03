/*
 * Copyright (C) 2015 Realtek Semiconductor Corp.
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
 * $Revision:  $
 * $Date: $
 *
 * Purpose :
 *
 * Feature :
 *
 */

#ifdef CONFIG_LAN_SDS_FEATURE

/* MODIFY ME - remove non-RTK API code */
#define USE_RTK_API 1

/* H/W download EEPROM to SRAM */
#define USE_EEPROM_DOWNLOAD_TRIGGER 0

/* H/W SRAM Modification Interrupt */
#define USE_SRAM_MODIFICATION_INTERRUPT 0

#define SFP_CMD_LOID_PWD_UPDATE     0x01
#define SFP_CMD_RESTORE             0x02
#define SFP_CMD_QUERY_LOIDAUTH      0x03

/*
 * Include Files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>


/* For RTK APIs */
#include <common/rt_error.h>
#include <hal/mac/reg.h>
#include <rtk/gpio.h>
#include <rtk/intr.h>
#include <rtk/irq.h>
#include <rtk/i2c.h>

#include <dal/rtl9601b/dal_rtl9601b.h>
#include <dal/rtl9601b/dal_rtl9601b_gpio.h>
#include <dal/rtl9601b/dal_rtl9601b_ponmac.h>
#include "../../module/pkt_redirect/pkt_redirect.h"

/*
 * Symbol Definition
 */
enum {
    LAN_SDS_MODE_GPHY,
    LAN_SDS_MODE_FIBER_1G,
    LAN_SDS_MODE_SGMII_PHY,
    LAN_SDS_MODE_SGMII_MAC,
    LAN_SDS_MODE_END
};

char *lan_sds_mode_string[] =
{
    "GPHY",
    "Fiber 1G",
    "SGMII PHY",
    "SGMII MAC",
    "Unknown"
};

/*
 * Macro Definition
 */
#define LAN_SDS_MONITOR_WAIT_CNT_DEFAULT    10
#define LAN_SDS_MONITOR_THRESHOLD_DEFAULT   0xff
#define LAN_SDS_DBG_PRINT(fmt, args...)     \
{                                           \
    if(lanSdsDbg)                           \
    {                                       \
        printk(fmt, ## args);               \
    }                                       \
}
#if CONFIG_SFP_APPLICATION

#define A0_ADDR(__addr__)                   (__addr__)
#define A2_ADDR(__addr__)                   (256 + __addr__)

#define SFP_EEPRO_SRAM_ADDR_MAX             511
#define SFP_I2C_BUSYCHECK_CNT               1000
#define SFP_SRAM_LOID_ADDR                  A2_ADDR(191)     /* A2h, byte 191 */
#define SFP_SRAM_LOID_LENGTH                24
#define SFP_SRAM_PWD_ADDR                   A2_ADDR(215)     /* A2h, byte 215 */
#define SFP_SRAM_PWD_LENGTH                 12
#define SFP_SRAM_RESTORE_ADDR               A2_ADDR(227)     /* A2h, byte 227 */
#define SFP_SRAM_REBOOT_ADDR                A2_ADDR(229)     /* A2h, byte 229 */
#define SFP_SRAM_STATUS_ADDR                A2_ADDR(110)     /* A2h, byte 110 */

#define SFP_DEFAULT_LOID                    "user"
#define SFP_DEFAULT_PWD                     "password"

/* Task bit */
#define SFP_TASK_MASK_EEPROM_110OR118       0x10
#define SFP_TASK_MASK_EEPROM_128TO247       0x20
#endif

/*
 * Data Declaration
 */
uint8 lanSdsDbg = 0;
uint16 waitCntCfg = LAN_SDS_MONITOR_WAIT_CNT_DEFAULT;
uint16 thresholdCfg = LAN_SDS_MONITOR_THRESHOLD_DEFAULT;
struct task_struct *pLanSdsMonitor = NULL;
struct proc_dir_entry *lan_sds_proc_dir = NULL;
struct proc_dir_entry *lan_sds_cfg_proc = NULL;
struct proc_dir_entry *lan_sds_debug_proc = NULL;
uint32 lan_sds_mode = LAN_SDS_MODE_END;

#if CONFIG_SFP_APPLICATION
struct task_struct *pSfpTask;
struct proc_dir_entry *sfp_app_proc = NULL;
struct proc_dir_entry *sfp_dump_mirror_proc = NULL;
struct proc_dir_entry *sfp_dump_eeprom_proc = NULL;
struct proc_dir_entry *sfp_mirror_proc = NULL;
struct proc_dir_entry *sfp_eeprom_proc = NULL;
uint32 sfp_app_state = DISABLED;
uint32 task_mask = 0;
uint8 loid[SFP_SRAM_LOID_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8 pwd[SFP_SRAM_PWD_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8 sram[SFP_EEPRO_SRAM_ADDR_MAX + 1];

#endif

/*
 * Function Declaration
 */

#if CONFIG_SFP_APPLICATION
#if USE_SRAM_MODIFICATION_INTERRUPT
static void sfp_isr_eeprom_update_110or118(void)
{
    int32                   ret;
    rtk_enable_t            enable;

    /* disable interrupt */
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }

    /* get interrupt status */
    if (RT_ERR_OK != (ret = rtk_intr_ims_get(INTR_TYPE_EEPROM_UPDATE_110OR118, &enable)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }
    if (!enable)
    {
        ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, ENABLED);
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }


    /* clear interrupt status */
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_110OR118)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }

    task_mask |= SFP_TASK_MASK_EEPROM_110OR118;
    ret = wake_up_process(pSfpTask);

    /* enable interrupt */
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, ENABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }
}

static void sfp_isr_eeprom_update_128to247(void)
{
    int32                   ret;
    rtk_enable_t            enable;

    printk("\n sfp_isr_eeprom_update_128to247 \n");

    /* disable interrupt */
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }

    /* get interrupt status */
    if (RT_ERR_OK != (ret = rtk_intr_ims_get(INTR_TYPE_EEPROM_UPDATE_128TO247, &enable)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }
    if (!enable)
    {
        ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, ENABLED);
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }


    /* clear interrupt status */
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_128TO247)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }

    task_mask |= SFP_TASK_MASK_EEPROM_128TO247;
    wake_up_process(pSfpTask);

    /* enable interrupt */
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, ENABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return;
    }
}
#endif

#if USE_RTK_API
#else
static int sfp_i2c_check_busy(void)
{
    int ret;
    uint32 cnt = SFP_I2C_BUSYCHECK_CNT;
    uint32 value;

    do {
        if((ret = reg_field_read(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_BUSYf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        if(0 == value)
        {
            return 0;
        }

        cnt --;
    } while(cnt != 0);

    return -1;
}
#endif

static int sfp_eeprom_byte_read(uint16 addr, uint8 *data)
{
    int ret;
    uint32 value;

    if(addr > SFP_EEPRO_SRAM_ADDR_MAX)
    {
        return -1;
    }
    if(NULL == data)
    {
        return -1;
    }
#if USE_RTK_API
    /* Configure I2C 8 bits data */
    if((ret = rtk_i2c_dataWidth_set(0, I2C_WIDTH_8bit)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure I2C dev ID
     * The actual EEPROM have two page @ 0xa0 and 0xa2
     * The access to different page is through different device ID
     */
    rtk_i2c_read(0, (addr >= 256) ? 0x51 : 0x50, addr % 256, &value);
    *data = (uint8) value;
#else
    /* Configure I2C 8 bits data */
    value = 0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_DWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure I2C dev ID
     * The actual EEPROM have two page @ 0xa0 and 0xa2
     * The access to different page is through different device ID
     */
    value = (addr >= 256) ? 0x51 : 0x50;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_IDf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Configure read addr
     * The address should resides in 0~255 for EEPROM
     */
    value = addr % 256;
    if((ret = reg_field_write(RTL9601B_I2C_IND_ADRr, RTL9601B_I2C_ADRf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure read eeprom */
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_WRENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set command enable */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_CMD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Read data back */
    if((ret = reg_field_read(RTL9601B_I2C_IND_RDr, RTL9601B_I2C_RD_DATf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    *data = value;
#endif

    return 0;
}

static int sfp_eeprom_byte_write(uint16 addr, uint8 data)
{
    int ret;
    uint32 value;

    if(addr > SFP_EEPRO_SRAM_ADDR_MAX)
    {
        return -1;
    }

#if USE_RTK_API
    /* Configure I2C 8 bits data */
    if((ret = rtk_i2c_dataWidth_set(0, I2C_WIDTH_8bit)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure I2C dev ID
     * The actual EEPROM have two page @ 0xa0 and 0xa2
     * The access to different page is through different device ID
     */
    value = (uint32) data;
    rtk_i2c_write(0, (addr >= 256) ? 0x51 : 0x50, addr % 256, value);
#else
    /* Configure I2C 8 bits data */
    value = 0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_DWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure I2C dev ID
     * The actual EEPROM have two page @ 0xa0 and 0xa2
     * The access to different page is through different device ID
     */
    value = (addr >= 256) ? 0x51 : 0x50;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_IDf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Configure write addr
     * The address should resides in 0~255 for EEPROM
     */
    value = addr % 256;
    if((ret = reg_field_write(RTL9601B_I2C_IND_ADRr, RTL9601B_I2C_ADRf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure write data */
    value = data;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WDr, RTL9601B_I2C_WR_DATf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure write eeprom */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_WRENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set command enable */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_CMD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }
#endif

    return 0;
}

#if USE_RTK_API
#else
static int sfp_eeprom_dWord_read(uint16 addr, uint32 *data)
{
    int ret;
    uint32 value;

    if(addr > SFP_EEPRO_SRAM_ADDR_MAX)
    {
        return -1;
    }
    if(NULL == data)
    {
        return -1;
    }

    /* Configure I2C 32 bits data */
    value = 3;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_DWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure I2C dev ID
     * The actual EEPROM have two page @ 0xa0 and 0xa2
     * The access to different page is through different device ID
     */
    value = (addr >= 256) ? 0x51 : 0x50;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_IDf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Configure read addr
     * The address should resides in 0~255 for EEPROM
     */
    value = addr % 256;
    if((ret = reg_field_write(RTL9601B_I2C_IND_ADRr, RTL9601B_I2C_ADRf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure read eeprom */
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_WRENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set command enable */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_CMD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Read data back */
    if((ret = reg_field_read(RTL9601B_I2C_IND_RDr, RTL9601B_I2C_RD_DATf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Re-order read bytes */
    *data =
        ((value & 0x000000ff) << 24) |
        ((value & 0x0000ff00) << 8) |
        ((value & 0x00ff0000) >> 8) |
        ((value & 0xff000000) >> 24);

    return 0;
}
#endif

static int sfp_sram_byte_read(uint16 addr, uint8 *data)
{
    int ret;
    uint32 value;

    if(addr > SFP_EEPRO_SRAM_ADDR_MAX)
    {
        return -1;
    }
    if(NULL == data)
    {
        return -1;
    }

#if USE_RTK_API
    /* Configure I2C 8 bits data */
    if((ret = rtk_i2c_dataWidth_set(0, I2C_WIDTH_8bit)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = rtk_i2c_eepMirror_read(addr, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    *data = (uint8) value;
#else
    /* Configure I2C 8 bits data */
    value = 0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_DWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Configure read addr */
    value = addr;
    if((ret = reg_field_write(RTL9601B_I2C_IND_ADRr, RTL9601B_I2C_ADRf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure read eeprom mirror (SRAM) */
    value = 0x2;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_WRENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set command enable */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_CMD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Read data back */
    if((ret = reg_field_read(RTL9601B_I2C_IND_RDr, RTL9601B_I2C_RD_DATf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    *data = value;
#endif

    return 0;
}

static int sfp_sram_byte_write(uint16 addr, uint8 data)
{
    int ret;
    uint32 value;

    if(addr > SFP_EEPRO_SRAM_ADDR_MAX)
    {
        return -1;
    }

#if USE_RTK_API
    /* Configure I2C 8 bits data */
    if((ret = rtk_i2c_dataWidth_set(0, I2C_WIDTH_8bit)) != RT_ERR_OK)
    {
        return ret;
    }

    value = data;
    if((ret = rtk_i2c_eepMirror_write(addr, value)) != RT_ERR_OK)
    {
        return ret;
    }
#else
    /* Configure I2C 8 bits data */
    value = 0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_I2C_DEVICE_DWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }

    /* Configure write addr */
    value = addr;
    if((ret = reg_field_write(RTL9601B_I2C_IND_ADRr, RTL9601B_I2C_ADRf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure write data */
    value = data;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WDr, RTL9601B_I2C_WR_DATf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure write eeprom mirror (SRAM) */
    value = 0x3;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_WRENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set command enable */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_CMDr, RTL9601B_I2C_CMD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    if((ret = sfp_i2c_check_busy()) != 0)
    {
        return ret;
    }
#endif

    return 0;
}

static void sfp_update_statistic(void)
{
    int i;
    uint32 buf96to119[6];

    /* Update byte A2 96 to 119 */
#if USE_RTK_API
    for(i = 0 ; i < 24 ; i++)
    {
        sfp_eeprom_byte_read(A2_ADDR(96) + i, &((uint8 *)buf96to119)[i]);
    }
#else
    for(i = 0 ; i < 6 ; i++)
    {
        sfp_eeprom_dWord_read(A2_ADDR(96) + (i << 2), &buf96to119[i]);
    }
#endif

    for(i = 0 ; i < 24 ; i++)
    {
        sfp_sram_byte_write(A2_ADDR(96) + i, ((uint8 *)buf96to119)[i]);
    }
}


static void sfp_relay_gpio(void)
{
    static rtk_enable_t last_los = RTK_ENABLE_END;
    static uint32 last_txDis = 0xffffffff;
    uint32 txFault;
    rtk_enable_t enable;
    uint32 data;
    uint8 sramData;

    /* Relay Tx_Dis, GPIO_15 -> GPIO_7 */
    rtk_gpio_databit_get(RTK_GPIO_15, &data);
    if(data != last_txDis)
    {
        rtk_gpio_databit_set(RTK_GPIO_7, data);
        last_txDis = data;
    }

    /* Relay LOS, LOS -> GPIO_13 */
    rtk_ponmac_losState_get(&enable);
    if(enable != last_los)
    {
        rtk_gpio_databit_set(RTK_GPIO_13, (enable == ENABLED) ? 1 : 0);
        last_los = enable;
    }

    /* Read TX_Fault, GPIO_14 */
    rtk_gpio_databit_get(RTK_GPIO_14, &data);
    txFault = data;

    /* Write Tx_Dis to A2_110[7] */
    /* Write Tx_Fault to A2_110[2] */
    /* Write LOS to A2_110[1] */
    sfp_sram_byte_read(SFP_SRAM_STATUS_ADDR, &sramData);

    if(last_txDis == 1)
        sramData |= (0x01 << 7);
    else
        sramData &= ~(0x01 << 7);

    if(txFault == 1)
        sramData |= (0x01 << 2);
    else
        sramData &= ~(0x01 << 2);

    if(last_los == ENABLED)
        sramData |= (0x01 << 1);
    else
        sramData &= ~(0x01 << 1);

    sfp_sram_byte_write(SFP_SRAM_STATUS_ADDR, sramData);
}

static void sfp_send_pkt(unsigned char *pFrame, unsigned int frame_len)
{
    int ret;

    ret = pkt_redirect_kernelApp_sendPkt(PR_USER_UID_SFPCTRL, 1, frame_len, pFrame);
    if(ret)
    {
        printk("send to user app (%d) fail (%d)\n", PR_USER_UID_SFPCTRL, ret);
    }
}

static void sfp_maintain_loid(void)
{
    uint8 sramLoid[SFP_SRAM_LOID_LENGTH];
    uint8 sramPwd[SFP_SRAM_PWD_LENGTH];
    uint8 data[SFP_SRAM_LOID_LENGTH + SFP_SRAM_PWD_LENGTH + 1];
    uint32 idx;

    /* Read LOID & Password from SRAM */
    for(idx = 0; idx < SFP_SRAM_LOID_LENGTH; idx++)
        sfp_sram_byte_read((SFP_SRAM_LOID_ADDR + idx), &sramLoid[idx]);

    for(idx = 0; idx < SFP_SRAM_PWD_LENGTH; idx++)
        sfp_sram_byte_read((SFP_SRAM_PWD_ADDR + idx), &sramPwd[idx]);

    /* Compare LOID & Password with last setting */
    if( (memcmp(loid, sramLoid, SFP_SRAM_LOID_LENGTH) != 0) || (memcmp(pwd, sramPwd, SFP_SRAM_PWD_LENGTH) != 0) )
    {
        /* if different, update database & write to EEPROM */
        memcpy(loid, sramLoid, SFP_SRAM_LOID_LENGTH);
        memcpy(pwd, sramPwd, SFP_SRAM_PWD_LENGTH);

        for(idx = 0; idx < SFP_SRAM_LOID_LENGTH; idx++)
            sfp_eeprom_byte_write((SFP_SRAM_LOID_ADDR + idx), loid[idx]);

        for(idx = 0; idx < SFP_SRAM_PWD_LENGTH; idx++)
            sfp_eeprom_byte_write((SFP_SRAM_PWD_ADDR + idx), pwd[idx]);

        /* Redirect to user space */
        data[0] = SFP_CMD_LOID_PWD_UPDATE;
        memcpy(&data[1], loid, SFP_SRAM_LOID_LENGTH);
        memcpy(&data[1 + SFP_SRAM_LOID_LENGTH], pwd, SFP_SRAM_PWD_LENGTH);
        sfp_send_pkt(data, (SFP_SRAM_LOID_LENGTH + SFP_SRAM_PWD_LENGTH + 1));
    }
}

static void sfp_maintain_restore(void)
{
    uint8 value;
    uint8 data;
    int idx;
    uint8 default_loid[SFP_SRAM_LOID_LENGTH];
    uint8 default_pwd[SFP_SRAM_PWD_LENGTH];

    sfp_sram_byte_read(SFP_SRAM_RESTORE_ADDR, &value);

    if(value == 0x01)
    {
        /* Send restore command to user space */
        data = SFP_CMD_RESTORE;
        sfp_send_pkt(&data, 1);

        /* Write Default LOID & Password to SRAM & EEPROM */
        memset(loid, 0x00, SFP_SRAM_LOID_LENGTH);
        memset(pwd, 0x00, SFP_SRAM_PWD_LENGTH);
        sprintf(default_loid, "%s", SFP_DEFAULT_LOID);
        sprintf(default_pwd, "%s", SFP_DEFAULT_PWD);

        for(idx = 0; idx < SFP_SRAM_LOID_LENGTH; idx++)
        {
            if(idx < (SFP_SRAM_LOID_LENGTH - strlen(default_loid)))
            {
                sfp_eeprom_byte_write((SFP_SRAM_LOID_ADDR + idx), 0x00);
                sfp_sram_byte_write((SFP_SRAM_LOID_ADDR + idx), 0x00);
                loid[idx] = 0x00;
            }
            else
            {
                sfp_eeprom_byte_write((SFP_SRAM_LOID_ADDR + idx), default_loid[idx - (SFP_SRAM_LOID_LENGTH - strlen(default_loid))]);
                sfp_sram_byte_write((SFP_SRAM_LOID_ADDR + idx), default_loid[idx - (SFP_SRAM_LOID_LENGTH - strlen(default_loid))]);
                loid[idx] = default_loid[idx - (SFP_SRAM_LOID_LENGTH - strlen(default_loid))];
            }
        }

        for(idx = 0; idx < SFP_SRAM_PWD_LENGTH; idx++)
        {
            if(idx < (SFP_SRAM_PWD_LENGTH - strlen(default_pwd)))
            {
                sfp_eeprom_byte_write((SFP_SRAM_PWD_ADDR + idx), 0x00);
                sfp_sram_byte_write((SFP_SRAM_PWD_ADDR + idx), 0x00);
                pwd[idx] = 0x00;
            }
            else
            {
                sfp_eeprom_byte_write((SFP_SRAM_PWD_ADDR + idx), default_pwd[idx - (SFP_SRAM_PWD_LENGTH - strlen(default_pwd))]);
                sfp_sram_byte_write((SFP_SRAM_PWD_ADDR + idx), default_pwd[idx - (SFP_SRAM_PWD_LENGTH - strlen(default_pwd))]);
                pwd[idx] = default_pwd[idx - (SFP_SRAM_PWD_LENGTH - strlen(default_pwd))];
            }
        }

        /* Write Restore bit = 0 */
        value = 0;
        sfp_sram_byte_write(SFP_SRAM_RESTORE_ADDR, value);
    }
}

static void sfp_query_loidAuth(void)
{
    uint8 data[10];

    data[0] = SFP_CMD_QUERY_LOIDAUTH;
    sfp_send_pkt(data, 1);
}

static void sfp_maintain_reboot(void)
{
    uint8 value;
    uint32 data;

    sfp_sram_byte_read(SFP_SRAM_REBOOT_ADDR, &value);

    if(value == 0x01)
    {
        data = 1;
        reg_field_write(RTL9601B_SOFTWARE_RSTr, RTL9601B_CMD_CHIP_RST_PSf, &data);
    }
}

static int sfp_main_thread(void *data)
{
    static uint8 updateStatCnt = 0;
    static uint8 updateAuthCnt = 0;
    spinlock_t lan_sds_spinlock = SPIN_LOCK_UNLOCKED;
    unsigned long flags;
    uint8 buff[SFP_SRAM_LOID_LENGTH + SFP_SRAM_PWD_LENGTH + 1];
    uint8 sramData;
    uint32 idx;

    /* Redirect LOID & Password to user space */
    buff[0] = SFP_CMD_LOID_PWD_UPDATE;
    memcpy(&buff[1], loid, SFP_SRAM_LOID_LENGTH);
    memcpy(&buff[1 + SFP_SRAM_LOID_LENGTH], pwd, SFP_SRAM_PWD_LENGTH);
    sfp_send_pkt(buff, (SFP_SRAM_LOID_LENGTH + SFP_SRAM_PWD_LENGTH + 1));

    while(!kthread_should_stop())
    {
        /* It can wake up earlier */
        set_current_state(TASK_INTERRUPTIBLE);
        /* Check every 10 ms */
        schedule_timeout(1 * HZ / 100);

#if USE_SRAM_MODIFICATION_INTERRUPT
        /* H/W Interrupt, do nothing */
#else
        /* Polling A2_118, A2_128 ~ A2_247 */
        sfp_sram_byte_read(A2_ADDR(118), &sramData);
        if(sramData != sram[A2_ADDR(118)])
        {
            sram[A2_ADDR(118)] = sramData;
            task_mask |= SFP_TASK_MASK_EEPROM_110OR118;
        }

        for(idx = 128; idx <= 247; idx++)
        {
            sfp_sram_byte_read(A2_ADDR(idx), &sramData);
            if(sramData != sram[A2_ADDR(idx)])
            {
                sram[A2_ADDR(idx)] = sramData;
                task_mask |= SFP_TASK_MASK_EEPROM_128TO247;
            }
        }
#endif

        if(task_mask & SFP_TASK_MASK_EEPROM_110OR118)
        {
            printk("%s:%d SFP_TASK_MASK_EEPROM_110OR118\n", __FILE__, __LINE__);

            /* Spin Lock before modify task_mask */
            spin_lock_irqsave(&lan_sds_spinlock, flags);
            task_mask &= ~SFP_TASK_MASK_EEPROM_110OR118;
            spin_unlock_irqrestore (&lan_sds_spinlock, flags);
        }
        if(task_mask & SFP_TASK_MASK_EEPROM_128TO247)
        {
            printk("%s:%d SFP_TASK_MASK_EEPROM_128TO247\n", __FILE__, __LINE__);

            /* Spin Lock before modify task_mask */
            spin_lock_irqsave(&lan_sds_spinlock, flags);
            task_mask &= ~SFP_TASK_MASK_EEPROM_128TO247;
            spin_unlock_irqrestore (&lan_sds_spinlock, flags);

            /* check LOID & Passward at SRAM, if modified, write them to EEPROM and redirect to user space for OMCI & Flash */
            sfp_maintain_loid();

            /* Check Restore Bit */
            sfp_maintain_restore();

            /* Check Reboot Bit */
            sfp_maintain_reboot();
        }

        /* Relay LOS & Tx_Dis */
        sfp_relay_gpio();

        /* Update Statistic every 50ms */
        updateStatCnt ++;
        if(0 == (updateStatCnt % 5))
        {
            updateStatCnt = 0;
            sfp_update_statistic();
        }

        /* Query LOID Auth status every 1 second */
        updateAuthCnt ++;
        if(0 == (updateAuthCnt % 100))
        {
            updateAuthCnt = 0;
            sfp_query_loidAuth();
        }
    }

    return 0;
}

void static sfp_app_loidAuth_rx(unsigned short dataLen, unsigned char *data)
{
    if(dataLen != 1)
        return;

    sfp_sram_byte_write(A2_ADDR(228), (uint8)data[0]);
}

static int sfp_app_init(void)
{
    int ret;
    uint32 value, idx;
    uint8 data;
    rtk_i2c_eeprom_mirror_t eeprom_mirr;

    /* =================================================
     * Hardware initialization
     * =================================================
     */
#if USE_RTK_API
    /* Enable I2C master */
    if((ret = rtk_i2c_enable_set(0, ENABLED)) != RT_ERR_OK)
    {
        return ret;
    }

    /*Enable open drain for I2C master */
    value = 1;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_M_SDA_PAD_OD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* I2C master configurations */
    /* Configure I2C master clock to 100KHz */
    if((ret = rtk_i2c_clock_set(0, 100)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Configure EEPROM DevID & address width */
    eeprom_mirr.devId = 0x50;
    eeprom_mirr.addrExtMode = 1;
    eeprom_mirr.addrWidth = 0;
    if((ret = rtk_i2c_eepMirror_set(eeprom_mirr)) != RT_ERR_OK)
    {
        return ret;
    }

#else
    /* Enable I2C master */
    value = 1;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_I2C_MASTER_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /*Enable open drain for I2C master */
    value = 1;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_M_SDA_PAD_OD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* I2C master configurations */
    /* Configure I2C master clock to 100KHz */
    value = 0x3E;
    if((ret = reg_field_write(RTL9601B_I2C_MASTER_CLOCK_DIVr, RTL9601B_I2C_CLOCK_DIVf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Configure EEPROM DevID & address width */
    value = 0x50;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_EEPROM_DEV_IDf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_EEPROM_AW_EXTf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_I2C_IND_WIDTHr, RTL9601B_EEPROM_AWf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
#endif

#if USE_EEPROM_DOWNLOAD_TRIGGER
    /* Trigger EEPROM download */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_EERPROM_LOAD_TRIGf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
#else
    /* Read all EEPROM and write it to SRAM */
    for(idx = 0; idx <= SFP_EEPRO_SRAM_ADDR_MAX; idx++)
    {
        sfp_eeprom_byte_read(idx, &data);
        sfp_sram_byte_write(idx, data);

        /* Keep the content to sofware database */
        sram[idx] = data;
    }
#endif

    /* Read LOID & Password from SRAM and keep it at software database */
    for(idx = 0; idx < SFP_SRAM_LOID_LENGTH; idx++)
        sfp_sram_byte_read((SFP_SRAM_LOID_ADDR + idx), &loid[idx]);

    for(idx = 0; idx < SFP_SRAM_PWD_LENGTH; idx++)
        sfp_sram_byte_read((SFP_SRAM_PWD_ADDR + idx), &pwd[idx]);

    /* Disable external CPU write SRAM */
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_WR_SRAM_A0_0_127_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_WR_SRAM_A0_128_255_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_WR_SRAM_A2_0_95_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_WR_SRAM_A2_96_127_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_WR_SRAM_A2_248_255_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Disable write to SRAM data auto write back to EEPROM */
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_EEPROM_AUTO_WRITE_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Configure GPIO */
    /* Set GPIO7,13,14,15 as GPIO mode */
    value = 0x0;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_DIS_TX_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_VOIP0_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_VOIP1_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_VOIP2_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Set GPIO13 to open drain mode */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_GPIO13_PAD_OD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Set GPIO14 to open drain mode */
    value = 0x1;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_GPIO14_PAD_OD_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set GPIO7,13 to output mode */
    rtk_gpio_mode_set( 7, GPIO_OUTPUT);
    rtk_gpio_mode_set(13, GPIO_OUTPUT);
    /* Set GPIO14,15 to input mode */
    rtk_gpio_mode_set(14, GPIO_INPUT);
    rtk_gpio_mode_set(15, GPIO_INPUT);
    /* Set GPIO14,15 to enable rising edge interrupt */
    //rtk_gpio_intr_set(14, GPIO_INTR_ENABLE_BOTH_EDGE);
    //rtk_gpio_intr_set(15, GPIO_INTR_ENABLE_BOTH_EDGE);

    /* enable interrupt mask */
    //ioal_mem32_write(0xB8003338, value)

    /* Disable i2c slave access swcore reg */
    value = 0;
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_SWCORE_REG_ACCESS_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Enable I2C slave */
    value = 0;
    if((ret = reg_field_write(RTL9601B_IO_MODE_ENr, RTL9601B_I2C_SLAVE_DISf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* =================================================
     * Software initialization
     * =================================================
     */
#if USE_SRAM_MODIFICATION_INTERRUPT
    /* A2_110_OR_118_UPDATED interrupt*/
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_irq_isr_register(INTR_TYPE_EEPROM_UPDATE_110OR118, sfp_isr_eeprom_update_110or118)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_110OR118)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, ENABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    /* A2_128_TO_247_UPDATED interrupt*/
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_irq_isr_register(INTR_TYPE_EEPROM_UPDATE_128TO247, sfp_isr_eeprom_update_128to247)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_128TO247)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, ENABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
#endif

    /* Register callback function */
    pkt_redirect_kernelApp_reg(PR_KERNEL_UID_SFPCTRL, sfp_app_loidAuth_rx);

    /* Start SFP thread */
    if(NULL == pSfpTask)
    {
        pSfpTask = kthread_create(sfp_main_thread, NULL, "sfp_main");
        if(IS_ERR(pSfpTask))
        {
            printk("%s:%d failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pSfpTask));
            return PTR_ERR(pSfpTask);
        }
        wake_up_process(pSfpTask);
    }

    return 0;
}

static int sfp_app_deInit(void)
{
    int ret;

    /* =================================================
     * Software initialization
     * =================================================
     */
#if USE_SRAM_MODIFICATION_INTERRUPT
    /* A2_110_OR_118_UPDATED interrupt*/
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_110OR118, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_irq_isr_unregister(INTR_TYPE_EEPROM_UPDATE_110OR118)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_110OR118)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    /* A2_128_TO_247_UPDATED interrupt*/
    if (RT_ERR_OK != (ret = rtk_intr_imr_set(INTR_TYPE_EEPROM_UPDATE_128TO247, DISABLED)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_irq_isr_unregister(INTR_TYPE_EEPROM_UPDATE_128TO247)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
    if (RT_ERR_OK != (ret = rtk_intr_ims_clear(INTR_TYPE_EEPROM_UPDATE_128TO247)))
    {
        RT_ERR(ret, (MOD_INTR), "");
        return ret;
    }
#endif

    pkt_redirect_kernelApp_dereg(PR_KERNEL_UID_SFPCTRL);

    /* Stop SFP thread */
    if(NULL != pSfpTask)
    {
        kthread_stop(pSfpTask);
        pSfpTask = NULL;
    }

    return 0;
}
static int sfp_app_set(unsigned int state)
{
    int ret;

    if(state > 1)
    {
        return -1;
    }

    if(state)
    {

        if((ret = sfp_app_init()) != 0)
        {
    		printk("sfp_app init failed %u\n", ret);
        }
        sfp_app_state = ENABLED;
    }
    else
    {
        if((ret = sfp_app_deInit()) != 0)
        {
    		printk("sfp_app deinit failed %u\n", ret);
        }
        sfp_app_state = DISABLED;
    }

    return 0;
}

static int sfp_app_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "SFP application %s\n", (sfp_app_state != DISABLED) ? "enabled" : "disabled");

	return len;
}

static int sfp_app_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
    unsigned int value;
	int len = (count > 15) ? 15 : count;
    int ret;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		value = simple_strtoul(tmpBuf, NULL, 10);
        if((ret = sfp_app_set(value)) != 0)
        {
    		printk("Incorrect state %u\n", value);
        	return -EFAULT;
        }

		return count;
	}
	return -EFAULT;
}

static int sfp_dump_mirror_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int i, row = 0;
	int len = 0;
    uint8 value;

	len += sprintf(page + len, "EEPROM Mirror(SRAM):\n");
	len += sprintf(page + len, "      00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");

    for(i = 0 ; i <= SFP_EEPRO_SRAM_ADDR_MAX ; i++) {
        if(0 == i % 16)
        {
            len += sprintf(page + len, "\n%04x  ", row);
            row += 16;
        }

        sfp_sram_byte_read(i, &value);
        len += sprintf(page + len, "%02x ", value);
    }
    len += sprintf(page + len, "\n");

	return len;
}

static int sfp_dump_eeprom_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int i, row = 0;
	int len = 0;
    uint8 value;

	len += sprintf(page + len, "EEPROM:\n");
	len += sprintf(page + len, "      00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");

    for(i = 0 ; i <= SFP_EEPRO_SRAM_ADDR_MAX ; i++) {
        if(0 == i % 16)
        {
            len += sprintf(page + len, "\n%04x  ", row);
            row += 16;
        }

        sfp_eeprom_byte_read(i, &value);
        len += sprintf(page + len, "%02x ", value);
    }
    len += sprintf(page + len, "\n");

	return len;
}

static int sfp_mirror_write(struct file *file, const char *buff, unsigned long len, void *data)
{
    int         ret;
    int         i, row;
    char 		tmpbuf[64];
    unsigned int	mem_addr, mem_data, mem_len;
    char		*strptr, *cmd_addr;
    char		*tokptr;
    uint8       value;

	if (buff && !copy_from_user(tmpbuf, buff, 63)) {
		tmpbuf[63] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		//printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "r", 1))
		{
			mem_addr=(unsigned int)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);

        	printk("EEPROM Mirror(SRAM):\n");
        	printk("      00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");

            row = mem_addr;
            for(i = 0 ; i < mem_len ; i++) {
                if(0 == i % 16)
                {
                    printk("\n%04x  ", row);
                    row += 16;
                }

                sfp_sram_byte_read((uint16)(i + mem_addr), &value);
                printk("%02x ", value);
            }
            printk("\n");
		}
		else if (!memcmp(cmd_addr, "w", 1))
		{
			mem_addr=(unsigned int)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);
            value = (uint8) mem_data;
            ret = sfp_sram_byte_write((uint16)(mem_addr), value);
            if(ret)
            {
    			printk("Write mirror failed(%d)\n", ret);
            }
            else
            {
    			printk("Write mirror 0x%04x data 0x%x\n", mem_addr, value);
            }
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("Mirror operation only support \"r\" and \"w\" as the first parameter\n");
		printk("Read format:	\"r <addr> <length>\"\n");
		printk("Write format:	\"w <addr> <byte_data>\"\n");
		printk("addr: 0~511\n");
	}

	return len;
}

static int sfp_eeprom_write(struct file *file, const char *buff, unsigned long len, void *data)
{
    int         ret;
    int         i, row;
    char 		tmpbuf[64];
    unsigned int	mem_addr, mem_data, mem_len;
    char		*strptr, *cmd_addr;
    char		*tokptr;
    uint8       value;

	if (buff && !copy_from_user(tmpbuf, buff, 63)) {
		tmpbuf[63] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		//printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "r", 1))
		{
			mem_addr=(unsigned int)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);

        	printk("EEPROM:\n");
        	printk("      00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");

            row = mem_addr;
            for(i = 0 ; i < mem_len ; i++) {
                if(0 == i % 16)
                {
                    printk("\n%04x  ", row);
                    row += 16;
                }

                sfp_eeprom_byte_read((uint16)(i + mem_addr), &value);
                printk("%02x ", value);
            }
            printk("\n");
		}
		else if (!memcmp(cmd_addr, "w", 1))
		{
			mem_addr=(unsigned int)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);
            value = (uint8) mem_data;
            ret = sfp_eeprom_byte_write((uint16)(mem_addr), value);
            if(ret)
            {
    			printk("Write eeprom failed(%d)\n", ret);
            }
            else
            {
    			printk("Write eeprom 0x%04x data 0x%x\n", mem_addr, value);
            }
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("EEPROM operation only support \"r\" and \"w\" as the first parameter\n");
		printk("Read format:	\"r <addr> <length>\"\n");
		printk("Write format:	\"w <addr> <byte_data>\"\n");
		printk("addr: 0~511\n");
	}

	return len;
}
#endif

static int lan_sds_mode_set(unsigned int mode)
{
    int ret;
    unsigned int value;

    if(mode >= LAN_SDS_MODE_END)
    {
        return -1;
    }

    switch(mode)
    {
    case LAN_SDS_MODE_GPHY:
        value = 1;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_MAC0_IFf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    case LAN_SDS_MODE_FIBER_1G:
    case LAN_SDS_MODE_SGMII_MAC:
        value = 0;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_MAC0_IFf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    case LAN_SDS_MODE_SGMII_PHY:
        value = 0;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_MAC0_IFf, &value)) != RT_ERR_OK)
        {
            return ret;
        }

        value = 0x16;
        if((ret = reg_field_write(RTL9601B_SDS0_PHY_SIDE_ABILITYr, RTL9601B_SDS0_PHY_SIDE_ABILITYf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    default:
        return -1;
        break;
    }

    /* Move from switch init */
    value = 1;
    if ((ret = reg_field_write(RTL9601B_WRAP_GPHY_MISCr,RTL9601B_PATCH_PHY_DONEf,&value)) != RT_ERR_OK)
    {
        return ret;
    }

    switch(mode)
    {
    case LAN_SDS_MODE_GPHY:
        value = 0x1f;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_CFG_SDS0_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        value = 1;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_SDS0_PHY_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    case LAN_SDS_MODE_FIBER_1G:
        value = 0x4;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_CFG_SDS0_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        value = 1;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_SDS0_PHY_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    case LAN_SDS_MODE_SGMII_PHY:
        value = 0x2;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_CFG_SDS0_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        value = 1;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_SDS0_PHY_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    case LAN_SDS_MODE_SGMII_MAC:
        value = 0x2;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_CFG_SDS0_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        value = 0;
        if((ret = reg_field_write(RTL9601B_LAN_PORT_INTERFACE_CFGr, RTL9601B_SDS0_PHY_MODEf, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    default:
        return -1;
        break;
    }
    lan_sds_mode = mode;

    return 0;
}

static int lan_sds_cfg_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "lan_sds_mode = %u(%s)\n", lan_sds_mode, lan_sds_mode_string[lan_sds_mode]);

	return len;
}

static int lan_sds_cfg_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
    unsigned int value;
	int len = (count > 15) ? 15 : count;
    int ret;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		value = simple_strtoul(tmpBuf, NULL, 10);
        if((ret = lan_sds_mode_set(value)) != 0)
        {
    		printk("Incorrect mode %u\n", value);
        	return -EFAULT;
        }
		printk("change mode to %u(%s)\n", lan_sds_mode, lan_sds_mode_string[lan_sds_mode]);

		return count;
	}
	return -EFAULT;
}

static int lan_sds_debug_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "LanSdsDbg: %s\n", lanSdsDbg ? "enable" : "disable");

	return len;
}

static int lan_sds_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;
    int value;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
        value = simple_strtoul(tmpBuf, NULL, 10);
        if(value != 0)
        {
            lanSdsDbg = 1;
        }
        else
        {
            lanSdsDbg = 0;
        }

		return len;
	}
	return -EFAULT;
}

static void lan_sds_proc_cmd_init(void)
{
    /* Create proc commands */
    if(NULL == lan_sds_proc_dir)
    {
        lan_sds_proc_dir = proc_mkdir("lan_sds", NULL);
    }
    if(lan_sds_proc_dir)
    {
    	lan_sds_cfg_proc = create_proc_entry("lan_sds_cfg", 0, lan_sds_proc_dir);
    	if(lan_sds_cfg_proc){
    		lan_sds_cfg_proc->read_proc = lan_sds_cfg_read;
    		lan_sds_cfg_proc->write_proc = lan_sds_cfg_write;
    	}
    }
    if(lan_sds_proc_dir)
    {
    	lan_sds_debug_proc = create_proc_entry("lan_sds_debug", 0, lan_sds_proc_dir);
    	if(lan_sds_debug_proc){
    		lan_sds_debug_proc->read_proc = lan_sds_debug_read;
    		lan_sds_debug_proc->write_proc = lan_sds_debug_write;
    	}
    }
#if CONFIG_SFP_APPLICATION
    if(lan_sds_proc_dir)
    {
    	sfp_app_proc = create_proc_entry("sfp_app", 0, lan_sds_proc_dir);
    	if(sfp_app_proc){
    		sfp_app_proc->read_proc = sfp_app_read;
    		sfp_app_proc->write_proc = sfp_app_write;
    	}
    }
    if(lan_sds_proc_dir)
    {
    	sfp_dump_mirror_proc = create_proc_entry("dump_mirror", 0, lan_sds_proc_dir);
    	if(sfp_dump_mirror_proc){
    		sfp_dump_mirror_proc->read_proc = sfp_dump_mirror_read;
    		sfp_dump_mirror_proc->write_proc = NULL;
    	}
    }
    if(lan_sds_proc_dir)
    {
    	sfp_dump_eeprom_proc = create_proc_entry("dump_eeprom", 0, lan_sds_proc_dir);
    	if(sfp_dump_eeprom_proc){
    		sfp_dump_eeprom_proc->read_proc = sfp_dump_eeprom_read;
    		sfp_dump_eeprom_proc->write_proc = NULL;
    	}
    }
    if(lan_sds_proc_dir)
    {
    	sfp_mirror_proc = create_proc_entry("mirror", 0, lan_sds_proc_dir);
    	if(sfp_mirror_proc){
    		sfp_mirror_proc->read_proc = NULL;
    		sfp_mirror_proc->write_proc = sfp_mirror_write;
    	}
    }
    if(lan_sds_proc_dir)
    {
    	sfp_eeprom_proc = create_proc_entry("eeprom", 0, lan_sds_proc_dir);
    	if(sfp_eeprom_proc){
    		sfp_eeprom_proc->read_proc = NULL;
    		sfp_eeprom_proc->write_proc = sfp_eeprom_write;
    	}
    }
#endif
}

static void lan_sds_proc_cmd_exit(void)
{
    /* Remove proc commands */
#if CONFIG_SFP_APPLICATION
    if(sfp_dump_eeprom_proc)
    {
    	remove_proc_entry("dump_eeprom", sfp_dump_eeprom_proc);
        sfp_dump_eeprom_proc = NULL;
    }
    if(sfp_dump_mirror_proc)
    {
    	remove_proc_entry("dump_mirror", sfp_dump_mirror_proc);
        sfp_dump_mirror_proc = NULL;
    }
    if(sfp_app_proc)
    {
    	remove_proc_entry("sfp_app", lan_sds_proc_dir);
        sfp_app_proc = NULL;
    }
#endif
    if(lan_sds_debug_proc)
    {
    	remove_proc_entry("lan_sds_debug", lan_sds_proc_dir);
        lan_sds_debug_proc = NULL;
    }
    if(lan_sds_cfg_proc)
    {
    	remove_proc_entry("lan_sds_cfg", lan_sds_proc_dir);
        lan_sds_cfg_proc = NULL;
    }
    if(lan_sds_proc_dir)
    {
        remove_proc_entry("lan_sds", NULL);
        lan_sds_proc_dir = NULL;
    }
}

static int lan_sds_cnt_reset(void)
{
    int ret;
    uint16 value;
    rtl9601b_sds_acc_t sds_acc;

    LAN_SDS_DBG_PRINT("counter reset\n");
    /* Select channel 0 counter */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 24;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value &= ~(0x7);
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Clear counter */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 2;
    value = 0;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}

static int lan_sds_cnt_get(uint32 *pCnt)
{
    int ret;
    uint16 value;
    rtl9601b_sds_acc_t sds_acc;

    LAN_SDS_DBG_PRINT("counter get\n");
    /* Select channel 0 counter */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 24;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value &= ~(0x7);
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }
    /* Retrive counter */
    /* bits [23:16] */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 3;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    *pCnt = (value & 0xff00) >> 8;
    *pCnt <<= 16;
    /* bits [15:0] */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 2;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    *pCnt |= value;

    return RT_ERR_OK;
}

static int lan_sds_rxEn_toggle(void)
{
    int ret;
    uint16 value;
    uint16 nwayAbility;
    rtl9601b_sds_acc_t sds_acc;

    LAN_SDS_DBG_PRINT("rx_sd toggle\n");

    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_FIB;
    sds_acc.regaddr = 0;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    nwayAbility = (value & (1 << 12)) ? 1 : 0;

    /* use the value and sds_acc from above read */
    if(nwayAbility)
    {
        /* Restart N-Way */
        value |= (1 << 9);
    }
    else
    {
        /* Enable fiber N-Way ability */
        value |= (1 << 12);
    }
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Wait 10 ms (at least 1 ms) */
    schedule_timeout(1 * HZ / 100);

    /* Force BER notify = 0 */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS;
    sds_acc.regaddr = 14;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value |= (1 << 12);
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_MISC;
    sds_acc.regaddr = 2;
    value = 0x1000;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Toggle RX_EN */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_MISC;
    sds_acc.regaddr = 0;
    value = 0x10;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }
    value = 0;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Wait 10 ms (at least 1 ms) */
    schedule_timeout(1 * HZ / 100);

    /* Release BER notify */
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_MISC;
    sds_acc.regaddr = 2;
    value = 0;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }
    sds_acc.index = RTL9601B_SDS_IDX_LAN;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS;
    sds_acc.regaddr = 14;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
    {
        return ret;
    }
    value &= ~(1 << 12);
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
    {
        return ret;
    }

    if(!nwayAbility)
    {
        sds_acc.index = RTL9601B_SDS_IDX_LAN;
        sds_acc.page = RTL9601B_SDS_PAGE_FIB;
        sds_acc.regaddr = 0;
        if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &value)) != RT_ERR_OK)
        {
            return ret;
        }
        value &= ~(1 << 12);
        if ((ret = _rtl9601b_serdes_ind_write(sds_acc, value)) != RT_ERR_OK)
        {
            return ret;
        }
    }

    /* Wait 50 ms */
    schedule_timeout(5 * HZ / 100);

    return 0;
}
static int lan_sds_monitor_thread(void *data)
{
    int ret;
    uint8 isWaiting = 0;
    uint16 value;
    uint16 accFail = 0;
    uint16 waitCnt = 0, threshold = 0;
    uint32 counter;
    rtl9601b_sds_acc_t sds_acc;

    while(!kthread_should_stop())
    {
        /* It is NOT possible to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(1 * HZ / 10); /* Wake up every 100 ms*/

        /* Check RXIDLE */
        sds_acc.index = RTL9601B_SDS_IDX_LAN;
        sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
        sds_acc.regaddr = 31;
        _rtl9601b_serdes_ind_read(sds_acc, &value);
        if(0 == (value & (1<<1)))
        {
            if(!isWaiting)
            {
                /* Reset counter */
                if ((ret = lan_sds_cnt_reset()) != RT_ERR_OK)
                {
                    printk("%s:%d lan sds counter reset failed(%d)\n", __FILE__, __LINE__, ret);
                }
                waitCnt = (waitCntCfg != 0) ? waitCntCfg : 1; /* Wait at least 1 time unit */
                threshold = thresholdCfg;
                isWaiting = 1;
            }
            else
            {
                waitCnt --;
                if(0 == waitCnt)
                {
                    /* Retrieve counter */
                    if ((ret = lan_sds_cnt_get(&counter)) != RT_ERR_OK)
                    {
                        printk("%s:%d lan sds counter get failed(%d)\n", __FILE__, __LINE__, ret);
                    }
                    if(counter >= threshold)
                    {
                        LAN_SDS_DBG_PRINT("exceed threshold (%d >= %d)\n", counter, threshold);
                        accFail ++;
                    }
                    else
                    {
                        accFail = 0;
                    }

                    if(accFail >= 2)
                    {
                        if ((ret = lan_sds_rxEn_toggle()) != RT_ERR_OK)
                        {
                            printk("%s:%d lan sds toggle failed(%d)\n", __FILE__, __LINE__, ret);
                        }
                        accFail = 0;
                    }
                    isWaiting = 0;
                }
            }
        }
        else
        {
            accFail = 0;
            isWaiting = 0;
        }
    }

    return 0;
}

int __init lan_sds_init(void)
{
    int ret;
    int value;

    if(NULL == pLanSdsMonitor)
    {
        pLanSdsMonitor = kthread_create(lan_sds_monitor_thread, NULL, "lanSdsMonitor");
        if(IS_ERR(pLanSdsMonitor))
        {
            printk("%s:%d lan sds monitor init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pLanSdsMonitor));
            pLanSdsMonitor = NULL;
        }
        else
        {
            wake_up_process(pLanSdsMonitor);
            printk("%s:%d lan sds monitor init complete!\n", __FILE__, __LINE__);
        }
    }
    /* MODIFY ME - remove when actulal release */
    value = 1;
    if((ret = reg_field_write(RTL9601B_EEPROM_DOWNLOADr, RTL9601B_SWCORE_REG_ACCESS_ENf, &value)) != RT_ERR_OK)
    {
        return ret;
    }

    lan_sds_proc_cmd_init();

    return 0;
}

void __exit lan_sds_exit(void)
{
    if(pLanSdsMonitor)
    {
        kthread_stop(pLanSdsMonitor);
    }

    lan_sds_proc_cmd_exit();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek PON SFP module");
MODULE_AUTHOR("Realtek");
module_init(lan_sds_init);
module_exit(lan_sds_exit);
#endif

