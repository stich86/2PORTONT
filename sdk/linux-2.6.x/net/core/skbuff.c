/*
 *	Routines having to do with the 'struct sk_buff' memory handlers.
 *
 *	Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 *			Florian La Roche <rzsfl@rz.uni-sb.de>
 *
 *	Fixes:
 *		Alan Cox	:	Fixed the worst of the load
 *					balancer bugs.
 *		Dave Platt	:	Interrupt stacking fix.
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	Changed buffer format.
 *		Alan Cox	:	destructor hook for AF_UNIX etc.
 *		Linus Torvalds	:	Better skb_clone.
 *		Alan Cox	:	Added skb_copy.
 *		Alan Cox	:	Added all the changed routines Linus
 *					only put in the headers
 *		Ray VanTassle	:	Fixed --skb->lock in free
 *		Alan Cox	:	skb_copy copy arp field
 *		Andi Kleen	:	slabified it.
 *		Robert Olsson	:	Removed skb_head_pool
 *
 *	NOTE:
 *		The __skb_ routines should be called with interrupts
 *	disabled, or you better be *real* sure that the operation is atomic
 *	with respect to whatever list is being frobbed (e.g. via lock_sock()
 *	or via disabling bottom half handlers, etc).
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

/*
 *	The functions in this file will not compile correctly with gcc 2.4.x
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#ifdef CONFIG_NET_CLS_ACT
#include <net/pkt_sched.h>
#endif
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/splice.h>
#include <linux/cache.h>
#include <linux/rtnetlink.h>
#include <linux/init.h>
#include <linux/scatterlist.h>
#include <linux/errqueue.h>

#include <net/protocol.h>
#include <net/dst.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <net/xfrm.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <trace/skb.h>

#include "kmap_skb.h"
#if defined(CONFIG_RTL_ETH_PRIV_SKB)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_nic.h>
#endif
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
#include "../../drivers/net/re_privskb_adv.h"
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV

#if defined(CONFIG_RTL8190_PRIV_SKB) && !defined(CONFIG_NO_WLAN_DRIVER)
extern int is_rtl8190_priv_buf(unsigned char *head);
extern void free_rtl8190_priv_buf(unsigned char *head);
#endif

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
extern int is_rtl865x_eth_priv_buf(unsigned char *head);
extern void free_rtl865x_eth_priv_buf(unsigned char *head);
#endif

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
#ifdef CONFIG_SKB_POOL_PREALLOC

#define DEBUG_SKB_ALLOC 0

#if 0
#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }
#endif
#define ASSERT(x)

#if 0

unsigned int debug_alloc_counter = 0;
unsigned int debug_alloc_fail_counter1 = 0;
unsigned int debug_alloc_fail_counter2 = 0;
unsigned int debug_free_counter = 0;

#endif


static unsigned int skb_pool_max;
unsigned int skb_pool_free;
static unsigned int skb_pool_priv_max;
unsigned int skb_pool_priv_free;


__IRAM_PP_HIGH void init_skb(struct sk_buff *skb,int allocsize)
{
	struct skb_shared_info *shinfo;
	skb->head=(unsigned char *)((u32)skb->head | 0x20000000);

	/* we should NOT change SKB's head and end; may lead to data corruption */
	skb->end=(unsigned char *)((u32)skb->end | 0x20000000);
	skb->data=skb->tail=skb->head;
	if (unlikely(((u32)skb->end - (u32)skb->head) < (allocsize))) {
		printk("%s(%d): skb too small %d (needed %d)\n", __FUNCTION__,__LINE__,
			(u32)skb->end - (u32)skb->head,allocsize);
		BUG();			
	}
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
	atomic_set(&(skb->users), 1);	
	shinfo=(struct skb_shared_info *)skb->end;
	shinfo->nr_frags=0;
	shinfo->frag_list=NULL;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	/*
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
	skb->data=skb->head;
	skb->tail=skb->head;
#endif
	*/
	skb->cloned = 0;
	skb->len=0;
	skb->data_len=0;
	skb->next=NULL;
	skb->protocol=0;
	skb->fcpu=0;
	skb->pptx=0;
}


#ifdef CONFIG_SKB_POOL_PRIV

#define MAGIC_CODE		0xbeefbabe
#define SKB_BUF_SIZE	(CONFIG_RTL867X_PREALLOCATE_SKB_SIZE)


struct priv_skb_buf {
	// make sure buf is on 32 byte boundry
	unsigned char buf[SKB_BUF_SIZE + NET_SKB_PAD + 
		((sizeof(struct skb_shared_info) + 8 + 31) & 0xFFFFFFE0) - 8];
	unsigned int magic;
	struct priv_skb_buf *next;
};

#define PRIV_SKB_MAX_NUM (CONFIG_SKB_POOL_PREALLOC_NUMBER + 100)
static struct priv_skb_buf skb_buf[PRIV_SKB_MAX_NUM] __attribute__ ((aligned (32)));
static struct priv_skb_buf *skbbuf_list;


static __init void priv_skb_init(void)
{
	int i;
	//unsigned int request;

	//request = size * sizeof(struct priv_skb_buf);
	/*
	if (0 != (skb_buf = kmalloc( request, GFP_KERNEL ))) {
		memset(skb_buf, 0, request);		
	} else {
		printk("%s(%d): can't alloc %d bytes of memory\n", __FUNCTION__,__LINE__,request);
		while (1);
	}*/
	skbbuf_list = skb_buf;
	for (i=0; i<(PRIV_SKB_MAX_NUM - 1); i++)  {		
		skb_buf[i].magic = MAGIC_CODE;
		skb_buf[i].next = &skb_buf[i+1];
	}
	skb_buf[i].magic = MAGIC_CODE;
	skb_buf[i].next = 0;

	skb_pool_priv_max = skb_pool_priv_free = PRIV_SKB_MAX_NUM;
	printk("%s: %d priv buffer inited.\n", __FUNCTION__, i+1);
}

static __inline__ unsigned char *priv_skb_data_get(void)
{	
	struct priv_skb_buf *tmp;
	unsigned long flags;
	unsigned char *data;

	local_irq_save(flags);
	if (likely(skbbuf_list)) {
		tmp = skbbuf_list;
		skbbuf_list = tmp->next;
		data = tmp->buf;
		skb_pool_priv_free--;
		//debug_alloc_counter++;
	} else {
		//debug_alloc_fail_counter1++;
		/*if(printk_ratelimit())
		printk("no more priv_skb , free = %d\n", skb_pool_priv_free);*/
		data = 0;
		
	}	
	local_irq_restore(flags);
	
	return data;
}

__IRAM_PP_HIGH static void priv_skb_data_free(unsigned char *head)
{
	
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf = (struct priv_skb_buf *)(((unsigned long)head) - offset);
	unsigned long flags;
	
	local_irq_save(flags);
	priv_buf->next = skbbuf_list;
	skbbuf_list = priv_buf;
	skb_pool_priv_free++;
	local_irq_restore(flags);	
	if (unlikely(skb_pool_priv_free > skb_pool_priv_max)) {
		printk("%s(%d): %d > %d\n", __FUNCTION__,__LINE__, skb_pool_priv_free, skb_pool_priv_max);
		dump_stack();		
		while (1);
	}
	//debug_free_counter++;
}


__IRAM_PP_HIGH static int is_priv_skb_data(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf = (struct priv_skb_buf *)(((unsigned long)head) - offset);

	if ((priv_buf->magic == MAGIC_CODE) &&
			((unsigned long)priv_buf->buf == (unsigned long)head))
		return 1;
	else
		return 0;
}

static inline struct sk_buff *dev_alloc_priv_skb(unsigned char *data, int size)
{
	kmem_cache_t *cache;
        struct sk_buff *skb;
	struct skb_shared_info *shinfo;
 
	cache = skbuff_head_cache;
 
    /* Get the HEAD */
	skb = kmem_cache_alloc(cache, GFP_ATOMIC & ~__GFP_DMA);
    if (!skb)
		goto out;
 
	memset(skb, 0, offsetof(struct sk_buff, truesize));
	skb->truesize = size + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb->tail = data;
	skb->end  = data + size;
	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	atomic_set(&shinfo->dataref, 1);
	shinfo->nr_frags  = 0;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	shinfo->frag_list = NULL;
 
#ifdef CONFIG_RTK_VOIP_VLAN_ID
	skb->rx_vlan = 0;
	skb->rx_wlan = 0;
	skb->priority = 0;
	skb_reserve(skb, 4); // for VLAN TAG insertion
#endif
 
	skb_reserve(skb, 80);
	return skb;
 
out:
	return NULL;
}

__IRAM_PP_HIGH struct sk_buff *priv_skb_alloc(unsigned int size)
{
	struct sk_buff *skb;
		
	unsigned char *data = priv_skb_data_get();
	ASSERT(size <= SKB_BUF_SIZE);
	if (unlikely(data == NULL)) {	
		return dev_alloc_skb(size);
	}

	skb = dev_alloc_priv_skb(data, size+NET_SKB_PAD);
	if (skb == NULL) {
		//debug_alloc_fail_counter2++;
		priv_skb_data_free(data);
		return NULL;
	}
	return skb;
}


#endif // CONFIG_SKB_POOL_PRIV

static struct sk_buff *skb_pool_ring[CONFIG_SKB_POOL_PREALLOC_NUMBER];

__init void prealloc_skb_init(unsigned int num)
{
	int i;	
	//unsigned int request;

	#ifdef CONFIG_SKB_POOL_PRIV 
	priv_skb_init();
	#endif 

	/*
	request = sizeof(struct sk_buff *) * num;

	if (0 != (skb_pool_ring = (struct sk_buff **)kmalloc(request, GFP_KERNEL))) {
		memset( skb_pool_ring, 0, request);
	} else {
		printk("%s(%d): can't alloc %d bytes of memory\n", __FUNCTION__,__LINE__,request);
		while (1);
	}
	*/
	
	memset(skb_pool_ring, 0, sizeof(skb_pool_ring));
	
	for(i=0;i<CONFIG_SKB_POOL_PREALLOC_NUMBER;i++)
	{
		skb_pool_ring[i]=
			#ifdef CONFIG_SKB_POOL_PRIV		
			priv_skb_alloc(CONFIG_RTL867X_PREALLOCATE_SKB_SIZE);
			#else
			dev_alloc_skb(CONFIG_RTL867X_PREALLOCATE_SKB_SIZE);
			#endif
		ASSERT((u32 *)skb_pool_ring[i]!=NULL);
		init_skb(skb_pool_ring[i],CONFIG_RTL867X_PREALLOCATE_SKB_SIZE);
		#if DEBUG_SKB_ALLOC
		skb_pool_ring[i]->use_cnt=0;
		#endif
	}	
	skb_pool_max = skb_pool_free = CONFIG_SKB_POOL_PREALLOC_NUMBER;	
	printk("%s: %d initialized, %d requested\n", __FUNCTION__, skb_pool_free, num);
}

__IRAM_PP_HIGH struct sk_buff *prealloc_skb_get(void)
{
	struct sk_buff *ret;
	unsigned long	flags;
	if (unlikely(skb_pool_free<=0)) {
		printk("free_cnt=%d\n",skb_pool_free); 
		BUG(); 
	}
	
	local_irq_save(flags);
	skb_pool_free--;
	ret=skb_pool_ring[skb_pool_free];
	#if DEBUG_SKB_ALLOC
	if (unlikely(ret->use_cnt)) {
		printk("%s(%d): cnt=%d skb %p(%p,%p)\n",__FUNCTION__,__LINE__,ret->use_cnt, ret, ret->head, ret->data);
		BUG();		
	} else {
		ret->use_cnt++;
	}
	#endif

	local_irq_restore(flags);

	return ret;
}

/*  return a pre-allocated skb */
__IRAM_PP_HIGH void prealloc_skb_free(struct sk_buff *skb) 
{
	unsigned long flags;
	local_irq_save(flags);

	ASSERT(!skb->fcpu);
	ASSERT(!skb_cloned(skb));
	#if DEBUG_SKB_ALLOC

	skb->use_cnt--;
	if (unlikely(skb->use_cnt!=0)) {	
		printk("%s(%d): cnt=%d skb %p(%p,%p)\n",__FUNCTION__,__LINE__,skb->use_cnt, skb, skb->head, skb->data);
		BUG();
}
	#endif
		
	init_skb(skb,CONFIG_RTL867X_PREALLOCATE_SKB_SIZE);
	skb_pool_ring[skb_pool_free]=skb;
	skb_pool_free++;

	if (unlikely(skb_pool_free > skb_pool_max))
		goto error;
	
	local_irq_restore(flags);
	return;

error:
	local_irq_restore(flags);	
	printk("%s(%d): error cnt=%d max=%d\n",__FUNCTION__, __LINE__, skb_pool_free, skb_pool_max);	
	dump_stack();
	while(1);
	
}

#endif //CONFIG_SKB_POOL_PREALLOC
#endif
#ifdef CONFIG_ETHWAN
__DRAM
#endif //CONFIG_ETHWAN
static struct kmem_cache *skbuff_head_cache __read_mostly;
#ifdef CONFIG_ETHWAN
__DRAM
#endif //CONFIG_ETHWAN
static struct kmem_cache *skbuff_fclone_cache __read_mostly;
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
static struct kmem_cache *skbuff_cb_store_cache __read_mostly;
#endif

static void sock_pipe_buf_release(struct pipe_inode_info *pipe,
				  struct pipe_buffer *buf)
{
	put_page(buf->page);
}

static void sock_pipe_buf_get(struct pipe_inode_info *pipe,
				struct pipe_buffer *buf)
{
	get_page(buf->page);
}

