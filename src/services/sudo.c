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

#ifndef _SUDO_C_
#define _SUDO_C_

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <stdarg.h>
#include <libintl.h>
#include "sudo.h"
#include "utils.h"

G_BEGIN_DECLS


#undef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)

typedef struct
{
	GladeXML *xml;
	GtkWidget *win;		/* the window */
	GtkWidget *pass;	/* the password entry */
	GtkWidget *command;	/* the 'Command to run' label */
	GtkWidget *verify;	/* the "verifying password..." label */
	GdkCursor *watch;	/* The watch busy cursor */
} SudoGUI;


static void *
safe_memset (void *s, int c, size_t n)
{
	/* Works around compiler optimizations which removes memset().
	   See http://bugzilla.gnome.org/show_bug.cgi?id=161213 */
	return memset (s, c, n);
}


static gboolean
pass_changed (GtkEntry *entry, GtkWidget *ok)
{
	gtk_widget_set_sensitive (ok, gtk_entry_get_text (entry) != NULL
		&& strlen (gtk_entry_get_text (entry)) > 0);
	return FALSE;
}


static SudoGUI *
init_gui (gchar *command)
{
	SudoGUI *gui;

	gui = (SudoGUI *) g_new0 (SudoGUI, 1);
	gui->xml = __libgnomesu_load_glade ("sudo.glade");
	glade_xml_signal_autoconnect (gui->xml);

	gui->win = glade_xml_get_widget (gui->xml, "SudoDialog");
	gtk_widget_realize (gui->win);
	gui->pass = glade_xml_get_widget (gui->xml, "password");
	g_signal_connect (gui->pass, "changed", G_CALLBACK (pass_changed),
		glade_xml_get_widget (gui->xml, "ok"));
	gui->command = glade_xml_get_widget (gui->xml, "command");
	gtk_label_set_text (GTK_LABEL (gui->command), command);
	gui->verify = glade_xml_get_widget (gui->xml, "verifying");
	gui->watch = gdk_cursor_new (GDK_WATCH);
	return gui;
}


static void
clear_entry (GtkWidget *entry)
{
	gchar *blank;

	/* Make a pathetic stab at clearing the GtkEntry field memory */
	blank = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
	if (blank && strlen (blank))
		safe_memset (blank, ' ', strlen (blank));

	blank = g_strdup (blank);
	if (strlen (blank))
		safe_memset (blank, ' ', strlen (blank));

	gtk_entry_set_text (GTK_ENTRY (entry), blank);
	gtk_entry_set_text (GTK_ENTRY (entry), "");
}


static void
fini_gui (SudoGUI *gui)
{
	clear_entry (gui->pass);
	gtk_widget_destroy (gui->win);
	g_object_unref (gui->xml);
	gdk_cursor_unref (gui->watch);
	g_free (gui);
	while (gtk_events_pending ())
		gtk_main_iteration ();
}


static void
get_password (SudoGUI *gui, gchar **password, gboolean previous_was_incorrect)
{
	gint response;

	if (previous_was_incorrect)
	{
		gchar *tmp;

		tmp = g_strdup_printf ("<i><b>%s</b></i>",
			_("Incorrect password, please try again."));
		gtk_label_set_markup (GTK_LABEL (gui->verify), tmp);
		g_free (tmp);
		gtk_widget_show (gui->verify);
	} else
		gtk_widget_hide (gui->verify);
	gtk_widget_set_sensitive (gui->win, TRUE);
	gdk_window_set_cursor (gui->win->window, NULL);
	gtk_widget_grab_focus (gui->pass);
	response = gtk_dialog_run (GTK_DIALOG (gui->win));
	
	if (response == GTK_RESPONSE_OK)
	{
		gchar *tmp;

		tmp = g_strdup_printf ("<i><b>%s</b></i>",
			_("Please wait, verifying password..."));
		gtk_label_set_markup (GTK_LABEL (gui->verify), tmp);
		g_free (tmp);
		gtk_widget_show (gui->verify);
		gdk_window_set_cursor (gui->win->window, gui->watch);
		gtk_widget_set_sensitive (gui->win, FALSE);

		*password = g_strdup (gtk_entry_get_text (GTK_ENTRY (gui->pass)));
	}

	clear_entry (gui->pass);
	while (gtk_events_pending ())
		gtk_main_iteration ();
}


