/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003  Hongli Lai
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _CONSOLEHELPER_C_
#define _CONSOLEHELPER_C_

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "pam.h"

G_BEGIN_DECLS


static gboolean
detect (gchar *exe, const gchar *user)
{
	gchar *fullpath, *link, *base, *path1;

	if (g_getenv ("GNOMESU_DISABLE_CONSOLEHELPER")
	    && strcmp (g_getenv ("GNOMESU_DISABLE_CONSOLEHELPER"), "1") == 0)
		return FALSE;

	/* Consolehelper only supports execution as root */
	if (user != NULL && strcmp (user, "root") != 0)
		return FALSE;


	fullpath = g_find_program_in_path (exe);

	/* Check whether the executable is a symlink to consolehelper */
	link = g_new0 (gchar, PATH_MAX + 1);
	if (readlink (fullpath, link, PATH_MAX) == -1)
	{
		g_free (fullpath);
		return FALSE;
	} else {
		path1 = g_find_program_in_path (link);

		if (strcmp (path1, "/usr/bin/consolehelper") != 0)
		{
			g_free (path1);
			return FALSE;
		} else
			g_free (path1);
	}


	/* Check whether there's a consolehelper entry and whether
	   consolehelper exists */
	base = g_path_get_basename (fullpath);
	path1 = g_build_filename ("/etc", "security", "console.apps",
		base, NULL);
	if (!g_file_test (path1, G_FILE_TEST_EXISTS)
		|| !g_file_test ("/usr/bin/consolehelper", G_FILE_TEST_EXISTS))
	{
		g_free (path1);
		return FALSE;
	} else {
		g_free (path1);
		return TRUE;
	}
}


static gboolean
spawn_async (gchar *user, gchar **argv, int *pid)
{
	pid_t mypid;

	g_return_val_if_fail (user == NULL || strcmp (user, "root") == 0, FALSE);
	g_return_val_if_fail (argv != NULL, FALSE);

	mypid = fork ();
	if (mypid == 0) /* child */
	{
		execvp (argv[0], argv);
		_exit (1);
	} else if (mypid < 0) /* error */
	{
		return FALSE;
	} else /* parent */
	{
		if (pid)
			*pid = mypid;
		return TRUE;
	}
}


GnomeSuService *
__gnomesu_consolehelper_service_new ()
{
	GnomeSuService *service;

	service = (GnomeSuService *) g_new0 (GnomeSuService, 1);
	service->detect = detect;
	service->spawn_async = spawn_async;
	return service;
}


G_END_DECLS

#endif /* _CONSOLEHELPER_C_ */
