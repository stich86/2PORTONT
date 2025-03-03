/*
 *	GRE over IPv4 demultiplexer driver
 *
 *	Authors: Dmitry Kozlov (xeb@mail.ru)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <net/protocol.h>
#include <net/gre.h>


static const struct gre_protocol *gre_proto[GREPROTO_MAX] __read_mostly;
static DEFINE_SPINLOCK(gre_proto_lock);

int ipv6_gre_add_protocol(const struct gre_protocol *proto, u8 version)
{
	if (version >= GREPROTO_MAX)
		goto err_out;

	spin_lock(&gre_proto_lock);
	if (gre_proto[version])
		goto err_out_unlock;

	rcu_assign_pointer(gre_proto[version], proto);
	spin_unlock(&gre_proto_lock);
	return 0;

err_out_unlock:
	spin_unlock(&gre_proto_lock);
err_out:
	return -1;
}
EXPORT_SYMBOL_GPL(ipv6_gre_add_protocol);

int ipv6_gre_del_protocol(const struct gre_protocol *proto, u8 version)
{
	if (version >= GREPROTO_MAX)
		goto err_out;

	spin_lock(&gre_proto_lock);
	if (rcu_dereference(gre_proto[version]) != proto)
		goto err_out_unlock;
	rcu_assign_pointer(gre_proto[version], NULL);
	spin_unlock(&gre_proto_lock);
	synchronize_rcu();
	return 0;

err_out_unlock:
	spin_unlock(&gre_proto_lock);
err_out:
	return -1;
}
EXPORT_SYMBOL_GPL(ipv6_gre_del_protocol);

static int gre_rcv(struct sk_buff *skb)
{
	const struct gre_protocol *proto;
	u8 ver;
	int ret;

	if (!pskb_may_pull(skb, 12))
		goto drop;

	ver = skb->data[1]&0x7f;
	if (ver >= GREPROTO_MAX)
		goto drop;

	rcu_read_lock();
	proto = rcu_dereference(gre_proto[ver]);
	if (!proto || !proto->handler)
		goto drop_unlock;
	ret = proto->handler(skb);
	rcu_read_unlock();
	return ret;

drop_unlock:
	rcu_read_unlock();
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static void gre_err(struct sk_buff *skb, u32 info)
{
	const struct gre_protocol *proto;
	u8 ver;

	if (!pskb_may_pull(skb, 12))
		goto drop;

	ver = skb->data[1]&0x7f;
	if (ver >= GREPROTO_MAX)
		goto drop;

	rcu_read_lock();
	proto = rcu_dereference(gre_proto[ver]);
	if (!proto || !proto->err_handler)
		goto drop_unlock;
	proto->err_handler(skb, info);
	rcu_read_unlock();
	return;

drop_unlock:
	rcu_read_unlock();
drop:
	/* QL fix bug: skb will be double free in icmp_rcv. */
	//kfree_skb(skb);
	return;
}

static const struct inet6_protocol net_gre_protocol = {
	.handler     = gre_rcv,
	.err_handler = gre_err,
	.flags		=	INET6_PROTO_NOPOLICY|INET6_PROTO_FINAL,
};

static int __init gre_init(void)
{
	pr_info("GRE over IPv6 demultiplexor driver");

	if (inet6_add_protocol(&net_gre_protocol, IPPROTO_GRE) < 0) {
		pr_err("gre: can't add protocol\n");
		return -EAGAIN;
	}

	return 0;
}

static void __exit gre_exit(void)
{
	inet6_del_protocol(&net_gre_protocol, IPPROTO_GRE);
}

module_init(gre_init);
module_exit(gre_exit);

MODULE_DESCRIPTION("GRE over IPv4 demultiplexer driver");
MODULE_AUTHOR("D. Kozlov (xeb@mail.ru)");
MODULE_LICENSE("GPL");

