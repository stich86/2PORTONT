# Makefile.in generated by automake 1.11.6 from Makefile.am.
# libtransmission/Makefile.  Generated from Makefile.in by configure.

# Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software
# Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.






am__make_dryrun = \
  { \
    am__dry=no; \
    case $$MAKEFLAGS in \
      *\\[\ \	]*) \
        echo 'am--echo: ; @echo "AM"  OK' | $(MAKE) -f - 2>/dev/null \
          | grep '^AM OK$$' >/dev/null || am__dry=yes;; \
      *) \
        for am__flg in $$MAKEFLAGS; do \
          case $$am__flg in \
            *=*|--*) ;; \
            *n*) am__dry=yes; break;; \
          esac; \
        done;; \
    esac; \
    test $$am__dry = yes; \
  }
pkgdatadir = $(datadir)/transmission
pkgincludedir = $(includedir)/transmission
pkglibdir = $(libdir)/transmission
pkglibexecdir = $(libexecdir)/transmission
am__cd = CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd
install_sh_DATA = $(install_sh) -c -m 644
install_sh_PROGRAM = $(install_sh) -c
install_sh_SCRIPT = $(install_sh) -c
INSTALL_HEADER = $(INSTALL_DATA)
transform = $(program_transform_name)
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_triplet = i686-pc-linux-gnu
host_triplet = mips-unknown-linux-gnu
TESTS = blocklist-test$(EXEEXT) bencode-test$(EXEEXT) \
	clients-test$(EXEEXT) history-test$(EXEEXT) json-test$(EXEEXT) \
	magnet-test$(EXEEXT) metainfo-test$(EXEEXT) \
	peer-msgs-test$(EXEEXT) rpc-test$(EXEEXT) \
	test-peer-id$(EXEEXT) utils-test$(EXEEXT)
noinst_PROGRAMS = $(am__EXEEXT_1)
subdir = libtransmission
DIST_COMMON = $(noinst_HEADERS) $(srcdir)/Makefile.am \
	$(srcdir)/Makefile.in
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
am__aclocal_m4_deps = $(top_srcdir)/m4/acx-pthread.m4 \
	$(top_srcdir)/m4/check-ssl.m4 $(top_srcdir)/m4/glib-gettext.m4 \
	$(top_srcdir)/m4/libtool.m4 $(top_srcdir)/m4/ltoptions.m4 \
	$(top_srcdir)/m4/ltsugar.m4 $(top_srcdir)/m4/ltversion.m4 \
	$(top_srcdir)/m4/lt~obsolete.m4 $(top_srcdir)/m4/pkg.m4 \
	$(top_srcdir)/m4/zlib.m4 $(top_srcdir)/configure.ac
am__configure_deps = $(am__aclocal_m4_deps) $(CONFIGURE_DEPENDENCIES) \
	$(ACLOCAL_M4)
mkinstalldirs = $(install_sh) -d
CONFIG_CLEAN_FILES =
CONFIG_CLEAN_VPATH_FILES =
LIBRARIES = $(noinst_LIBRARIES)
ARFLAGS = cru
AM_V_AR = $(am__v_AR_$(V))
am__v_AR_ = $(am__v_AR_$(AM_DEFAULT_VERBOSITY))
am__v_AR_0 = @echo "  AR    " $@;
AM_V_at = $(am__v_at_$(V))
am__v_at_ = $(am__v_at_$(AM_DEFAULT_VERBOSITY))
am__v_at_0 = @
libtransmission_a_AR = $(AR) $(ARFLAGS)
libtransmission_a_LIBADD =
am_libtransmission_a_OBJECTS = announcer.$(OBJEXT) \
	announcer-http.$(OBJEXT) announcer-udp.$(OBJEXT) \
	bandwidth.$(OBJEXT) bencode.$(OBJEXT) bitfield.$(OBJEXT) \
	blocklist.$(OBJEXT) cache.$(OBJEXT) clients.$(OBJEXT) \
	completion.$(OBJEXT) ConvertUTF.$(OBJEXT) crypto.$(OBJEXT) \
	fdlimit.$(OBJEXT) handshake.$(OBJEXT) history.$(OBJEXT) \
	inout.$(OBJEXT) json.$(OBJEXT) JSON_parser.$(OBJEXT) \
	list.$(OBJEXT) magnet.$(OBJEXT) makemeta.$(OBJEXT) \
	metainfo.$(OBJEXT) natpmp.$(OBJEXT) net.$(OBJEXT) \
	peer-io.$(OBJEXT) peer-mgr.$(OBJEXT) peer-msgs.$(OBJEXT) \
	platform.$(OBJEXT) port-forwarding.$(OBJEXT) \
	ptrarray.$(OBJEXT) resume.$(OBJEXT) rpcimpl.$(OBJEXT) \
	rpc-server.$(OBJEXT) session.$(OBJEXT) stats.$(OBJEXT) \
	torrent.$(OBJEXT) torrent-ctor.$(OBJEXT) \
	torrent-magnet.$(OBJEXT) tr-dht.$(OBJEXT) tr-lpd.$(OBJEXT) \
	tr-udp.$(OBJEXT) tr-utp.$(OBJEXT) tr-getopt.$(OBJEXT) \
	trevent.$(OBJEXT) upnp.$(OBJEXT) utils.$(OBJEXT) \
	verify.$(OBJEXT) web.$(OBJEXT) webseed.$(OBJEXT) \
	wildmat.$(OBJEXT)
libtransmission_a_OBJECTS = $(am_libtransmission_a_OBJECTS)
am__EXEEXT_1 = blocklist-test$(EXEEXT) bencode-test$(EXEEXT) \
	clients-test$(EXEEXT) history-test$(EXEEXT) json-test$(EXEEXT) \
	magnet-test$(EXEEXT) metainfo-test$(EXEEXT) \
	peer-msgs-test$(EXEEXT) rpc-test$(EXEEXT) \
	test-peer-id$(EXEEXT) utils-test$(EXEEXT)
