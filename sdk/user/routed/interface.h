/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	From: @(#)interface.h	5.6 (Berkeley) 6/1/90
 *	From: @(#)interface.h	8.1 (Berkeley) 6/5/93
 *	$Id: interface.h,v 1.6 2010/11/12 06:57:09 masonyu Exp $
 */

/*
 * Routing table management daemon.
 */

/*
 * An ``interface'' is similar to an ifnet structure,
 * except it doesn't contain q'ing info, and it also
 * handles ``logical'' interfaces (remote gateways
 * that we want to keep polling even if they go down).
 * The list of interfaces which we maintain is used
 * in supplying the gratuitous routing table updates.
 */
struct interface {
	struct	interface *int_next;
	struct	sockaddr int_addr;		/* address on this host */
	union {
		struct	sockaddr intu_broadaddr;
		struct	sockaddr intu_dstaddr;
	} int_intu;
#define	int_broadaddr	int_intu.intu_broadaddr	/* broadcast address */
#define	int_dstaddr	int_intu.intu_dstaddr	/* other end of p-to-p link */
	int	int_metric;			/* init's routing entry */
	int	int_flags;			/* see below */
	/* START INTERNET SPECIFIC */
	u_long	int_net;			/* network # */
	u_long	int_netmask;			/* net mask for addr */
	u_long	int_subnet;			/* subnet # (classless) */
	u_long	int_subnetmask;			/* subnet mask for addr (classless)*/
	/* END INTERNET SPECIFIC */
	struct	ifdebug int_input, int_output;	/* packet tracing stuff */
	int	int_ipackets;			/* input packets received */
	int	int_opackets;			/* output packets sent */
	char	*int_name;			/* from kernel if structure */
	u_short	int_transitions;		/* times gone up-down */
};


#if (defined(__GLIBC__) && (__GLIBC__ >= 2)) || defined(__UC_LIBC__)
#include <net/if.h>
#else

/*
 * 0x1 to 0x10 are reused from the kernel's ifnet definitions,
 * the others agree with the RTS_ flags defined elsewhere.
 */
#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* software loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */

#endif

#define	IFF_SUBNET	0x100000	/* interface on subnetted network */
#define	IFF_PASSIVE	0x200000	/* can't tell if up/down */
#define	IFF_INTERFACE	0x400000	/* hardware interface */
#define	IFF_REMOTE	0x800000	/* interface isn't on this machine */

//ql
#include "../../config/autoconf.h"

#define RIP_IFF_ACTIVE	0
#define RIP_IFF_PASSIVE 1

struct cfg_interface {
	struct cfg_interface *if_next;
	char 	if_name[IFNAMSIZ];
	int	if_flags;
	int	receive_mode;        // Added by Mason Yu for bind interface
	int	send_mode;           // Added by Mason Yu for bind interface
#ifdef CONFIG_BOA_WEB_E8B_CH
	int operation;			//Added by ql_xu for select operation mode: 0-active  1-passive
#endif
};

extern struct interface *ifnet;

struct	interface *if_ifwithaddr(struct sockaddr *);
struct	interface *if_ifwithdstaddr(struct sockaddr *);
struct	interface *if_ifwithnet(struct sockaddr *);
// Mason Yu. Parse subnet error
//struct interface *if_iflookup(struct sockaddr *);
struct interface *if_iflookup(struct sockaddr *addr, char * ifname);
struct interface *if_iflookup_by_name(char * ifname);
struct interface *if_iflookup_p2p_by_name(char * ifname);