static int sock_pipe_buf_steal(struct pipe_inode_info *pipe,
			       struct pipe_buffer *buf)
{
	return 1;
}

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
/* Control buffer save/restore for IMQ devices */
struct skb_cb_table {
	void			*cb_next;
	atomic_t		refcnt;
	char      		cb[48];
};

static DEFINE_SPINLOCK(skb_cb_store_lock);

int skb_save_cb(struct sk_buff *skb)
{
	struct skb_cb_table *next;

	next = kmem_cache_alloc(skbuff_cb_store_cache, GFP_ATOMIC);
	if (!next)
		return -ENOMEM;

	BUILD_BUG_ON(sizeof(skb->cb) != sizeof(next->cb));

	memcpy(next->cb, skb->cb, sizeof(skb->cb));
	next->cb_next = skb->cb_next;

	atomic_set(&next->refcnt, 1);

	skb->cb_next = next;
	return 0;
}
EXPORT_SYMBOL(skb_save_cb);

int skb_restore_cb(struct sk_buff *skb)
{
	struct skb_cb_table *next;

	if (!skb->cb_next)
		return 0;

	next = skb->cb_next;

	BUILD_BUG_ON(sizeof(skb->cb) != sizeof(next->cb));

	memcpy(skb->cb, next->cb, sizeof(skb->cb));
	skb->cb_next = next->cb_next;

	spin_lock(&skb_cb_store_lock);

	if (atomic_dec_and_test(&next->refcnt)) {
		kmem_cache_free(skbuff_cb_store_cache, next);
	}

	spin_unlock(&skb_cb_store_lock);

	return 0;
}
EXPORT_SYMBOL(skb_restore_cb);

static void skb_copy_stored_cb(struct sk_buff *new, struct sk_buff *old)
{
	struct skb_cb_table *next;

	if (!old->cb_next) {
		new->cb_next = 0;
		return;
	}

	spin_lock(&skb_cb_store_lock);

	next = old->cb_next;
	atomic_inc(&next->refcnt);
	new->cb_next = next;

	spin_unlock(&skb_cb_store_lock);
}
#endif

/* Pipe buffer operations for a socket. */
static struct pipe_buf_operations sock_pipe_buf_ops = {
	.can_merge = 0,
	.map = generic_pipe_buf_map,
	.unmap = generic_pipe_buf_unmap,
	.confirm = generic_pipe_buf_confirm,
	.release = sock_pipe_buf_release,
	.steal = sock_pipe_buf_steal,
	.get = sock_pipe_buf_get,
};

/*
 *	Keep out-of-line to prevent kernel bloat.
 *	__builtin_return_address is not used because it is not always
 *	reliable.
 */

/**
 *	skb_over_panic	- 	private function
 *	@skb: buffer
 *	@sz: size
 *	@here: address
 *
 *	Out of line support code for skb_put(). Not user callable.
 */
void skb_over_panic(struct sk_buff *skb, int sz, void *here)
{
	printk(KERN_EMERG "skb_over_panic: text:%p len:%d put:%d head:%p "
			  "data:%p tail:%#lx end:%#lx dev:%s\n",
	       here, skb->len, sz, skb->head, skb->data,
	       (unsigned long)skb->tail, (unsigned long)skb->end,
	       skb->dev ? skb->dev->name : "<NULL>");
	BUG();
}
EXPORT_SYMBOL(skb_over_panic);

/**
 *	skb_under_panic	- 	private function
 *	@skb: buffer
 *	@sz: size
 *	@here: address
 *
 *	Out of line support code for skb_push(). Not user callable.
 */

void skb_under_panic(struct sk_buff *skb, int sz, void *here)
{
	printk(KERN_EMERG "skb_under_panic: text:%p len:%d put:%d head:%p "
			  "data:%p tail:%#lx end:%#lx dev:%s\n",
	       here, skb->len, sz, skb->head, skb->data,
	       (unsigned long)skb->tail, (unsigned long)skb->end,
	       skb->dev ? skb->dev->name : "<NULL>");
	BUG();
}
EXPORT_SYMBOL(skb_under_panic);


// Kaohj -- sar rx pre-allocation
#ifdef CONFIG_SAR_FASTSKB
#include "../../drivers/867x_sar/fastskb.h"
#define DATA_BUFFER_SIZE (1600+NET_SKB_PAD)//modified for tkip issue
#endif

/* 	Allocate a new skbuff. We do this ourselves so we can fill in a few
 *	'private' fields and also do memory statistics to find all the
 *	[BEEP] leaks.
 *
 */

/**
 *	__alloc_skb	-	allocate a network buffer
 *	@size: size to allocate
 *	@gfp_mask: allocation mask
 *	@fclone: allocate from fclone cache instead of head cache
 *		and allocate a cloned (child) skb
 *	@node: numa node to allocate memory on
 *
 *	Allocate a new &sk_buff. The returned buffer has no headroom and a
 *	tail room of size bytes. The object has a reference count of one.
 *	The return is the buffer. On a failure the return is %NULL.
 *
 *	Buffers may only be allocated from interrupts using a @gfp_mask of
 *	%GFP_ATOMIC.
 */
#ifdef CONFIG_ETHWAN
__IRAM
#endif //CONFIG_ETHWAN
struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
			    int fclone, int node)
{
	struct kmem_cache *cache;
	struct skb_shared_info *shinfo;
	struct sk_buff *skb;
	u8 *data;

	cache = fclone ? skbuff_fclone_cache : skbuff_head_cache;
	if(size!=(CONFIG_RTL867X_PREALLOCATE_SKB_SIZE+NET_SKB_PAD)) size+=8;	//cathy, for wlan tkip
	/* Get the HEAD */
	skb = kmem_cache_alloc_node(cache, gfp_mask & ~__GFP_DMA, node);
	if (!skb)
		goto out;

	// Kaohj -- sar rx pre-allocation
	#ifdef CONFIG_SAR_FASTSKB
	if (size==(DATA_BUFFER_SIZE+8)) {  //try to get from private data pool
		if (NULL!=(data = get_skb_data_bfr())) {
			//printk("get skb_data to pool(%d)\n", size);
    			size = SKB_DATA_ALIGN(size);
			goto data_ready;
		}
	} 
	#endif

//#if defined(CONFIG_RTL8190) || defined(CONFIG_RTL8192SE) || defined(CONFIG_RTL8192CD)
//	size = SKB_DATA_ALIGN(size+128);
//#else
	size = SKB_DATA_ALIGN(size);
//#endif

	data = kmalloc_node_track_caller(size + sizeof(struct skb_shared_info),
			gfp_mask, node);
	if (!data)
		goto nodata;

	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
#ifdef CONFIG_SAR_FASTSKB
data_ready:
#endif
	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->truesize = size + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
#ifdef CONFIG_RTL_HARDWARE_MULTICAST
	skb->srcPort=0xFFFF;
	skb->srcVlanId=0;
#endif
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	skb->srcVlanPriority=0;
#endif

#if defined(CONFIG_RTL_8676HWNAT) && defined(CONFIG_RTL8676_Static_ACL)
	skb->acl_forward_to_extPort=0;
#endif

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)
	skb->srcPhyPort=0xFF;
	skb->dstPhyPort=0xFF;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	skb->tag.v = 0;
#endif	

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	skb->srcLocalPublicIp=0;
	skb->fromLocalPublic=0;
	skb->toLocalPublic=0;
#endif


	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	atomic_set(&shinfo->dataref, 1);
	shinfo->nr_frags  = 0;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	shinfo->tx_flags.flags = 0;
	shinfo->frag_list = NULL;
	memset(&shinfo->hwtstamps, 0, sizeof(shinfo->hwtstamps));

	if (fclone) {
		struct sk_buff *child = skb + 1;
		atomic_t *fclone_ref = (atomic_t *) (child + 1);

		skb->fclone = SKB_FCLONE_ORIG;
		atomic_set(fclone_ref, 1);

		child->fclone = SKB_FCLONE_UNAVAILABLE;
	}
out:
	return skb;
nodata:
	kmem_cache_free(cache, skb);
	skb = NULL;
	goto out;
}
EXPORT_SYMBOL(__alloc_skb);

#if defined(CONFIG_RTL8190_PRIV_SKB) && !defined(CONFIG_NO_WLAN_DRIVER) || defined(CONFIG_RTL_ETH_PRIV_SKB) || defined(CONFIG_RTL865X_ETH_PRIV_SKB)
#define RTL_PRIV_DATA_SIZE	128
struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size)
{
        struct sk_buff *skb;
	struct skb_shared_info *shinfo;

 	struct kmem_cache *cache;
 	cache = skbuff_head_cache;
	/* Get the HEAD */
        skb = kmem_cache_alloc(cache, GFP_ATOMIC & ~__GFP_DMA);
        if (!skb)
                goto out;

        memset(skb, 0, offsetof(struct sk_buff, truesize));
        atomic_set(&skb->users, 1);
        skb->head = data;
        skb->data = data;
        skb->tail = data;

		size = (size+128+NET_SKB_PAD);

        skb->end  = data + size;
        skb->truesize = size + sizeof(struct sk_buff);

        /* make sure we initialize shinfo sequentially */
        shinfo = skb_shinfo(skb);
        atomic_set(&shinfo->dataref, 1);
        shinfo->nr_frags  = 0;
        shinfo->gso_size = 0;
        shinfo->gso_segs = 0;
        shinfo->gso_type = 0;
        shinfo->ip6_frag_id = 0;
        shinfo->frag_list = NULL;
 
#ifdef CONFIG_RTK_VOIP_VLAN_ID
        skb->rx_vlan = 0;
        skb->rx_wlan = 0;
        skb->priority = 0;
#endif

#ifdef CONFIG_RTL_HARDWARE_MULTICAST
	skb->srcPort=0xFFFF;
	skb->srcVlanId=0;
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	skb->srcVlanPriority=0;
#endif

#if defined(CONFIG_RTL_8676HWNAT) && defined(CONFIG_RTL8676_Static_ACL)
	skb->acl_forward_to_extPort=0;
#endif

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)
	skb->srcPhyPort=0xFF;
	skb->dstPhyPort=0xFF;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	skb->tag.v = 0;
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	skb->srcLocalPublicIp=0;
	skb->fromLocalPublic=0;
	skb->toLocalPublic=0;
#endif



#ifdef CONFIG_RTK_VOIP_VLAN_ID
	skb_reserve(skb, RTL_PRIV_DATA_SIZE+4); // for VLAN TAG insertion
#else
	skb_reserve(skb, RTL_PRIV_DATA_SIZE);
#endif

        return skb;
 
out:
        return NULL;
}
#endif // CONFIG_RTL8190_PRIV_SKB

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
void reinit_skbhdr(struct sk_buff *skb, 
					void (*prealloc_cb)(struct sk_buff *, unsigned))
{
	unsigned char *data, *head;
	unsigned int size;
	struct skb_shared_info *shinfo;
	volatile unsigned int skb_addr = (unsigned int)skb;

	data = skb->data;
	head = skb->head;
	size = skb->len;
#if 0
	memset(skb, 0, offsetof(struct sk_buff, truesize));
#else
	#ifdef CONFIG_CPU_RLX5281
	__asm__ __volatile__ (
		"	.set	noreorder				\n"
		"	.set	noat					\n"
		"	li $1, 0						\n"
		"1:	st $0, 0(%1)					\n"
		"	bne	%0, %1, 1b					\n"
		"	addiu %1, %1, 8					\n"
		"	.set	at						\n"
		"	.set	reorder					"
		:
		: "r" (skb_addr + SKB_ALIGNED_SIZE - 8), "r" (skb_addr));
	#else
	__asm__ __volatile__ (
		"	.set	noreorder				\n"
		"1:	sw $0, 0(%1)					\n"
		"	bne	%0, %1, 1b					\n"
		"	addiu %1, %1, 4					\n"
		"	.set	reorder					"
		:
		: "r" (skb_addr + SKB_ALIGNED_SIZE - 4), "r" (skb_addr));
	#endif
#endif

	atomic_set(&skb->users, 1);
	skb->head = head;
	skb->data = data;
	skb->tail = skb->data + size;
	skb->len  = size;

	skb->end  = skb->head + ETH_SKB_DATA_LEN;
	skb->truesize = ETH_SKB_DATA_LEN + sizeof(struct sk_buff);

	skb->prealloc_cb = prealloc_cb;
	skb->prealloc_flags = (SKB_PREALLOC | SKB_DATA_PREALLOC);

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	atomic_set(&shinfo->dataref, 1);
	shinfo->nr_frags  = 0;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	shinfo->frag_list = NULL;
}
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV

/**
 *	__netdev_alloc_skb - allocate an skbuff for rx on a specific device
 *	@dev: network device to receive on
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory.
 */
struct sk_buff *__netdev_alloc_skb(struct net_device *dev,
		unsigned int length, gfp_t gfp_mask)
{
	int node = dev->dev.parent ? dev_to_node(dev->dev.parent) : -1;
	struct sk_buff *skb;

	skb = __alloc_skb(length + NET_SKB_PAD, gfp_mask, 0, node);
	if (likely(skb)) {
		skb_reserve(skb, NET_SKB_PAD);
		skb->dev = dev;
	}
	return skb;
}
EXPORT_SYMBOL(__netdev_alloc_skb);

struct page *__netdev_alloc_page(struct net_device *dev, gfp_t gfp_mask)
{
	int node = dev->dev.parent ? dev_to_node(dev->dev.parent) : -1;
	struct page *page;

