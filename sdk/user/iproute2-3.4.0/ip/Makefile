IPOBJ=ip.o ipaddress.o ipaddrlabel.o iproute.o iprule.o ipnetns.o \
    rtm_map.o iptunnel.o ip6tunnel.o tunnel.o ipneigh.o ipntable.o iplink.o \
    ipmaddr.o ipmonitor.o ipmroute.o ipprefix.o iptuntap.o \
    ipxfrm.o xfrm_state.o xfrm_policy.o xfrm_monitor.o \
    iplink_vlan.o link_veth.o link_gre.o iplink_can.o \
    iplink_macvlan.o iplink_macvtap.o ipl2tp.o

RTMONOBJ=rtmon.o

include ../Config

ifeq ($(IP_CONFIG_SETNS),y)
	CFLAGS += -DHAVE_SETNS
endif

ALLOBJ=$(IPOBJ) $(RTMONOBJ)
SCRIPTS=ifcfg rtpr routel routef
TARGETS=ip rtmon

all: $(TARGETS) $(SCRIPTS)

ip: $(IPOBJ) $(LIBNETLINK)


rtmon: $(LIBNETLINK) $(RTMONOBJ)

romfs:
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_IFCFG  /bin/ifcfg
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_IP     /bin/ip
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_ROUTEF /bin/routef
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_ROUTEL /bin/routel
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTACCT /bin/rtacct
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTMON  /bin/rtmon
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTPR   /bin/rtpr

clean:
	rm -f $(ALLOBJ) $(TARGETS)

SHARED_LIBS ?= y
ifeq ($(SHARED_LIBS),y)

LDLIBS += -ldl
LDFLAGS += -Wl,-export-dynamic

else

ip: static-syms.o
static-syms.o: static-syms.h
static-syms.h: $(wildcard *.c)
	files="$^" ; \
	for s in `grep -B 3 '\<dlsym' $$files | sed -n '/snprintf/{s:.*"\([^"]*\)".*:\1:;s:%s::;p}'` ; do \
		sed -n '/'$$s'[^ ]* =/{s:.* \([^ ]*'$$s'[^ ]*\) .*:extern char \1[] __attribute__((weak)); if (!strcmp(sym, "\1")) return \1;:;p}' $$files ; \
	done > $@

endif
