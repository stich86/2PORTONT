/*
 * Direct MTD block device access
 *
 * (C) 2000-2003 Nicolas Pitre <nico@cam.org>
 * (C) 1999-2003 David Woodhouse <dwmw2@infradead.org>
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mutex.h>

#include <linux/config.h>

#ifndef CONFIG_RTL8672
#ifdef CONFIG_RTL_819X
// david ---------------
//#define CONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 3


#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
int flash_write_flag=0;		// 1: hw setting 2: default setting, 4: current setting, 8: system image
#else
int flash_hw_start=0x6000;	// hw setting start address
int flash_hw_len=0x2000;	// hw setting length
int flash_ds_start=0x8000;	// default & current setting start address
int flash_ds_len=0x8000;	// default & current setting length

int flash_write_flag=0;		// 1: hw setting 2: default setting, 4: current setting, 8: system image
#endif
#endif /*#ifdef CONFIG_RTL_819X */
#endif //CONFIG_RTL8672

static struct mtdblk_dev {
	struct mtd_info *mtd;
	int count;
	struct mutex cache_mutex;
	unsigned char *cache_data;
	unsigned long cache_offset;
	unsigned int cache_size;
	enum { STATE_EMPTY, STATE_CLEAN, STATE_DIRTY } cache_state;
} *mtdblks[MAX_MTD_DEVICES];

/*
 * Cache stuff...
 *
 * Since typical flash erasable sectors are much larger than what Linux's
 * buffer cache can handle, we must implement read-modify-write on flash
 * sectors for each block write requests.  To avoid over-erasing flash sectors
 * and to speed things up, we locally cache a whole flash sector while it is
 * being written to until a different sector is required.
 */

static void erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *)done->priv;
	wake_up(wait_q);
}

static int erase_write (struct mtd_info *mtd, unsigned long pos,
			int len, const char *buf)
{
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	size_t retlen;
	int ret;

	/*
	 * First, let's erase the flash block.
	 */

	init_waitqueue_head(&wait_q);
	erase.mtd = mtd;
	erase.callback = erase_callback;
	erase.addr = pos;
	erase.len = len;
	erase.priv = (u_long)&wait_q;

	set_current_state(TASK_INTERRUPTIBLE);
	add_wait_queue(&wait_q, &wait);

	ret = mtd->erase(mtd, &erase);
	if (ret) {
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&wait_q, &wait);
		printk (KERN_WARNING "mtdblock: erase of region [0x%lx, 0x%x] "
				     "on \"%s\" failed\n",
			pos, len, mtd->name);
		return ret;
	}

	schedule();  /* Wait for erase to finish. */
	remove_wait_queue(&wait_q, &wait);

	/*
	 * Next, writhe data to flash.
	 */

	ret = mtd->write(mtd, pos, len, &retlen, buf);
	if (ret)
		return ret;
	if (retlen != len)
		return -EIO;
	return 0;
}


static int write_cached_data (struct mtdblk_dev *mtdblk)
{
	struct mtd_info *mtd = mtdblk->mtd;
	int ret;

	if (mtdblk->cache_state != STATE_DIRTY)
		return 0;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: writing cached data for \"%s\" "
			"at 0x%lx, size 0x%x\n", mtd->name,
			mtdblk->cache_offset, mtdblk->cache_size);

	ret = erase_write (mtd, mtdblk->cache_offset,
			   mtdblk->cache_size, mtdblk->cache_data);
	if (ret)
		return ret;

	/*
	 * Here we could argubly set the cache state to STATE_CLEAN.
	 * However this could lead to inconsistency since we will not
	 * be notified if this content is altered on the flash by other
	 * means.  Let's declare it empty and leave buffering tasks to
	 * the buffer cache instead.
	 */
	mtdblk->cache_state = STATE_EMPTY;
	return 0;
}


static int do_cached_write (struct mtdblk_dev *mtdblk, unsigned long pos,
			    int len, const char *buf)
{
	struct mtd_info *mtd = mtdblk->mtd;
	unsigned int sect_size = mtdblk->cache_size;
	size_t retlen;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: write on \"%s\" at 0x%lx, size 0x%x\n",
		mtd->name, pos, len);

