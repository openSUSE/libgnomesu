/* libgnomesu - Library for providing superuser privileges to GNOME apps.
   Copyright (C) 2004  Hongli Lai

   su for GNU.  Run a shell with substitute user and group IDs.
   Copyright (C) 1992-1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/* Code used by both gnomesu-backend and gnomesu-pam-backend. */

#ifndef __XSTRDUP_H_
#define __XSTRDUP_H_

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef PARAMS
# if defined PROTOTYPES || (defined __STDC__ && __STDC__)
#  define PARAMS(Args) Args
# else
#  define PARAMS(Args) ()
# endif
#endif

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#include <sys/types.h>

char *xstrdup (const char *string);

#endif /* __XSTRDUP_H_ */