PROGRAMS = $(noinst_PROGRAMS)
am_bencode_test_OBJECTS = bencode-test.$(OBJEXT)
bencode_test_OBJECTS = $(am_bencode_test_OBJECTS)
am__DEPENDENCIES_1 = ./libtransmission.a
bencode_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
AM_V_lt = $(am__v_lt_$(V))
am__v_lt_ = $(am__v_lt_$(AM_DEFAULT_VERBOSITY))
am__v_lt_0 = --silent
bencode_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(bencode_test_LDFLAGS) $(LDFLAGS) -o $@
am_blocklist_test_OBJECTS = blocklist-test.$(OBJEXT)
blocklist_test_OBJECTS = $(am_blocklist_test_OBJECTS)
blocklist_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
blocklist_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link $(CCLD) \
	$(AM_CFLAGS) $(CFLAGS) $(blocklist_test_LDFLAGS) $(LDFLAGS) -o \
	$@
am_clients_test_OBJECTS = clients-test.$(OBJEXT)
clients_test_OBJECTS = $(am_clients_test_OBJECTS)
clients_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
clients_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(clients_test_LDFLAGS) $(LDFLAGS) -o $@
am_history_test_OBJECTS = history-test.$(OBJEXT)
history_test_OBJECTS = $(am_history_test_OBJECTS)
history_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
history_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(history_test_LDFLAGS) $(LDFLAGS) -o $@
am_json_test_OBJECTS = json-test.$(OBJEXT)
json_test_OBJECTS = $(am_json_test_OBJECTS)
json_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
json_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(json_test_LDFLAGS) $(LDFLAGS) -o $@
am_magnet_test_OBJECTS = magnet-test.$(OBJEXT)
magnet_test_OBJECTS = $(am_magnet_test_OBJECTS)
magnet_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
magnet_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(magnet_test_LDFLAGS) $(LDFLAGS) -o $@
am_metainfo_test_OBJECTS = metainfo-test.$(OBJEXT)
metainfo_test_OBJECTS = $(am_metainfo_test_OBJECTS)
metainfo_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
metainfo_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(metainfo_test_LDFLAGS) $(LDFLAGS) -o $@
am_peer_msgs_test_OBJECTS = peer-msgs-test.$(OBJEXT)
peer_msgs_test_OBJECTS = $(am_peer_msgs_test_OBJECTS)
peer_msgs_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
peer_msgs_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link $(CCLD) \
	$(AM_CFLAGS) $(CFLAGS) $(peer_msgs_test_LDFLAGS) $(LDFLAGS) -o \
	$@
am_rpc_test_OBJECTS = rpc-test.$(OBJEXT)
rpc_test_OBJECTS = $(am_rpc_test_OBJECTS)
rpc_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
rpc_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(rpc_test_LDFLAGS) $(LDFLAGS) -o $@
am_test_peer_id_OBJECTS = test-peer-id.$(OBJEXT)
test_peer_id_OBJECTS = $(am_test_peer_id_OBJECTS)
test_peer_id_DEPENDENCIES = $(am__DEPENDENCIES_1)
test_peer_id_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(test_peer_id_LDFLAGS) $(LDFLAGS) -o $@
am_utils_test_OBJECTS = utils-test.$(OBJEXT)
utils_test_OBJECTS = $(am_utils_test_OBJECTS)
utils_test_DEPENDENCIES = $(am__DEPENDENCIES_1)
utils_test_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(utils_test_LDFLAGS) $(LDFLAGS) -o $@
DEFAULT_INCLUDES = -I.
depcomp = $(SHELL) $(top_srcdir)/depcomp
am__depfiles_maybe = depfiles
am__mv = mv -f
COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
LTCOMPILE = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=compile $(CC) $(DEFS) \
	$(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
	$(AM_CFLAGS) $(CFLAGS)
AM_V_CC = $(am__v_CC_$(V))
am__v_CC_ = $(am__v_CC_$(AM_DEFAULT_VERBOSITY))
am__v_CC_0 = @echo "  CC    " $@;
CCLD = $(CC)
LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
	$(AM_LDFLAGS) $(LDFLAGS) -o $@
AM_V_CCLD = $(am__v_CCLD_$(V))
am__v_CCLD_ = $(am__v_CCLD_$(AM_DEFAULT_VERBOSITY))
am__v_CCLD_0 = @echo "  CCLD  " $@;
AM_V_GEN = $(am__v_GEN_$(V))
am__v_GEN_ = $(am__v_GEN_$(AM_DEFAULT_VERBOSITY))
am__v_GEN_0 = @echo "  GEN   " $@;
SOURCES = $(libtransmission_a_SOURCES) $(bencode_test_SOURCES) \
	$(blocklist_test_SOURCES) $(clients_test_SOURCES) \
	$(history_test_SOURCES) $(json_test_SOURCES) \
	$(magnet_test_SOURCES) $(metainfo_test_SOURCES) \
	$(peer_msgs_test_SOURCES) $(rpc_test_SOURCES) \
	$(test_peer_id_SOURCES) $(utils_test_SOURCES)
DIST_SOURCES = $(libtransmission_a_SOURCES) $(bencode_test_SOURCES) \
	$(blocklist_test_SOURCES) $(clients_test_SOURCES) \
	$(history_test_SOURCES) $(json_test_SOURCES) \
	$(magnet_test_SOURCES) $(metainfo_test_SOURCES) \
	$(peer_msgs_test_SOURCES) $(rpc_test_SOURCES) \
	$(test_peer_id_SOURCES) $(utils_test_SOURCES)
am__can_run_installinfo = \
  case $$AM_UPDATE_INFO_DIR in \
    n|no|NO) false;; \
    *) (install-info --version) >/dev/null 2>&1;; \
  esac