	page = alloc_pages_node(node, gfp_mask, 0);
	return page;
}
EXPORT_SYMBOL(__netdev_alloc_page);

void skb_add_rx_frag(struct sk_buff *skb, int i, struct page *page, int off,
		int size)
{
	skb_fill_page_desc(skb, i, page, off, size);
	skb->len += size;
	skb->data_len += size;
	skb->truesize += size;
}
EXPORT_SYMBOL(skb_add_rx_frag);

/**
 *	dev_alloc_skb - allocate an skbuff for receiving
 *	@length: length to allocate
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory. Although this function
 *	allocates memory it can be called from an interrupt.
 */
struct sk_buff *dev_alloc_skb(unsigned int length)
{
	/*
	 * There is more code here than it seems:
	 * __dev_alloc_skb is an inline
	 */
	return __dev_alloc_skb(length, GFP_ATOMIC);
}
EXPORT_SYMBOL(dev_alloc_skb);

static void skb_drop_list(struct sk_buff **listp)
{
	struct sk_buff *list = *listp;

	*listp = NULL;

	do {
		struct sk_buff *this = list;
		list = list->next;
		kfree_skb(this);
	} while (list);
}

static inline void skb_drop_fraglist(struct sk_buff *skb)
{
	skb_drop_list(&skb_shinfo(skb)->frag_list);
}

static void skb_clone_fraglist(struct sk_buff *skb)
{
	struct sk_buff *list;

	for (list = skb_shinfo(skb)->frag_list; list; list = list->next)
		skb_get(list);
}

static void skb_release_data(struct sk_buff *skb)
{
	if (!skb->cloned ||
	    !atomic_sub_return(skb->nohdr ? (1 << SKB_DATAREF_SHIFT) + 1 : 1,
			       &skb_shinfo(skb)->dataref)) {
		if (skb_shinfo(skb)->nr_frags) {
			int i;
			for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
				put_page(skb_shinfo(skb)->frags[i].page);
		}

		if (skb_shinfo(skb)->frag_list)
			skb_drop_fraglist(skb);

#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
		/*
		 * If skb->prealloc_flags is 1, that means the data buffer was pre-allocated
		 * by our network driver.
		 */
		if (skb->prealloc_cb && (skb->prealloc_flags & SKB_DATA_PREALLOC)) {
			(*skb->prealloc_cb)(skb, RETFREEQ_DATA);
			skb->prealloc_flags &= ~SKB_DATA_PREALLOC;
			return;
		}
		else
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV
#if defined(CONFIG_RTL_ETH_PRIV_SKB) || defined(CONFIG_RTL865X_ETH_PRIV_SKB)
		if (is_rtl865x_eth_priv_buf(skb->head)) {
			free_rtl865x_eth_priv_buf(skb->head);
		}            
		else
#endif
#if defined(CONFIG_RTL8190_PRIV_SKB) && !defined(CONFIG_NO_WLAN_DRIVER)
		if (is_rtl8190_priv_buf(skb->head))
		{
			free_rtl8190_priv_buf(skb->head);
		}
		else
#endif
		{

			// Kaohj -- sar rx pre-allocation
			#ifdef CONFIG_SAR_FASTSKB
			if (skb->truesize==(SKB_DATA_ALIGN(DATA_BUFFER_SIZE+8)+sizeof(struct sk_buff))) {
				if (1==put_skb_data_bfr(skb->head))  //return success
				{
					//printk("put skb_data to pool(%d)\n", skb->truesize);
					return;
				}
			};
			#endif

			kfree(skb->head);
		}
	}
}

/*
 *	Free an skbuff by memory without cleaning the state.
 */
static void kfree_skbmem(struct sk_buff *skb)
{
	struct sk_buff *other;
	atomic_t *fclone_ref;

	/*linux-2.6.19*/ 
	// Kaohj
	skb->from_dev = NULL;
	
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
	/*
	 * If skb->prealloc_flags is 1, that means the data buffer was pre-allocated
	 * by our network driver.
	 */
	if (skb->prealloc_cb && (skb->prealloc_flags & SKB_PREALLOC))
	{
		(*skb->prealloc_cb)(skb, RETFREEQ_SKB);
		return;
	}
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV
	switch (skb->fclone) {
	case SKB_FCLONE_UNAVAILABLE:
		kmem_cache_free(skbuff_head_cache, skb);
		break;

	case SKB_FCLONE_ORIG:
		fclone_ref = (atomic_t *) (skb + 2);
		if (atomic_dec_and_test(fclone_ref))
			kmem_cache_free(skbuff_fclone_cache, skb);
		break;

	case SKB_FCLONE_CLONE:
		fclone_ref = (atomic_t *) (skb + 1);
		other = skb - 1;

		/* The clone portion is available for
		 * fast-cloning again.
		 */
		skb->fclone = SKB_FCLONE_UNAVAILABLE;

		if (atomic_dec_and_test(fclone_ref))
			kmem_cache_free(skbuff_fclone_cache, other);
		break;
	}
}

static void skb_release_head_state(struct sk_buff *skb)
{
	dst_release(skb->dst);
#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
#endif
	if (skb->destructor) {
		WARN_ON(in_irq());
		skb->destructor(skb);
	}
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	/* This should not happen. When it does, avoid memleak by restoring
	the chain of cb-backups. */
	while(skb->cb_next != NULL) {
		printk(KERN_WARNING "kfree_skb: skb->cb_next: %08x\n",
			skb->cb_next);
		skb_restore_cb(skb);
	}
#endif
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put(skb->nfct);
	nf_conntrack_put_reasm(skb->nfct_reasm);
#endif
#ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(skb->nf_bridge);
#endif
/* XXX: IS this still necessary? - JHS */
#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
#endif
#endif
}

/* Free everything but the sk_buff shell. */
static void skb_release_all(struct sk_buff *skb)
{
	skb_release_head_state(skb);
	skb_release_data(skb);
}

/**
 *	__kfree_skb - private function
 *	@skb: buffer
 *
 *	Free an sk_buff. Release anything attached to the buffer.
 *	Clean the state. This is an internal helper function. Users should
 *	always call kfree_skb
 */

#if defined( CONFIG_RTL8672NIC) || defined( CONFIG_RTL8686NIC) || defined( CONFIG_RTL8686NIC_MODULE)
/*linux-2.6.19*/
atomic_t re8670_rxskb_num = ATOMIC_INIT(0);
EXPORT_SYMBOL(re8670_rxskb_num);
#endif

#ifdef CONFIG_ETHWAN
__IRAM
#endif //CONFIG_ETHWAN
void __kfree_skb(struct sk_buff *skb)
{

#if defined( CONFIG_RTL8672NIC) || defined( CONFIG_RTL8686NIC) || defined( CONFIG_RTL8686NIC_MODULE)
    /*linux-2.6.19*/
    if (skb->src_port==IF_ETH && !skb_cloned(skb)) {
    	atomic_dec(&re8670_rxskb_num);
	skb->src_port=0;
    }
#endif
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
	if (skb->prealloc_cb && (skb->prealloc_flags & RETFREEQ_FF))
	{
		dst_release(skb->dst);

		(*skb->prealloc_cb)(skb, RETFREEQ_SKB|RETFREEQ_DATA);
		return;
	}
#endif
	skb_release_all(skb);
	kfree_skbmem(skb);
}
EXPORT_SYMBOL(__kfree_skb);

/**
 *	kfree_skb - free an sk_buff
 *	@skb: buffer to free
 *
 *	Drop a reference to the buffer and free it if the usage count has
 *	hit zero.
 */
#ifdef CONFIG_ETHWAN
__IRAM
#endif //CONFIG_ETHWAN
void kfree_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return;
	if (likely(atomic_read(&skb->users) == 1))
		smp_rmb();
	else if (likely(!atomic_dec_and_test(&skb->users)))
		return;
	trace_kfree_skb(skb, __builtin_return_address(0));
	__kfree_skb(skb);
}
EXPORT_SYMBOL(kfree_skb);

/**
 *	consume_skb - free an skbuff
 *	@skb: buffer to free
 *
 *	Drop a ref to the buffer and free it if the usage count has hit zero
 *	Functions identically to kfree_skb, but kfree_skb assumes that the frame
 *	is being dropped after a failure and notes that
 */
void consume_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return;
	if (likely(atomic_read(&skb->users) == 1))
		smp_rmb();
	else if (likely(!atomic_dec_and_test(&skb->users)))
		return;
	__kfree_skb(skb);
}
EXPORT_SYMBOL(consume_skb);

/**
 *	skb_recycle_check - check if skb can be reused for receive
 *	@skb: buffer
 *	@skb_size: minimum receive buffer size
 *
 *	Checks that the skb passed in is not shared or cloned, and
 *	that it is linear and its head portion at least as large as
 *	skb_size so that it can be recycled as a receive buffer.
 *	If these conditions are met, this function does any necessary
 *	reference count dropping and cleans up the skbuff as if it
 *	just came from __alloc_skb().
 */
int skb_recycle_check(struct sk_buff *skb, int skb_size)
{
	struct skb_shared_info *shinfo;

	if (skb_is_nonlinear(skb) || skb->fclone != SKB_FCLONE_UNAVAILABLE)
		return 0;

	skb_size = SKB_DATA_ALIGN(skb_size + NET_SKB_PAD);
	if (skb_end_pointer(skb) - skb->head < skb_size)
		return 0;

	if (skb_shared(skb) || skb_cloned(skb))
		return 0;

	skb_release_head_state(skb);
	shinfo = skb_shinfo(skb);
	atomic_set(&shinfo->dataref, 1);
	shinfo->nr_frags = 0;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	shinfo->tx_flags.flags = 0;
	shinfo->frag_list = NULL;
	memset(&shinfo->hwtstamps, 0, sizeof(shinfo->hwtstamps));

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->data = skb->head + NET_SKB_PAD;
	skb_reset_tail_pointer(skb);

	return 1;
}
EXPORT_SYMBOL(skb_recycle_check);

static void __copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	new->tstamp		= old->tstamp;
	new->dev		= old->dev;
	new->transport_header	= old->transport_header;
	new->network_header	= old->network_header;
	new->mac_header		= old->mac_header;
	new->dst		= dst_clone(old->dst);
#ifdef CONFIG_XFRM
	new->sp			= secpath_get(old->sp);
#endif
	memcpy(new->cb, old->cb, sizeof(old->cb));
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	skb_copy_stored_cb(new, old);
#endif
	new->csum_start		= old->csum_start;
	new->csum_offset	= old->csum_offset;
	new->local_df		= old->local_df;
	new->pkt_type		= old->pkt_type;
	new->ip_summed		= old->ip_summed;
	skb_copy_queue_mapping(new, old);
	new->priority		= old->priority;
#if defined(CONFIG_IP_VS) || defined(CONFIG_IP_VS_MODULE)
	new->ipvs_property	= old->ipvs_property;
#endif
	new->protocol		= old->protocol;
	new->mark		= old->mark;
	__nf_copy(new, old);
#if defined(CONFIG_NETFILTER_XT_TARGET_TRACE) || \
    defined(CONFIG_NETFILTER_XT_TARGET_TRACE_MODULE)
	new->nf_trace		= old->nf_trace;
#endif

#ifdef CONFIG_RTL_HARDWARE_MULTICAST
	new->srcPort=old->srcPort;
	new->srcVlanId=old->srcVlanId;
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	new->srcVlanPriority=old->srcVlanPriority;
#endif

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)
	new->srcPhyPort=old->srcPhyPort;
	new->dstPhyPort=old->dstPhyPort;
#endif


#ifdef CONFIG_NET_SCHED
	new->tc_index		= old->tc_index;
#ifdef CONFIG_NET_CLS_ACT
	new->tc_verd		= old->tc_verd;
#endif
#endif
	new->vlan_tci		= old->vlan_tci;

	//linux-2.6.19
	//august
        new->isVlanPack=old->isVlanPack;


#ifdef CONFIG_NEW_PORTMAPPING
	new->fgroup = old->fgroup;
#endif

	/*linux-2.6.19*/ 
	// Kaohj
	new->vlan_member = old->vlan_member;
	new->pvid = old->pvid;
	new->from_dev = old->from_dev;
	new->switch_port = old->switch_port;
	//jim 20080609 port from skb_clone...
#ifdef CONFIG_PPPOE_PROXY_FASTPATH
	new->islanPPP = old->islanPPP;
#endif
#ifdef CONFIG_PPPOE_PROXY
	new->source_port = old->source_port;
#endif

	skb_copy_secmark(new, old);
}

static struct sk_buff *__skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
#define C(x) n->x = skb->x

	n->next = n->prev = NULL;
	n->sk = NULL;
	__copy_skb_header(n, skb);

	C(len);
	C(data_len);
	C(mac_len);
	n->hdr_len = skb->nohdr ? skb_headroom(skb) : skb->hdr_len;
	n->cloned = 1;
	n->nohdr = 0;
	n->destructor = NULL;
	C(iif);
	C(tail);
	C(end);
	C(head);
	C(data);
	C(truesize);

#ifdef CONFIG_PPPOE_PROXY_FASTPATH
       C(islanPPP);
#endif
#ifdef CONFIG_PPPOE_PROXY
	C(source_port);
#endif
	/*linux-2.6.19*/ 
	/*patch from linux 2.4*/
	// Kaohj
	n->switch_port = skb->switch_port;

#ifdef CONFIG_NEW_PORTMAPPING
	n->fgroup = skb->fgroup;
#endif


	C(vlan_member);
	C(vlan_passthrough);
	C(from_dev);
	C(pvid);
	C(src_port);


