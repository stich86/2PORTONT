/* -*- c++ -*- */
#ifndef _SNMP_MSG_H_
#define _SNMP_MSG_H_

/**********************************************************************
 *
 *           Copyright 1997 by Carnegie Mellon University
 * 
 *                       All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of CMU not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * 
 * CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 * Author: Ryan Troll <ryan+@andrew.cmu.edu>
 * 
 * $Id: snmp_msg.h,v 1.1.1.1 2003/08/18 05:40:24 kaohj Exp $
 * 
 **********************************************************************/

#define SNMP_VERSION_1	    0	/* RFC 1157 */
#define SNMP_VERSION_2	    1	/* RFC 1901 */

#ifdef __cplusplus
extern "C" {
#endif

    u_char *snmp_msg_Encode(u_char *, int *, u_char *,
	int, int, struct snmp_pdu *);
    u_char *snmp_msg_Decode(u_char *, int *, u_char *,
	int *, int *, struct snmp_pdu *);

#ifdef __cplusplus
}

#endif
#endif				/* _SNMP_MSG_H_ */