HEADERS = $(noinst_HEADERS)
ETAGS = etags
CTAGS = ctags
am__tty_colors = \
red=; grn=; lgn=; blu=; std=
DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
ACLOCAL = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/missing --run aclocal-1.11
ALL_LINGUAS = 
AMTAR = $${TAR-tar}
AM_DEFAULT_VERBOSITY = 0
AR = ar
AUTOCONF = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/missing --run autoconf
AUTOHEADER = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/missing --run autoheader
AUTOMAKE = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/missing --run automake-1.11
AWK = gawk
CATALOGS = 
CATOBJEXT = NONE
CC = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2 -std=gnu99 -ggdb3 -Wall -W -Wpointer-arith -Wformat-security -Wcast-align -Wundef -Wcast-align -Wstrict-prototypes -Wmissing-declarations -Wmissing-format-attribute -Wredundant-decls -Wnested-externs -Wunused-parameter -Wwrite-strings -Winline -Wfloat-equal -Wextra -Wdeclaration-after-statement -Winit-self -Wvariadic-macros
CPP = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-gcc -E
CPPFLAGS =  -DNDEBUG
CURL_MINIMUM = 7.15.4
CXX = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-g++
CXXCPP = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-g++ -E
CXXDEPMODE = depmode=gcc3
CXXFLAGS = -g -O2
CYGPATH_W = echo
DATADIRNAME = lib
DEFS = -DPACKAGE_NAME=\"transmission\" -DPACKAGE_TARNAME=\"transmission\" -DPACKAGE_VERSION=\"2.73\" -DPACKAGE_STRING=\"transmission\ 2.73\" -DPACKAGE_BUGREPORT=\"http://trac.transmissionbt.com/newticket\" -DPACKAGE_URL=\"\" -DPACKAGE=\"transmission\" -DVERSION=\"2.73\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DSTDC_HEADERS=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_STDBOOL_H=1 -DHAVE_ICONV_OPEN=1 -DHAVE_PREAD=1 -DHAVE_PWRITE=1 -DHAVE_STRLCPY=1 -DHAVE_DAEMON=1 -DHAVE_DIRNAME=1 -DHAVE_BASENAME=1 -DHAVE_STRCASECMP=1 -DHAVE_LOCALTIME_R=1 -DHAVE_POSIX_FALLOCATE=1 -DHAVE_MEMMEM=1 -DHAVE_STRSEP=1 -DHAVE_STRTOLD=1 -DHAVE_SYSLOG=1 -DHAVE_VALLOC=1 -DHAVE_GETPAGESIZE=1 -DHAVE_POSIX_MEMALIGN=1 -DHAVE_STATVFS=1 -DHAVE_MKDTEMP=1 -DHAVE_PTHREAD=1 -DHAVE__TMP_DUMMY1_ZLIB_H=1 -D_FILE_OFFSET_BITS=64 -DHAVE_LSEEK64=1 -DHAVE_DECL_POSIX_FADVISE=1 -DHAVE_POSIX_FADVISE=1 -DWITH_INOTIFY=1 -DHAVE_SYS_STATVFS_H=1 -DWITH_UTP=1 -DHAVE_MINIUPNP_16=1 -DGETTEXT_PACKAGE=\"transmission-gtk\" -DHAVE_LOCALE_H=1 -DHAVE_LC_MESSAGES=1 -DHAVE_ZLIB=1
DEPDIR = .deps
DHT_CFLAGS = -I$(top_srcdir)/third-party/dht
DHT_LIBS = $(top_builddir)/third-party/dht/libdht.a
DLLTOOL = false
DSYMUTIL = 
DUMPBIN = :
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EGREP = /bin/grep -E
EXEEXT = 
FGREP = /bin/grep -F
GETTEXT_PACKAGE = transmission-gtk
GIO_MINIMUM = 2.26.0
GLIB_MINIMUM = 2.32.0
GMOFILES = 
GMSGFMT = /usr/bin/msgfmt
GREP = /bin/grep
GTK_CFLAGS = 
GTK_LIBS = 
GTK_MINIMUM = 3.4.0
HAVE_CXX = yes
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = $(install_sh) -c -s
INSTOBJEXT = 
INTLLIBS = 
INTLTOOL_EXTRACT = /usr/bin/intltool-extract
INTLTOOL_MERGE = /usr/bin/intltool-merge
INTLTOOL_PERL = /usr/bin/perl
INTLTOOL_UPDATE = /usr/bin/intltool-update
INTLTOOL_V_MERGE = $(INTLTOOL__v_MERGE_$(V))
INTLTOOL_V_MERGE_OPTIONS = $(intltool__v_merge_options_$(V))
INTLTOOL__v_MERGE_ = $(INTLTOOL__v_MERGE_$(AM_DEFAULT_VERBOSITY))
INTLTOOL__v_MERGE_0 = @echo "  ITMRG " $@;
LD = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-ld
LDFLAGS = 
LIBAPPINDICATOR_CFLAGS = 
LIBAPPINDICATOR_LIBS = 
LIBAPPINDICATOR_MINIMUM = 0.4.90
LIBCURL_CFLAGS = -I/home/ql_xu/develop/uClinux-dist/user/curl-7.36.0/include
LIBCURL_LIBS = -L/home/ql_xu/develop/uClinux-dist/user/curl-7.36.0/lib/.libs -lcurl
LIBEVENT_CFLAGS = -I/home/ql_xu/develop/uClinux-dist/lib/libevent/include
LIBEVENT_LIBS = -L/home/ql_xu/develop/uClinux-dist/lib/libevent/.libs -levent
LIBEVENT_MINIUM = 
LIBNATPMP_CFLAGS = -I$(top_srcdir)/third-party/libnatpmp/
LIBNATPMP_LIBS = $(top_builddir)/third-party/libnatpmp/libnatpmp.a
LIBNATPMP_LIBS_QT = $${TRANSMISSION_TOP}/third-party/libnatpmp/libnatpmp.a
LIBOBJS = 
LIBS = -lm -L/home/ql_xu/develop/uClinux-dist/lib/zlib -lz
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBUPNP_CFLAGS = -I$(top_srcdir)/third-party/
LIBUPNP_LIBS = $(top_builddir)/third-party/miniupnp/libminiupnp.a
LIBUPNP_LIBS_QT = $${TRANSMISSION_TOP}/third-party/miniupnp/libminiupnp.a
LIBUTP_CFLAGS = -I$(top_srcdir)/third-party/
LIBUTP_LIBS = $(top_builddir)/third-party/libutp/libutp.a -lrt
LIBUTP_LIBS_QT = $${TRANSMISSION_TOP}/third-party/libutp/libutp.a -lrt
LIPO = 
LN_S = ln -s
LTLIBOBJS = 
MAKEINFO = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/missing --run makeinfo
MANIFEST_TOOL = :
MKDIR_P = /bin/mkdir -p
MKINSTALLDIRS = ./mkinstalldirs
MSGFMT = /usr/bin/msgfmt
MSGMERGE = /usr/bin/msgmerge
NM = nm
NMEDIT = 
OBJDUMP = objdump
OBJEXT = o
OPENSSL_CFLAGS = -I/home/ql_xu/develop/uClinux-dist/lib/libssl/include
OPENSSL_LIBS = -L/home/ql_xu/develop/uClinux-dist/lib/libssl -lssl -lcrypto
OPENSSL_MINIMUM = 0.9.4
OTOOL = 
OTOOL64 = 
PACKAGE = transmission
PACKAGE_BUGREPORT = http://trac.transmissionbt.com/newticket
PACKAGE_NAME = transmission
PACKAGE_STRING = transmission 2.73
PACKAGE_TARNAME = transmission
PACKAGE_URL = 
PACKAGE_VERSION = 2.73
PATH_SEPARATOR = :
PEERID_PREFIX = -TR2730-
PKG_CONFIG = /usr/bin/pkg-config
POFILES = 
POSUB = po
PO_IN_DATADIR_FALSE = 
PO_IN_DATADIR_TRUE = 
PTHREAD_CC = /rsdk-4.4.7-5281-EB-2.6.30-0.9.33-m32ut-140520/bin/rsdk-linux-gcc
PTHREAD_CFLAGS = -pthread
PTHREAD_LIBS = 
RANLIB = ranlib
SED = /bin/sed
SET_MAKE = 
SHELL = /bin/sh
STRIP = strip
USERAGENT_PREFIX = 2.73
USE_NLS = yes
VERSION = 2.73
WINDRES = 
XGETTEXT = :
ZLIB_CFLAGS =  -I/home/ql_xu/develop/uClinux-dist/lib/zlib/
ZLIB_LDFLAGS = -L/home/ql_xu/develop/uClinux-dist/lib/zlib
ZLIB_LIBS = -lm -lz
abs_builddir = /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/libtransmission
abs_srcdir = /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/libtransmission
abs_top_builddir = /home/ql_xu/develop/uClinux-dist/user/transmission-2.73
abs_top_srcdir = /home/ql_xu/develop/uClinux-dist/user/transmission-2.73
ac_ct_AR = ar
ac_ct_CC = 
ac_ct_CXX = 
ac_ct_DUMPBIN = link -dump
acx_pthread_config = 
am__include = include
am__leading_dot = .
am__quote = 
am__tar = tar --format=posix -chf - "$$tardir"
am__untar = tar -xf -
bindir = ${exec_prefix}/bin
build = i686-pc-linux-gnu
build_alias = 
build_cpu = i686
build_os = linux-gnu
build_vendor = pc
builddir = .
datadir = ${datarootdir}
datarootdir = ${prefix}/share
docdir = ${datarootdir}/doc/${PACKAGE_TARNAME}
dvidir = ${docdir}
exec_prefix = ${prefix}
host = mips-unknown-linux-gnu
host_alias = mips-linux
host_cpu = mips
host_os = linux-gnu
host_vendor = unknown
htmldir = ${docdir}
includedir = ${prefix}/include
infodir = ${datarootdir}/info
install_sh = ${SHELL} /home/ql_xu/develop/uClinux-dist/user/transmission-2.73/install-sh
intltool__v_merge_options_ = $(intltool__v_merge_options_$(AM_DEFAULT_VERBOSITY))
intltool__v_merge_options_0 = -q
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localedir = ${datarootdir}/locale
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
mkdir_p = /bin/mkdir -p
oldincludedir = /usr/include
pdfdir = ${docdir}
prefix = /usr/local
program_transform_name = s,x,x,
psdir = ${docdir}
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
target_alias = 
top_build_prefix = ../
top_builddir = ..
top_srcdir = ..
transmissionlocaledir = ${prefix}/${DATADIRNAME}/locale
AM_CPPFLAGS = \
    -I$(top_srcdir) \
    -D__TRANSMISSION__ \
    -DPACKAGE_DATA_DIR=\""$(datadir)"\"