#ifdef CONFIG_RTL_HARDWARE_MULTICAST
	C(srcPort);
	C(srcVlanId);
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	C(srcVlanPriority);
#endif

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)
	C(srcPhyPort);
	C(dstPhyPort);
#endif

#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
	if (n->prealloc_flags & SKB_PREALLOC) {
		C(prealloc_flags);
		n->prealloc_flags |= SKB_PREALLOC;
	}
	else {
		C(prealloc_flags);
		n->prealloc_flags &= ~SKB_PREALLOC;
	}
	C(prealloc_cb);
	C(prealloc_next);
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV

    C(isVlanPack);


#if defined(CONFIG_MAC80211) || defined(CONFIG_MAC80211_MODULE)
	C(do_not_encrypt);
	C(requeue);
#endif
	atomic_set(&n->users, 1);

	atomic_inc(&(skb_shinfo(skb)->dataref));
	skb->cloned = 1;

	return n;
#undef C
}

/**
 *	skb_morph	-	morph one skb into another
 *	@dst: the skb to receive the contents
 *	@src: the skb to supply the contents
 *
 *	This is identical to skb_clone except that the target skb is
 *	supplied by the user.
 *
 *	The target skb is returned upon exit.
 */
struct sk_buff *skb_morph(struct sk_buff *dst, struct sk_buff *src)
{
	skb_release_all(dst);
	return __skb_clone(dst, src);
}
EXPORT_SYMBOL_GPL(skb_morph);

/**
 *	skb_clone	-	duplicate an sk_buff
 *	@skb: buffer to clone
 *	@gfp_mask: allocation priority
 *
 *	Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *	copies share the same packet data but not structure. The new
 *	buffer has a reference count of 1. If the allocation fails the
 *	function returns %NULL otherwise the new buffer is returned.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 */

struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff *n;

	n = skb + 1;
	if (skb->fclone == SKB_FCLONE_ORIG &&
	    n->fclone == SKB_FCLONE_UNAVAILABLE) {
		atomic_t *fclone_ref = (atomic_t *) (n + 1);
		n->fclone = SKB_FCLONE_CLONE;
		atomic_inc(fclone_ref);
	} else {
		n = kmem_cache_alloc(skbuff_head_cache, gfp_mask);
		if (!n)
			return NULL;
		n->fclone = SKB_FCLONE_UNAVAILABLE;
	}

//#define C(x) n->x = skb->x
#ifdef CONFIG_RTK_VLAN_SUPPORT
	//C(tag.v);
	n->tag.v = skb->tag.v;
#endif
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	n->srcLocalPublicIp = skb->srcLocalPublicIp;
	n->fromLocalPublic = skb->fromLocalPublic;
	n->toLocalPublic = skb->toLocalPublic;
#endif
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
	n->prealloc_cb = NULL;
	n->prealloc_next = NULL;
	n->prealloc_flags = 0;
#endif //end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV
	return __skb_clone(n, skb);
}
EXPORT_SYMBOL(skb_clone);

#if (defined(CONFIG_RTL_ETH_PRIV_SKB) || defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)) && (defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG) || defined(CONFIG_WIRELESS))
void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
#else
static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
#endif
{
#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/*
	 *	Shift between the two data areas in bytes
	 */
	unsigned long offset = new->data - old->data;
#endif

	__copy_skb_header(new, old);

#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/* {transport,network,mac}_header are relative to skb->head */
	new->transport_header += offset;
	new->network_header   += offset;
	new->mac_header	      += offset;
#endif
#ifdef CONFIG_RTK_VLAN_SUPPORT	
	new->tag.v = old->tag.v;
#endif
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	new->srcLocalPublicIp=old->srcLocalPublicIp;
	new->fromLocalPublic=old->fromLocalPublic;
	new->toLocalPublic=old->toLocalPublic;
#endif
	skb_shinfo(new)->gso_size = skb_shinfo(old)->gso_size;
	skb_shinfo(new)->gso_segs = skb_shinfo(old)->gso_segs;
	skb_shinfo(new)->gso_type = skb_shinfo(old)->gso_type;
}

/**
 *	skb_copy	-	create private copy of an sk_buff
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data. This is used when the
 *	caller wishes to modify the data and needs a private copy of the
 *	data to alter. Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	As by-product this function converts non-linear &sk_buff to linear
 *	one, so that &sk_buff becomes completely private and caller is allowed
 *	to modify all the data of returned buffer. This means that this
 *	function is not recommended for use in circumstances when only
 *	header is going to be modified. Use pskb_copy() instead.
 */

struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp_mask)
{
	int headerlen = skb->data - skb->head;
	/*
	 *	Allocate the copy buffer
	 */
	struct sk_buff *n;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	n = alloc_skb(skb->end + skb->data_len, gfp_mask);
#else
	n = alloc_skb(skb->end - skb->head + skb->data_len, gfp_mask);
#endif
	if (!n)
		return NULL;

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	if (skb_copy_bits(skb, -headerlen, n->head, headerlen + skb->len))
		BUG();

	copy_skb_header(n, skb);
	return n;
}
EXPORT_SYMBOL(skb_copy);

/**
 *	pskb_copy	-	create copy of an sk_buff with private head.
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and part of its data, located
 *	in header. Fragmented data remain shared. This is used when
 *	the caller wishes to modify only header of &sk_buff and needs
 *	private copy of the header to alter. Returns %NULL on failure
 *	or the pointer to the buffer on success.
 *	The returned buffer has a reference count of 1.
 */

struct sk_buff *pskb_copy(struct sk_buff *skb, gfp_t gfp_mask)
{
	/*
	 *	Allocate the copy buffer
	 */
	struct sk_buff *n;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	n = alloc_skb(skb->end, gfp_mask);
#else
	n = alloc_skb(skb->end - skb->head, gfp_mask);
#endif
	if (!n)
		goto out;


	/*linux-2.6.19*/ 
	/*patch from linux 2.4*/
	// Kaohj --- keep vlan_member for multicast fragment
	//  to keep port mapping vlan_member information
	n->vlan_member = skb->vlan_member;
	n->from_dev = skb->from_dev;


	/* Set the data pointer */
	skb_reserve(n, skb->data - skb->head);
	/* Set the tail pointer and length */
	skb_put(n, skb_headlen(skb));
	/* Copy the bytes */
	skb_copy_from_linear_data(skb, n->data, n->len);

	n->truesize += skb->data_len;
	n->data_len  = skb->data_len;
	n->len	     = skb->len;

	if (skb_shinfo(skb)->nr_frags) {
		int i;

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			skb_shinfo(n)->frags[i] = skb_shinfo(skb)->frags[i];
			get_page(skb_shinfo(n)->frags[i].page);
		}
		skb_shinfo(n)->nr_frags = i;
	}

	if (skb_shinfo(skb)->frag_list) {
		skb_shinfo(n)->frag_list = skb_shinfo(skb)->frag_list;
		skb_clone_fraglist(n);
	}

	copy_skb_header(n, skb);
out:
	return n;
}
EXPORT_SYMBOL(pskb_copy);

/**
 *	pskb_expand_head - reallocate header of &sk_buff
 *	@skb: buffer to reallocate
 *	@nhead: room to add at head
 *	@ntail: room to add at tail
 *	@gfp_mask: allocation priority
 *
 *	Expands (or creates identical copy, if &nhead and &ntail are zero)
 *	header of skb. &sk_buff itself is not changed. &sk_buff MUST have
 *	reference count of 1. Returns zero in the case of success or error,
 *	if expansion failed. In the last case, &sk_buff is not changed.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

int pskb_expand_head(struct sk_buff *skb, int nhead, int ntail,
		     gfp_t gfp_mask)
{
	int i;
	u8 *data;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	int size = nhead + skb->end + ntail;
#else
	int size = nhead + (skb->end - skb->head) + ntail;
#endif
	long off;

	BUG_ON(nhead < 0);

	if (skb_shared(skb))
		BUG();

	size = SKB_DATA_ALIGN(size);

	data = kmalloc(size + sizeof(struct skb_shared_info), gfp_mask);
	if (!data)
		goto nodata;

	/* Copy only real data... and, alas, header. This should be
	 * optimized for the cases when header is void. */
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	memcpy(data + nhead, skb->head, skb->tail);
#else
	memcpy(data + nhead, skb->head, skb->tail - skb->head);
#endif
	memcpy(data + size, skb_end_pointer(skb),
	       sizeof(struct skb_shared_info));

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
		get_page(skb_shinfo(skb)->frags[i].page);

	if (skb_shinfo(skb)->frag_list)
		skb_clone_fraglist(skb);

	skb_release_data(skb);

	off = (data + nhead) - skb->head;

	skb->head     = data;
	skb->data    += off;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->end      = size;
	off           = nhead;
#else
	skb->end      = skb->head + size;
#endif
	/* {transport,network,mac}_header and tail are relative to skb->head */
	skb->tail	      += off;
	skb->transport_header += off;
	skb->network_header   += off;
	skb->mac_header	      += off;
	skb->csum_start       += nhead;
	skb->cloned   = 0;
	skb->hdr_len  = 0;
	skb->nohdr    = 0;
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
	/*skb header should return back to pool*/
	skb->prealloc_flags &= ~SKB_DATA_PREALLOC;
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV
	atomic_set(&skb_shinfo(skb)->dataref, 1);
	return 0;

nodata:
	return -ENOMEM;
}
EXPORT_SYMBOL(pskb_expand_head);

/* Make private copy of skb with writable head and some headroom */

struct sk_buff *skb_realloc_headroom(struct sk_buff *skb, unsigned int headroom)
{
	struct sk_buff *skb2;
	int delta = headroom - skb_headroom(skb);

	if (delta <= 0)
		skb2 = pskb_copy(skb, GFP_ATOMIC);
	else {
		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 && pskb_expand_head(skb2, SKB_DATA_ALIGN(delta), 0,
					     GFP_ATOMIC)) {
			kfree_skb(skb2);
			skb2 = NULL;
		}
	}
	return skb2;
}
EXPORT_SYMBOL(skb_realloc_headroom);

/**
 *	skb_copy_expand	-	copy and expand sk_buff
 *	@skb: buffer to copy
 *	@newheadroom: new free bytes at head
 *	@newtailroom: new free bytes at tail
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data and while doing so
 *	allocate additional space.
 *
 *	This is used when the caller wishes to modify the data and needs a
 *	private copy of the data to alter as well as more space for new fields.
 *	Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	You must pass %GFP_ATOMIC as the allocation priority if this function
 *	is called from an interrupt.
 */
struct sk_buff *skb_copy_expand(const struct sk_buff *skb,
				int newheadroom, int newtailroom,
				gfp_t gfp_mask)
{
	/*
	 *	Allocate the copy buffer
	 */
	struct sk_buff *n = alloc_skb(newheadroom + skb->len + newtailroom,
				      gfp_mask);
	int oldheadroom = skb_headroom(skb);
	int head_copy_len, head_copy_off;
	int off;

	if (!n)
		return NULL;

	skb_reserve(n, newheadroom);

	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	head_copy_len = oldheadroom;
	head_copy_off = 0;
	if (newheadroom <= head_copy_len)
		head_copy_len = newheadroom;
	else
		head_copy_off = newheadroom - head_copy_len;

	/* Copy the linear header and data. */
	if (skb_copy_bits(skb, -head_copy_len, n->head + head_copy_off,
			  skb->len + head_copy_len))
		BUG();

	copy_skb_header(n, skb);

	off                  = newheadroom - oldheadroom;
	n->csum_start       += off;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	n->transport_header += off;
	n->network_header   += off;
	n->mac_header	    += off;
#endif

	return n;
}
EXPORT_SYMBOL(skb_copy_expand);

/**
 *	skb_pad			-	zero pad the tail of an skb
 *	@skb: buffer to pad
 *	@pad: space to pad
 *
 *	Ensure that a buffer is followed by a padding area that is zero
 *	filled. Used by network drivers which may DMA or transfer data
 *	beyond the buffer end onto the wire.
 *
 *	May return error in out of memory cases. The skb is freed on error.
 */

int skb_pad(struct sk_buff *skb, int pad)
{
	int err;
	int ntail;

	/* If the skbuff is non linear tailroom is always zero.. */
	if (!skb_cloned(skb) && skb_tailroom(skb) >= pad) {
		memset(skb->data+skb->len, 0, pad);
		return 0;
	}

	ntail = skb->data_len + pad - (skb->end - skb->tail);
	if (likely(skb_cloned(skb) || ntail > 0)) {
		err = pskb_expand_head(skb, 0, ntail, GFP_ATOMIC);
		if (unlikely(err))
			goto free_skb;
	}

	/* FIXME: The use of this function with non-linear skb's really needs
	 * to be audited.
	 */
	err = skb_linearize(skb);
	if (unlikely(err))
		goto free_skb;

	memset(skb->data + skb->len, 0, pad);
	return 0;

free_skb:
	kfree_skb(skb);
	return err;
}
EXPORT_SYMBOL(skb_pad);

/**
 *	skb_put - add data to a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned.
 */
unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb_tail_pointer(skb);
	SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len  += len;
	if (unlikely(skb->tail > skb->end))
		skb_over_panic(skb, len, __builtin_return_address(0));
	return tmp;
}
EXPORT_SYMBOL(skb_put);

/**
 *	skb_push - add data to the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned.
 */
unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
	if (unlikely(skb->data<skb->head))
		skb_under_panic(skb, len, __builtin_return_address(0));
	return skb->data;
}
EXPORT_SYMBOL(skb_push);

