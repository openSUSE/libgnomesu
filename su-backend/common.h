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

#ifndef __COMMON_H_
#define __COMMON_H_

#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>

#define _PATH_LOGINDEFS_SU  "/etc/default/su"
#define _PATH_DEFPATH          "/usr/local/bin:/bin:/usr/bin"
#define _PATH_DEFPATH_ROOT "/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin"

struct item {
	char *name;		/* name of the option.	*/
	char *value;		/* value of the option.	*/
	char *path;	 	/* name of config file for this option.	*/

	struct item *next;	/* pointer to next option.	*/
};

int getlogindefs_bool(const char *name, int dflt);
void logindefs_load_file(const char *filename);
const char *getlogindefs_str(const char *name, const char *dflt);
int logindefs_setenv(const char *name, const char *conf, const char *dflt);

char *concat (const char *s1, const char *s2, const char *s3);
void xputenv (const char *val);
void init_xauth (const struct passwd *pw);
void setup_xauth (const struct passwd *pw);
void init_groups (const struct passwd *pw);
int  change_identity (const struct passwd *pw);
void modify_environment (const struct passwd *pw);
void *safe_memset (void *s, int c, size_t n);

#endif /* __COMMON_H_ */
