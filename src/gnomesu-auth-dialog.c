/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2004  Hongli Lai
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

#ifndef _GNOMESU_AUTH_DIALOG_C_
#define _GNOMESU_AUTH_DIALOG_C_

#include <string.h>

#include "gnomesu-auth-dialog.h"
#include "auth-icon.csource"

#undef _
#define _(x) ((gchar *) dgettext (GETTEXT_PACKAGE, x))


G_BEGIN_DECLS

static gpointer parent_class = NULL;

struct _GnomesuAuthDialogPrivate {
	GtkWidget *left_action_area;
	GtkWidget *icon;
	GtkWidget *desc_label;
	GtkWidget *command_desc_label;
	GtkWidget *command_label;
	GtkWidget *prompt_label;
	GtkWidget *input;
	GtkWidget *mode_label;
	GdkCursor *watch;
};


static void *
safe_memset (void *s, int c, size_t n)
{
	/* Works around compiler optimizations which removes memset().
	   See http://bugzilla.gnome.org/show_bug.cgi?id=161213 */
	return memset (s, c, n);
}


static GtkWidget *
create_stock_button (const gchar *stock, const gchar *labelstr)
{
	GtkWidget *button;
	GtkWidget *align, *hbox, *image, *label;

	button = gtk_button_new ();
	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (button), align);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (align), hbox);
	image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	label = gtk_label_new (labelstr);
	gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	gtk_widget_show_all (button);
	return button;
}


static void
clear_entry (GtkWidget *entry)
{
	gchar *blank;
	size_t len;

	/* Make a pathetic stab at clearing the GtkEntry field memory */
	blank = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
	if (blank) {
		len = strlen (blank);
		if (len)
			safe_memset (blank, ' ', len);

		blank = g_strdup (blank);
		gtk_entry_set_text (GTK_ENTRY (entry), blank);
	}

	gtk_entry_set_text (GTK_ENTRY (entry), "");
	if (blank) g_free (blank);
}