/**
 *	skb_pull - remove data from the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to remove
 *
 *	This function removes data from the start of a buffer, returning
 *	the memory to the headroom. A pointer to the next data in the buffer
 *	is returned. Once the data has been pulled future pushes will overwrite
 *	the old data.
 */
unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
	return unlikely(len > skb->len) ? NULL : __skb_pull(skb, len);
}
EXPORT_SYMBOL(skb_pull);

/**
 *	skb_trim - remove end from a buffer
 *	@skb: buffer to alter
 *	@len: new length
 *
 *	Cut the length of a buffer down by removing data from the tail. If
 *	the buffer is already under the length specified it is not modified.
 *	The skb must be linear.
 */
void skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->len > len)
		__skb_trim(skb, len);
}
EXPORT_SYMBOL(skb_trim);

/* Trims skb to length len. It can change skb pointers.
 */

int ___pskb_trim(struct sk_buff *skb, unsigned int len)
{
	struct sk_buff **fragp;
	struct sk_buff *frag;
	int offset = skb_headlen(skb);
	int nfrags = skb_shinfo(skb)->nr_frags;
	int i;
	int err;

	if (skb_cloned(skb) &&
	    unlikely((err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC))))
		return err;

	i = 0;
	if (offset >= len)
		goto drop_pages;

	for (; i < nfrags; i++) {
		int end = offset + skb_shinfo(skb)->frags[i].size;

		if (end < len) {
			offset = end;
			continue;
		}

		skb_shinfo(skb)->frags[i++].size = len - offset;

drop_pages:
		skb_shinfo(skb)->nr_frags = i;

		for (; i < nfrags; i++)
			put_page(skb_shinfo(skb)->frags[i].page);

		if (skb_shinfo(skb)->frag_list)
			skb_drop_fraglist(skb);
		goto done;
	}

	for (fragp = &skb_shinfo(skb)->frag_list; (frag = *fragp);
	     fragp = &frag->next) {
		int end = offset + frag->len;

		if (skb_shared(frag)) {
			struct sk_buff *nfrag;

			nfrag = skb_clone(frag, GFP_ATOMIC);
			if (unlikely(!nfrag))
				return -ENOMEM;

			nfrag->next = frag->next;
			kfree_skb(frag);
			frag = nfrag;
			*fragp = frag;
		}

		if (end < len) {
			offset = end;
			continue;
		}

		if (end > len &&
		    unlikely((err = pskb_trim(frag, len - offset))))
			return err;

		if (frag->next)
			skb_drop_list(&frag->next);
		break;
	}

done:
	if (len > skb_headlen(skb)) {
		skb->data_len -= skb->len - len;
		skb->len       = len;
	} else {
		skb->len       = len;
		skb->data_len  = 0;
		skb_set_tail_pointer(skb, len);
	}

	return 0;
}
EXPORT_SYMBOL(___pskb_trim);

/**
 *	__pskb_pull_tail - advance tail of skb header
 *	@skb: buffer to reallocate
 *	@delta: number of bytes to advance tail
 *
 *	The function makes a sense only on a fragmented &sk_buff,
 *	it expands header moving its tail forward and copying necessary
 *	data from fragmented part.
 *
 *	&sk_buff MUST have reference count of 1.
 *
 *	Returns %NULL (and &sk_buff does not change) if pull failed
 *	or value of new tail of skb in the case of success.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

/* Moves tail of skb head forward, copying data from fragmented part,
 * when it is necessary.
 * 1. It may fail due to malloc failure.
 * 2. It may change skb pointers.
 *
 * It is pretty complicated. Luckily, it is called only in exceptional cases.
 */
unsigned char *__pskb_pull_tail(struct sk_buff *skb, int delta)
{
	/* If skb has not enough free space at tail, get new one
	 * plus 128 bytes for future expansions. If we have enough
	 * room at tail, reallocate without expansion only if skb is cloned.
	 */
	int i, k, eat = (skb->tail + delta) - skb->end;

	if (eat > 0 || skb_cloned(skb)) {
		if (pskb_expand_head(skb, 0, eat > 0 ? eat + 128 : 0,
				     GFP_ATOMIC))
			return NULL;
	}

	if (skb_copy_bits(skb, skb_headlen(skb), skb_tail_pointer(skb), delta))
		BUG();

	/* Optimization: no fragments, no reasons to preestimate
	 * size of pulled pages. Superb.
	 */
	if (!skb_shinfo(skb)->frag_list)
		goto pull_pages;

	/* Estimate size of pulled pages. */
	eat = delta;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		if (skb_shinfo(skb)->frags[i].size >= eat)
			goto pull_pages;
		eat -= skb_shinfo(skb)->frags[i].size;
	}

	/* If we need update frag list, we are in troubles.
	 * Certainly, it possible to add an offset to skb data,
	 * but taking into account that pulling is expected to
	 * be very rare operation, it is worth to fight against
	 * further bloating skb head and crucify ourselves here instead.
	 * Pure masohism, indeed. 8)8)
	 */
	if (eat) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;
		struct sk_buff *clone = NULL;
		struct sk_buff *insp = NULL;

		do {
			BUG_ON(!list);

			if (list->len <= eat) {
				/* Eaten as whole. */
				eat -= list->len;
				list = list->next;
				insp = list;
			} else {
				/* Eaten partially. */

				if (skb_shared(list)) {
					/* Sucks! We need to fork list. :-( */
					clone = skb_clone(list, GFP_ATOMIC);
					if (!clone)
						return NULL;
					insp = list->next;
					list = clone;
				} else {
					/* This may be pulled without
					 * problems. */
					insp = list;
				}
				if (!pskb_pull(list, eat)) {
					kfree_skb(clone);
					return NULL;
				}
				break;
			}
		} while (eat);

		/* Free pulled out fragments. */
		while ((list = skb_shinfo(skb)->frag_list) != insp) {
			skb_shinfo(skb)->frag_list = list->next;
			kfree_skb(list);
		}
		/* And insert new clone at head. */
		if (clone) {
			clone->next = list;
			skb_shinfo(skb)->frag_list = clone;
		}
	}
	/* Success! Now we may commit changes to skb data. */

pull_pages:
	eat = delta;
	k = 0;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		if (skb_shinfo(skb)->frags[i].size <= eat) {
			put_page(skb_shinfo(skb)->frags[i].page);
			eat -= skb_shinfo(skb)->frags[i].size;
		} else {
			skb_shinfo(skb)->frags[k] = skb_shinfo(skb)->frags[i];
			if (eat) {
				skb_shinfo(skb)->frags[k].page_offset += eat;
				skb_shinfo(skb)->frags[k].size -= eat;
				eat = 0;
			}
			k++;
		}
	}
	skb_shinfo(skb)->nr_frags = k;

	skb->tail     += delta;
	skb->data_len -= delta;

	return skb_tail_pointer(skb);
}
EXPORT_SYMBOL(__pskb_pull_tail);

/* Copy some data bits from skb to kernel buffer. */

int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int i, copy;
	int start = skb_headlen(skb);

	if (offset > (int)skb->len - len)
		goto fault;

	/* Copy header. */
	if ((copy = start - offset) > 0) {
		if (copy > len)
			copy = len;
		skb_copy_from_linear_data_offset(skb, offset, to, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		to     += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end - offset) > 0) {
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = kmap_skb_frag(&skb_shinfo(skb)->frags[i]);
			memcpy(to,
			       vaddr + skb_shinfo(skb)->frags[i].page_offset+
			       offset - start, copy);
			kunmap_skb_frag(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to     += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list; list = list->next) {
			int end;

			WARN_ON(start > offset + len);

			end = start + list->len;
			if ((copy = end - offset) > 0) {
				if (copy > len)
					copy = len;
				if (skb_copy_bits(list, offset - start,
						  to, copy))
					goto fault;
				if ((len -= copy) == 0)
					return 0;
				offset += copy;
				to     += copy;
			}
			start = end;
		}
	}
	if (!len)
		return 0;

fault:
	return -EFAULT;
}
EXPORT_SYMBOL(skb_copy_bits);

/*
 * Callback from splice_to_pipe(), if we need to release some pages
 * at the end of the spd in case we error'ed out in filling the pipe.
 */
static void sock_spd_release(struct splice_pipe_desc *spd, unsigned int i)
{
	put_page(spd->pages[i]);
}

static inline struct page *linear_to_page(struct page *page, unsigned int *len,
					  unsigned int *offset,
					  struct sk_buff *skb, struct sock *sk)
{
	struct page *p = sk->sk_sndmsg_page;
	unsigned int off;

	if (!p) {
new_page:
		p = sk->sk_sndmsg_page = alloc_pages(sk->sk_allocation, 0);
		if (!p)
			return NULL;

		off = sk->sk_sndmsg_off = 0;
		/* hold one ref to this page until it's full */
	} else {
		unsigned int mlen;

		off = sk->sk_sndmsg_off;
		mlen = PAGE_SIZE - off;
		if (mlen < 64 && mlen < *len) {
			put_page(p);
			goto new_page;
		}

		*len = min_t(unsigned int, *len, mlen);
	}

	memcpy(page_address(p) + off, page_address(page) + *offset, *len);
	sk->sk_sndmsg_off += *len;
	*offset = off;
	get_page(p);

	return p;
}

/*
 * Fill page/offset/length into spd, if it can hold more pages.
 */
static inline int spd_fill_page(struct splice_pipe_desc *spd, struct page *page,
				unsigned int *len, unsigned int offset,
				struct sk_buff *skb, int linear,
				struct sock *sk)
{
	if (unlikely(spd->nr_pages == PIPE_BUFFERS))
		return 1;

	if (linear) {
		page = linear_to_page(page, len, &offset, skb, sk);
		if (!page)
			return 1;
	} else
		get_page(page);

	spd->pages[spd->nr_pages] = page;
	spd->partial[spd->nr_pages].len = *len;
	spd->partial[spd->nr_pages].offset = offset;
	spd->nr_pages++;

	return 0;
}

static inline void __segment_seek(struct page **page, unsigned int *poff,
				  unsigned int *plen, unsigned int off)
{
	unsigned long n;

	*poff += off;
	n = *poff / PAGE_SIZE;
	if (n)
		*page = nth_page(*page, n);

	*poff = *poff % PAGE_SIZE;
	*plen -= off;
}

static inline int __splice_segment(struct page *page, unsigned int poff,
				   unsigned int plen, unsigned int *off,
				   unsigned int *len, struct sk_buff *skb,
				   struct splice_pipe_desc *spd, int linear,
				   struct sock *sk)
{
	if (!*len)
		return 1;

	/* skip this segment if already processed */
	if (*off >= plen) {
		*off -= plen;
		return 0;
	}

	/* ignore any bits we already processed */
	if (*off) {
		__segment_seek(&page, &poff, &plen, *off);
		*off = 0;
	}

	do {
		unsigned int flen = min(*len, plen);

		/* the linear region may spread across several pages  */
		flen = min_t(unsigned int, flen, PAGE_SIZE - poff);

		if (spd_fill_page(spd, page, &flen, poff, skb, linear, sk))
			return 1;

		__segment_seek(&page, &poff, &plen, flen);
		*len -= flen;

	} while (*len && plen);

	return 0;
}

/*
 * Map linear and fragment data from the skb to spd. It reports failure if the
 * pipe is full or if we already spliced the requested length.
 */
static int __skb_splice_bits(struct sk_buff *skb, unsigned int *offset,
			     unsigned int *len, struct splice_pipe_desc *spd,
			     struct sock *sk)
{
	int seg;

	/*
	 * map the linear part
	 */
	if (__splice_segment(virt_to_page(skb->data),
			     (unsigned long) skb->data & (PAGE_SIZE - 1),
			     skb_headlen(skb),
			     offset, len, skb, spd, 1, sk))
		return 1;

	/*
	 * then map the fragments
	 */
	for (seg = 0; seg < skb_shinfo(skb)->nr_frags; seg++) {
		const skb_frag_t *f = &skb_shinfo(skb)->frags[seg];

		if (__splice_segment(f->page, f->page_offset, f->size,
				     offset, len, skb, spd, 0, sk))
			return 1;
	}

	return 0;
}

/*
 * Map data from the skb to a pipe. Should handle both the linear part,
 * the fragments, and the frag list. It does NOT handle frag lists within
 * the frag list, if such a thing exists. We'd probably need to recurse to
 * handle that cleanly.
 */
int skb_splice_bits(struct sk_buff *skb, unsigned int offset,
		    struct pipe_inode_info *pipe, unsigned int tlen,
		    unsigned int flags)
{
	struct partial_page partial[PIPE_BUFFERS];
	struct page *pages[PIPE_BUFFERS];
	struct splice_pipe_desc spd = {
		.pages = pages,
		.partial = partial,
		.flags = flags,
		.ops = &sock_pipe_buf_ops,
		.spd_release = sock_spd_release,
	};
	struct sock *sk = skb->sk;

	/*
	 * __skb_splice_bits() only fails if the output has no room left,
	 * so no point in going over the frag_list for the error case.
	 */
	if (__skb_splice_bits(skb, &offset, &tlen, &spd, sk))
		goto done;
	else if (!tlen)
		goto done;

	/*
	 * now see if we have a frag_list to map
	 */
	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list && tlen; list = list->next) {
			if (__skb_splice_bits(list, &offset, &tlen, &spd, sk))
				break;
		}
	}