AM_CFLAGS = \
    -I$(top_srcdir)/third-party/dht \
    -I$(top_srcdir)/third-party/ \
    -I$(top_srcdir)/third-party/ \
    -I$(top_srcdir)/third-party/libnatpmp/ \
    -I/home/ql_xu/develop/uClinux-dist/lib/libevent/include \
    -I/home/ql_xu/develop/uClinux-dist/user/curl-7.36.0/include \
    -I/home/ql_xu/develop/uClinux-dist/lib/libssl/include \
    -pthread \
     -I/home/ql_xu/develop/uClinux-dist/lib/zlib/

noinst_LIBRARIES = libtransmission.a
libtransmission_a_SOURCES = \
    announcer.c \
    announcer-http.c \
    announcer-udp.c \
    bandwidth.c \
    bencode.c \
    bitfield.c \
    blocklist.c \
    cache.c \
    clients.c \
    completion.c \
    ConvertUTF.c \
    crypto.c \
    fdlimit.c \
    handshake.c \
    history.c \
    inout.c \
    json.c \
    JSON_parser.c \
    list.c \
    magnet.c \
    makemeta.c \
    metainfo.c \
    natpmp.c \
    net.c \
    peer-io.c \
    peer-mgr.c \
    peer-msgs.c \
    platform.c \
    port-forwarding.c \
    ptrarray.c \
    resume.c \
    rpcimpl.c \
    rpc-server.c \
    session.c \
    stats.c \
    torrent.c \
    torrent-ctor.c \
    torrent-magnet.c \
    tr-dht.c \
    tr-lpd.c \
    tr-udp.c \
    tr-utp.c \
    tr-getopt.c \
    trevent.c \
    upnp.c \
    utils.c \
    verify.c \
    web.c \
    webseed.c \
    wildmat.c

