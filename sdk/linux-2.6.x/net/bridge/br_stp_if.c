/*
 *	Spanning tree protocol; interface code
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>

#include "br_private.h"
#include "br_private_stp.h"
#include <net/rtl/rtl_alias.h>

#if defined (CONFIG_RTL_STP)
#include <rtl_nic.h>
#include <l2Driver/rtl865x_stp.h>
#endif

/* Port id is composed of priority and port number.
 * NB: least significant bits of priority are dropped to
 *     make room for more ports.
 */
static inline port_id br_make_port_id(__u8 priority, __u16 port_no)
{
	return ((u16)priority << BR_PORT_BITS)
		| (port_no & ((1<<BR_PORT_BITS)-1));
}

/* called under bridge lock */
void br_init_port(struct net_bridge_port *p)
{
#if defined (CONFIG_RTL_STP)
	int retval=0, Port;
	char name[IFNAMSIZ];
	struct net_device *pseudo_dev;		
#endif
#if defined(CONFIG_RTL_HW_STP)
	int retval, i;
	uint32 vid, portMask;
#endif

	p->port_id = br_make_port_id(p->priority, p->port_no);
	br_become_designated_port(p);
	p->state = BR_STATE_BLOCKING;
	
#if defined (CONFIG_RTL_STP)
	if(strncmp(p->dev->name,"port",4) == 0)
	{

		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];
		#if defined (CONFIG_RTK_MESH)
		if((Port != WLAN_PSEUDO_IF_INDEX) && (Port != WLAN_MESH_PSEUDO_IF_INDEX) &&(Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif	
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_BLOCKING);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_BLOCKING);
		}
		else  if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_IF_NAME)) == NULL)
			{	
				return ;
			}
			pseudo_dev->br_port->state = BR_STATE_BLOCKING;
			retval = SUCCESS;
		}
		#if defined (CONFIG_RTK_MESH)
		else  if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_MESH_IF_NAME)) == NULL)
			{	
				return ;
			}
			pseudo_dev->br_port->state = BR_STATE_BLOCKING;
			retval = SUCCESS;
		}
		#endif
		else if(Port == NO_MAPPING)
		{
			p->state = BR_STATE_DISABLED;
		}
	}
#endif

#if defined(CONFIG_RTL_HW_STP)
	vid=0;
	portMask=0;

	#if define(CONFIG_RTL_NETIF_MAPPING)
	{
		ps_drv_netif_mapping_t *entry;
		entry = rtl_get_ps_drv_netif_mapping_by_psdev(p->dev);

		retval = rtl865x_getNetifVid(entry?entry->drvName:p->dev->name,&vid);
	}
	#else
	if(strcmp(p->dev->name,"eth0")==0)
	{
		retval=rtl865x_getNetifVid("br0", &vid);
	}	
	else
		retval=rtl865x_getNetifVid(p->dev->name, &vid);
	#endif
	
	if(retval==FAILED){
//		printk("%s(%d): rtl865x_getNetifVid failed.\n",__FUNCTION__,__LINE__);
	}
	else{
		portMask=rtl865x_getVlanPortMask(vid);
		for ( i = 0 ; i < MAX_RTL_STP_PORT_WH; i ++ ){
			if((1<<i)&portMask){
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_BLOCKING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_BLOCKING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif
	
	p->topology_change_ack = 0;
	p->config_pending = 0;
#if 0
// Chris: stp+mesh
#ifdef STP_DISABLE_ETH
	p->disable_by_mesh = 0;
	del_timer(&p->eth_disable_timer);
	init_timer(&p->eth_disable_timer);
#endif
#endif

}

/* called under bridge lock */
void br_stp_enable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	mod_timer(&br->hello_timer, jiffies + br->hello_time);
	mod_timer(&br->gc_timer, jiffies + HZ/10);

	br_config_bpdu_generation(br);

	list_for_each_entry(p, &br->port_list, list) {
		if ((p->dev->flags & IFF_UP) && netif_carrier_ok(p->dev))
			br_stp_enable_port(p);

	}
	spin_unlock_bh(&br->lock);
}

/* NO locks held */
void br_stp_disable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED)
	{
	#if 0
	#if defined (STP_DISABLE_ETH)
	//Chris:  stp+mesh
			p->disable_by_mesh = 0;
	#endif
	#endif 
			br_stp_disable_port(p);
}

	}

	br->topology_change = 0;
	br->topology_change_detected = 0;
	spin_unlock_bh(&br->lock);

	del_timer_sync(&br->hello_timer);
	del_timer_sync(&br->topology_change_timer);
	del_timer_sync(&br->tcn_timer);
	del_timer_sync(&br->gc_timer);
}

