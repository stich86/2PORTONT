/* $Id: conf-riscos.h,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $ */

/*
 * conf-riscos4.h (for SYSTYPE_BSD43)
 *
 *	Tripwire configuration file
 *
 * Gene Kim
 * Purdue University
 *
 * Ported to RiscOS 
 *	Harlan Stenn <harlan@mumps.pfcs.com>
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

#undef SYSV

/* 
 * does your system have a <malloc.h> like System V? 
 */

#undef MALLOCH 	

/* 
 * does your system have a <stdlib.h> like POSIX says you should? 
 */

#undef STDLIBH

/*
 * does your system use readdir(3) that returns (struct dirent *)?
 */

#undef DIRENT

/*
 * is #include <string.h> ok?  (as opposed to <strings.h>)
 */

#define STRINGH
 
/* 
 * does your system have gethostname(2) (instead of uname(2))?
 */

#define GETHOSTNAME