noinst_HEADERS = \
    announcer.h \
    announcer-common.h \
    bandwidth.h \
    bencode.h \
    bitfield.h \
    blocklist.h \
    cache.h \
    clients.h \
    ConvertUTF.h \
    crypto.h \
    completion.h \
    fdlimit.h \
    handshake.h \
    history.h \
    inout.h \
    json.h \
    JSON_parser.h \
    libtransmission-test.h \
    list.h \
    magnet.h \
    makemeta.h \
    metainfo.h \
    natpmp_local.h \
    net.h \
    peer-common.h \
    peer-io.h \
    peer-mgr.h \
    peer-msgs.h \
    platform.h \
    port-forwarding.h \
    ptrarray.h \
    resume.h \
    rpcimpl.h \
    rpc-server.h \
    session.h \
    stats.h \
    torrent.h \
    torrent-magnet.h \
    tr-getopt.h \
    transmission.h \
    tr-dht.h \
    tr-udp.h \
    tr-utp.h \
    tr-lpd.h \
    trevent.h \
    upnp.h \
    utils.h \
    verify.h \
    version.h \
    web.h \
    webseed.h

apps_ldflags = \
    

apps_ldadd = \
    ./libtransmission.a  \
    $(top_builddir)/third-party/miniupnp/libminiupnp.a \
    $(top_builddir)/third-party/libnatpmp/libnatpmp.a \
     \
    $(top_builddir)/third-party/dht/libdht.a \
    $(top_builddir)/third-party/libutp/libutp.a -lrt \
    -L/home/ql_xu/develop/uClinux-dist/user/curl-7.36.0/lib/.libs -lcurl \
    -L/home/ql_xu/develop/uClinux-dist/lib/libevent/.libs -levent \
    -L/home/ql_xu/develop/uClinux-dist/lib/libssl -lssl -lcrypto \
    -L/home/ql_xu/develop/uClinux-dist/lib/zlib -lz \
     \
    -lm 

bencode_test_SOURCES = bencode-test.c
bencode_test_LDADD = ${apps_ldadd}
bencode_test_LDFLAGS = ${apps_ldflags}
blocklist_test_SOURCES = blocklist-test.c
blocklist_test_LDADD = ${apps_ldadd}
blocklist_test_LDFLAGS = ${apps_ldflags}
clients_test_SOURCES = clients-test.c
clients_test_LDADD = ${apps_ldadd}
clients_test_LDFLAGS = ${apps_ldflags}
history_test_SOURCES = history-test.c
history_test_LDADD = ${apps_ldadd}
history_test_LDFLAGS = ${apps_ldflags}
json_test_SOURCES = json-test.c
json_test_LDADD = ${apps_ldadd}
json_test_LDFLAGS = ${apps_ldflags}
magnet_test_SOURCES = magnet-test.c
magnet_test_LDADD = ${apps_ldadd}
magnet_test_LDFLAGS = ${apps_ldflags}
metainfo_test_SOURCES = metainfo-test.c
metainfo_test_LDADD = ${apps_ldadd}
metainfo_test_LDFLAGS = ${apps_ldflags}
peer_msgs_test_SOURCES = peer-msgs-test.c
peer_msgs_test_LDADD = ${apps_ldadd}
peer_msgs_test_LDFLAGS = ${apps_ldflags}
rpc_test_SOURCES = rpc-test.c
rpc_test_LDADD = ${apps_ldadd}
rpc_test_LDFLAGS = ${apps_ldflags}
test_peer_id_SOURCES = test-peer-id.c
test_peer_id_LDADD = ${apps_ldadd}
test_peer_id_LDFLAGS = ${apps_ldflags}
utils_test_SOURCES = utils-test.c
utils_test_LDADD = ${apps_ldadd}
utils_test_LDFLAGS = ${apps_ldflags}
all: all-am

.SUFFIXES:
.SUFFIXES: .c .lo .o .obj
$(srcdir)/Makefile.in:  $(srcdir)/Makefile.am  $(am__configure_deps)
	@for dep in $?; do \
	  case '$(am__configure_deps)' in \
	    *$$dep*) \
	      ( cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh ) \
	        && { if test -f $@; then exit 0; else break; fi; }; \
	      exit 1;; \
	  esac; \
	done; \
	echo ' cd $(top_srcdir) && $(AUTOMAKE) --gnu libtransmission/Makefile'; \
	$(am__cd) $(top_srcdir) && \
	  $(AUTOMAKE) --gnu libtransmission/Makefile
.PRECIOUS: Makefile
Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@case '$?' in \
	  *config.status*) \
	    cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh;; \
	  *) \
	    echo ' cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe)'; \
	    cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe);; \
	esac;