done:
	if (spd.nr_pages) {
		int ret;

		/*
		 * Drop the socket lock, otherwise we have reverse
		 * locking dependencies between sk_lock and i_mutex
		 * here as compared to sendfile(). We enter here
		 * with the socket lock held, and splice_to_pipe() will
		 * grab the pipe inode lock. For sendfile() emulation,
		 * we call into ->sendpage() with the i_mutex lock held
		 * and networking will grab the socket lock.
		 */
		release_sock(sk);
		ret = splice_to_pipe(pipe, &spd);
		lock_sock(sk);
		return ret;
	}

	return 0;
}

/**
 *	skb_store_bits - store bits from kernel buffer to skb
 *	@skb: destination buffer
 *	@offset: offset in destination
 *	@from: source buffer
 *	@len: number of bytes to copy
 *
 *	Copy the specified number of bytes from the source buffer to the
 *	destination skb.  This function handles all the messy bits of
 *	traversing fragment lists and such.
 */

int skb_store_bits(struct sk_buff *skb, int offset, const void *from, int len)
{
	int i, copy;
	int start = skb_headlen(skb);

	if (offset > (int)skb->len - len)
		goto fault;

	if ((copy = start - offset) > 0) {
		if (copy > len)
			copy = len;
		skb_copy_to_linear_data_offset(skb, offset, from, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		from += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		int end;

		WARN_ON(start > offset + len);

		end = start + frag->size;
		if ((copy = end - offset) > 0) {
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = kmap_skb_frag(frag);
			memcpy(vaddr + frag->page_offset + offset - start,
			       from, copy);
			kunmap_skb_frag(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			from += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list; list = list->next) {
			int end;

			WARN_ON(start > offset + len);

			end = start + list->len;
			if ((copy = end - offset) > 0) {
				if (copy > len)
					copy = len;
				if (skb_store_bits(list, offset - start,
						   from, copy))
					goto fault;
				if ((len -= copy) == 0)
					return 0;
				offset += copy;
				from += copy;
			}
			start = end;
		}
	}
	if (!len)
		return 0;

fault:
	return -EFAULT;
}
EXPORT_SYMBOL(skb_store_bits);

/* Checksum skb data. */

__wsum skb_checksum(const struct sk_buff *skb, int offset,
			  int len, __wsum csum)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	int pos = 0;

	/* Checksum header. */
	if (copy > 0) {
		if (copy > len)
			copy = len;
		csum = csum_partial(skb->data + offset, copy, csum);
		if ((len -= copy) == 0)
			return csum;
		offset += copy;
		pos	= copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end - offset) > 0) {
			__wsum csum2;
			u8 *vaddr;
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			if (copy > len)
				copy = len;
			vaddr = kmap_skb_frag(frag);
			csum2 = csum_partial(vaddr + frag->page_offset +
					     offset - start, copy, 0);
			kunmap_skb_frag(vaddr);
			csum = csum_block_add(csum, csum2, pos);
			if (!(len -= copy))
				return csum;
			offset += copy;
			pos    += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list; list = list->next) {
			int end;

			WARN_ON(start > offset + len);

			end = start + list->len;
			if ((copy = end - offset) > 0) {
				__wsum csum2;
				if (copy > len)
					copy = len;
				csum2 = skb_checksum(list, offset - start,
						     copy, 0);
				csum = csum_block_add(csum, csum2, pos);
				if ((len -= copy) == 0)
					return csum;
				offset += copy;
				pos    += copy;
			}
			start = end;
		}
	}
	BUG_ON(len);

	return csum;
}
EXPORT_SYMBOL(skb_checksum);

/* Both of above in one bottle. */

__wsum skb_copy_and_csum_bits(const struct sk_buff *skb, int offset,
				    u8 *to, int len, __wsum csum)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	int pos = 0;

	/* Copy header. */
	if (copy > 0) {
		if (copy > len)
			copy = len;
		csum = csum_partial_copy_nocheck(skb->data + offset, to,
						 copy, csum);
		if ((len -= copy) == 0)
			return csum;
		offset += copy;
		to     += copy;
		pos	= copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end - offset) > 0) {
			__wsum csum2;
			u8 *vaddr;
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			if (copy > len)
				copy = len;
			vaddr = kmap_skb_frag(frag);
			csum2 = csum_partial_copy_nocheck(vaddr +
							  frag->page_offset +
							  offset - start, to,
							  copy, 0);
			kunmap_skb_frag(vaddr);
			csum = csum_block_add(csum, csum2, pos);
			if (!(len -= copy))
				return csum;
			offset += copy;
			to     += copy;
			pos    += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list; list = list->next) {
			__wsum csum2;
			int end;

			WARN_ON(start > offset + len);

			end = start + list->len;
			if ((copy = end - offset) > 0) {
				if (copy > len)
					copy = len;
				csum2 = skb_copy_and_csum_bits(list,
							       offset - start,
							       to, copy, 0);
				csum = csum_block_add(csum, csum2, pos);
				if ((len -= copy) == 0)
					return csum;
				offset += copy;
				to     += copy;
				pos    += copy;
			}
			start = end;
		}
	}
	BUG_ON(len);
	return csum;
}
EXPORT_SYMBOL(skb_copy_and_csum_bits);

void skb_copy_and_csum_dev(const struct sk_buff *skb, u8 *to)
{
	__wsum csum;
	long csstart;

	if (skb->ip_summed == CHECKSUM_PARTIAL)
		csstart = skb->csum_start - skb_headroom(skb);
	else
		csstart = skb_headlen(skb);

	BUG_ON(csstart > skb_headlen(skb));

	skb_copy_from_linear_data(skb, to, csstart);

	csum = 0;
	if (csstart != skb->len)
		csum = skb_copy_and_csum_bits(skb, csstart, to + csstart,
					      skb->len - csstart, 0);

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		long csstuff = csstart + skb->csum_offset;

		*((__sum16 *)(to + csstuff)) = csum_fold(csum);
	}
}
EXPORT_SYMBOL(skb_copy_and_csum_dev);

/**
 *	skb_dequeue - remove from the head of the queue
 *	@list: list to dequeue from
 *
 *	Remove the head of the list. The list lock is taken so the function
 *	may be used safely with other locking list functions. The head item is
 *	returned or %NULL if the list is empty.
 */

struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}
EXPORT_SYMBOL(skb_dequeue);

/**
 *	skb_dequeue_tail - remove from the tail of the queue
 *	@list: list to dequeue from
 *
 *	Remove the tail of the list. The list lock is taken so the function
 *	may be used safely with other locking list functions. The tail item is
 *	returned or %NULL if the list is empty.
 */
struct sk_buff *skb_dequeue_tail(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue_tail(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}
EXPORT_SYMBOL(skb_dequeue_tail);

/**
 *	skb_queue_purge - empty a list
 *	@list: list to empty
 *
 *	Delete all buffers on an &sk_buff list. Each buffer is removed from
 *	the list and one reference dropped. This function takes the list
 *	lock and is atomic with respect to other list locking functions.
 */
void skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(list)) != NULL)
		kfree_skb(skb);
}
EXPORT_SYMBOL(skb_queue_purge);

/**
 *	skb_queue_head - queue a buffer at the list head
 *	@list: list to use
 *	@newsk: buffer to queue
 *
 *	Queue a buffer at the start of the list. This function takes the
 *	list lock and can be used safely with other locking &sk_buff functions
 *	safely.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_head(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_queue_head);

/**
 *	skb_queue_tail - queue a buffer at the list tail
 *	@list: list to use
 *	@newsk: buffer to queue
 *
 *	Queue a buffer at the tail of the list. This function takes the
 *	list lock and can be used safely with other locking &sk_buff functions
 *	safely.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_tail(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_queue_tail);

/**
 *	skb_unlink	-	remove a buffer from a list
 *	@skb: buffer to remove
 *	@list: list to use
 *
 *	Remove a packet from a list. The list locks are taken and this
 *	function is atomic with respect to other list locked calls
 *
 *	You must know what list the SKB is on.
 */
void skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_unlink(skb, list);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_unlink);

/**
 *	skb_append	-	append a buffer
 *	@old: buffer to insert after
 *	@newsk: buffer to insert
 *	@list: list to use
 *
 *	Place a packet after a given packet in a list. The list locks are taken
 *	and this function is atomic with respect to other list locked calls.
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_append(struct sk_buff *old, struct sk_buff *newsk, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_after(list, old, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_append);

/**
 *	skb_insert	-	insert a buffer
 *	@old: buffer to insert before
 *	@newsk: buffer to insert
 *	@list: list to use
 *
 *	Place a packet before a given packet in a list. The list locks are
 * 	taken and this function is atomic with respect to other list locked
 *	calls.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_insert(struct sk_buff *old, struct sk_buff *newsk, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_insert(newsk, old->prev, old, list);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_insert);

static inline void skb_split_inside_header(struct sk_buff *skb,
					   struct sk_buff* skb1,
					   const u32 len, const int pos)
{
	int i;

	skb_copy_from_linear_data_offset(skb, len, skb_put(skb1, pos - len),
					 pos - len);
	/* And move data appendix as is. */
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
		skb_shinfo(skb1)->frags[i] = skb_shinfo(skb)->frags[i];

	skb_shinfo(skb1)->nr_frags = skb_shinfo(skb)->nr_frags;
	skb_shinfo(skb)->nr_frags  = 0;
	skb1->data_len		   = skb->data_len;
	skb1->len		   += skb1->data_len;
	skb->data_len		   = 0;
	skb->len		   = len;
	skb_set_tail_pointer(skb, len);
}

static inline void skb_split_no_header(struct sk_buff *skb,
				       struct sk_buff* skb1,
				       const u32 len, int pos)
{
	int i, k = 0;
	const int nfrags = skb_shinfo(skb)->nr_frags;

	skb_shinfo(skb)->nr_frags = 0;
	skb1->len		  = skb1->data_len = skb->len - len;
	skb->len		  = len;
	skb->data_len		  = len - pos;

	for (i = 0; i < nfrags; i++) {
		int size = skb_shinfo(skb)->frags[i].size;

		if (pos + size > len) {
			skb_shinfo(skb1)->frags[k] = skb_shinfo(skb)->frags[i];

			if (pos < len) {
				/* Split frag.
				 * We have two variants in this case:
				 * 1. Move all the frag to the second
				 *    part, if it is possible. F.e.
				 *    this approach is mandatory for TUX,
				 *    where splitting is expensive.
				 * 2. Split is accurately. We make this.
				 */
				get_page(skb_shinfo(skb)->frags[i].page);
				skb_shinfo(skb1)->frags[0].page_offset += len - pos;
				skb_shinfo(skb1)->frags[0].size -= len - pos;
				skb_shinfo(skb)->frags[i].size	= len - pos;
				skb_shinfo(skb)->nr_frags++;
			}
			k++;
		} else
			skb_shinfo(skb)->nr_frags++;
		pos += size;
	}
	skb_shinfo(skb1)->nr_frags = k;
}

/**
 * skb_split - Split fragmented skb to two parts at length len.
 * @skb: the buffer to split
 * @skb1: the buffer to receive the second part
 * @len: new length for skb
 */
void skb_split(struct sk_buff *skb, struct sk_buff *skb1, const u32 len)
{
	int pos = skb_headlen(skb);

	if (len < pos)	/* Split line is inside header. */
		skb_split_inside_header(skb, skb1, len, pos);
	else		/* Second chunk has no header, nothing to copy. */
		skb_split_no_header(skb, skb1, len, pos);
}
EXPORT_SYMBOL(skb_split);

/* Shifting from/to a cloned skb is a no-go.
 *
 * Caller cannot keep skb_shinfo related pointers past calling here!
 */
static int skb_prepare_for_shift(struct sk_buff *skb)
{
	return skb_cloned(skb) && pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}

/**
 * skb_shift - Shifts paged data partially from skb to another
 * @tgt: buffer into which tail data gets added
 * @skb: buffer from which the paged data comes from
 * @shiftlen: shift up to this many bytes
 *
 * Attempts to shift up to shiftlen worth of bytes, which may be less than
 * the length of the skb, from tgt to skb. Returns number bytes shifted.
 * It's up to caller to free skb if everything was shifted.
 *
 * If @tgt runs out of frags, the whole operation is aborted.
 *
 * Skb cannot include anything else but paged data while tgt is allowed
 * to have non-paged data as well.
 *
 * TODO: full sized shift could be optimized but that would need
 * specialized skb free'er to handle frags without up-to-date nr_frags.
 */
