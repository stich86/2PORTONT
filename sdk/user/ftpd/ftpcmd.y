/*
 * Copyright (c) 1985, 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ftpcmd.y	8.3 (Berkeley) 4/6/94
 */

/*
 * Grammar for FTP commands.
 * See RFC 959.
 */

%{

#ifndef lint
static char sccsid[] = "@(#)ftpcmd.y	8.3 (Berkeley) 4/6/94";
#endif /* not lint */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/ftp.h>

#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <unistd.h>
#include <limits.h>
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
/* Include glob.h last, because it may define "const" which breaks
   system headers on some platforms. */
#include <glob.h>
#include <netdb.h>

#include "extern.h"

#if ! defined (NBBY) && defined (CHAR_BIT)
#define NBBY CHAR_BIT
#endif

off_t restart_point;

#ifdef FULL_FTPD
static char cbuf[512];           /* Command Buffer.  */
#else
static char cbuf[128];           /* Command Buffer.  */
#endif
static char *fromname;
static int cmd_type;
static int cmd_form;
static int cmd_bytesz;

extern struct tab sitetab[];
static char *copy         __P ((char *));
static void help          __P ((struct tab *, char *));
static struct tab *lookup __P ((struct tab *, const char *));
static void sizecmd       __P ((char *));
static int yylex          __P ((void));
static void yyerror       __P ((const char *s));
static int port_check(const char *);
static int port_check_v6(const char *);
static void v4map_data_dest();

%}

%union {
	off_t	o;
	char   *s;
}

%token
	A	B	C	E	F	I
	L	N	P	R	S	T
	ALL

	SP	CRLF	COMMA

	USER	PASS	ACCT	REIN	QUIT	PORT
	PASV	TYPE	STRU	MODE	RETR	STOR
	APPE	MLFL	MAIL	MSND	MSOM	MSAM
	MRSQ	MRCP	ALLO	REST	RNFR	RNTO
	ABOR	DELE	CWD	LIST	NLST	SITE
	STAT	HELP	NOOP	MKD	RMD	PWD
	CDUP	STOU	SMNT	SYST	SIZE	MDTM
	LPRT	LPSV	EPRT	EPSV

	FEAT

	UMASK	IDLE	CHMOD

	LEXERR

%token	<s> STRING
%token	<o> NUMBER

%type	<o> check_login octal_number byte_size
%type	<o> struct_code mode_code type_code form_code
%type	<s> pathstring pathname password username
%type	<s> ALL

%start	cmd_list

%%

cmd_list
	: /* empty */
	| cmd_list cmd
		{
			if (fromname != NULL)
				free (fromname);
			fromname = (char *) 0;
			restart_point = (off_t) 0;
		}
	| cmd_list rcmd
	;