$(top_builddir)/config.status: $(top_srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh

$(top_srcdir)/configure:  $(am__configure_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
$(ACLOCAL_M4):  $(am__aclocal_m4_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
$(am__aclocal_m4_deps):

clean-noinstLIBRARIES:
	-test -z "$(noinst_LIBRARIES)" || rm -f $(noinst_LIBRARIES)
libtransmission.a: $(libtransmission_a_OBJECTS) $(libtransmission_a_DEPENDENCIES) $(EXTRA_libtransmission_a_DEPENDENCIES) 
	$(AM_V_at)-rm -f libtransmission.a
	$(AM_V_AR)$(libtransmission_a_AR) libtransmission.a $(libtransmission_a_OBJECTS) $(libtransmission_a_LIBADD)
	$(AM_V_at)$(RANLIB) libtransmission.a

clean-noinstPROGRAMS:
	@list='$(noinst_PROGRAMS)'; test -n "$$list" || exit 0; \
	echo " rm -f" $$list; \
	rm -f $$list || exit $$?; \
	test -n "$(EXEEXT)" || exit 0; \
	list=`for p in $$list; do echo "$$p"; done | sed 's/$(EXEEXT)$$//'`; \
	echo " rm -f" $$list; \
	rm -f $$list
bencode-test$(EXEEXT): $(bencode_test_OBJECTS) $(bencode_test_DEPENDENCIES) $(EXTRA_bencode_test_DEPENDENCIES) 
	@rm -f bencode-test$(EXEEXT)
	$(AM_V_CCLD)$(bencode_test_LINK) $(bencode_test_OBJECTS) $(bencode_test_LDADD) $(LIBS)
blocklist-test$(EXEEXT): $(blocklist_test_OBJECTS) $(blocklist_test_DEPENDENCIES) $(EXTRA_blocklist_test_DEPENDENCIES) 
	@rm -f blocklist-test$(EXEEXT)
	$(AM_V_CCLD)$(blocklist_test_LINK) $(blocklist_test_OBJECTS) $(blocklist_test_LDADD) $(LIBS)
clients-test$(EXEEXT): $(clients_test_OBJECTS) $(clients_test_DEPENDENCIES) $(EXTRA_clients_test_DEPENDENCIES) 
	@rm -f clients-test$(EXEEXT)
	$(AM_V_CCLD)$(clients_test_LINK) $(clients_test_OBJECTS) $(clients_test_LDADD) $(LIBS)
history-test$(EXEEXT): $(history_test_OBJECTS) $(history_test_DEPENDENCIES) $(EXTRA_history_test_DEPENDENCIES) 
	@rm -f history-test$(EXEEXT)
	$(AM_V_CCLD)$(history_test_LINK) $(history_test_OBJECTS) $(history_test_LDADD) $(LIBS)
json-test$(EXEEXT): $(json_test_OBJECTS) $(json_test_DEPENDENCIES) $(EXTRA_json_test_DEPENDENCIES) 
	@rm -f json-test$(EXEEXT)
	$(AM_V_CCLD)$(json_test_LINK) $(json_test_OBJECTS) $(json_test_LDADD) $(LIBS)
magnet-test$(EXEEXT): $(magnet_test_OBJECTS) $(magnet_test_DEPENDENCIES) $(EXTRA_magnet_test_DEPENDENCIES) 
	@rm -f magnet-test$(EXEEXT)
	$(AM_V_CCLD)$(magnet_test_LINK) $(magnet_test_OBJECTS) $(magnet_test_LDADD) $(LIBS)
metainfo-test$(EXEEXT): $(metainfo_test_OBJECTS) $(metainfo_test_DEPENDENCIES) $(EXTRA_metainfo_test_DEPENDENCIES) 
	@rm -f metainfo-test$(EXEEXT)
	$(AM_V_CCLD)$(metainfo_test_LINK) $(metainfo_test_OBJECTS) $(metainfo_test_LDADD) $(LIBS)
peer-msgs-test$(EXEEXT): $(peer_msgs_test_OBJECTS) $(peer_msgs_test_DEPENDENCIES) $(EXTRA_peer_msgs_test_DEPENDENCIES) 
	@rm -f peer-msgs-test$(EXEEXT)
	$(AM_V_CCLD)$(peer_msgs_test_LINK) $(peer_msgs_test_OBJECTS) $(peer_msgs_test_LDADD) $(LIBS)
rpc-test$(EXEEXT): $(rpc_test_OBJECTS) $(rpc_test_DEPENDENCIES) $(EXTRA_rpc_test_DEPENDENCIES) 
	@rm -f rpc-test$(EXEEXT)
	$(AM_V_CCLD)$(rpc_test_LINK) $(rpc_test_OBJECTS) $(rpc_test_LDADD) $(LIBS)
test-peer-id$(EXEEXT): $(test_peer_id_OBJECTS) $(test_peer_id_DEPENDENCIES) $(EXTRA_test_peer_id_DEPENDENCIES) 
	@rm -f test-peer-id$(EXEEXT)
	$(AM_V_CCLD)$(test_peer_id_LINK) $(test_peer_id_OBJECTS) $(test_peer_id_LDADD) $(LIBS)
utils-test$(EXEEXT): $(utils_test_OBJECTS) $(utils_test_DEPENDENCIES) $(EXTRA_utils_test_DEPENDENCIES) 
	@rm -f utils-test$(EXEEXT)
	$(AM_V_CCLD)$(utils_test_LINK) $(utils_test_OBJECTS) $(utils_test_LDADD) $(LIBS)

mostlyclean-compile:
	-rm -f *.$(OBJEXT)

distclean-compile:
	-rm -f *.tab.c

include ./$(DEPDIR)/ConvertUTF.Po
include ./$(DEPDIR)/JSON_parser.Po
include ./$(DEPDIR)/announcer-http.Po
include ./$(DEPDIR)/announcer-udp.Po
include ./$(DEPDIR)/announcer.Po
include ./$(DEPDIR)/bandwidth.Po
include ./$(DEPDIR)/bencode-test.Po
include ./$(DEPDIR)/bencode.Po
include ./$(DEPDIR)/bitfield.Po
include ./$(DEPDIR)/blocklist-test.Po
include ./$(DEPDIR)/blocklist.Po
include ./$(DEPDIR)/cache.Po
include ./$(DEPDIR)/clients-test.Po
include ./$(DEPDIR)/clients.Po
include ./$(DEPDIR)/completion.Po
include ./$(DEPDIR)/crypto.Po
include ./$(DEPDIR)/fdlimit.Po
include ./$(DEPDIR)/handshake.Po
include ./$(DEPDIR)/history-test.Po
include ./$(DEPDIR)/history.Po
include ./$(DEPDIR)/inout.Po
include ./$(DEPDIR)/json-test.Po
include ./$(DEPDIR)/json.Po
include ./$(DEPDIR)/list.Po
include ./$(DEPDIR)/magnet-test.Po
include ./$(DEPDIR)/magnet.Po
include ./$(DEPDIR)/makemeta.Po
include ./$(DEPDIR)/metainfo-test.Po
include ./$(DEPDIR)/metainfo.Po
include ./$(DEPDIR)/natpmp.Po
include ./$(DEPDIR)/net.Po
include ./$(DEPDIR)/peer-io.Po
include ./$(DEPDIR)/peer-mgr.Po
include ./$(DEPDIR)/peer-msgs-test.Po
include ./$(DEPDIR)/peer-msgs.Po
include ./$(DEPDIR)/platform.Po
include ./$(DEPDIR)/port-forwarding.Po
include ./$(DEPDIR)/ptrarray.Po
include ./$(DEPDIR)/resume.Po
include ./$(DEPDIR)/rpc-server.Po
include ./$(DEPDIR)/rpc-test.Po
include ./$(DEPDIR)/rpcimpl.Po
include ./$(DEPDIR)/session.Po
include ./$(DEPDIR)/stats.Po
include ./$(DEPDIR)/test-peer-id.Po
include ./$(DEPDIR)/torrent-ctor.Po
include ./$(DEPDIR)/torrent-magnet.Po
include ./$(DEPDIR)/torrent.Po
include ./$(DEPDIR)/tr-dht.Po
include ./$(DEPDIR)/tr-getopt.Po
include ./$(DEPDIR)/tr-lpd.Po
include ./$(DEPDIR)/tr-udp.Po
include ./$(DEPDIR)/tr-utp.Po
include ./$(DEPDIR)/trevent.Po
include ./$(DEPDIR)/upnp.Po
include ./$(DEPDIR)/utils-test.Po
include ./$(DEPDIR)/utils.Po
include ./$(DEPDIR)/verify.Po
include ./$(DEPDIR)/web.Po
include ./$(DEPDIR)/webseed.Po
include ./$(DEPDIR)/wildmat.Po

.c.o:
	$(AM_V_CC)$(COMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	$(AM_V_at)$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po
#	$(AM_V_CC)source='$<' object='$@' libtool=no \
#	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) \
#	$(AM_V_CC_no)$(COMPILE) -c $<

.c.obj:
	$(AM_V_CC)$(COMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ `$(CYGPATH_W) '$<'`
	$(AM_V_at)$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po
#	$(AM_V_CC)source='$<' object='$@' libtool=no \
#	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) \
#	$(AM_V_CC_no)$(COMPILE) -c `$(CYGPATH_W) '$<'`

.c.lo:
	$(AM_V_CC)$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	$(AM_V_at)$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
#	$(AM_V_CC)source='$<' object='$@' libtool=yes \
#	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) \
#	$(AM_V_CC_no)$(LTCOMPILE) -c -o $@ $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

ID: $(HEADERS) $(SOURCES) $(LISP) $(TAGS_FILES)
	list='$(SOURCES) $(HEADERS) $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '{ files[$$0] = 1; nonempty = 1; } \
	      END { if (nonempty) { for (i in files) print i; }; }'`; \
	mkid -fID $$unique
tags: TAGS

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) \
		$(TAGS_FILES) $(LISP)
	set x; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)  $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '{ files[$$0] = 1; nonempty = 1; } \
	      END { if (nonempty) { for (i in files) print i; }; }'`; \
	shift; \
	if test -z "$(ETAGS_ARGS)$$*$$unique"; then :; else \
	  test -n "$$unique" || unique=$$empty_fix; \
	  if test $$# -gt 0; then \
	    $(ETAGS) $(ETAGSFLAGS) $(AM_ETAGSFLAGS) $(ETAGS_ARGS) \
	      "$$@" $$unique; \
	  else \
	    $(ETAGS) $(ETAGSFLAGS) $(AM_ETAGSFLAGS) $(ETAGS_ARGS) \
	      $$unique; \
	  fi; \
	fi
ctags: CTAGS
CTAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) \
		$(TAGS_FILES) $(LISP)
	list='$(SOURCES) $(HEADERS)  $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '{ files[$$0] = 1; nonempty = 1; } \
	      END { if (nonempty) { for (i in files) print i; }; }'`; \
	test -z "$(CTAGS_ARGS)$$unique" \
	  || $(CTAGS) $(CTAGSFLAGS) $(AM_CTAGSFLAGS) $(CTAGS_ARGS) \
	     $$unique

GTAGS:
	here=`$(am__cd) $(top_builddir) && pwd` \
	  && $(am__cd) $(top_srcdir) \
	  && gtags -i $(GTAGS_ARGS) "$$here"

distclean-tags:
	-rm -f TAGS ID GTAGS GRTAGS GSYMS GPATH tags

check-TESTS: $(TESTS)
	@failed=0; all=0; xfail=0; xpass=0; skip=0; \
	srcdir=$(srcdir); export srcdir; \
	list=' $(TESTS) '; \
	$(am__tty_colors); \
	if test -n "$$list"; then \
	  for tst in $$list; do \
	    if test -f ./$$tst; then dir=./; \
	    elif test -f $$tst; then dir=; \
	    else dir="$(srcdir)/"; fi; \
	    if $(TESTS_ENVIRONMENT) $${dir}$$tst; then \
	      all=`expr $$all + 1`; \
	      case " $(XFAIL_TESTS) " in \
	      *[\ \	]$$tst[\ \	]*) \
		xpass=`expr $$xpass + 1`; \
		failed=`expr $$failed + 1`; \
		col=$$red; res=XPASS; \
	      ;; \
	      *) \
		col=$$grn; res=PASS; \
	      ;; \
	      esac; \
	    elif test $$? -ne 77; then \
	      all=`expr $$all + 1`; \
	      case " $(XFAIL_TESTS) " in \
	      *[\ \	]$$tst[\ \	]*) \
		xfail=`expr $$xfail + 1`; \
		col=$$lgn; res=XFAIL; \
	      ;; \
	      *) \
		failed=`expr $$failed + 1`; \
		col=$$red; res=FAIL; \
	      ;; \
	      esac; \
	    else \
	      skip=`expr $$skip + 1`; \
	      col=$$blu; res=SKIP; \
	    fi; \
	    echo "$${col}$$res$${std}: $$tst"; \
	  done; \
	  if test "$$all" -eq 1; then \
	    tests="test"; \
	    All=""; \
	  else \
	    tests="tests"; \
	    All="All "; \
	  fi; \
	  if test "$$failed" -eq 0; then \
	    if test "$$xfail" -eq 0; then \
	      banner="$$All$$all $$tests passed"; \
	    else \
	      if test "$$xfail" -eq 1; then failures=failure; else failures=failures; fi; \
	      banner="$$All$$all $$tests behaved as expected ($$xfail expected $$failures)"; \
	    fi; \
	  else \
	    if test "$$xpass" -eq 0; then \
	      banner="$$failed of $$all $$tests failed"; \
	    else \
	      if test "$$xpass" -eq 1; then passes=pass; else passes=passes; fi; \
	      banner="$$failed of $$all $$tests did not behave as expected ($$xpass unexpected $$passes)"; \
	    fi; \
	  fi; \
	  dashes="$$banner"; \
	  skipped=""; \
	  if test "$$skip" -ne 0; then \
	    if test "$$skip" -eq 1; then \
	      skipped="($$skip test was not run)"; \
	    else \
	      skipped="($$skip tests were not run)"; \
	    fi; \
	    test `echo "$$skipped" | wc -c` -le `echo "$$banner" | wc -c` || \
	      dashes="$$skipped"; \
	  fi; \
	  report=""; \
	  if test "$$failed" -ne 0 && test -n "$(PACKAGE_BUGREPORT)"; then \
	    report="Please report to $(PACKAGE_BUGREPORT)"; \
	    test `echo "$$report" | wc -c` -le `echo "$$banner" | wc -c` || \
	      dashes="$$report"; \
	  fi; \
	  dashes=`echo "$$dashes" | sed s/./=/g`; \
	  if test "$$failed" -eq 0; then \
	    col="$$grn"; \
	  else \
	    col="$$red"; \
	  fi; \
	  echo "$${col}$$dashes$${std}"; \
	  echo "$${col}$$banner$${std}"; \
	  test -z "$$skipped" || echo "$${col}$$skipped$${std}"; \
	  test -z "$$report" || echo "$${col}$$report$${std}"; \
	  echo "$${col}$$dashes$${std}"; \
	  test "$$failed" -eq 0; \
	else :; fi

