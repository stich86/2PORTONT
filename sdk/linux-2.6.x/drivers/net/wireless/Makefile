#
# Makefile for the Linux Wireless network device drivers.
#

obj-$(CONFIG_IPW2100) += ipw2x00/
obj-$(CONFIG_IPW2200) += ipw2x00/

obj-$(CONFIG_STRIP) += strip.o
obj-$(CONFIG_ARLAN) += arlan.o 

arlan-objs := arlan-main.o arlan-proc.o

# Obsolete cards
obj-$(CONFIG_WAVELAN)		+= wavelan.o
obj-$(CONFIG_PCMCIA_NETWAVE)	+= netwave_cs.o
obj-$(CONFIG_PCMCIA_WAVELAN)	+= wavelan_cs.o

obj-$(CONFIG_HERMES)		+= orinoco/

obj-$(CONFIG_AIRO)		+= airo.o
obj-$(CONFIG_AIRO_CS)		+= airo_cs.o airo.o

obj-$(CONFIG_ATMEL)             += atmel.o
obj-$(CONFIG_PCI_ATMEL)         += atmel_pci.o 
obj-$(CONFIG_PCMCIA_ATMEL)      += atmel_cs.o

obj-$(CONFIG_AT76C50X_USB)      += at76c50x-usb.o

obj-$(CONFIG_PRISM54)		+= prism54/

obj-$(CONFIG_HOSTAP)		+= hostap/
obj-$(CONFIG_B43)		+= b43/
obj-$(CONFIG_B43LEGACY)		+= b43legacy/
obj-$(CONFIG_ZD1211RW)		+= zd1211rw/
obj-$(CONFIG_RTL8180)		+= rtl818x/
obj-$(CONFIG_RTL8187)		+= rtl818x/

# 16-bit wireless PCMCIA client drivers
obj-$(CONFIG_PCMCIA_RAYCS)	+= ray_cs.o
obj-$(CONFIG_PCMCIA_WL3501)	+= wl3501_cs.o

obj-$(CONFIG_USB_NET_RNDIS_WLAN)	+= rndis_wlan.o

obj-$(CONFIG_USB_ZD1201)	+= zd1201.o
obj-$(CONFIG_LIBERTAS)		+= libertas/

obj-$(CONFIG_LIBERTAS_THINFIRM)	+= libertas_tf/

obj-$(CONFIG_ADM8211)	+= adm8211.o

obj-$(CONFIG_MWL8K)	+= mwl8k.o

obj-$(CONFIG_IWLWIFI)	+= iwlwifi/
obj-$(CONFIG_RT2X00)	+= rt2x00/

obj-$(CONFIG_P54_COMMON)	+= p54/

obj-$(CONFIG_ATH5K)	+= ath5k/
obj-$(CONFIG_ATH9K)	+= ath9k/
obj-$(CONFIG_AR9170_USB)	+= ar9170/

obj-$(CONFIG_MAC80211_HWSIM)	+= mac80211_hwsim.o
obj-$(CONFIG_RTL8192SE)         += rtl8190/

ifneq ($(CONFIG_NO_WLAN_DRIVER),y)
obj-$(CONFIG_RTL8192CD)         += rtl8192cd/
endif

obj-$(CONFIG_RTL8192SE_mac80211)         += rtlmac80211Drv/

obj-$(CONFIG_VIRTUAL_WLAN_DRIVER) 	 += vwlan.o