int skb_shift(struct sk_buff *tgt, struct sk_buff *skb, int shiftlen)
{
	int from, to, merge, todo;
	struct skb_frag_struct *fragfrom, *fragto;

	BUG_ON(shiftlen > skb->len);
	BUG_ON(skb_headlen(skb));	/* Would corrupt stream */

	todo = shiftlen;
	from = 0;
	to = skb_shinfo(tgt)->nr_frags;
	fragfrom = &skb_shinfo(skb)->frags[from];

	/* Actual merge is delayed until the point when we know we can
	 * commit all, so that we don't have to undo partial changes
	 */
	if (!to ||
	    !skb_can_coalesce(tgt, to, fragfrom->page, fragfrom->page_offset)) {
		merge = -1;
	} else {
		merge = to - 1;

		todo -= fragfrom->size;
		if (todo < 0) {
			if (skb_prepare_for_shift(skb) ||
			    skb_prepare_for_shift(tgt))
				return 0;

			/* All previous frag pointers might be stale! */
			fragfrom = &skb_shinfo(skb)->frags[from];
			fragto = &skb_shinfo(tgt)->frags[merge];

			fragto->size += shiftlen;
			fragfrom->size -= shiftlen;
			fragfrom->page_offset += shiftlen;

			goto onlymerged;
		}

		from++;
	}

	/* Skip full, not-fitting skb to avoid expensive operations */
	if ((shiftlen == skb->len) &&
	    (skb_shinfo(skb)->nr_frags - from) > (MAX_SKB_FRAGS - to))
		return 0;

	if (skb_prepare_for_shift(skb) || skb_prepare_for_shift(tgt))
		return 0;

	while ((todo > 0) && (from < skb_shinfo(skb)->nr_frags)) {
		if (to == MAX_SKB_FRAGS)
			return 0;

		fragfrom = &skb_shinfo(skb)->frags[from];
		fragto = &skb_shinfo(tgt)->frags[to];

		if (todo >= fragfrom->size) {
			*fragto = *fragfrom;
			todo -= fragfrom->size;
			from++;
			to++;

		} else {
			get_page(fragfrom->page);
			fragto->page = fragfrom->page;
			fragto->page_offset = fragfrom->page_offset;
			fragto->size = todo;

			fragfrom->page_offset += todo;
			fragfrom->size -= todo;
			todo = 0;

			to++;
			break;
		}
	}

	/* Ready to "commit" this state change to tgt */
	skb_shinfo(tgt)->nr_frags = to;

	if (merge >= 0) {
		fragfrom = &skb_shinfo(skb)->frags[0];
		fragto = &skb_shinfo(tgt)->frags[merge];

		fragto->size += fragfrom->size;
		put_page(fragfrom->page);
	}

	/* Reposition in the original skb */
	to = 0;
	while (from < skb_shinfo(skb)->nr_frags)
		skb_shinfo(skb)->frags[to++] = skb_shinfo(skb)->frags[from++];
	skb_shinfo(skb)->nr_frags = to;

	BUG_ON(todo > 0 && !skb_shinfo(skb)->nr_frags);

onlymerged:
	/* Most likely the tgt won't ever need its checksum anymore, skb on
	 * the other hand might need it if it needs to be resent
	 */
	tgt->ip_summed = CHECKSUM_PARTIAL;
	skb->ip_summed = CHECKSUM_PARTIAL;

	/* Yak, is it really working this way? Some helper please? */
	skb->len -= shiftlen;
	skb->data_len -= shiftlen;
	skb->truesize -= shiftlen;
	tgt->len += shiftlen;
	tgt->data_len += shiftlen;
	tgt->truesize += shiftlen;

	return shiftlen;
}

/**
 * skb_prepare_seq_read - Prepare a sequential read of skb data
 * @skb: the buffer to read
 * @from: lower offset of data to be read
 * @to: upper offset of data to be read
 * @st: state variable
 *
 * Initializes the specified state variable. Must be called before
 * invoking skb_seq_read() for the first time.
 */
void skb_prepare_seq_read(struct sk_buff *skb, unsigned int from,
			  unsigned int to, struct skb_seq_state *st)
{
	st->lower_offset = from;
	st->upper_offset = to;
	st->root_skb = st->cur_skb = skb;
	st->frag_idx = st->stepped_offset = 0;
	st->frag_data = NULL;
}
EXPORT_SYMBOL(skb_prepare_seq_read);

/**
 * skb_seq_read - Sequentially read skb data
 * @consumed: number of bytes consumed by the caller so far
 * @data: destination pointer for data to be returned
 * @st: state variable
 *
 * Reads a block of skb data at &consumed relative to the
 * lower offset specified to skb_prepare_seq_read(). Assigns
 * the head of the data block to &data and returns the length
 * of the block or 0 if the end of the skb data or the upper
 * offset has been reached.
 *
 * The caller is not required to consume all of the data
 * returned, i.e. &consumed is typically set to the number
 * of bytes already consumed and the next call to
 * skb_seq_read() will return the remaining part of the block.
 *
 * Note 1: The size of each block of data returned can be arbitary,
 *       this limitation is the cost for zerocopy seqeuental
 *       reads of potentially non linear data.
 *
 * Note 2: Fragment lists within fragments are not implemented
 *       at the moment, state->root_skb could be replaced with
 *       a stack for this purpose.
 */
unsigned int skb_seq_read(unsigned int consumed, const u8 **data,
			  struct skb_seq_state *st)
{
	unsigned int block_limit, abs_offset = consumed + st->lower_offset;
	skb_frag_t *frag;

	if (unlikely(abs_offset >= st->upper_offset))
		return 0;

next_skb:
	block_limit = skb_headlen(st->cur_skb) + st->stepped_offset;

	if (abs_offset < block_limit && !st->frag_data) {
		*data = st->cur_skb->data + (abs_offset - st->stepped_offset);
		return block_limit - abs_offset;
	}

	if (st->frag_idx == 0 && !st->frag_data)
		st->stepped_offset += skb_headlen(st->cur_skb);

	while (st->frag_idx < skb_shinfo(st->cur_skb)->nr_frags) {
		frag = &skb_shinfo(st->cur_skb)->frags[st->frag_idx];
		block_limit = frag->size + st->stepped_offset;

		if (abs_offset < block_limit) {
			if (!st->frag_data)
				st->frag_data = kmap_skb_frag(frag);

			*data = (u8 *) st->frag_data + frag->page_offset +
				(abs_offset - st->stepped_offset);

			return block_limit - abs_offset;
		}

		if (st->frag_data) {
			kunmap_skb_frag(st->frag_data);
			st->frag_data = NULL;
		}

		st->frag_idx++;
		st->stepped_offset += frag->size;
	}

	if (st->frag_data) {
		kunmap_skb_frag(st->frag_data);
		st->frag_data = NULL;
	}

	if (st->root_skb == st->cur_skb &&
	    skb_shinfo(st->root_skb)->frag_list) {
		st->cur_skb = skb_shinfo(st->root_skb)->frag_list;
		st->frag_idx = 0;
		goto next_skb;
	} else if (st->cur_skb->next) {
		st->cur_skb = st->cur_skb->next;
		st->frag_idx = 0;
		goto next_skb;
	}

	return 0;
}
EXPORT_SYMBOL(skb_seq_read);

/**
 * skb_abort_seq_read - Abort a sequential read of skb data
 * @st: state variable
 *
 * Must be called if skb_seq_read() was not called until it
 * returned 0.
 */
void skb_abort_seq_read(struct skb_seq_state *st)
{
	if (st->frag_data)
		kunmap_skb_frag(st->frag_data);
}
EXPORT_SYMBOL(skb_abort_seq_read);

#define TS_SKB_CB(state)	((struct skb_seq_state *) &((state)->cb))

static unsigned int skb_ts_get_next_block(unsigned int offset, const u8 **text,
					  struct ts_config *conf,
					  struct ts_state *state)
{
	return skb_seq_read(offset, text, TS_SKB_CB(state));
}

static void skb_ts_finish(struct ts_config *conf, struct ts_state *state)
{
	skb_abort_seq_read(TS_SKB_CB(state));
}

/**
 * skb_find_text - Find a text pattern in skb data
 * @skb: the buffer to look in
 * @from: search offset
 * @to: search limit
 * @config: textsearch configuration
 * @state: uninitialized textsearch state variable
 *
 * Finds a pattern in the skb data according to the specified
 * textsearch configuration. Use textsearch_next() to retrieve
 * subsequent occurrences of the pattern. Returns the offset
 * to the first occurrence or UINT_MAX if no match was found.
 */
unsigned int skb_find_text(struct sk_buff *skb, unsigned int from,
			   unsigned int to, struct ts_config *config,
			   struct ts_state *state)
{
	unsigned int ret;

	config->get_next_block = skb_ts_get_next_block;
	config->finish = skb_ts_finish;

	skb_prepare_seq_read(skb, from, to, TS_SKB_CB(state));

	ret = textsearch_find(config, state);
	return (ret <= to - from ? ret : UINT_MAX);
}
EXPORT_SYMBOL(skb_find_text);

/**
 * skb_append_datato_frags: - append the user data to a skb
 * @sk: sock  structure
 * @skb: skb structure to be appened with user data.
 * @getfrag: call back function to be used for getting the user data
 * @from: pointer to user message iov
 * @length: length of the iov message
 *
 * Description: This procedure append the user data in the fragment part
 * of the skb if any page alloc fails user this procedure returns  -ENOMEM
 */
int skb_append_datato_frags(struct sock *sk, struct sk_buff *skb,
			int (*getfrag)(void *from, char *to, int offset,
					int len, int odd, struct sk_buff *skb),
			void *from, int length)
{
	int frg_cnt = 0;
	skb_frag_t *frag = NULL;
	struct page *page = NULL;
	int copy, left;
	int offset = 0;
	int ret;

	do {
		/* Return error if we don't have space for new frag */
		frg_cnt = skb_shinfo(skb)->nr_frags;
		if (frg_cnt >= MAX_SKB_FRAGS)
			return -EFAULT;

		/* allocate a new page for next frag */
		page = alloc_pages(sk->sk_allocation, 0);

		/* If alloc_page fails just return failure and caller will
		 * free previous allocated pages by doing kfree_skb()
		 */
		if (page == NULL)
			return -ENOMEM;

		/* initialize the next frag */
		sk->sk_sndmsg_page = page;
		sk->sk_sndmsg_off = 0;
		skb_fill_page_desc(skb, frg_cnt, page, 0, 0);
		skb->truesize += PAGE_SIZE;
		atomic_add(PAGE_SIZE, &sk->sk_wmem_alloc);

		/* get the new initialized frag */
		frg_cnt = skb_shinfo(skb)->nr_frags;
		frag = &skb_shinfo(skb)->frags[frg_cnt - 1];

		/* copy the user data to page */
		left = PAGE_SIZE - frag->page_offset;
		copy = (length > left)? left : length;

		ret = getfrag(from, (page_address(frag->page) +
			    frag->page_offset + frag->size),
			    offset, copy, 0, skb);
		if (ret < 0)
			return -EFAULT;

		/* copy was successful so update the size parameters */
		sk->sk_sndmsg_off += copy;
		frag->size += copy;
		skb->len += copy;
		skb->data_len += copy;
		offset += copy;
		length -= copy;

	} while (length > 0);

	return 0;
}
EXPORT_SYMBOL(skb_append_datato_frags);

/**
 *	skb_pull_rcsum - pull skb and update receive checksum
 *	@skb: buffer to update
 *	@len: length of data pulled
 *
 *	This function performs an skb_pull on the packet and updates
 *	the CHECKSUM_COMPLETE checksum.  It should be used on
 *	receive path processing instead of skb_pull unless you know
 *	that the checksum difference is zero (e.g., a valid IP header)
 *	or you are setting ip_summed to CHECKSUM_NONE.
 */
unsigned char *skb_pull_rcsum(struct sk_buff *skb, unsigned int len)
{
	BUG_ON(len > skb->len);
	skb->len -= len;
	BUG_ON(skb->len < skb->data_len);
	skb_postpull_rcsum(skb, skb->data, len);
	return skb->data += len;
}

EXPORT_SYMBOL_GPL(skb_pull_rcsum);

/**
 *	skb_segment - Perform protocol segmentation on skb.
 *	@skb: buffer to segment
 *	@features: features for the output path (see dev->features)
 *
 *	This function performs segmentation on the given skb.  It returns
 *	a pointer to the first in a list of new skbs for the segments.
 *	In case of error it returns ERR_PTR(err).
 */
struct sk_buff *skb_segment(struct sk_buff *skb, int features)
{
	struct sk_buff *segs = NULL;
	struct sk_buff *tail = NULL;
	struct sk_buff *fskb = skb_shinfo(skb)->frag_list;
	unsigned int mss = skb_shinfo(skb)->gso_size;
	unsigned int doffset = skb->data - skb_mac_header(skb);
	unsigned int offset = doffset;
	unsigned int headroom;
	unsigned int len;
	int sg = features & NETIF_F_SG;
	int nfrags = skb_shinfo(skb)->nr_frags;
	int err = -ENOMEM;
	int i = 0;
	int pos;

	__skb_push(skb, doffset);
	headroom = skb_headroom(skb);
	pos = skb_headlen(skb);

	do {
		struct sk_buff *nskb;
		skb_frag_t *frag;
		int hsize;
		int size;

		len = skb->len - offset;
		if (len > mss)
			len = mss;

		hsize = skb_headlen(skb) - offset;
		if (hsize < 0)
			hsize = 0;
		if (hsize > len || !sg)
			hsize = len;

		if (!hsize && i >= nfrags) {
			BUG_ON(fskb->len != len);

			pos += len;
			nskb = skb_clone(fskb, GFP_ATOMIC);
			fskb = fskb->next;

			if (unlikely(!nskb))
				goto err;

			hsize = skb_end_pointer(nskb) - nskb->head;
			if (skb_cow_head(nskb, doffset + headroom)) {
				kfree_skb(nskb);
				goto err;
			}

			nskb->truesize += skb_end_pointer(nskb) - nskb->head -
					  hsize;
			skb_release_head_state(nskb);
			__skb_push(nskb, doffset);
		} else {
			nskb = alloc_skb(hsize + doffset + headroom,
					 GFP_ATOMIC);

			if (unlikely(!nskb))
				goto err;

			skb_reserve(nskb, headroom);
			__skb_put(nskb, doffset);
		}

		if (segs)
			tail->next = nskb;
		else
			segs = nskb;
		tail = nskb;

		__copy_skb_header(nskb, skb);
		nskb->mac_len = skb->mac_len;

		skb_reset_mac_header(nskb);
		skb_set_network_header(nskb, skb->mac_len);
		nskb->transport_header = (nskb->network_header +
					  skb_network_header_len(skb));
		skb_copy_from_linear_data(skb, nskb->data, doffset);

		if (fskb != skb_shinfo(skb)->frag_list)
			continue;

		if (!sg) {
			nskb->ip_summed = CHECKSUM_NONE;
			nskb->csum = skb_copy_and_csum_bits(skb, offset,
							    skb_put(nskb, len),
							    len, 0);
			continue;
		}

		frag = skb_shinfo(nskb)->frags;

		skb_copy_from_linear_data_offset(skb, offset,
						 skb_put(nskb, hsize), hsize);

		while (pos < offset + len && i < nfrags) {
			*frag = skb_shinfo(skb)->frags[i];
			get_page(frag->page);
			size = frag->size;

			if (pos < offset) {
				frag->page_offset += offset - pos;
				frag->size -= offset - pos;
			}

			skb_shinfo(nskb)->nr_frags++;

			if (pos + size <= offset + len) {
				i++;
				pos += size;
			} else {
				frag->size -= pos + size - (offset + len);
				goto skip_fraglist;
			}

			frag++;
		}

		if (pos < offset + len) {
			struct sk_buff *fskb2 = fskb;

			BUG_ON(pos + fskb->len != offset + len);

			pos += fskb->len;
			fskb = fskb->next;

			if (fskb2->next) {
				fskb2 = skb_clone(fskb2, GFP_ATOMIC);
				if (!fskb2)
					goto err;
			} else
				skb_get(fskb2);

			BUG_ON(skb_shinfo(nskb)->frag_list);
			skb_shinfo(nskb)->frag_list = fskb2;
		}

skip_fraglist:
		nskb->data_len = len - hsize;
		nskb->len += nskb->data_len;
		nskb->truesize += nskb->data_len;
	} while ((offset += len) < skb->len);