static void
gnomesu_auth_dialog_instance_init (GTypeInstance *instance, gpointer g_class)
{
	GtkDialog *dialog = (GtkDialog *) instance;
	GnomesuAuthDialog *adialog = (GnomesuAuthDialog *) instance;
	GtkWidget *action_area_parent, *hbox;
	GtkWidget *left_action_area;
	GtkWidget *vbox;
	GtkWidget *icon, *label;
	GtkWidget *table, *input;
	GtkWidget *button;

	adialog->_priv = g_new0 (GnomesuAuthDialogPrivate, 1);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Password needed"));
	gtk_dialog_set_has_separator (dialog, FALSE);
	gtk_widget_realize (GTK_WIDGET (dialog));
	adialog->_priv->watch = gdk_cursor_new (GDK_WATCH);


	/* Reparent dialog->action_area into a hbox */
	g_object_ref (dialog->action_area);
	action_area_parent = gtk_widget_get_parent (dialog->action_area);
	gtk_container_remove (GTK_CONTAINER (action_area_parent), dialog->action_area);
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
	gtk_box_pack_end (GTK_BOX (action_area_parent), hbox, FALSE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (action_area_parent), hbox, -1);

	/* Add another (left-aligned) button box to the dialog */
	left_action_area = gtk_hbutton_box_new ();
	gtk_container_set_border_width (GTK_CONTAINER (left_action_area), 6);
	/* gtk_button_box_set_spacing (GTK_BUTTON_BOX (left_action_area), 12); */
	gtk_box_set_spacing (GTK_BOX (left_action_area), 12);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (left_action_area), GTK_BUTTONBOX_START);
	adialog->_priv->left_action_area = left_action_area;
	gtk_box_pack_start (GTK_BOX (hbox), left_action_area, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (hbox), dialog->action_area, TRUE, TRUE, 0);
	g_object_unref (dialog->action_area);
	gtk_widget_show_all (action_area_parent);


	/* HBox with icon and description label */
	vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
	gtk_box_pack_start (GTK_BOX (dialog->vbox), vbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);

	adialog->_priv->icon = icon = gtk_image_new ();
	gtk_misc_set_alignment (GTK_MISC (icon), 0.5, 0.0);
	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	adialog->_priv->desc_label = label = gtk_label_new ("");
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);


	/* Command label */
	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	adialog->_priv->command_desc_label = label = gtk_label_new (_("Command:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label,
		0, 1, 0, 1,
		GTK_FILL, GTK_FILL,
		0, 0);

	adialog->_priv->command_label = label = gtk_label_new ("");
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label,
		1, 2, 0, 1,
		GTK_FILL | GTK_EXPAND, GTK_FILL,
		0, 0);


	/* Input entry */
	adialog->_priv->prompt_label = label = gtk_label_new ("_Password:");
	gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label,
		0, 1, 1, 2,
		GTK_FILL, GTK_FILL,
		0, 0);

	adialog->_priv->input = input = gtk_entry_new ();
	g_signal_connect (input, "destroy", G_CALLBACK (clear_entry), NULL);
	gtk_entry_set_activates_default (GTK_ENTRY (input), TRUE);
	gtk_entry_set_visibility (GTK_ENTRY (input), FALSE);
	gtk_table_attach (GTK_TABLE (table), input,
		1, 2, 1, 2,
		GTK_EXPAND | GTK_FILL, 0,
		0, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), input);


	/* Mode label */
	adialog->_priv->mode_label = label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_modify_font (label, pango_font_description_from_string ("Bold"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


	/* Add OK and Cancel buttons */
	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_widget_show (button);
	gtk_dialog_add_action_widget (dialog, button, GTK_RESPONSE_CANCEL);

	button = create_stock_button (GTK_STOCK_OK, "C_ontinue");
	GTK_WIDGET_SET_FLAGS (button, GTK_HAS_DEFAULT | GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	gtk_dialog_add_action_widget (dialog, button, GTK_RESPONSE_OK);
	gtk_widget_grab_default (button);



	gtk_widget_show_all (dialog->vbox);
	gnomesu_auth_dialog_set_desc (adialog, NULL);
	gnomesu_auth_dialog_set_icon (adialog, NULL);
	gnomesu_auth_dialog_set_command (adialog, NULL);
	gnomesu_auth_dialog_set_mode (adialog, GNOMESU_MODE_NORMAL);
	g_object_set (dialog,
		"resizable", FALSE,
		NULL);
}


static void
gnomesu_auth_dialog_finalize (GObject *obj)
{
	gdk_cursor_unref (GNOMESU_AUTH_DIALOG (obj)->_priv->watch);
	g_free  (GNOMESU_AUTH_DIALOG (obj)->_priv);
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}


static void
gnomesu_auth_dialog_response (GtkDialog *dialog, gint response_id)
{
	if (response_id != GTK_RESPONSE_OK)
		clear_entry (GNOMESU_AUTH_DIALOG (dialog)->_priv->input);
	if (GTK_DIALOG_CLASS (parent_class)->response)
		GTK_DIALOG_CLASS (parent_class)->response (dialog, response_id);
}


static void
gnomesu_auth_dialog_class_init (gpointer c, gpointer d)
{
	GtkDialogClass *class = (GtkDialogClass *) c;
	GObjectClass *oclass = (GObjectClass *) c;

	parent_class = g_type_class_peek_parent (class);
	class->response = gnomesu_auth_dialog_response;
	oclass->finalize = gnomesu_auth_dialog_finalize;
}


GType
gnomesu_auth_dialog_get_type ()
{
	static GType class_type = 0;
	if (!class_type) {
		static const GTypeInfo class_info = {
			sizeof (GnomesuAuthDialogClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			gnomesu_auth_dialog_class_init,		/* class_init */
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GnomesuAuthDialog),
			0,		/* n_preallocs */
			gnomesu_auth_dialog_instance_init	/* instance_init */
		};
		class_type = g_type_register_static (GTK_TYPE_DIALOG,
			"GnomesuAuthDialog", &class_info, 0);
	}
	return class_type;
}


GtkWidget *
gnomesu_auth_dialog_new (void)
{
	GtkWidget *dialog;

	dialog = gtk_widget_new (GNOMESU_TYPE_AUTH_DIALOG, NULL);
	gtk_widget_realize (dialog);
	return dialog;
}


GtkWidget *
gnomesu_auth_dialog_add_button (GnomesuAuthDialog *dialog, const char *stock_id,
				const char *label, gint response_id)
{
	GtkWidget *button;

	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog), NULL);

	if (stock_id)
		button = create_stock_button (stock_id, label);
	else
		button = gtk_button_new_with_label (label);
	gnomesu_auth_dialog_add_custom_button (dialog, button, response_id);
	return button;
}


static void
on_action_button_click (GtkWidget *button, gpointer response)
{
	GtkDialog *dialog = GTK_DIALOG (gtk_widget_get_toplevel (button));
	gtk_dialog_response (dialog, GPOINTER_TO_INT (response));
}


void
gnomesu_auth_dialog_add_custom_button (GnomesuAuthDialog *dialog, GtkWidget *button,
					gint response_id)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	gtk_container_add (GTK_CONTAINER (dialog->_priv->left_action_area), button);
	gtk_widget_show (button);
	g_signal_connect (button, "clicked", G_CALLBACK (on_action_button_click),
		GINT_TO_POINTER (response_id));
}


