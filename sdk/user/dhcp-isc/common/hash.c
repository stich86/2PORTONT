/* hash.c

   Routines for manipulating hash tables... */

/*
 * Copyright (c) 1995, 1996, 1997, 1998 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#ifndef lint
#ifndef EMBED
static char copyright[] =
"$Id: hash.c,v 1.1.1.1 2003/08/18 05:41:48 kaohj Exp $ Copyright (c) 1995, 1996, 1997, 1998 The Internet Software Consortium.  All rights reserved.\n";
#endif /*!EMBED*/
#endif /* not lint */

#include "dhcpd.h"

static INLINE int do_hash PROTO ((unsigned char *, int, int));

struct hash_table *new_hash ()
{
	struct hash_table *rv = new_hash_table (DEFAULT_HASH_SIZE, "new_hash");
	if (!rv)
		return rv;
	memset (&rv -> buckets [0], 0,
		DEFAULT_HASH_SIZE * sizeof (struct hash_bucket *));
	return rv;
}

static INLINE int do_hash (name, len, size)
	unsigned char *name;
	int len;
	int size;
{
	register int accum = 0;
	register unsigned char *s = name;
	int i = len;
	while (i--) {
		/* Add the character in... */
		accum += *s++;
		/* Add carry back in... */
		while (accum > 255) {
			accum = (accum & 255) + (accum >> 8);
		}
	}
	return accum % size;
}

void add_hash (table, name, len, pointer)
	struct hash_table *table;
	int len;
	unsigned char *name;
	unsigned char *pointer;
{
	int hashno;
	struct hash_bucket *bp;

	if (!table)
		return;
	if (!len)
		len = strlen ((char *)name);

	hashno = do_hash (name, len, table -> hash_count);
	bp = new_hash_bucket ("add_hash");

	if (!bp) {
		warn ("Can't add %s to hash table.", name);
		return;
	}
	bp -> name = name;
	bp -> value = pointer;
	bp -> next = table -> buckets [hashno];
	bp -> len = len;
	table -> buckets [hashno] = bp;
}

void delete_hash_entry (table, name, len)
	struct hash_table *table;
	int len;
	unsigned char *name;
{
	int hashno;
	struct hash_bucket *bp, *pbp = (struct hash_bucket *)0;

	if (!table)
		return;
	if (!len)
		len = strlen ((char *)name);

	hashno = do_hash (name, len, table -> hash_count);

	/* Go through the list looking for an entry that matches;
	   if we find it, delete it. */
	for (bp = table -> buckets [hashno]; bp; bp = bp -> next) {
		if ((!bp -> len &&
		     !strcmp ((char *)bp -> name, (char *)name)) ||
		    (bp -> len == len &&
		     !memcmp (bp -> name, name, len))) {
			if (pbp) {
				pbp -> next = bp -> next;
			} else {
				table -> buckets [hashno] = bp -> next;
			}
			free_hash_bucket (bp, "delete_hash_entry");
			break;
		}
		pbp = bp;	/* jwg, 9/6/96 - nice catch! */
	}
}

unsigned char *hash_lookup (table, name, len)
	struct hash_table *table;
	unsigned char *name;
	int len;
{
	int hashno;
	struct hash_bucket *bp;

	if (!table)
		return (unsigned char *)0;

	if (!len)
		len = strlen ((char *)name);

	hashno = do_hash (name, len, table -> hash_count);

	for (bp = table -> buckets [hashno]; bp; bp = bp -> next) {
		if (len == bp -> len && !memcmp (bp -> name, name, len))
			return bp -> value;
	}
	return (unsigned char *)0;
}