cmd
	: USER SP username CRLF
		{
			user($3);
			free($3);
		}
	| PASS SP password CRLF
		{
			pass($3);
			free($3);
		}
	| PORT check_login SP host_port CRLF
		{
			if (epsvall) {
				reply(501, "no PORT allowed after EPSV ALL");
				goto port_done;
			}
			if (!$2)
				goto port_done;
			if (port_check("PORT") == 1)
				goto port_done;
#ifdef IPV6
			if ((his_addr.su_family != AF_INET6 ||
			     !IN6_IS_ADDR_V4MAPPED(&his_addr.su_sin6.sin6_addr))) {
				/* shoud never happen */
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto port_done;
			}
			port_check_v6("pcmd");
#endif
		port_done:
			;
		}
	| LPRT check_login SP host_long_port CRLF
		{
			if (epsvall) {
				reply(501, "no LPRT allowed after EPSV ALL");
				goto lprt_done;
			}
			if (!$2)
				goto lprt_done;
			if (port_check("LPRT") == 1)
				goto lprt_done;
#ifdef IPV6
			if (his_addr.su_family != AF_INET6) {
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto lprt_done;
			}
			if (port_check_v6("LPRT") == 1)
				goto lprt_done;
#endif
		lprt_done:
			;
		}
	| EPRT check_login SP STRING CRLF
		{
			char delim;
			char *tmp = NULL;
			char *p, *q;
			char *result[3];
			struct addrinfo hints;
			struct addrinfo *res;
			int i;

			if (epsvall) {
				reply(501, "no EPRT allowed after EPSV ALL");
				goto eprt_done;
			}
			if (!$2)
				goto eprt_done;

			memset(&data_dest, 0, sizeof(data_dest));
			tmp = strdup($4);
			if (debug)
				syslog(LOG_DEBUG, "%s", tmp);
			if (!tmp) {
				fatal("not enough core");
				/*NOTREACHED*/
			}
			p = tmp;
			delim = p[0];
			p++;
			memset(result, 0, sizeof(result));
			for (i = 0; i < 3; i++) {
				q = strchr(p, delim);
				if (!q || *q != delim) {
		parsefail:
					reply(500,
						"Invalid argument, rejected.");
					if (tmp)
						free(tmp);
					usedefault = 1;
					goto eprt_done;
				}
				*q++ = '\0';
				result[i] = p;
				if (debug)
					syslog(LOG_DEBUG, "%d: %s", i, p);
				p = q;
			}

			/* some more sanity check */
			p = result[0];
			while (*p) {
				if (!isdigit(*p))
					goto parsefail;
				p++;
			}
			p = result[2];
			while (*p) {
				if (!isdigit(*p))
					goto parsefail;
				p++;
			}

			/* grab address */
			memset(&hints, 0, sizeof(hints));
			if (atoi(result[0]) == 1)
				hints.ai_family = AF_INET;
#ifdef IPV6
			else if (atoi(result[0]) == 2)
				hints.ai_family = AF_INET6;
#endif
			else
				hints.ai_family = AF_UNSPEC;	/*XXX*/
			hints.ai_socktype = SOCK_STREAM;
			i = getaddrinfo(result[1], result[2], &hints, &res);
			if (i)
				goto parsefail;
			memcpy(&data_dest, res->ai_addr, res->ai_addrlen);
#ifdef IPV6
			if (his_addr.su_family == AF_INET6
			    && data_dest.su_family == AF_INET6) {
				/* XXX more sanity checks! */
				data_dest.su_sin6.sin6_scope_id =
					his_addr.su_sin6.sin6_scope_id;
			}
#endif
			free(tmp);
			tmp = NULL;

			if (port_check("EPRT") == 1)
				goto eprt_done;
#ifdef IPV6
			if (his_addr.su_family != AF_INET6) {
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto eprt_done;
			}
			if (port_check_v6("EPRT") == 1)
				goto eprt_done;
#endif
		eprt_done:;
		}
	| PASV check_login CRLF
		{
			if (epsvall)
				reply(501, "no PASV allowed after EPSV ALL");
			else if ($2)
				passive();
		}
	| LPSV check_login CRLF
		{
			if (epsvall)
				reply(501, "no LPSV allowed after EPSV ALL");
			else if ($2)
				long_passive("LPSV", AF_UNSPEC);
		}
	| EPSV check_login SP NUMBER CRLF
		{
			if ($2) {
				int af;
				switch ($4) {
				case 1:
					af = AF_INET;
					break;
#ifdef IPV6
				case 2:
					af = AF_INET6;
					break;
#endif
				default:
					af = -1;	/*junk value*/
					break;
				}
				long_passive("EPSV", af);
			}
		}
	| EPSV check_login SP ALL CRLF
		{
			if ($2) {
				reply(200,
				      "EPSV ALL command successful.");
				epsvall++;
			}
		}
	| EPSV check_login CRLF
		{
			if ($2)
				long_passive("EPSV", AF_UNSPEC);
		}
	| FEAT CRLF
		{
			feat();
		}
	| TYPE SP type_code CRLF
		{
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
#if defined (NBBY) && NBBY == 8
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
#else /* NBBY == 8 */
				UNIMPLEMENTED for NBBY != 8
#endif /* NBBY == 8 */
			}
		}
	| STRU SP struct_code CRLF
		{
			switch ($3) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(504, "Unimplemented STRU type.");
			}
		}
	| MODE SP mode_code CRLF
		{
			switch ($3) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		}
	| ALLO SP NUMBER CRLF
		{
			reply(202, "ALLO command ignored.");
		}
	| ALLO SP NUMBER SP R SP NUMBER CRLF
		{
			reply(202, "ALLO command ignored.");
		}
	| RETR check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				retrieve((char *) 0, $4);
			if ($4 != NULL)
				free($4);
		}
	| STOR check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				store($4, "w", 0);
			if ($4 != NULL)
				free($4);
		}
	| APPE check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				store($4, "a", 0);
			if ($4 != NULL)
				free($4);
		}
	| NLST check_login CRLF
		{
			if ($2)
				send_file_list(".");
		}
	| NLST check_login SP STRING CRLF
		{
			if ($2 && $4 != NULL)
				send_file_list($4);
			if ($4 != NULL)
				free($4);
		}
	| LIST check_login CRLF
		{
			if ($2)
				retrieve("/bin/ls -lgA", "");
		}
	| LIST check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				retrieve("/bin/ls -lgA %s", $4);
			if ($4 != NULL)
				free($4);
		}
	| STAT check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				statfilecmd($4);
			if ($4 != NULL)
				free($4);
		}
	| STAT CRLF
		{
			statcmd();
		}
	| DELE check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				delete($4);
			if ($4 != NULL)
				free($4);
		}
	| RNTO check_login SP pathname CRLF
		{
		    if ($2) {
			if (fromname) {
				renamecmd(fromname, $4);
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, "Bad sequence of commands.");
			}
		    }
		    free ($4);
		}
	| ABOR CRLF
		{
			reply(225, "ABOR command successful.");
		}
	| CWD check_login CRLF
		{
			if ($2)
				cwd(cred.homedir);
		}
	| CWD check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				cwd($4);
			if ($4 != NULL)
				free($4);
		}
	| HELP CRLF
		{
			help(cmdtab, (char *) 0);
		}
	| HELP SP STRING CRLF
		{
			char *cp = $3;

			if (strncasecmp(cp, "SITE", 4) == 0) {
				cp = $3 + 4;
				if (*cp == ' ')
					cp++;
				if (*cp)
					help(sitetab, cp);
				else
					help(sitetab, (char *) 0);
			} else
				help(cmdtab, $3);
			if ($3 != NULL)
			    free ($3);
		}
	| NOOP CRLF
		{
			reply(200, "NOOP command successful.");
		}
	| MKD check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				makedir($4);
			if ($4 != NULL)
				free($4);
		}
	| RMD check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				removedir($4);
			if ($4 != NULL)
				free($4);
		}
	| PWD check_login CRLF
		{
			if ($2)
				pwd();
		}
	| CDUP check_login CRLF
		{
			if ($2)
				cwd("..");
		}
	| SITE SP HELP CRLF
		{
			help(sitetab, (char *) 0);
		}
	| SITE SP HELP SP STRING CRLF
		{
			help(sitetab, $5);
			if ($5 != NULL)
			    free ($5);
		}
	| SITE SP UMASK check_login CRLF
		{
			int oldmask;

			if ($4) {
				oldmask = umask(0);
				(void) umask(oldmask);
				reply(200, "Current UMASK is %03o", oldmask);
			}
		}
	| SITE SP UMASK check_login SP octal_number CRLF
		{
			int oldmask;

			if ($4) {
				if (($6 == -1) || ($6 > 0777)) {
					reply(501, "Bad UMASK value");
				} else {
					oldmask = umask($6);
					reply(200,
					    "UMASK set to %03o (was %03o)",
					    $6, oldmask);
				}
			}
		}
	| SITE SP CHMOD check_login SP octal_number SP pathname CRLF
		{
			if ($4 && ($8 != NULL)) {
				if ($6 > 0777)
					reply(501,
				"CHMOD: Mode value must be between 0 and 0777");
				else if (chmod($8, $6) < 0)
					perror_reply(550, $8);
				else
					reply(200, "CHMOD command successful.");
			}
			if ($8 != NULL)
				free($8);
		}
	| SITE SP IDLE CRLF
		{
			reply(200,
			    "Current IDLE time limit is %d seconds; max %d",
				timeout, maxtimeout);
		}
	| SITE SP check_login IDLE SP NUMBER CRLF
		{
		    	if ($3) {
			    if ($6 < 30 || $6 > maxtimeout) {
				reply (501,
			"Maximum IDLE time must be between 30 and %d seconds",
					maxtimeout);
			    } else {
				timeout = $6;
				(void) alarm((unsigned) timeout);
				reply(200,
					"Maximum IDLE time set to %d seconds",
					timeout);
			    }
			}
		}
	| STOU check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				store($4, "w", 1);
			if ($4 != NULL)
				free($4);
		}
	| SYST CRLF
		{
		        const char *sys_type; /* Official rfc-defined os type.  */
			char *version = 0; /* A more specific type. */

#ifdef HAVE_UNAME
			struct utsname u;
			if (uname (&u) == 0) {
				version =
				  malloc (strlen (u.sysname)
					  + 1 + strlen (u.release) + 1);
				if (version)
					sprintf (version, "%s %s",
						 u.sysname, u.release);
		        }
#else
#ifdef BSD
			version = "BSD";
#endif
#endif

#ifdef unix
			sys_type = "UNIX";
#else
			sys_type = "UNKNOWN";
#endif

			if (version)
				reply(215, "%s Type: L%d Version: %s",
				      sys_type, NBBY, version);
			else
				reply(215, "%s Type: L%d", sys_type, NBBY);

#ifdef HAVE_UNAME
			if (version)
				free (version);
#endif
		}

		/*
		 * SIZE is not in RFC959, but Postel has blessed it and
		 * it will be in the updated RFC.
		 *
		 * Return size of file in a format suitable for
		 * using with RESTART (we just count bytes).
		 */
	| SIZE check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL)
				sizecmd($4);
			if ($4 != NULL)
				free($4);
		}

		/*
		 * MDTM is not in RFC959, but Postel has blessed it and
		 * it will be in the updated RFC.
		 *
		 * Return modification time of file as an ISO 3307
		 * style time. E.g. YYYYMMDDHHMMSS or YYYYMMDDHHMMSS.xxx
		 * where xxx is the fractional second (of any precision,
		 * not necessarily 3 digits)
		 */
	| MDTM check_login SP pathname CRLF
		{
			if ($2 && $4 != NULL) {
				struct stat stbuf;
				if (stat($4, &stbuf) < 0)
					reply(550, "%s: %s",
					    $4, strerror(errno));
				else if (!S_ISREG(stbuf.st_mode)) {
					reply(550, "%s: not a plain file.", $4);
				} else {
					struct tm *t;
					t = gmtime(&stbuf.st_mtime);
					reply(213,
					    "%04d%02d%02d%02d%02d%02d",
					    1900 + t->tm_year, t->tm_mon+1,
					    t->tm_mday, t->tm_hour, t->tm_min,
					    t->tm_sec);
				}
			}
			if ($4 != NULL)
				free($4);
		}
	| QUIT CRLF
		{
			reply(221, "Goodbye.");
			dologout(0);
		}
	| error CRLF
		{
			yyerrok;
		}
	;