#ifndef CONFIG_RTL8672	
#ifdef CONFIG_RTL_819X
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	/*since len is normal 0x200 less than every section*/
	if(flash_write_flag != 0x8000) 
	{
		flash_write_flag = 0;
		if ( pos >= CONFIG_RTL_HW_SETTING_OFFSET && pos < CONFIG_RTL_DEFAULT_SETTING_OFFSET ) {
			flash_write_flag |= 1;
			if ((pos+len) > CONFIG_RTL_DEFAULT_SETTING_OFFSET ) {
				flash_write_flag |= 2;
				if ((pos+len) > CONFIG_RTL_CURRENT_SETTING_OFFSET )
					flash_write_flag |= 4;
			}
		}
		if ( pos >= CONFIG_RTL_DEFAULT_SETTING_OFFSET && pos < CONFIG_RTL_CURRENT_SETTING_OFFSET ) {
			flash_write_flag |= 2;
			if ((pos+len) > CONFIG_RTL_CURRENT_SETTING_OFFSET ) {
				flash_write_flag |= 4;
			}
		}
		else if ( pos >= CONFIG_RTL_CURRENT_SETTING_OFFSET && pos < CONFIG_RTL_WEB_PAGES_OFFSET ){
			flash_write_flag |= 4;
		}
	}
#else
// david --------------
	if ( flash_write_flag != 0x8000) 
	{
		flash_write_flag = 0;
		if (pos >= flash_hw_start && pos < (flash_hw_start+flash_hw_len) ) {
			flash_write_flag |= 1;
			if ((len - flash_hw_len) > 0) {
				flash_write_flag |= 2;
				if ((len - flash_ds_len -flash_hw_len) > 0)
					flash_write_flag |= 4;
			}
		}
		if (pos >= flash_ds_start && pos < (flash_ds_start+flash_ds_len) ) {
			flash_write_flag |= 2;
			if ((len - flash_ds_len) > 0) {
				flash_write_flag |= 4;
			}
		}
		else if ( pos >= (flash_ds_start+flash_ds_len) ){
			flash_write_flag |= 4;
		}
	}
//---------------------
#endif //CONFIG_RTL_FLASH_MAPPING_ENABLE
#endif
#endif //CONFIG_RTL8672
	if (!sect_size)
		return mtd->write(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos/sect_size)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if( size > len )
			size = len;

		if (size == sect_size) {
			/*
			 * We are covering a whole sector.  Thus there is no
			 * need to bother with the cache while it may still be
			 * useful for other partial writes.
			 */
			ret = erase_write (mtd, pos, size, buf);
			if (ret)
				return ret;
		} else {
			/* Partial sector: need to use the cache */

			if (mtdblk->cache_state == STATE_DIRTY &&
			    mtdblk->cache_offset != sect_start) {
				ret = write_cached_data(mtdblk);
				if (ret)
					return ret;
			}

			if (mtdblk->cache_state == STATE_EMPTY ||
			    mtdblk->cache_offset != sect_start) {
				/* fill the cache with the current sector */
				mtdblk->cache_state = STATE_EMPTY;
				ret = mtd->read(mtd, sect_start, sect_size,
						&retlen, mtdblk->cache_data);
				if (ret)
					return ret;
				if (retlen != sect_size)
					return -EIO;

				mtdblk->cache_offset = sect_start;
				mtdblk->cache_size = sect_size;
				mtdblk->cache_state = STATE_CLEAN;
			}

			/* write data to our local cache */
			memcpy (mtdblk->cache_data + offset, buf, size);
			mtdblk->cache_state = STATE_DIRTY;
		}

		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}


static int do_cached_read (struct mtdblk_dev *mtdblk, unsigned long pos,
			   int len, char *buf)
{
	struct mtd_info *mtd = mtdblk->mtd;
	unsigned int sect_size = mtdblk->cache_size;
	size_t retlen;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: read on \"%s\" at 0x%lx, size 0x%x\n",
			mtd->name, pos, len);

	if (!sect_size)
		return mtd->read(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos/sect_size)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if (size > len)
			size = len;

		/*
		 * Check if the requested data is already cached
		 * Read the requested amount of data from our internal cache if it
		 * contains what we want, otherwise we read the data directly
		 * from flash.
		 */
		if (mtdblk->cache_state != STATE_EMPTY &&
		    mtdblk->cache_offset == sect_start) {
			memcpy (buf, mtdblk->cache_data + offset, size);
		} else {
			ret = mtd->read(mtd, pos, size, &retlen, buf);
			if (ret)
				return ret;
			if (retlen != size)
				return -EIO;
		}

		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}

static int mtdblock_readsect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];
	return do_cached_read(mtdblk, block<<9, 512, buf);
}

