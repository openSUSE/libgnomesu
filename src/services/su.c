/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003,2004  Hongli Lai
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

#ifndef _SU_C_
#define _SU_C_

#include <gtk/gtk.h>

#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

#include "su.h"
#include "utils.h"
#include "libgnomesu.h"
#include "prefix.h"
#include "gnomesu-auth-dialog.h"

G_BEGIN_DECLS


#undef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)


/* Show an error message */
static void
bomb (GnomesuAuthDialog *auth, gchar *format, ...)
{
	GtkWidget *dialog;
	va_list ap;
	gchar *msg;

	va_start (ap, format);
	msg = g_strdup_vprintf (format, ap);
	va_end (ap);

	dialog = gtk_message_dialog_new ((GtkWindow *) auth,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		msg);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}


static gboolean
detect (gchar *exe, const gchar *user)
{
	struct stat buf;
	gchar *filename;

	/* Check whether gnomesu-backend is present and setuid root */
	filename = g_strdup_printf ("%s/gnomesu-backend", LIBEXECDIR);
	if (stat (filename, &buf) == -1) {
		g_free (filename);
		return FALSE;
	} else {
		g_free (filename);
		return (buf.st_uid == 0) && (buf.st_mode & S_ISUID);
	}
}


static gboolean
spawn_async (gchar *user, gchar **argv, int *pid)
{
	int mypid, parent_pipe[2], child_pipe[2];

	g_return_val_if_fail (argv != NULL, FALSE);

	if (!user)
		user = "root";


	if (pipe (parent_pipe) == -1)
		return FALSE;
	if (pipe (child_pipe) == -1) {
		close (parent_pipe[0]);
		close (parent_pipe[1]);
		return FALSE;
	}


	mypid = fork ();
	switch (mypid) {
	case -1: /* error */
		close (parent_pipe[0]);
		close (parent_pipe[1]);
		close (child_pipe[0]);
		close (child_pipe[1]);
		return FALSE;

	case 0: /* child */
	    {
	        GList *args = NULL;
	        gchar **su_argv;

		close (child_pipe[1]);
		close (parent_pipe[0]);

		glt_add (args, strf ("%s/gnomesu-backend", LIBEXECDIR));
		glt_add (args, strf ("%d", child_pipe[0]));
		glt_add (args, strf ("%d", parent_pipe[1]));
		glt_add (args, user);
		glt_addv (args, argv);
		su_argv = glt_to_vector (args, NULL);

		putenv ("_GNOMESU_BACKEND_START=1");
		execv (su_argv[0], su_argv);
		_exit (1);
		break;
	    }

	default: /* parent */
	    {
		gchar buf[1024];
		FILE *f;
		int status;
		GnomesuAuthDialog *gui = NULL;
		guint tries = 0;

		close (parent_pipe[1]);
		close (child_pipe[0]);
		f = fdopen (parent_pipe[0], "r");
		if (!f)
			return FALSE;

		while (!feof (f) && fgets (buf, sizeof (buf), f)) {
			if (cmp (buf, "DONE\n")) {
				if (gui)
					gtk_widget_destroy (GTK_WIDGET (gui));
				while (gtk_events_pending ())
					gtk_main_iteration ();
				fclose (f);
				close (parent_pipe[0]);
				close (child_pipe[1]);
				if (pid)
					*pid = mypid;
				return TRUE;

			} else if (cmp (buf, "INCORRECT_PASSWORD\n")) {
				tries++;
				if (tries >= 2)
					gnomesu_auth_dialog_set_mode (gui, GNOMESU_MODE_LAST_CHANCE);
				else
					gnomesu_auth_dialog_set_mode (gui, GNOMESU_MODE_WRONG_PASSWORD);

			} else if (cmp (buf, "ASK_PASS\n")) {
				gchar *password = NULL;

				if (!gui) {
					gchar *tmp;

					gui = (GnomesuAuthDialog *) gnomesu_auth_dialog_new ();
					tmp = LGSD(create_command) (argv);
					gnomesu_auth_dialog_set_command (gui, tmp);
					g_free (tmp);

					if (strcmp (user, "root") != 0) {
						gchar *tmp2;

						tmp = strf (_("Please enter %s's password and click Continue."), user);
						tmp2 = g_strdup_printf ("<b>%s</b>\n%s",
							_("The requested action needs further authentication."),
							tmp);
						gnomesu_auth_dialog_set_desc (gui, tmp2);
						g_free (tmp);
						g_free (tmp2);

						tmp = g_strdup_printf (_("%s's _password:"), user);
						gnomesu_auth_dialog_set_prompt (gui, tmp);
						g_free (tmp);
					}
				}

				password = gnomesu_auth_dialog_prompt (gui);
				if (!password)
					break;

				write (child_pipe[1], password, strlen (password));
				gnomesu_free_password (&password);
				write (child_pipe[1], "\n", 1);

			/* These are all errors */
			} else if (cmp (buf, "PASSWORD_FAIL\n")) {
				break;

			} else if (cmp (buf, "NO_SUCH_USER\n")) {
				bomb (gui, _("User '%s' doesn't exist."), user);
				break;

			} else if (cmp (buf, "ERROR\n")) {
				bomb (gui, _("An unknown error occured while authenticating."));
				break;

			} else if (cmp (buf, "DENIED\n")) {
				bomb (gui, _("You do not have permission to authenticate."));
				break;

			} else
				break;
		}

		if (gui)
			gtk_widget_destroy (GTK_WIDGET (gui));
		while (gtk_events_pending ())
			gtk_main_iteration ();

		fclose (f);
		close (child_pipe[1]);

		while (waitpid (mypid, &status, WNOHANG) == 0) {
			while (gtk_events_pending ())
				gtk_main_iteration ();
			usleep (100000);
		}

		return FALSE;
	    }
	}

	return FALSE;
}


GnomeSuService *
__gnomesu_su_service_new (void)
{
	GnomeSuService *service;

	service = (GnomeSuService *) g_new0 (GnomeSuService, 1);
	service->detect = detect;
	service->spawn_async = spawn_async;
	return service;
}


G_END_DECLS

#endif /* _SU_C_ */