rcmd
	: RNFR check_login SP pathname CRLF
		{
			restart_point = (off_t) 0;
			if ($2 && $4) {
			    if (fromname != NULL)
				free (fromname);
			    fromname = renamefrom($4);
			}
			if (fromname == (char *) 0 && $4)
			    free($4);
		}
	| REST SP NUMBER CRLF
		{
		    	if (fromname != NULL)
				free (fromname);
			fromname = (char *) 0;
			restart_point = $3;
			reply(350,
			      (sizeof(restart_point) > sizeof(long)
			       ? "Restarting at %qd. %s"
			       : "Restarting at %ld. %s"), restart_point,
			    "Send STORE or RETRIEVE to initiate transfer.");
		}
	;

username
	: STRING
	;

password
	: /* empty */
		{
			$$ = (char *)calloc(1, sizeof(char));
		}
	| STRING
	;

byte_size
	: NUMBER
		{
			$$ = $1;
		}
	;

host_port
	: NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER
		{
			char *a, *p;

			data_dest.su_family = AF_INET;
			p = (char *)&data_dest.su_sin.sin_port;
			p[0] = $9; p[1] = $11;
			a = (char *)&data_dest.su_sin.sin_addr;
			a[0] = $1; a[1] = $3; a[2] = $5; a[3] = $7;
		}
	;

