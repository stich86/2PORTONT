/* $Id: bsd-snprintf.h,v 1.1.1.1 2003/08/18 05:40:23 kaohj Exp $ */

#ifndef _BSD_SNPRINTF_H
#define _BSD_SNPRINTF_H

#include "config.h"

#include <sys/types.h> /* For size_t */

#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t count, const char *fmt, ...);
#endif /* !HAVE_SNPRINTF */

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *str, size_t count, const char *fmt, va_list args);
#endif /* !HAVE_SNPRINTF */


#endif /* _BSD_SNPRINTF_H */