/* called under bridge lock */
void br_stp_enable_port(struct net_bridge_port *p)
{
	br_init_port(p);
	br_port_state_selection(p->br);
}

/* called under bridge lock */
void br_stp_disable_port(struct net_bridge_port *p)
{
	struct net_bridge *br;
	int wasroot;
#if defined(CONFIG_RTL_HW_STP)
	int retval, i;
	uint32 vid, portMask;
#endif

	br = p->br;

#if defined(CONFIG_RTL_HW_STP)
	vid=0;
	portMask=0;

	#if define(CONFIG_RTL_NETIF_MAPPING)
	{
		ps_drv_netif_mapping_t *entry;
		entry = rtl_get_ps_drv_netif_mapping_by_psdev(p->dev);

		retval = rtl865x_getNetifVid(entry?entry->drvName:p->dev->name,&vid);
	}
	#else
	if(strcmp(p->dev->name,"eth0")==0)
	{
		retval=rtl865x_getNetifVid("br0", &vid);
	}	
	else
		retval=rtl865x_getNetifVid(p->dev->name, &vid);
	#endif
	
	if(retval==FAILED){
//		printk("%s(%d): rtl865x_getNetifVid failed.\n",__FUNCTION__,__LINE__);
	}
	else{
		portMask=rtl865x_getVlanPortMask(vid);
		for ( i = 0 ; i < MAX_RTL_STP_PORT_WH; i ++ ){
			if((1<<i)&portMask){
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_DISABLED);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_DISABLED);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

	printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
	       br->dev->name, p->port_no, p->dev->name, "disabled");

	wasroot = br_is_root_bridge(br);
	br_become_designated_port(p);
	p->state = BR_STATE_DISABLED;
	p->topology_change_ack = 0;
	p->config_pending = 0;

	del_timer(&p->message_age_timer);
	del_timer(&p->forward_delay_timer);
	del_timer(&p->hold_timer);

	br_fdb_delete_by_port(br, p, 0);

#if 0
#if defined (STP_DISABLE_ETH)
	// Chris: stp+mesh
	if (p->disable_by_mesh == 0)
		del_timer(&p->eth_disable_timer);
	else {
		//printk("set disable timer\n");
		mod_timer(&p->eth_disable_timer, jiffies+ETH_CHK_INTVL);
	}
#endif
#endif

	br_configuration_update(br);

	br_port_state_selection(br);

	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

static void br_stp_start(struct net_bridge *br)
{
	int r;
	char *argv[] = { BR_STP_PROG, br->dev->name, "start", NULL };
	char *envp[] = { NULL };

	r = call_usermodehelper(BR_STP_PROG, argv, envp, UMH_WAIT_PROC);
	if (r == 0) {
		br->stp_enabled = BR_USER_STP;
		printk(KERN_INFO "%s: userspace STP started\n", br->dev->name);
	} else {
		br->stp_enabled = BR_KERNEL_STP;
		printk(KERN_INFO "%s: starting userspace STP failed, "
				"starting kernel STP\n", br->dev->name);

		/* To start timers on any ports left in blocking */
		spin_lock_bh(&br->lock);
		br_port_state_selection(br);
		spin_unlock_bh(&br->lock);
	}
}

static void br_stp_stop(struct net_bridge *br)
{
	int r;
	char *argv[] = { BR_STP_PROG, br->dev->name, "stop", NULL };
	char *envp[] = { NULL };

	if (br->stp_enabled == BR_USER_STP) {
		r = call_usermodehelper(BR_STP_PROG, argv, envp, 1);
		printk(KERN_INFO "%s: userspace STP stopped, return code %d\n",
			br->dev->name, r);


		/* To start timers on any ports left in blocking */
		spin_lock_bh(&br->lock);
		br_port_state_selection(br);
		spin_unlock_bh(&br->lock);
	}

	br->stp_enabled = BR_NO_STP;
}

void br_stp_set_enabled(struct net_bridge *br, unsigned long val)
{
	ASSERT_RTNL();

	if (val) {
		if (br->stp_enabled == BR_NO_STP)
			br_stp_start(br);
	} else {
		if (br->stp_enabled != BR_NO_STP)
			br_stp_stop(br);
	}
}

/* called under bridge lock */
void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *addr)
{
	/* should be aligned on 2 bytes for compare_ether_addr() */
	unsigned short oldaddr_aligned[ETH_ALEN >> 1];
	unsigned char *oldaddr = (unsigned char *)oldaddr_aligned;
	struct net_bridge_port *p;
	int wasroot;

	wasroot = br_is_root_bridge(br);

	memcpy(oldaddr, br->bridge_id.addr, ETH_ALEN);
	memcpy(br->bridge_id.addr, addr, ETH_ALEN);
	memcpy(br->dev->dev_addr, addr, ETH_ALEN);

	list_for_each_entry(p, &br->port_list, list) {
		if (!compare_ether_addr(p->designated_bridge.addr, oldaddr))
			memcpy(p->designated_bridge.addr, addr, ETH_ALEN);

		if (!compare_ether_addr(p->designated_root.addr, oldaddr))
			memcpy(p->designated_root.addr, addr, ETH_ALEN);

	}

	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

/* should be aligned on 2 bytes for compare_ether_addr() */
static const unsigned short br_mac_zero_aligned[ETH_ALEN >> 1];

/* called under bridge lock */
void br_stp_recalculate_bridge_id(struct net_bridge *br)
{
	const unsigned char *br_mac_zero =
			(const unsigned char *)br_mac_zero_aligned;
	const unsigned char *addr = br_mac_zero;
	struct net_bridge_port *p;

	/* user has chosen a value so keep it */
	if (br->flags & BR_SET_MAC_ADDR)
		return;

	list_for_each_entry(p, &br->port_list, list) {
		if (addr == br_mac_zero ||
		    memcmp(p->dev->dev_addr, addr, ETH_ALEN) < 0)
			addr = p->dev->dev_addr;

	}

	if (compare_ether_addr(br->bridge_id.addr, addr))
		br_stp_change_bridge_id(br, addr);
}

/* called under bridge lock */
void br_stp_set_bridge_priority(struct net_bridge *br, u16 newprio)
{
	struct net_bridge_port *p;
	int wasroot;

	wasroot = br_is_root_bridge(br);

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p)) {
			p->designated_bridge.prio[0] = (newprio >> 8) & 0xFF;
			p->designated_bridge.prio[1] = newprio & 0xFF;
		}

	}

	br->bridge_id.prio[0] = (newprio >> 8) & 0xFF;
	br->bridge_id.prio[1] = newprio & 0xFF;
	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

/* called under bridge lock */
void br_stp_set_port_priority(struct net_bridge_port *p, u8 newprio)
{
	port_id new_port_id = br_make_port_id(newprio, p->port_no);

	if (br_is_designated_port(p))
		p->designated_port = new_port_id;

	p->port_id = new_port_id;
	p->priority = newprio;
	if (!memcmp(&p->br->bridge_id, &p->designated_bridge, 8) &&
	    p->port_id < p->designated_port) {
		br_become_designated_port(p);
		br_port_state_selection(p->br);
	}
}

/* called under bridge lock */
void br_stp_set_path_cost(struct net_bridge_port *p, u32 path_cost)
{
	p->path_cost = path_cost;
	br_configuration_update(p->br);
	br_port_state_selection(p->br);
}

ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id)
{
	return sprintf(buf, "%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x\n",
	       id->prio[0], id->prio[1],
	       id->addr[0], id->addr[1], id->addr[2],
	       id->addr[3], id->addr[4], id->addr[5]);
}