host_long_port
	: NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER
		{
			char *a, *p;

			memset(&data_dest, 0, sizeof(data_dest));
			data_dest.su_family = AF_INET6;
			p = (char *)&data_dest.su_port;
			p[0] = $39; p[1] = $41;
			a = (char *)&data_dest.su_sin6.sin6_addr;
			 a[0] =  $5;  a[1] =  $7;  a[2] =  $9;  a[3] = $11;
			 a[4] = $13;  a[5] = $15;  a[6] = $17;  a[7] = $19;
			 a[8] = $21;  a[9] = $23; a[10] = $25; a[11] = $27;
			a[12] = $29; a[13] = $31; a[14] = $33; a[15] = $35;
			if (his_addr.su_family == AF_INET6) {
				/* XXX more sanity checks! */
				data_dest.su_sin6.sin6_scope_id =
					his_addr.su_sin6.sin6_scope_id;
			}
			if ($1 != 6 || $3 != 16 || $37 != 2)
				memset(&data_dest, 0, sizeof(data_dest));
		}
	| NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA
		NUMBER
		{
			char *a, *p;

			memset(&data_dest, 0, sizeof(data_dest));
			data_dest.su_family = AF_INET;
			p = (char *)&data_dest.su_port;
			p[0] = $15; p[1] = $17;
			a = (char *)&data_dest.su_sin.sin_addr;
			a[0] =  $5;  a[1] =  $7;  a[2] =  $9;  a[3] = $11;
			if ($1 != 4 || $3 != 4 || $13 != 2)
				memset(&data_dest, 0, sizeof(data_dest));
		}
	;