static int mtdblock_writesect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];
	if (unlikely(!mtdblk->cache_data && mtdblk->cache_size)) {
		mtdblk->cache_data = vmalloc(mtdblk->mtd->erasesize);
		if (!mtdblk->cache_data)
			return -EINTR;
		/* -EINTR is not really correct, but it is the best match
		 * documented in man 2 write for all cases.  We could also
		 * return -EAGAIN sometimes, but why bother?
		 */
	}
	return do_cached_write(mtdblk, block<<9, 512, buf);
}

static int mtdblock_open(struct mtd_blktrans_dev *mbd)
{
	struct mtdblk_dev *mtdblk;
	struct mtd_info *mtd = mbd->mtd;
	int dev = mbd->devnum;

	DEBUG(MTD_DEBUG_LEVEL1,"mtdblock_open\n");

	if (mtdblks[dev]) {
		mtdblks[dev]->count++;
		return 0;
	}

	/* OK, it's not open. Create cache info for it */
	mtdblk = kzalloc(sizeof(struct mtdblk_dev), GFP_KERNEL);
	if (!mtdblk)
		return -ENOMEM;

	mtdblk->count = 1;
	mtdblk->mtd = mtd;

	mutex_init(&mtdblk->cache_mutex);
	mtdblk->cache_state = STATE_EMPTY;
	if ( !(mtdblk->mtd->flags & MTD_NO_ERASE) && mtdblk->mtd->erasesize) {
		mtdblk->cache_size = mtdblk->mtd->erasesize;
		mtdblk->cache_data = NULL;
	}

	mtdblks[dev] = mtdblk;

	DEBUG(MTD_DEBUG_LEVEL1, "ok\n");

	return 0;
}

static int mtdblock_release(struct mtd_blktrans_dev *mbd)
{
	int dev = mbd->devnum;
	struct mtdblk_dev *mtdblk = mtdblks[dev];

   	DEBUG(MTD_DEBUG_LEVEL1, "mtdblock_release\n");

	mutex_lock(&mtdblk->cache_mutex);
	write_cached_data(mtdblk);
	mutex_unlock(&mtdblk->cache_mutex);

	if (!--mtdblk->count) {
		/* It was the last usage. Free the device */
		mtdblks[dev] = NULL;
		if (mtdblk->mtd->sync)
			mtdblk->mtd->sync(mtdblk->mtd);
		vfree(mtdblk->cache_data);
		kfree(mtdblk);
	}
	DEBUG(MTD_DEBUG_LEVEL1, "ok\n");

	return 0;
}

static int mtdblock_flush(struct mtd_blktrans_dev *dev)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];

	mutex_lock(&mtdblk->cache_mutex);
	write_cached_data(mtdblk);
	mutex_unlock(&mtdblk->cache_mutex);

	if (mtdblk->mtd->sync)
		mtdblk->mtd->sync(mtdblk->mtd);
	return 0;
}

static void mtdblock_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	struct mtd_blktrans_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev)
		return;

	dev->mtd = mtd;
	dev->devnum = mtd->index;
	dev->size = mtd->size >> 9;
	dev->tr = tr;

	if (!(mtd->flags & MTD_WRITEABLE))
		dev->readonly = 1;

	add_mtd_blktrans_dev(dev);
}

static void mtdblock_remove_dev(struct mtd_blktrans_dev *dev)
{
	del_mtd_blktrans_dev(dev);
	kfree(dev);
}

static struct mtd_blktrans_ops mtdblock_tr = {
	.name		= "mtdblock",
	.major		= 31,
	.part_bits	= 0,
	.blksize 	= 512,
	.open		= mtdblock_open,
	.flush		= mtdblock_flush,
	.release	= mtdblock_release,
	.readsect	= mtdblock_readsect,
	.writesect	= mtdblock_writesect,
	.add_mtd	= mtdblock_add_mtd,
	.remove_dev	= mtdblock_remove_dev,
	.owner		= THIS_MODULE,
};

static int __init init_mtdblock(void)
{
	return register_mtd_blktrans(&mtdblock_tr);
}

static void __exit cleanup_mtdblock(void)
{
	deregister_mtd_blktrans(&mtdblock_tr);
}

module_init(init_mtdblock);
module_exit(cleanup_mtdblock);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Pitre <nico@cam.org> et al.");
MODULE_DESCRIPTION("Caching read/erase/writeback block device emulation access to MTD devices");
