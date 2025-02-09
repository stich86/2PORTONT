/*      $Id: hw_creative.h,v 1.1.1.1 2003/08/18 05:41:30 kaohj Exp $      */

/****************************************************************************
 ** hw_creative.h **********************************************************
 ****************************************************************************
 *
 * routines for Creative receiver 
 * 
 * Copyright (C) 1999 Christoph Bartelmus <lirc@bartelmus.de>
 *	modified for creative receiver by Isaac Lauer <inl101@alumni.psu.edu>
 */

#ifndef _HW_CREATIVE_H
#define _HW_CREATIVE_H

#include <linux/lirc.h>

int creative_decode(struct ir_remote *remote,
		  ir_code *prep,ir_code *codep,ir_code *postp,
		  int *repeat_flagp,lirc_t *remaining_gapp);
int creative_init(void);
int creative_deinit(void);
char *creative_rec(struct ir_remote *remotes);

#endif
