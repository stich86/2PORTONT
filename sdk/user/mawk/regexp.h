
/********************************************
regexp.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*$Log: regexp.h,v $
/*Revision 1.1.1.1  2003/08/18 05:41:27  kaohj
/*initial import into CVS
/*
 * Revision 1.1.1.1  1993/07/03  18:58:19  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:30  brennan
 * 1.1 pre-release
 *
*/

#include <stdio.h>

PTR   PROTO( REcompile , (char *) ) ;
int   PROTO( REtest, (char *, PTR) ) ;
char *PROTO( REmatch, (char *, PTR, unsigned *) ) ;
void  PROTO( REmprint, (PTR , FILE*) ) ;

extern  int  REerrno ;
extern  char *REerrlist[] ;


