/* vi: set sw=4 ts=4: */
/*
 * Mini xgethostbyname implementation.
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

//#include <netdb.h>
#include "libbb.h"

struct hostent* FAST_FUNC xgethostbyname(const char *name)
{
	struct hostent *retval = gethostbyname(name);
#if ENABLE_FEATURE_IPV6
	if (!retval)
		retval = gethostbyname2(name, AF_INET6);
#endif
	if (!retval)
		bb_herror_msg_and_die("%s", name);
	return retval;
}
