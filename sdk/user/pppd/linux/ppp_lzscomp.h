/*
 * $Id: ppp_lzscomp.h,v 1.1.1.1 2003/08/18 05:40:57 kaohj Exp $
 *
 * Header for isdn_lzscomp.c
 * Concentrated here to not mess up half a dozen kernel headers with code
 * snippets
 *
 */

/*
 *  ==FILEVERSION 981016==
 *
 *  NOTE TO MAINTAINERS:
 *     If you modify this file at all, please set the above date.
 *     ppp_defs.h is shipped with a PPP distribution as well as with the kernel;
 *     if everyone increases the FILEVERSION number above, then scripts
 *     can do the right thing when deciding whether to install a new ppp_defs.h
 *     file.  Don't change the format of that line otherwise, so the
 *     installation script can recognize it.
 */

#define CI_LZS_COMPRESS		17
#define CILEN_LZS_COMPRESS	5

#define LZS_CMODE_NONE		0
#define LZS_CMODE_LCB		1
#define LZS_CMODE_CRC		2
#define LZS_CMODE_SEQNO		3	/* MUST be implemented (default) */
#define LZS_CMODE_EXT		4	/* Seems to be what Win0.95 uses */

#define LZS_COMP_MAX_HISTS	1	/* Don't waste peers ressources */
#define LZS_COMP_DEF_HISTS	1	/* Most likely to negotiate */
#define LZS_DECOMP_MAX_HISTS	32	/* More is really nonsense */
#define LZS_DECOMP_DEF_HISTS	8	/* If we get it, this may be optimal */

#define LZS_HIST_BYTE1(word)   	(word>>8)	/* Just for better reading */
#define LZS_HIST_BYTE2(word)	(word&0xff)	/* of this big endian stuff */
#define LZS_HIST_WORD(b1,b2)	((b1<<8)|b2)	/* (network byte order rulez) */
