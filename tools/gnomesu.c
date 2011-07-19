/* 
 * Gnome SuperUser
 * Copyright (C) 2003,2004  Hongli Lai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <string.h>
#include <libintl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "libgnomesu.h"
#include "prefix.h"


static gchar *command = NULL;
static gchar *user = NULL;
static gint   final_status = 0;
GMainLoop    *main_loop;

static GOptionEntry entries[] =
{
  { "command", 'c', 0, G_OPTION_ARG_STRING, &command, N_("Pass the command to execute as one single string."), N_("COMMAND") },
  { "user", 'u', 0, G_OPTION_ARG_STRING, &user, N_("Run as this user instead of as root."), N_("USERNAME") },
  { NULL }
  };

static void
child_exit_cb (GPid pid, gint status, gpointer data)
{
	final_status = status;
	g_main_loop_quit (main_loop);
}

int
main (int argc, char *argv[])
{
	GError *error;
	const char *desktop_startup_id;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* get the startup id before GTK+ unsets it, so we can forward it if
	 * there's no authentication dialog */
	desktop_startup_id = g_getenv ("DESKTOP_STARTUP_ID");
	if (desktop_startup_id)
		g_setenv ("_GNOMESU_DESKTOP_STARTUP_ID", desktop_startup_id, TRUE);

	error = NULL;
	if (!gtk_init_with_args (&argc, &argv, NULL, entries, GETTEXT_PACKAGE, &error)) {
		if (error) {
			g_printerr ("%s\n", error->message);
			g_error_free (error);
		} else
			g_printerr (_("An unknown error occurred.\n"));

		return 1;
	}

	g_set_application_name (_("GNOME SuperUser"));
	gtk_window_set_default_icon_name (GTK_STOCK_DIALOG_AUTHENTICATION);

	main_loop = g_main_loop_new (NULL, FALSE);

	if (!command) {
		gchar **args;
		int pid, i = 0;

		/* skipping the program at argv[0] */
		argc--;
		argv++;

		/* Skip any potential leading "--" in argv.
		 * This can happen with "gnomesu -- gnomesu -- ls" */
		while (argc > 0 && argv[0] && strcmp (argv[0], "--") == 0) {
			argc--;
			argv++;
		}

		if (argc == 0) {
			GSettings *settings;
			gchar *terminal;

			settings = g_settings_new ("org.gnome.desktop.default-applications.terminal");
			terminal = g_settings_get_string (settings, "exec");
			g_object_unref (settings);

			if (!terminal || !terminal[0])
				terminal = g_strdup ("gnome-terminal");

			/* Default action: launch a terminal */
			if (!gnomesu_spawn_command_async (user, terminal, &pid)) {
				g_free (terminal);
				return 255;
			}

			g_free (terminal);

			g_child_watch_add (pid, child_exit_cb, NULL);
			g_main_loop_run (main_loop);

			return WEXITSTATUS (final_status);
		}

		/* we have to copy argv to have a NULL-terminated array */
		args = g_new0 (gchar *, argc + 1);
		for (i = 0; i < argc; i++)
			args[i] = argv[i];

		if (!gnomesu_spawn_async (user, args, &pid))
			return 255;

		g_child_watch_add (pid, child_exit_cb, NULL);
		g_main_loop_run (main_loop);

		g_free (args);
		return WEXITSTATUS (final_status);

	} else {
		int pid;

		if (!gnomesu_spawn_command_async (user, command, &pid))
			return 255;

		g_child_watch_add (pid, child_exit_cb, NULL);
		g_main_loop_run (main_loop);

		return WEXITSTATUS (final_status);
	}

	return 0;
}
