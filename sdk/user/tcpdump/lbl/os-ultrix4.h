/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @(#) $Header: /usr/local/dslrepos/uClinux-dist/user/tcpdump/lbl/os-ultrix4.h,v 1.1.1.1 2003/08/18 05:40:17 kaohj Exp $ (LBL)
 */

/* Prototypes missing in Ultrix 4 */
int	bcmp(const char *, const char *, u_int);
void	bcopy(const void *, void *, u_int);
void	bzero(void *, u_int);
void	endservent(void);
int	getopt(int, char * const *, const char *);
#ifdef __STDC__
struct timeval;
struct timezone;
#endif
int	gettimeofday(struct timeval *, struct timezone *);
int	ioctl(int, int, caddr_t);
int	pfopen(char *, int);
int	setlinebuf(FILE *);
int	socket(int, int, int);
int	strcasecmp(const char *, const char *);