form_code
	: N
		{
			$$ = FORM_N;
		}
	| T
		{
			$$ = FORM_T;
		}
	| C
		{
			$$ = FORM_C;
		}
	;

type_code
	: A
		{
			cmd_type = TYPE_A;
			cmd_form = FORM_N;
		}
	| A SP form_code
		{
			cmd_type = TYPE_A;
			cmd_form = $3;
		}
	| E
		{
			cmd_type = TYPE_E;
			cmd_form = FORM_N;
		}
	| E SP form_code
		{
			cmd_type = TYPE_E;
			cmd_form = $3;
		}
	| I
		{
			cmd_type = TYPE_I;
		}
	| L
		{
			cmd_type = TYPE_L;
			cmd_bytesz = NBBY;
		}
	| L SP byte_size
		{
			cmd_type = TYPE_L;
			cmd_bytesz = $3;
		}
		/* this is for a bug in the BBN ftp */
	| L byte_size
		{
			cmd_type = TYPE_L;
			cmd_bytesz = $2;
		}
	;

struct_code
	: F
		{
			$$ = STRU_F;
		}
	| R
		{
			$$ = STRU_R;
		}
	| P
		{
			$$ = STRU_P;
		}
	;

mode_code
	: S
		{
			$$ = MODE_S;
		}
	| B
		{
			$$ = MODE_B;
		}
	| C
		{
			$$ = MODE_C;
		}
	;

