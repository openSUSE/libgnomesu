/* libgnomesu - Library for providing superuser privileges to GNOME apps.
   Copyright (C) 2003-2004  Hongli Lai

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

#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"


/* The default PATH for simulated logins to non-superuser accounts.  */
#undef DEFAULT_LOGIN_PATH
#define DEFAULT_LOGIN_PATH "/bin:/usr/bin:/usr/local/bin:/usr/bin/X11:/usr/X11R6/bin"

/* The default PATH for simulated logins to superuser accounts.  */
#undef DEFAULT_ROOT_LOGIN_PATH
#define DEFAULT_ROOT_LOGIN_PATH "/usr/sbin:/usr/bin:/sbin:/bin:/usr/X11R6/bin:/usr/bin/X11:/root/bin"


void *
safe_memset (void *s, int c, size_t n)
{
	/* Works around compiler optimizations which removes memset().
	   See http://bugzilla.gnome.org/show_bug.cgi?id=161213 */
	return memset (s, c, n);
}


/* Return a newly-allocated string whose contents concatenate
   those of S1, S2, S3.  */
char *
concat (const char *s1, const char *s2, const char *s3)
{
	int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
	char *result = (char *) g_malloc (len1 + len2 + len3 + 1);

	strcpy (result, s1);
	strcpy (result + len1, s2);
	strcpy (result + len1 + len2, s3);
	result[len1 + len2 + len3] = 0;

	return result;
}


/* Add VAL to the environment, checking for out of memory errors.  */
void
xputenv (const char *val)
{
	if (putenv ((char *) val))
		fprintf (stderr, "virtual memory exhausted");
}


static gchar *xauth_data = NULL;

static void
saveXauth (void)
{
	FILE *p;
	GString *data;
	gchar line[1024];

	/* Save X authorization data for after we changed identity */
	p = popen ("xauth nlist", "r");
	if (!p) return;

	data = g_string_new ("");
	while (!feof (p) && fgets (line, sizeof (line), p))
	{
		g_string_append (data, line);
	}
	pclose (p);
	xauth_data = data->str;
	g_string_free (data, FALSE);
}

void
init_xauth (const struct passwd *pw)
{
	const char *env_term;
	const char *env_xauthority;

	env_term = g_getenv ("TERM");
	env_xauthority = g_getenv ("XAUTHORITY");

	/* Sanity-check the environment variables as best we can: those
	 * which aren't path names shouldn't contain "/", and none of
	 * them should contain ".." or "%". */
        if (env_term &&
            (strstr(env_term, "..") ||
             strchr(env_term, '%')))
                setenv ("XAUTHORITY", "dumb", 1);
        if (env_xauthority &&
            (strstr(env_xauthority , "..") ||
             strchr(env_xauthority , '%')))
                unsetenv ("XAUTHORITY");

        /* Setup X authentication stuff. */
        saveXauth ();
        xputenv (concat ("XAUTHORITY=", pw->pw_dir, "/.Xauthority"));
}


/* Update environment variables for the new user. */
void
modify_environment (const struct passwd *pw)
{
	const gchar *path;
	const char *env_term;
	const char *env_display, *env_shell;
	const char *env_lang, *env_lcall, *env_lcmsgs;
	const char *env_dbus;

	/* Sanity-check the environment variables as best we can: those
	 * which aren't path names shouldn't contain "/", and none of
	 * them should contain ".." or "%". */
	env_display = getenv("DISPLAY");
	env_lang = g_getenv ("LANG");
	env_lcall = g_getenv ("LC_ALL");
	env_lcmsgs = g_getenv ("LC_MESSAGES");
	env_shell = g_getenv ("SHELL");
	env_term = g_getenv ("TERM");

	if (env_display &&
	    (strstr(env_display, "..") ||
	     strchr(env_display, '%')))
		unsetenv ("DISPLAY");
	if (env_lang &&
	    (strstr(env_lang, "/") ||
	     strstr(env_lang, "..") ||
	     strchr(env_lang, '%')))
		unsetenv ("LANG");
	if (env_lcall &&
	    (strstr(env_lcall, "/") ||
	     strstr(env_lcall, "..") ||
	     strchr(env_lcall, '%')))
		unsetenv ("LC_ALL");
	if (env_lcmsgs &&
	    (strstr(env_lcmsgs, "/") ||
	     strstr(env_lcmsgs, "..") ||
	     strchr(env_lcmsgs, '%')))
		unsetenv ("LC_MESSAGES");
	if (env_shell &&
	    (strstr(env_shell, "..") ||
	     strchr(env_shell, '%')))
		unsetenv ("SHELL");

	if (!g_getenv ("ICEAUTHORITY"))
		xputenv (concat ("ICEAUTHORITY=", pw->pw_dir, "/.ICEauthority"));

	/* Set HOME, SHELL, USER and LOGNAME.  */
	xputenv (concat ("HOME", "=", pw->pw_dir));
	xputenv (concat ("SHELL", "=", pw->pw_shell));
	xputenv (concat ("USER", "=", pw->pw_name));
	xputenv (concat ("LOGNAME", "=", pw->pw_name));

	/* Sanity-check PATH. It shouldn't contain . entries! */
	path = g_getenv ("PATH");
	if (path && (strstr (path, ":.:") || strncmp (path, ".:", 2) == 0
	    || (strlen (path) > 2 && strcmp (path + strlen (path) - 2, ":.") == 0)
	    || strcmp (path, ".") == 0))
	{
		char **paths;
		char **new_paths;
		int    path_len;
		int    i, j;

		paths = g_strsplit (path, ":", -1);
		path_len = g_strv_length (paths);
		new_paths = g_new0 (char *, path_len);

		j = 0;
		for (i = 0; i < path_len; i++) {
			if (paths[i] && !strchr(paths[i], '.')) {
				new_paths[j++] = g_strdup (paths[i]);
			}
		}

		g_strfreev (paths);
		if (j != 0) {
			char *new_path;
			new_path = g_strjoinv (":", new_paths);
			setenv ("PATH", new_path, 1);
			g_free (new_path);
		} else {
			/* make sure we set PATH to something below */
			path = NULL;
		}
	}

	if (!path)
		xputenv (concat ("PATH", "=",
			(pw->pw_uid) ? DEFAULT_LOGIN_PATH : DEFAULT_ROOT_LOGIN_PATH));

	/* Unset environment variables */
	env_dbus = g_getenv ("DBUS_SESSION_BUS_ADDRESS");
	if (env_dbus)
		unsetenv ("DBUS_SESSION_BUS_ADDRESS");
}

/* Become the user and group(s) specified by PW.  */
void
init_groups (const struct passwd *pw)
{
#ifdef HAVE_INITGROUPS
	errno = 0;
	initgroups (pw->pw_name, pw->pw_gid);
	endgrent ();
#endif
}

void
change_identity (const struct passwd *pw)
{
	if (setgid (pw->pw_gid))
		perror ("cannot set group id");
	if (setuid (pw->pw_uid))
		perror ("cannot set user id");
}

void
setup_xauth (const struct passwd *pw)
{
	FILE *p;
	gchar *command;

        command = g_strdup_printf ("xauth -q remove %s/unix:0", g_get_host_name ());
        g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);

	/* Create a new .Xauthorization file */
	if (!xauth_data) return;
	p = popen ("xauth -q nmerge - 2>/dev/null", "w");
	if (!p) return;

	fwrite (xauth_data, strlen (xauth_data), 1, p);
	safe_memset (xauth_data, 0, strlen (xauth_data));
	g_free (xauth_data);
	pclose (p);
}
