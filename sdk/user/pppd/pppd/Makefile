EXEC = pppd
OBJS = main.o magic.o fsm.o lcp.o ipcp.o ccp.o  upap.o chap.o \
	auth.o options.o utils.o sys-linux.o md5.o
PLUGINOBJS =

PPP_EXINCS = -I./ $(INCNET)
#PPP_DEFS += -D_linux_=1 -DNO_DRAND48 -DHAVE_PATHS_H -DXDEBUGALL -DPLUGIN
PPP_DEFS += -D_linux_=1 -DNO_DRAND48 -DHAVE_PATHS_H -DXDEBUGALL -DEMBED
CFLAGS += $(PPP_DEFS) $(PPP_EXINCS)
EXTRALIBS = 
FLTFLAGS += -s 8192

ifeq ($(CONFIG_USER_FLATFSD_FLATFSD),y)
  CFLAGS += -DPATH_ETC_CONFIG
endif

#MPPE=$(CONFIG_USER_PPPD_WITH_MPPE)
#DYNAMIC=y
#CBCP_SUPPORT=y

ifeq ($(MPPE),y)
  OBJS += md4.o chap_ms.o cbcp.o mppe.o sha1dgst.o extra_crypto.o
  PPP_EXINCS += $(INCDES)
  CFLAGS += -DCHAPMS=1 -DMSLANMAN=1 -DMPPE=1
  EXTRALIBS += $(LIBDES)
  ifdef DYNAMIC
    CFLAGS += -DDYNAMIC=1
  endif
  ifdef CBCP_SUPPORT
    CFLAGS += -DCBCP_SUPPORT=1
  endif
endif

ifeq ($(CONFIG_USER_PPPD_WITH_TACACS),y)
  PLUGINOBJS += plugins/tacacs.o plugins/tacc/lib/libtac.a
  CFLAGS += -DPLUGIN_TACACS
endif

ifeq ($(CONFIG_USER_PPPD_WITH_RADIUS),y)
  PLUGINOBJS += plugins/radius.o plugins/librad.o
  CFLAGS += -DPLUGIN_RADIUS
endif

ifeq ($(CONFIG_USER_PPPD_WITH_PPPOE),y)
	PLUGINOBJS += plugins/pppoe/libpppoe.a 
	OBJS += plugins/pppoe/pppoe.o
	CFLAGS += -DPLUGIN_PPPOE
endif

ifeq ($(CONFIG_USER_PPPD_WITH_PPPOA),y)
	PLUGINOBJS += plugins/pppoa/libpppoa.a 
	OBJS += plugins/pppoa/pppoatm.o
	CFLAGS += -DPLUGIN_PPPOA
endif

EXTRALIBS += $(LIBCRYPT)

all:
	[ "$(CONFIG_USER_PPPD_WITH_TACACS)" != "y" ] || make -C plugins tacacs
	[ "$(CONFIG_USER_PPPD_WITH_RADIUS)" != "y" ] || make -C plugins radius
	[ "$(CONFIG_USER_PPPD_WITH_PPPOE)" != "y" ] || make -C plugins pppoe
	[ "$(CONFIG_USER_PPPD_WITH_PPPOA)" != "y" ] || make -C plugins pppoa
	make $(EXEC)

$(EXEC): $(OBJS) $(PLUGINOBJS)
	$(CC) $(LDFLAGS)   -o $@ $(OBJS) $(PLUGINOBJS) $(EXTRALIBS) $(LDLIBS$(LDLIBS-$(@))) -L../boa/src/LINUX $(LIBMIB)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	make -C plugins clean
	-rm -f $(EXEC) *.elf *.gdb *.o

