#ifndef HAVE_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"

void
error (int exit_code, int errcode, char *format, ...)
{
	va_list ap;
	char msg[1024];

	safe_memset (msg, 0, sizeof (msg));
	va_start (ap, format);
	vsnprintf (msg, sizeof (msg), format, ap);
	va_end (ap);

	fprintf (stderr, "%s: %s\n", msg, strerror (errcode));
	exit (exit_code);
}

#endif /* HAVE_ERROR_H */