	return segs;

err:
	while ((skb = segs)) {
		segs = skb->next;
		kfree_skb(skb);
	}
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(skb_segment);

int skb_gro_receive(struct sk_buff **head, struct sk_buff *skb)
{
	struct sk_buff *p = *head;
	struct sk_buff *nskb;
	unsigned int headroom;
	unsigned int len = skb_gro_len(skb);

	if (p->len + len >= 65536)
		return -E2BIG;

	if (skb_shinfo(p)->frag_list)
		goto merge;
	else if (skb_headlen(skb) <= skb_gro_offset(skb)) {
		if (skb_shinfo(p)->nr_frags + skb_shinfo(skb)->nr_frags >
		    MAX_SKB_FRAGS)
			return -E2BIG;

		skb_shinfo(skb)->frags[0].page_offset +=
			skb_gro_offset(skb) - skb_headlen(skb);
		skb_shinfo(skb)->frags[0].size -=
			skb_gro_offset(skb) - skb_headlen(skb);

		memcpy(skb_shinfo(p)->frags + skb_shinfo(p)->nr_frags,
		       skb_shinfo(skb)->frags,
		       skb_shinfo(skb)->nr_frags * sizeof(skb_frag_t));

		skb_shinfo(p)->nr_frags += skb_shinfo(skb)->nr_frags;
		skb_shinfo(skb)->nr_frags = 0;

		skb->truesize -= skb->data_len;
		skb->len -= skb->data_len;
		skb->data_len = 0;

		NAPI_GRO_CB(skb)->free = 1;
		goto done;
	}

	headroom = skb_headroom(p);
	nskb = netdev_alloc_skb(p->dev, headroom + skb_gro_offset(p));
	if (unlikely(!nskb))
		return -ENOMEM;

	__copy_skb_header(nskb, p);
	nskb->mac_len = p->mac_len;

	skb_reserve(nskb, headroom);
	__skb_put(nskb, skb_gro_offset(p));

	skb_set_mac_header(nskb, skb_mac_header(p) - p->data);
	skb_set_network_header(nskb, skb_network_offset(p));
	skb_set_transport_header(nskb, skb_transport_offset(p));

	__skb_pull(p, skb_gro_offset(p));
	memcpy(skb_mac_header(nskb), skb_mac_header(p),
	       p->data - skb_mac_header(p));

	*NAPI_GRO_CB(nskb) = *NAPI_GRO_CB(p);
	skb_shinfo(nskb)->frag_list = p;
	skb_shinfo(nskb)->gso_size = skb_shinfo(p)->gso_size;
	skb_header_release(p);
	nskb->prev = p;

	nskb->data_len += p->len;
	nskb->truesize += p->len;
	nskb->len += p->len;

	*head = nskb;
	nskb->next = p->next;
	p->next = NULL;

	p = nskb;

merge:
	if (skb_gro_offset(skb) > skb_headlen(skb)) {
		skb_shinfo(skb)->frags[0].page_offset +=
			skb_gro_offset(skb) - skb_headlen(skb);
		skb_shinfo(skb)->frags[0].size -=
			skb_gro_offset(skb) - skb_headlen(skb);
		skb_gro_reset_offset(skb);
		skb_gro_pull(skb, skb_headlen(skb));
	}

	__skb_pull(skb, skb_gro_offset(skb));

	p->prev->next = skb;
	p->prev = skb;
	skb_header_release(skb);

done:
	NAPI_GRO_CB(p)->count++;
	p->data_len += len;
	p->truesize += len;
	p->len += len;

	NAPI_GRO_CB(skb)->same_flow = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(skb_gro_receive);

void __init skb_init(void)
{
	skbuff_head_cache = kmem_cache_create("skbuff_head_cache",
					      sizeof(struct sk_buff),
					      0,
					      SLAB_HWCACHE_ALIGN|SLAB_PANIC,
					      NULL);
	skbuff_fclone_cache = kmem_cache_create("skbuff_fclone_cache",
						(2*sizeof(struct sk_buff)) +
						sizeof(atomic_t),
						0,
						SLAB_HWCACHE_ALIGN|SLAB_PANIC,
						NULL);
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	skbuff_cb_store_cache = kmem_cache_create("skbuff_cb_store_cache",
						  sizeof(struct skb_cb_table),
						  0,
						  SLAB_HWCACHE_ALIGN|SLAB_PANIC,
						  NULL);
#endif
}

/**
 *	skb_to_sgvec - Fill a scatter-gather list from a socket buffer
 *	@skb: Socket buffer containing the buffers to be mapped
 *	@sg: The scatter-gather list to map into
 *	@offset: The offset into the buffer's contents to start mapping
 *	@len: Length of buffer space to be mapped
 *
 *	Fill the specified scatter-gather list with mappings/pointers into a
 *	region of the buffer space attached to a socket buffer.
 */
static int
__skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg, int offset, int len)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	int elt = 0;

	if (copy > 0) {
		if (copy > len)
			copy = len;
		sg_set_buf(sg, skb->data + offset, copy);
		elt++;
		if ((len -= copy) == 0)
			return elt;
		offset += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end - offset) > 0) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			if (copy > len)
				copy = len;
			sg_set_page(&sg[elt], frag->page, copy,
					frag->page_offset+offset-start);
			elt++;
			if (!(len -= copy))
				return elt;
			offset += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;

		for (; list; list = list->next) {
			int end;

			WARN_ON(start > offset + len);

			end = start + list->len;
			if ((copy = end - offset) > 0) {
				if (copy > len)
					copy = len;
				elt += __skb_to_sgvec(list, sg+elt, offset - start,
						      copy);
				if ((len -= copy) == 0)
					return elt;
				offset += copy;
			}
			start = end;
		}
	}
	BUG_ON(len);
	return elt;
}

int skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg, int offset, int len)
{
	int nsg = __skb_to_sgvec(skb, sg, offset, len);

	sg_mark_end(&sg[nsg - 1]);

	return nsg;
}
EXPORT_SYMBOL_GPL(skb_to_sgvec);

/**
 *	skb_cow_data - Check that a socket buffer's data buffers are writable
 *	@skb: The socket buffer to check.
 *	@tailbits: Amount of trailing space to be added
 *	@trailer: Returned pointer to the skb where the @tailbits space begins
 *
 *	Make sure that the data buffers attached to a socket buffer are
 *	writable. If they are not, private copies are made of the data buffers
 *	and the socket buffer is set to use these instead.
 *
 *	If @tailbits is given, make sure that there is space to write @tailbits
 *	bytes of data beyond current end of socket buffer.  @trailer will be
 *	set to point to the skb in which this space begins.
 *
 *	The number of scatterlist elements required to completely map the
 *	COW'd and extended socket buffer will be returned.
 */
int skb_cow_data(struct sk_buff *skb, int tailbits, struct sk_buff **trailer)
{
	int copyflag;
	int elt;
	struct sk_buff *skb1, **skb_p;

	/* If skb is cloned or its head is paged, reallocate
	 * head pulling out all the pages (pages are considered not writable
	 * at the moment even if they are anonymous).
	 */
	if ((skb_cloned(skb) || skb_shinfo(skb)->nr_frags) &&
	    __pskb_pull_tail(skb, skb_pagelen(skb)-skb_headlen(skb)) == NULL)
		return -ENOMEM;

	/* Easy case. Most of packets will go this way. */
	if (!skb_shinfo(skb)->frag_list) {
		/* A little of trouble, not enough of space for trailer.
		 * This should not happen, when stack is tuned to generate
		 * good frames. OK, on miss we reallocate and reserve even more
		 * space, 128 bytes is fair. */

		if (skb_tailroom(skb) < tailbits &&
		    pskb_expand_head(skb, 0, tailbits-skb_tailroom(skb)+128, GFP_ATOMIC))
			return -ENOMEM;

		/* Voila! */
		*trailer = skb;
		return 1;
	}

	/* Misery. We are in troubles, going to mincer fragments... */

	elt = 1;
	skb_p = &skb_shinfo(skb)->frag_list;
	copyflag = 0;

	while ((skb1 = *skb_p) != NULL) {
		int ntail = 0;

		/* The fragment is partially pulled by someone,
		 * this can happen on input. Copy it and everything
		 * after it. */

		if (skb_shared(skb1))
			copyflag = 1;

		/* If the skb is the last, worry about trailer. */

		if (skb1->next == NULL && tailbits) {
			if (skb_shinfo(skb1)->nr_frags ||
			    skb_shinfo(skb1)->frag_list ||
			    skb_tailroom(skb1) < tailbits)
				ntail = tailbits + 128;
		}

		if (copyflag ||
		    skb_cloned(skb1) ||
		    ntail ||
		    skb_shinfo(skb1)->nr_frags ||
		    skb_shinfo(skb1)->frag_list) {
			struct sk_buff *skb2;

			/* Fuck, we are miserable poor guys... */
			if (ntail == 0)
				skb2 = skb_copy(skb1, GFP_ATOMIC);
			else
				skb2 = skb_copy_expand(skb1,
						       skb_headroom(skb1),
						       ntail,
						       GFP_ATOMIC);
			if (unlikely(skb2 == NULL))
				return -ENOMEM;

			if (skb1->sk)
				skb_set_owner_w(skb2, skb1->sk);

			/* Looking around. Are we still alive?
			 * OK, link new skb, drop old one */

			skb2->next = skb1->next;
			*skb_p = skb2;
			kfree_skb(skb1);
			skb1 = skb2;
		}
		elt++;
		*trailer = skb1;
		skb_p = &skb1->next;
	}

	return elt;
}
EXPORT_SYMBOL_GPL(skb_cow_data);

void skb_tstamp_tx(struct sk_buff *orig_skb,
		struct skb_shared_hwtstamps *hwtstamps)
{
	struct sock *sk = orig_skb->sk;
	struct sock_exterr_skb *serr;
	struct sk_buff *skb;
	int err;

	if (!sk)
		return;

	skb = skb_clone(orig_skb, GFP_ATOMIC);
	if (!skb)
		return;

	if (hwtstamps) {
		*skb_hwtstamps(skb) =
			*hwtstamps;
	} else {
		/*
		 * no hardware time stamps available,
		 * so keep the skb_shared_tx and only
		 * store software time stamp
		 */
		skb->tstamp = ktime_get_real();
	}

	serr = SKB_EXT_ERR(skb);
	memset(serr, 0, sizeof(*serr));
	serr->ee.ee_errno = ENOMSG;
	serr->ee.ee_origin = SO_EE_ORIGIN_TIMESTAMPING;
	err = sock_queue_err_skb(sk, skb);
	if (err)
		kfree_skb(skb);
}
EXPORT_SYMBOL_GPL(skb_tstamp_tx);


/**
 * skb_partial_csum_set - set up and verify partial csum values for packet
 * @skb: the skb to set
 * @start: the number of bytes after skb->data to start checksumming.
 * @off: the offset from start to place the checksum.
 *
 * For untrusted partially-checksummed packets, we need to make sure the values
 * for skb->csum_start and skb->csum_offset are valid so we don't oops.
 *
 * This function checks and sets those values and skb->ip_summed: if this
 * returns false you should drop the packet.
 */
bool skb_partial_csum_set(struct sk_buff *skb, u16 start, u16 off)
{
	if (unlikely(start > skb->len - 2) ||
	    unlikely((int)start + off > skb->len - 2)) {
		if (net_ratelimit())
			printk(KERN_WARNING
			       "bad partial csum: csum=%u/%u len=%u\n",
			       start, off, skb->len);
		return false;
	}
	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->csum_start = skb_headroom(skb) + start;
	skb->csum_offset = off;
	return true;
}
EXPORT_SYMBOL_GPL(skb_partial_csum_set);

void __skb_warn_lro_forwarding(const struct sk_buff *skb)
{
	if (net_ratelimit())
		pr_warning("%s: received packets cannot be forwarded"
			   " while LRO is enabled\n", skb->dev->name);
}
EXPORT_SYMBOL(__skb_warn_lro_forwarding);