pathname
	: pathstring
		{
			/*
			 * Problem: this production is used for all pathname
			 * processing, but only gives a 550 error reply.
			 * This is a valid reply in some cases but not in others.
			 */
			if (cred.logged_in && $1 && *$1 == '~') {
				glob_t gl;
				int flags = GLOB_NOCHECK;

#ifdef GLOB_BRACE
				flags |= GLOB_BRACE;
#endif
#ifdef GLOB_QUOTE
				flags |= GLOB_QUOTE;
#endif
#ifdef GLOB_TILDE
				flags |= GLOB_TILDE;
#endif

				memset(&gl, 0, sizeof(gl));
				if (glob($1, flags, NULL, &gl) ||
				    gl.gl_pathc == 0) {
					reply(550, "not found");
					$$ = NULL;
				} else {
					$$ = strdup(gl.gl_pathv[0]);
				}
				globfree(&gl);
				free($1);
			} else
				$$ = $1;
		}
	;

pathstring
	: STRING
	;

octal_number
	: NUMBER
		{
			int ret, dec, multby, digit;

			/*
			 * Convert a number that was read as decimal number
			 * to what it would be if it had been read as octal.
			 */
			dec = $1;
			multby = 1;
			ret = 0;
			while (dec) {
				digit = dec%10;
				if (digit > 7) {
					ret = -1;
					break;
				}
				ret += digit * multby;
				multby *= 8;
				dec /= 10;
			}
			$$ = ret;
		}
	;


check_login
	: /* empty */
		{
			if (cred.logged_in)
				$$ = 1;
			else {
				reply(530, "Please login with USER and PASS.");
				$$ = 0;
			}
		}
	;

%%

#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional SP then STRING */
#define	ZSTR1	5	/* SP then optional STRING */
#define	ZSTR2	6	/* optional STRING after SP */
#define	SITECMD	7	/* SITE command */
#define	NSTR	8	/* Number followed by a string */

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, ZSTR1, 1,	"<sp> password" },
#ifdef FULL_FTPD
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "SMNT", SMNT, ARGS, 0,	"(structure mount)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
#endif
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "LPRT", LPRT, ARGS, 1,	"<sp> af, hal, h1, h2, h3,..., pal, p1, p2..." },
	{ "EPRT", EPRT, STR1, 1,	"<sp> |af|addr|port|" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "LPSV", LPSV, ARGS, 1,	"(set server in passive mode)" },
	{ "EPSV", EPSV, ARGS, 1,	"[<sp> af|ALL]" },
	{ "FEAT", FEAT, ARGS, 1,	"(display extended features)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
#ifdef FULL_FTPD
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
#endif
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
#ifdef FULL_FTPD
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
#endif
	{ "REST", REST, ARGS, 1,	"<sp> offset (restart command)" },
#ifdef FULL_FTPD
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
#endif
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
#ifdef FULL_FTPD
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name ]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
#endif
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
#ifdef FULL_FTPD
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, SITECMD, 1,	"site-cmd [ <sp> arguments ]" },
	{ "SYST", SYST, ARGS, 1,	"(get type of operating system)" },
	{ "STAT", STAT, OSTR, 1,	"[ <sp> path-name ]" },
#endif
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
#ifdef FULL_FTPD
	{ "MKD",  MKD,  STR1, 1,	"<sp> path-name" },
	{ "XMKD", MKD,  STR1, 1,	"<sp> path-name" },
	{ "RMD",  RMD,  STR1, 1,	"<sp> path-name" },
	{ "XRMD", RMD,  STR1, 1,	"<sp> path-name" },
#endif
	{ "PWD",  PWD,  ARGS, 1,	"(return current directory)" },
	{ "XPWD", PWD,  ARGS, 1,	"(return current directory)" },
#ifdef FULL_FTPD
	{ "CDUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ "SIZE", SIZE, OSTR, 1,	"<sp> path-name" },
	{ "MDTM", MDTM, OSTR, 1,	"<sp> path-name" },
#endif
	{ NULL,   0,    0,    0,	0 }
};

struct tab sitetab[] = {
	{ "UMASK", UMASK, ARGS, 1,	"[ <sp> umask ]" },
	{ "IDLE", IDLE, ARGS, 1,	"[ <sp> maximum-idle-time ]" },
	{ "CHMOD", CHMOD, NSTR, 1,	"<sp> mode <sp> file-name" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ NULL,   0,    0,    0,	0 }
};

