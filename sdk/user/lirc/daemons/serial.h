/*      $Id: serial.h,v 1.1.1.1 2003/08/18 05:41:32 kaohj Exp $      */

/****************************************************************************
 ** serial.c ****************************************************************
 ****************************************************************************
 *
 * common routines for hardware that uses the standard serial port driver
 * 
 * Copyright (C) 1999 Christoph Bartelmus <lirc@bartelmus.de>
 *
 */

#ifndef _SERIAL_H
#define _SERIAL_H

int tty_reset(int fd);
int tty_setrtscts(int fd,int enable);
int tty_setbaud(int fd,int baud);
int tty_create_lock(char *name);
int tty_delete_lock(void);
int tty_set(int fd,int rts,int dtr);
int tty_clear(int fd,int rts,int dtr);
int tty_write(int fd,char byte);
int tty_read(int fd,char *byte);
int tty_write_echo(int fd,char byte);

#endif