gchar *
gnomesu_auth_dialog_prompt (GnomesuAuthDialog *dialog)
{
	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog), NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
		gnomesu_auth_dialog_set_mode (dialog, GNOMESU_MODE_CHECKING);
		return gnomesu_auth_dialog_get_password (dialog);
	} else
		return NULL;
}


void
gnomesu_auth_dialog_set_desc (GnomesuAuthDialog *dialog, const gchar *text)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (text)
		gtk_label_set_markup (GTK_LABEL (dialog->_priv->desc_label), text);
	else
		gtk_label_set_markup (GTK_LABEL (dialog->_priv->desc_label),
			_("<b>Administrator (root) privilege is required.</b>\n"
			"Please enter the root password to continue."));
}


void
gnomesu_auth_dialog_set_icon (GnomesuAuthDialog *dialog, GdkPixbuf *pixbuf)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (!pixbuf)
		pixbuf = gdk_pixbuf_new_from_inline (sizeof (auth_icon), auth_icon, FALSE, NULL);
	else
		g_object_ref (pixbuf);
	gtk_image_set_from_pixbuf (GTK_IMAGE (dialog->_priv->icon), pixbuf);
	g_object_unref (pixbuf);
}


void
gnomesu_auth_dialog_set_command (GnomesuAuthDialog *dialog, const gchar *command)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (command) {
		gtk_label_set_text (GTK_LABEL (dialog->_priv->command_label), command);
		gtk_widget_show (dialog->_priv->command_desc_label);
		gtk_widget_show (dialog->_priv->command_label);
	} else {
		gtk_widget_hide (dialog->_priv->command_desc_label);
		gtk_widget_hide (dialog->_priv->command_label);
	}
}


void
gnomesu_auth_dialog_set_prompt (GnomesuAuthDialog *dialog, const gchar *prompt)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (prompt) {
		gtk_label_set_text_with_mnemonic (GTK_LABEL (dialog->_priv->prompt_label), prompt);
	} else {
		gtk_label_set_text_with_mnemonic (GTK_LABEL (dialog->_priv->prompt_label), _("_Password:"));
	}
}


static gboolean
stop_loop (GMainLoop *loop)
{
	g_main_loop_quit (loop);
	return FALSE;
}


void
gnomesu_auth_dialog_set_mode (GnomesuAuthDialog *dialog, GnomesuAuthDialogMode mode)
{
	gboolean enabled = TRUE;
	gboolean redraw = TRUE;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	switch (mode) {
	case GNOMESU_MODE_CHECKING:
		gtk_label_set_text (GTK_LABEL (dialog->_priv->mode_label),
			_("Please wait, verifying password..."));
		gtk_widget_show (dialog->_priv->mode_label);
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, dialog->_priv->watch);
		enabled = FALSE;
		break;

	case GNOMESU_MODE_WRONG_PASSWORD:
		gtk_label_set_text (GTK_LABEL (dialog->_priv->mode_label),
			_("Incorrect password, please try again."));
		gtk_widget_show (dialog->_priv->mode_label);
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, NULL);
		break;

	case GNOMESU_MODE_LAST_CHANCE:
		gtk_label_set_text (GTK_LABEL (dialog->_priv->mode_label),
			_("Incorrect password, please try again. "
			"You have one more chance."));
		gtk_widget_show (dialog->_priv->mode_label);
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, NULL);
		break;

	default:
		gtk_widget_hide (dialog->_priv->mode_label);
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, NULL);
		redraw = FALSE;
		break;
	}

	gtk_widget_set_sensitive (dialog->_priv->input, enabled);
	gtk_widget_set_sensitive (dialog->_priv->left_action_area, enabled);
	gtk_widget_set_sensitive (GTK_DIALOG (dialog)->action_area, enabled);
	if (enabled)
		gtk_widget_grab_focus (dialog->_priv->input);


	/* Attempts to immediately redraw the label */
	if (redraw) {
		GMainLoop *loop;

		gtk_widget_queue_draw (GTK_WIDGET (dialog));
		while (gtk_events_pending ())
			gtk_main_iteration ();

		/* Apparently the above isn't enough */
		loop = g_main_loop_new (NULL, FALSE);
		gtk_timeout_add (100, (GtkFunction) stop_loop, loop);
		g_main_loop_run (loop);
		g_main_loop_unref (loop);
	}
}


gchar *
gnomesu_auth_dialog_get_password (GnomesuAuthDialog *dialog)
{
	gchar *password;

	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog), NULL);

	password = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->_priv->input)));
	clear_entry (dialog->_priv->input);
	return password;
}


void
gnomesu_free_password (gchar **password)
{
	size_t len;

	if (!password || !*password)
		return;

	len = strlen (*password);
	safe_memset (*password, ' ', len);
	g_free (*password);
	*password = NULL;
}


G_END_DECLS

#endif /* _GNOMESU_AUTH_DIALOG_C_ */