static struct tab *
lookup(struct tab *p, const char *cmd)
{

	for (; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
telnet_fgets(char *s, int n, FILE *iop)
{
	int c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug)
				syslog(LOG_DEBUG, "command: %s", s);
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0)
			tmpline[0] = '\0';
	}
	while ((c = getc(iop)) != EOF) {
		c &= 0377;
		if (c == IAC) {
		    if ((c = getc(iop)) != EOF) {
			c &= 0377;
			switch (c) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case IAC:
				break;
			default:
				continue;	/* ignore command */
			}
		    }
		}
		*cs++ = c;
		if (--n <= 0 || c == '\n')
			break;
	}
	if (c == EOF && cs == s)
	    return (NULL);
	*cs++ = '\0';
	if (debug) {
		if (!cred.guest && strncasecmp("pass ", s, 5) == 0) {
			/* Don't syslog passwords */
			syslog(LOG_DEBUG, "command: %.5s ???", s);
		} else {
			register char *cp;
			register int len;

			/* Don't syslog trailing CR-LF */
			len = strlen(s);
			cp = s + len - 1;
			while (cp >= s && (*cp == '\n' || *cp == '\r')) {
				--cp;
				--len;
			}
			syslog(LOG_DEBUG, "command: %.*s", len, s);
		}
	}
	return (s);
}

void
toolong(int signo)
{
  (void)signo;
	reply(421,
	    "Timeout (%d seconds): closing control connection.", timeout);
	if (logging)
		syslog(LOG_INFO, "User %s timed out after %d seconds",
		    (cred.name ? cred.name : "unknown"), timeout);
	dologout(1);
}

static int
yylex()
{
	static int cpos, state;
	char *cp, *cp2;
	struct tab *p;
	int n;
	char c;

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (telnet_fgets(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			(void) alarm(0);
#ifdef HAVE_SETPROCTITLE
			if (strncasecmp(cbuf, "PASS", 4) != NULL)
				setproctitle("%s: %s", proctitle, cbuf);
#endif /* HAVE_SETPROCTITLE */
			if ((cp = strchr(cbuf, '\r'))) {
				*cp++ = '\n';
				*cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")))
				cpos = cp - cbuf;
			if (cpos == 0)
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cmdtab, cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				yylval.s = (char *)p->name;
				return (p->token);
			}
			break;

		case SITECMD:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			cp = &cbuf[cpos];
			if ((cp2 = strpbrk(cp, " \n")))
				cpos = cp2 - cbuf;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cp);
			p = lookup(sitetab, cp);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					state = CMD;
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				yylval.s = (char *)p->name;
				return (p->token);
			}
			state = CMD;
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR1:
		case ZSTR1:
		dostr1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = state == OSTR ? STR2 : state + 1;
				return (SP);
			}
			break;

		case ZSTR2:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				yylval.s = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case NSTR:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval.o = atoi(cp);
				cbuf[cpos] = c;
				state = STR1;
				return (NUMBER);
			}
			state = STR1;
			goto dostr1;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval.o = strtoull(cp, NULL, 10);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

void
upper(char *s)
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

static char *
copy(char *s)
{
	char *p;

	p = malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	(void) strcpy(p, s);
	return (p);
}

