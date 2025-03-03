#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/ 
#include <bspchip.h>

//ccwei: 120208-WPS
#define WPS_PIN RTL8192CD_GPIO_2

#define LOW_ACTIVE	1
#if LOW_ACTIVE

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioClear(w); \
	else gpioSet(w); \
} while (0);

#define GPIO_READ(w) (!gpioRead(w))
		
#else

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioSet(w); \
	else gpioClear(w); \
} while (0);

#define GPIO_READ(w) (gpioRead(w))

#endif

void PCIE_reset_pin(int *reset){
	*reset = GPIO_H_2;
}
EXPORT_SYMBOL(PCIE_reset_pin);

//ccwei: 120208-WPS
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
void get_wifi_wake_pin(int *gpio_num)
{
    *gpio_num = GPIO_H_0;
}

void get_wifi_wps_pin(int *gpio_num)
{
    *gpio_num = (WPS_PIN - RTL8192CD_GPIO_0);
}
#endif
//end

#ifdef FWUPGRADE_LED
//jiunming, for telefonica, Recoverable Error during Firmware upgrade
extern unsigned char fwupgradeerrstate;
unsigned char flag_led_power_green;
unsigned char flag_led_power_red;
unsigned char flag_led_dsl;
unsigned char flag_led_internet_green;
unsigned char flag_led_internet_red;
unsigned char flag_led_wps_green;
unsigned char flag_led_wps_red;
unsigned char flag_led_wps_yellow;
#endif

static void board_10_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);

#ifdef FWUPGRADE_LED
	//jiunming, for telefonica, Recoverable Error during Firmware upgrade	
	if( fwupgradeerrstate!=1 )
	{
		switch (which) {		
		case LED_POWER_GREEN: 
			flag_led_power_green=op; 
			break;
		case LED_POWER_RED: 
			flag_led_power_red=op; 
			break;
		case LED_DSL:
			flag_led_dsl=op;
			break;
		case LED_INTERNET_GREEN:
			flag_led_internet_green=op;
			break;
		case LED_INTERNET_RED:
			flag_led_internet_red=op;
			break;
		case LED_WPS_GREEN:
			flag_led_wps_green=op;
			break;
		case LED_WPS_RED:
			flag_led_wps_red=op;
			break;
		case LED_WPS_YELLOW:
			flag_led_wps_yellow=op;
			break;		
		}
	}
	if( fwupgradeerrstate==2 ) return;
#endif
	
	switch (which) {
#ifndef CONFIG_JTAG_USABLE
	case LED_DSL:	
		GPIO_SET(GPIO_H_3, op);
		break;
#endif
	case LED_INTERNET_GREEN:
	case LED_PPP_GREEN:
		GPIO_SET(GPIO_G_1, op);
		break;
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	case LED_WPS_GREEN:
		GPIO_SET(RTL8192CD_GPIO_3, op); 
		break;
	case LED_WPS_RED:
		GPIO_SET(RTL8192CD_GPIO_0, op); 
		break;
	case LED_WPS_YELLOW:
		GPIO_SET(RTL8192CD_GPIO_1, op);
		break;
#endif
#ifdef CONFIG_SW_USB_LED1
	case LED_USB_1:
		GPIO_SET(GPIO_F_6, op);
		break;
#endif //CONFIG_SW_USB_LED1
#ifdef CONFIG_SW_USB_LED0
	case LED_USB_0:		
		GPIO_SET(GPIO_F_5, op);
		break;
#endif //CONFIG_SW_USB_LED1
	default:
		led_handle_set(which, op);
	}
}

static void board_10_handle_init(void) {
	board_10_handle_set(LED_DSL, LED_OFF);
	board_10_handle_set(LED_INTERNET_GREEN, LED_OFF);
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	board_10_handle_set(LED_WPS_GREEN, LED_OFF);
	board_10_handle_set(LED_WPS_RED, LED_OFF);
	board_10_handle_set(LED_WPS_YELLOW, LED_OFF);
#endif
#ifdef CONFIG_SW_USB_LED1
	board_10_handle_set(LED_USB_1, LED_OFF);
#endif
#ifdef CONFIG_SW_USB_LED0
	board_10_handle_set(LED_USB_0, LED_OFF);
#endif

};

static struct led_operations board_10_operation = {
	.name = "board_10",
	.handle_init = board_10_handle_init,
	.handle_set = board_10_handle_set,
};


static void board_10_pb_init(void) {
};

static int board_10_pb_is_pushed(int which) {
	switch(which) {
#ifndef CONFIG_JTAG_USABLE
	case PB_RESET:
		return GPIO_READ(GPIO_G_7);

	case PB_WIFISW:
		return GPIO_READ(GPIO_H_0);
#endif //CONFIG_JTAG_USABLE
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	case PB_WPS:
		//120208-WPS
		return GPIO_READ(WPS_PIN);
		//return GPIO_READ(RTL8192CD_GPIO_2);
#endif
	}

	return 0;
}

static struct pushbutton_operations board_10_pb_op = {
	.handle_init = board_10_pb_init,
	.handle_is_pushed = board_10_pb_is_pushed,
};

static int __init board_10_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif //CONFIG_JTAG_USABLE
	REG32(BSP_MISC_PINMUX) &= ~(BSP_UR0_GPG0G1 
#ifdef CONFIG_SW_USB_LED0
		| BSP_USBLED0_GPF5
#endif //CONFIG_SW_USB_LED0
#ifdef CONFIG_SW_USB_LED1
		| BSP_USBLED1_GPF6
#endif //CONFIG_SW_USB_LED1
	);
	
	//REG32(BSP_MISC_PINMUX) |= (BSP_USBLED1_GPF6 | BSP_USBLED0_GPF5); // enable this if using HW USBLED

	REG32(BSP_MISC_PINMUX) |= (BSP_LEDS0_GPH4 | BSP_LEDS1_GPH5 | BSP_LEDS2_GPH6 | BSP_LEDS3_GPH7 | BSP_LEDP0_GPF0);	//setup switch LED
	if (IS_8676V2)
		REG32(BSP_RGMII_PAD_CTRL) &= ~0x40000000; // Set to GPIO mode (USBLED1)
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
	REG32(BSP_IP_SEL) |= (BSP_EN_PCM | BSP_EN_VOIPACC);
	REG32(BSP_MISC_PINMUX) |= BSP_VOIP_GPIO_PINMUX_G1;
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
	REG32(BSP_PCM_SPI_IND_CTRL) &= ~BSP_SPI_GPIO_G1; //G1 GPIO-simulated SPI
	REG32(BSP_VOIP_SPI_CS_IND_CTRL) &= ~BSP_VOIP_SPI_CS1; //GPIO-simulated SPI CS1
//	REG32(BSP_VOIP_SPI_CS_IND_CTRL) &= ~BSP_VOIP_SPI_CS2; //GPIO-simulated SPI CS2
//#elif defined (CONFIG_RTK_VOIP_8676_ISI_ZSI)
//	REG32(BSP_MISC_PINMUX) |= (BSP_ZSI_ISI_PINMUX_C0 | BSP_ZSI_ISI_PINMUX_C1);
#endif
#endif
	led_register_operations(&board_10_operation);
	pb_register_operations(&board_10_pb_op);
	return 0;
}

static void __exit board_10_led_exit(void) {
}


module_init(board_10_led_init);
module_exit(board_10_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



