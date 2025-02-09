/* Copyright (C) 1994, 1995 Aladdin Enterprises.  All rights reserved.
  
  This file is part of GNU Ghostscript.
  
  GNU Ghostscript is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility to
  anyone for the consequences of using it or for whether it serves any
  particular purpose or works at all, unless he says so in writing.  Refer to
  the GNU General Public License for full details.
  
  Everyone is granted permission to copy, modify and redistribute GNU
  Ghostscript, but only under the conditions described in the GNU General
  Public License.  A copy of this license is supposed to have been given to
  you along with GNU Ghostscript so you can know your rights and
  responsibilities.  It should be in a file named COPYING.  Among other
  things, the copyright notice and this notice must be preserved on all
  copies.
  
  Aladdin Enterprises is not affiliated with the Free Software Foundation or
  the GNU Project.  GNU Ghostscript, as distributed by Aladdin Enterprises,
  does not depend on any other GNU software.
*/

/* idebug.h */
/* Prototypes for debugging procedures in idebug.c */

/* Print individual values. */
void debug_print_name(P1(const ref *));
void debug_print_ref(P1(const ref *));

/* Dump regions of memory. */
void debug_dump_one_ref(P1(const ref *));
void debug_dump_refs(P3(const ref *from, uint size, const char *msg));
void debug_dump_array(P1(const ref *array));

/* Dump a stack.  Using this requires istack.h. */
#ifndef ref_stack_DEFINED
typedef struct ref_stack_s ref_stack;	/* also defined in istack.h */
#  define ref_stack_DEFINED
#endif
void debug_dump_stack(P2(const ref_stack *pstack, const char *msg));
