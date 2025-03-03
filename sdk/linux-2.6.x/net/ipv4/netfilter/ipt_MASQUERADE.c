/* Masquerade.  Simple mapping which alters range to a local IP address
   (depending on route). */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <net/netfilter/nf_nat_rule.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>

#include <net/rtl/rtl_types.h>
#ifdef CONFIG_RTL_HARDWARE_NAT 
#include <net/rtl/rtl867x_hwnat_api.h>

/*2007-12-19*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ip_api.h>
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L4
#include <net/rtl/rtl865x_nat.h>
#endif

#ifndef CONFIG_RTL_MULTI_ETH_WAN
extern char masq_if[IFNAMSIZ];
#else
#include "rtl865x_ip.h"
extern char masq_if[IP_NUMBER][IFNAMSIZ];
#endif

extern unsigned int hw_napt_ip;
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <net/rtl/rtl865x_localPublic.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: automatic-address SNAT");

/* Lock protects masq region inside conntrack */
static DEFINE_RWLOCK(masq_lock);

/* FIXME: Multiple targets. --RR */
static bool masquerade_tg_check(const struct xt_tgchk_param *par)
{
	const struct nf_nat_multi_range_compat *mr = par->targinfo;

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		pr_debug("masquerade_check: bad MAP_IPS.\n");
		return false;
	}
	if (mr->rangesize != 1) {
		pr_debug("masquerade_check: bad rangesize %u\n", mr->rangesize);
		return false;
	}
	return true;
}

static unsigned int
masquerade_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	struct nf_conn *ct;
	struct nf_conn_nat *nat;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_range newrange;
	const struct nf_nat_multi_range_compat *mr;
	const struct rtable *rt;
	__be32 newsrc;

	NF_CT_ASSERT(par->hooknum == NF_INET_POST_ROUTING);

	ct = nf_ct_get(skb, &ctinfo);
	nat = nfct_nat(ct);

	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED
			    || ctinfo == IP_CT_RELATED + IP_CT_IS_REPLY));

	/* Source address is 0.0.0.0 - locally generated packet that is
	 * probably not supposed to be masqueraded.
	 */
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
		return NF_ACCEPT;

	mr = par->targinfo;
	rt = skb->rtable;

	newsrc = inet_select_addr(par->out, rt->rt_gateway, RT_SCOPE_UNIVERSE);
	if (!newsrc) {
		printk("MASQUERADE: %s ate my IP address\n", par->out->name);
		return NF_DROP;
	}


	write_lock_bh(&masq_lock);
	nat->masq_index = par->out->ifindex;
	write_unlock_bh(&masq_lock);

	/* Transfer from original range. */
	newrange = ((struct nf_nat_range)
		{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS,
		  newsrc, newsrc,
		  mr->range[0].min, mr->range[0].max });

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_SRC);
}

static int
device_cmp(struct nf_conn *i, void *ifindex)
{
	const struct nf_conn_nat *nat = nfct_nat(i);
	int ret;

	if (!nat)
		return 0;

	read_lock_bh(&masq_lock);
	ret = (nat->masq_index == (int)(long)ifindex);
	read_unlock_bh(&masq_lock);

	return ret;
}

static int masq_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	const struct net_device *dev = ptr;
	struct net *net = dev_net(dev);

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		NF_CT_ASSERT(dev->ifindex != 0);

		nf_ct_iterate_cleanup(net, device_cmp,
				      (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int masq_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;
	
	if(event == NETDEV_DOWN)
		masq_device_event(this, event, dev);

#if defined(CONFIG_RTL_HARDWARE_NAT  ) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if(rtl8676_napt_extip_handle(dev->name, event, ((struct in_ifaddr *)ptr)->ifa_local)!=0){
		printk("extip process failed! name:%s event:%d ip:%x\n", dev->name, (unsigned int)event, (unsigned int)((struct in_ifaddr *)ptr)->ifa_local);
	}
#endif

	return NOTIFY_DONE;

}

static struct notifier_block masq_dev_notifier = {
	.notifier_call	= masq_device_event,
};

static struct notifier_block masq_inet_notifier = {
	.notifier_call	= masq_inet_event,
};

static struct xt_target masquerade_tg_reg __read_mostly = {
	.name		= "MASQUERADE",
	.family		= NFPROTO_IPV4,
	.target		= masquerade_tg,
	.targetsize	= sizeof(struct nf_nat_multi_range_compat),
	.table		= "nat",
	.hooks		= 1 << NF_INET_POST_ROUTING,
	.checkentry	= masquerade_tg_check,
	.me		= THIS_MODULE,
};

static int __init masquerade_tg_init(void)
{
	int ret;

	ret = xt_register_target(&masquerade_tg_reg);

	if (ret == 0) {
		/* Register for device down reports */
		register_netdevice_notifier(&masq_dev_notifier);
		/* Register IP address change reports */
		register_inetaddr_notifier(&masq_inet_notifier);
	}

	return ret;
}

static void __exit masquerade_tg_exit(void)
{
	xt_unregister_target(&masquerade_tg_reg);
	unregister_netdevice_notifier(&masq_dev_notifier);
	unregister_inetaddr_notifier(&masq_inet_notifier);
}

module_init(masquerade_tg_init);
module_exit(masquerade_tg_exit);
