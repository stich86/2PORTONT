/* $Id: conf-epix.h,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $ */

/*
 * conf-umips.h
 *
 *	Tripwire configuration file
 *
 */

/***
 *** Operating System specifics
 ***	
 ***	If the answer to a question in the comment is "Yes", then
 ***	change the corresponding "#undef" to a "#define"
 ***/

/*
 * is your OS a System V derivitive?  if so, what version?
 *			(e.g., define SYSV 4)
 */

#define SYSV 4

/* 
 * does your system have a <malloc.h> like System V? 
 */

#define MALLOCH 	

/* 
 * does your system have a <stdlib.h> like POSIX says you should? 
 */

#undef STDLIBH

/*
 * does your system use readdir(3) that returns (struct dirent *)?
 */

#define DIRENT

/*
 * is #include <string.h> ok?  (as opposed to <strings.h>)
 */

#define STRINGH

/* 
 * does your system have gethostname(2) (instead of uname(2))?
 */

#define GETHOSTNAME