distdir: $(DISTFILES)
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list='$(DISTFILES)'; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) $(MKDIR_P) `echo "$$dist_files" | \
			   sed '/\//!d;s|^|$(distdir)/|;s,/[^/]*$$,,' | \
			   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d "$(distdir)/$$file"; then \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -fpR $(srcdir)/$$file "$(distdir)$$dir" || exit 1; \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    cp -fpR $$d/$$file "$(distdir)$$dir" || exit 1; \
	  else \
	    test -f "$(distdir)/$$file" \
	    || cp -p $$d/$$file "$(distdir)/$$file" \
	    || exit 1; \
	  fi; \
	done
check-am: all-am
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS
check: check-am
all-am: Makefile $(LIBRARIES) $(PROGRAMS) $(HEADERS)
installdirs:
install: install-am
install-exec: install-exec-am
install-data: install-data-am
uninstall: uninstall-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-am
install-strip:
	if test -z '$(STRIP)'; then \
	  $(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	    install_sh_PROGRAM="$(INSTALL_STRIP_PROGRAM)" INSTALL_STRIP_FLAG=-s \
	      install; \
	else \
	  $(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	    install_sh_PROGRAM="$(INSTALL_STRIP_PROGRAM)" INSTALL_STRIP_FLAG=-s \
	    "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'" install; \
	fi
mostlyclean-generic:

clean-generic:

distclean-generic:
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)
	-test . = "$(srcdir)" || test -z "$(CONFIG_CLEAN_VPATH_FILES)" || rm -f $(CONFIG_CLEAN_VPATH_FILES)

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-am

clean-am: clean-generic clean-libtool clean-noinstLIBRARIES \
	clean-noinstPROGRAMS mostlyclean-am

distclean: distclean-am
	-rm -rf ./$(DEPDIR)
	-rm -f Makefile
distclean-am: clean-am distclean-compile distclean-generic \
	distclean-tags

dvi: dvi-am

dvi-am:

html: html-am

html-am:

info: info-am

info-am:

install-data-am:

install-dvi: install-dvi-am

install-dvi-am:

install-exec-am:

install-html: install-html-am

install-html-am:

install-info: install-info-am

install-info-am:

install-man:

install-pdf: install-pdf-am

install-pdf-am:

install-ps: install-ps-am

install-ps-am:

installcheck-am:

maintainer-clean: maintainer-clean-am
	-rm -rf ./$(DEPDIR)
	-rm -f Makefile
maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-am

mostlyclean-am: mostlyclean-compile mostlyclean-generic \
	mostlyclean-libtool

pdf: pdf-am

pdf-am:

ps: ps-am

ps-am:

uninstall-am:

.MAKE: check-am install-am install-strip

.PHONY: CTAGS GTAGS all all-am check check-TESTS check-am clean \
	clean-generic clean-libtool clean-noinstLIBRARIES \
	clean-noinstPROGRAMS ctags distclean distclean-compile \
	distclean-generic distclean-libtool distclean-tags distdir dvi \
	dvi-am html html-am info info-am install install-am \
	install-data install-data-am install-dvi install-dvi-am \
	install-exec install-exec-am install-html install-html-am \
	install-info install-info-am install-man install-pdf \
	install-pdf-am install-ps install-ps-am install-strip \
	installcheck installcheck-am installdirs maintainer-clean \
	maintainer-clean-generic mostlyclean mostlyclean-compile \
	mostlyclean-generic mostlyclean-libtool pdf pdf-am ps ps-am \
	tags uninstall uninstall-am


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
