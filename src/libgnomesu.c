/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003-2004  Hongli Lai
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

#ifndef _LIBGNOMESU_C_
#define _LIBGNOMESU_C_

#include <sys/wait.h>
#include <unistd.h>
#include <libintl.h>

#include "libgnomesu.h"
#include "utils.h"

#ifdef HAVE_PAM
	#include "services/pam.h"
#endif
#include "services/consolehelper.h"
#include "services/su.h"


G_BEGIN_DECLS

#undef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)


static GnomeSuServiceConstructor services[] = {
	__gnomesu_consolehelper_service_new,
#ifdef HAVE_PAM
	__gnomesu_pam_service_new,
#endif
	__gnomesu_su_service_new
};
static const guint services_size = sizeof (services) / sizeof (GnomeSuServiceConstructor);


static GnomeSuService *
find_best_service (gchar *exe, const gchar *user)
{
	guint i;

	for (i = 0; i < services_size; i++) {
		GnomeSuService *service;

		service = (*(services[i])) ();
		if ((*service->detect) (exe, user))
		{
			return service;
		} else
			g_free (service);
	}
	return NULL;
}


/**
 * gnomesu_exec:
 * @commandline: The command to execute.
 *
 * Execute @command as root and wait until the subprocess has exited.
 *
 * Returns: TRUE on success; FALSE on error. Note that this has got
 * nothing to do with the exit status of the subprocess.
 */
gboolean
gnomesu_exec (gchar *commandline)
{
	return gnomesu_spawn_command_sync (NULL, commandline);
}


/**
 * gnomesu_spawn_command_sync:
 * @user: Execute the command with this username. If this is %NULL, "root" is assumed.
 * @commandline: The command to execute.
 *
 * Execute @command as @user and wait for the subprocess to exit.
 *
 * Returns: %TRUE on success; %FALSE on error. Note that this has got
 * nothing to do with the exit status of the subprocess.
 */
gboolean
gnomesu_spawn_command_sync (gchar *user, gchar *commandline)
{
	int pid, status;

	g_return_val_if_fail (commandline != NULL, FALSE);

	if (!gnomesu_spawn_command_async (user, commandline, &pid))
		return FALSE;
	waitpid (pid, &status, 0);
	return TRUE;
}


/**
 * gnomesu_spawn_command_async:
 * @user: Execute the command with this username. If this is %NULL, "root" is assumed.
 * @commandline: The command to execute.
 * @pid: The return address of the subprocess's PID, or %NULL.
 *
 * Execute @command as @user. Unlike gnomesu_spawn_command_sync(), this
 * function does not wait for the subprocess to exit.
 *
 * Returns: %TRUE on success; %FALSE on error. Note that this has got
 * nothing to do with the exit status of the subprocess.
 */
gboolean
gnomesu_spawn_command_async (gchar *user, gchar *commandline, int *pid)
{
	gchar **argv;
	gboolean result;

	g_return_val_if_fail (commandline != NULL, FALSE);

	argv = NULL;
	if (!g_shell_parse_argv (commandline, NULL, &argv, NULL))
		return FALSE;
	result = gnomesu_spawn_async (user, argv, pid);
	g_strfreev (argv);
	return result;
}


gboolean
gnomesu_spawn_sync (gchar *user, gchar **argv)
{
	int pid, status;

	g_return_val_if_fail (argv != NULL, FALSE);

	if (!gnomesu_spawn_async (user, argv, &pid))
		return FALSE;

	waitpid (pid, &status, 0);
	return TRUE;
}


gboolean
gnomesu_spawn_async (gchar *user, gchar **argv, int *pid)
{
	GnomeSuService *service;
	gboolean result;

	g_return_val_if_fail (argv != NULL, FALSE);

	LGSD(libgnomesu_init) ();

	service = find_best_service (argv[0], user);
	if (!service) {
		g_critical (_("No services for libgnomesu are available.\n"));
		return FALSE;
	}

	result = (*service->spawn_async) (user, argv, pid);
	g_free (service);
	return result;
}


G_END_DECLS

#endif /* _LIBGNOMESU_C_ */