/* Show an error message */
static void
bomb (SudoGUI *gui, gchar *format, ...)
{
	GtkWidget *dialog, *win = NULL;
	va_list ap;
	gchar *msg;

	va_start (ap, format);
	msg = g_strdup_vprintf (format, ap);
	va_end (ap);

	if (gui)
		win = gui->win;
	dialog = gtk_message_dialog_new (GTK_WINDOW (win),
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
	GConfClient *client;

	/* sudo support is disabled. We used to use a helper program
	 * (gnomesu-sudo-helper) but it broke settings in sudoers so I
	 * removed it. Now there's no way to verify that the password has
	 * been successfully accepted, so this backend is broken.
	 * Unless I find a good way to fix this, this backend may be
	 * removed in the future.
	 */
	return FALSE;

	/* There is no way to autodetect whether sudo is available without
	 * asking for a password first, so this is a configuration option. */

	client = gconf_client_get_default ();
	if (!client)
		return FALSE;

	return gconf_client_get_bool (client,
		"/desktop/gnome/superuser/use_sudo", NULL);
}


static gboolean
spawn_async (gchar *user, gchar **argv, int *pid)
{
	int parent_pipe[2], child_pipe[2];
	int mypid;

	g_return_val_if_fail (argv != NULL, FALSE);

	if (!user)
		user = "root";

	if (pipe (parent_pipe) == -1)
		return FALSE;
	if (pipe (child_pipe) == -1)
	{
		close (parent_pipe[0]);
		close (parent_pipe[1]);
		return FALSE;
	}


	mypid = fork ();
	switch (mypid)
	{
	case -1: /* error */
		close (parent_pipe[0]);
		close (parent_pipe[1]);
		close (child_pipe[0]);
		close (child_pipe[1]);
		return FALSE;
	case 0: /* child */
	    {
	        gchar **sudo_argv;
	        guint i, c;

		/* Communicate with sudo using stdin and stderr */
		dup2 (child_pipe[0], STDIN_FILENO);
		dup2 (parent_pipe[1], STDERR_FILENO);

		c = __libgnomesu_count_args (argv);
		sudo_argv = g_new0 (gchar *, c + 8);
		sudo_argv[0] = "/usr/bin/sudo";
		sudo_argv[1] = "-H";
		sudo_argv[2] = "-S";
		sudo_argv[3] = "-u";
		sudo_argv[4] = user;
		sudo_argv[5] = "-p";
		sudo_argv[6] = "GNOMESU_SUDO_PASS\n";
		for (i = 0; i < c; i++)
			sudo_argv[i + 7] = argv[i];

		execv ("/usr/bin/sudo", sudo_argv);
		_exit (1);
		break;
	    }
	default: /* parent */
	    {
		gchar buf[1024];
		FILE *f;
		int status;
		SudoGUI *gui;
		gboolean previous_incorrect = FALSE;
		gchar *commandline;

		close (parent_pipe[1]);
		f = fdopen (parent_pipe[0], "r");
		if (!f)
			return FALSE;

		commandline = __libgnomesu_create_command (argv);
		gui = init_gui (commandline);
		g_free (commandline);

		while (!feof (f) && fgets (buf, sizeof (buf), f))
		{
			if (strcmp (buf, "GNOMESU_SUDO_DONE\n") == 0)
			{
				fini_gui (gui);
				fclose (f);
				close (parent_pipe[0]);
				close (child_pipe[1]);
				if (pid)
					*pid = mypid;
				return TRUE;
			}
			else if (strcmp (buf, "Sorry, try again.\n") == 0)
			{
				previous_incorrect = TRUE;
			}
			else if (strcmp (buf, "GNOMESU_SUDO_PASS\n") == 0)
			{
				gchar *password = NULL;

				get_password (gui, &password, previous_incorrect);
				if (!password)
					break;

				write (child_pipe[1], password, strlen (password));
				safe_memset (password, 0, strlen (password));
				g_free (password);
				write (child_pipe[1], "\n", 1);
			} else if (strstr (buf, " is not in the sudoers file.  This incident will be reported."))
			{
				bomb (gui, _("You do not have permission gain superuser (root) privileges. "
					     "This incident will be reported."));
				break;
			}
		}

		fini_gui (gui);
		fclose (f);
		close (child_pipe[1]);

		while (waitpid (mypid, &status, WNOHANG) == 0)
		{
			while (gtk_events_pending ())
				gtk_main_iteration ();
			usleep (100000);
		}

		return FALSE;
	    }
	}
}


GnomeSuService *
__gnomesu_sudo_service_new (void)
{
	GnomeSuService *service;

	service = (GnomeSuService *) g_new0 (GnomeSuService, 1);
	service->detect = detect;
	service->spawn_async = spawn_async;
	return service;
}


G_END_DECLS

#endif /* _SUDO_C_ */
