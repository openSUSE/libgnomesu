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

#include <gtk/gtk.h>
#include <gnome.h>
#include <gconf/gconf-client.h>

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

static struct poptOption options[] = {
	{ "command", 'c', POPT_ARG_STRING, &command, 0,
	  N_("Pass the command to execute as one single string."),
	  N_("COMMAND") },

	{ "user", 'u', POPT_ARG_STRING, &user, 0,
	  N_("Run as this user instead of as root."),
	  N_("USERNAME") },

	{ NULL, '\0', 0, NULL, 0 }
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
	GnomeProgram *program;
	GValue value = { 0 };
	poptContext pctx;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	program = gnome_program_init ("gnomesu", VERSION,
			LIBGNOMEUI_MODULE, argc, argv,
			GNOME_PARAM_POPT_TABLE, options,
			GNOME_PARAM_HUMAN_READABLE_NAME, _("GNOME SuperUser"),
			GNOME_PARAM_APP_PREFIX, GNOMESU_PREFIX,
			NULL);
	g_object_get_property (G_OBJECT (program),
		GNOME_PARAM_POPT_CONTEXT,
		g_value_init (&value, G_TYPE_POINTER));
	pctx = g_value_get_pointer (&value);

	main_loop = g_main_loop_new (NULL, FALSE);

	if (!command) {
		GList *arglist = NULL;
		gchar *arg, **args;
		int status, pid, i = 0;

		while ((arg = (gchar *) poptGetArg (pctx)) != NULL)
			arglist = g_list_append (arglist, arg);

		if (g_list_length (arglist) == 0) {
			gchar *terminal;

			terminal = gconf_client_get_string (gconf_client_get_default (),
				"/desktop/gnome/applications/terminal/exec", NULL);
			if (!terminal)
				terminal = "gnome-terminal";

			/* Default action: launch a terminal */
			if (!gnomesu_spawn_command_async (user, terminal, &pid))
				return 255;

			g_child_watch_add (pid, child_exit_cb, NULL);
			g_main_loop_run (main_loop);

			return WEXITSTATUS (final_status);
		}

		args = g_new0 (gchar *, g_list_length (arglist) + 1);
		for (arglist = g_list_first (arglist); arglist != NULL; arglist = arglist->next) {
			args[i] = arglist->data;
			i++;
		}

		if (!gnomesu_spawn_async (user, args, &pid))
			return 255;

		g_child_watch_add (pid, child_exit_cb, NULL);
		g_main_loop_run (main_loop);

		g_list_free (arglist);
		g_free (args);
		return WEXITSTATUS (final_status);

	} else {
		int status, pid;

		if (!gnomesu_spawn_command_async (user, command, &pid))
			return 255;

		g_child_watch_add (pid, child_exit_cb, NULL);
		g_main_loop_run (main_loop);

		return WEXITSTATUS (final_status);
	}

	return 0;
}