static void
help(struct tab *ctab, char *s)
{
	struct tab *c;
	int width, NCMDS;
	const char *help_type;

	if (ctab == sitetab)
		help_type = "SITE ";
	else
		help_type = "";
	width = 0, NCMDS = 0;
	for (c = ctab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		int i, j, w;
		int columns, lines;

		lreply(214, "The following %scommands are recognized %s.",
		    help_type, "(* =>'s unimplemented)");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("   ");
			for (j = 0; j < columns; j++) {
				c = ctab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &ctab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		(void) fflush(stdout);
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(ctab, s);
	if (c == (struct tab *)0) {
		reply(502, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s%s %s", help_type, c->name, c->help);
	else
		reply(214, "%s%-*s\t%s; unimplemented.", help_type, width,
		    c->name, c->help);
}

static void
sizecmd(char *filename)
{
	switch (type) {
	case TYPE_L:
	case TYPE_I: {
		struct stat stbuf;
		if (stat(filename, &stbuf) < 0 || !S_ISREG(stbuf.st_mode))
			reply(550, "%s: not a plain file.", filename);
		else
			reply(213,
			      (sizeof (stbuf.st_size) > sizeof(long)
			       ? "%qu" : "%lu"), stbuf.st_size);
		break; }
	case TYPE_A: {
		FILE *fin;
		int c;
		off_t count;
		struct stat stbuf;
		fin = fopen(filename, "r");
		if (fin == NULL) {
			perror_reply(550, filename);
			return;
		}
		if (fstat(fileno(fin), &stbuf) < 0 || !S_ISREG(stbuf.st_mode)) {
			reply(550, "%s: not a plain file.", filename);
			(void) fclose(fin);
			return;
		}

		count = 0;
		while((c=getc(fin)) != EOF) {
			if (c == '\n')	/* will get expanded to \r\n */
				count++;
			count++;
		}
		(void) fclose(fin);

		reply(213, sizeof(count) > sizeof(long) ? "%qd" : "%ld",
		      count);
		break; }
	default:
		reply(504, "SIZE not implemented for Type %c.", "?AEIL"[type]);
	}
}

/* Return 1, if port check is done. Return 0, if not yet. */
static int
port_check(pcmd)
	const char *pcmd;
{
	if (his_addr.su_family == AF_INET) {
		if (data_dest.su_family != AF_INET) {
			usedefault = 1;
			reply(500, "Invalid address rejected.");
			return 1;
		}
		if (paranoid &&
		    ((ntohs(data_dest.su_port) < IPPORT_RESERVED) ||
		     memcmp(&data_dest.su_sin.sin_addr,
			    &his_addr.su_sin.sin_addr,
			    sizeof(data_dest.su_sin.sin_addr)))) {
			usedefault = 1;
			reply(500, "Illegal PORT range rejected.");
		} else {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "%s command successful.", pcmd);
		}
		return 1;
	}
	return 0;
}

#ifdef IPV6
/* Return 1, if port check is done. Return 0, if not yet. */
static int
port_check_v6(pcmd)
	const char *pcmd;
{
	if (his_addr.su_family == AF_INET6) {
		if (IN6_IS_ADDR_V4MAPPED(&his_addr.su_sin6.sin6_addr))
			/* Convert data_dest into v4 mapped sockaddr.*/
			v4map_data_dest();
		if (data_dest.su_family != AF_INET6) {
			usedefault = 1;
			reply(500, "Invalid address rejected.");
			return 1;
		}
		if (paranoid &&
		    ((ntohs(data_dest.su_port) < IPPORT_RESERVED) ||
		     memcmp(&data_dest.su_sin6.sin6_addr,
			    &his_addr.su_sin6.sin6_addr,
			    sizeof(data_dest.su_sin6.sin6_addr)))) {
			usedefault = 1;
			reply(500, "Illegal PORT range rejected.");
		} else {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "%s command successful.", pcmd);
		}
		return 1;
	}
	return 0;
}

static void
v4map_data_dest()
{
	struct in_addr savedaddr;
	in_port_t savedport;

	if (data_dest.su_family != AF_INET) {
		usedefault = 1;
		reply(500, "Invalid address rejected.");
		return;
	}

	savedaddr = data_dest.su_sin.sin_addr;
	savedport = data_dest.su_port;

	memset(&data_dest, 0, sizeof(data_dest));
	data_dest.su_sin6.sin6_family = AF_INET6;
	data_dest.su_sin6.sin6_port = savedport;
	memset(&data_dest.su_sin6.sin6_addr.s6_addr[10], 0xff, 2);
	memcpy(&data_dest.su_sin6.sin6_addr.s6_addr[12],
	       &savedaddr, sizeof(savedaddr));
}
#endif

static void
yyerror(const char *s)
{
  char *cp;

  (void)s;
  cp = strchr(cbuf,'\n');
  if (cp != NULL)
    *cp = '\0';
  reply(500, "'%s': command not understood.", cbuf);
}
