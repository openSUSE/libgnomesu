#ifndef __ERRMSG_H__
#define __ERRMSG_H__

#ifdef HAVE_ERROR_H

#include "error.h"

#else /* HAVE_ERROR_H */

void error (int exit_code, int errcode, char *format, ...);

#endif /* HAVE_ERROR_H */

#endif /* __ERRMSG_H__ */
