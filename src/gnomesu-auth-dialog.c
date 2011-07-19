/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2004 Hongli Lai
 * Copyright (C) 2007 David Zeuthen <david@fubar.dk>
 * Copyright (C) 2008 Novell, Inc.
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

#undef GTK_DISABLE_DEPRECATED
#include <string.h>
#include <libintl.h>

#include "gnomesu-auth-dialog.h"
#include "utils.h"

#undef _
#define _(x) ((gchar *) dgettext (GETTEXT_PACKAGE, x))

G_BEGIN_DECLS

struct _GnomesuAuthDialogPrivate {
	GdkCursor *watch;
	GtkWidget *left_action_area;
	GtkWidget *message_label;
	GtkWidget *message_label_secondary;
	GtkWidget *user_combobox;
	GtkWidget *prompt_label;
	GtkWidget *command_desc_label;
	GtkWidget *command_label;
	GtkWidget *password_entry;
	GtkWidget *details_expander;
	GtkWidget *icon;
};

G_DEFINE_TYPE (GnomesuAuthDialog, gnomesu_auth_dialog, GTK_TYPE_DIALOG);

// ============================================================================
// PRIVATE UTILITIES
// ============================================================================

static GList *
split_lines_strip_markup (const gchar *text, gssize length, gint max_lines)
{
	GList *lines = NULL;
	gint line_count = 0;
	GString *str;
	const gchar *p;
	const gchar *end;
	gint ignore_ref = 0;

	g_return_val_if_fail (text != NULL, NULL);

	if (length < 0)
		length = strlen (text);

	str = g_string_sized_new (length);

	p = text;
	end = text + length;

	while (p != end) {
		const gchar *next;
		next = g_utf8_next_char (p);

		switch (*p) {
			case '<': ignore_ref++; break;
			case '>': ignore_ref--; break;
			default:
				if (ignore_ref <= 0) {
					if (*p == '\n' && (max_lines <= 0 || line_count < max_lines - 1)) {
						lines = g_list_append (lines, g_string_free (str, FALSE));
						str = g_string_sized_new (end - p + 1);
						line_count++;
						break;
					}

					g_string_append_len (str, p, next - p);
				}
				break;
		}

		p = next;
	}

	lines = g_list_append (lines, g_string_free (str, FALSE));
	return lines;
}

static void
gnomesu_auth_dialog_set_icon_internal (GnomesuAuthDialog *auth_dialog, GdkPixbuf *vendor_pixbuf)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *copy_pixbuf;

	gint icon_size = 48, half_size;
	gint v_width, v_height;
	gint v_scale_width, v_scale_height;
	gint ofs_x, ofs_y;
	gdouble s_x, s_y;

	pixbuf = NULL;
	copy_pixbuf = NULL;
	half_size = icon_size / 2;

	pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		GTK_STOCK_DIALOG_AUTHENTICATION, icon_size, 0, NULL);
	if (pixbuf == NULL)
		goto out;

	if (vendor_pixbuf == NULL) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (auth_dialog->_priv->icon), pixbuf);
		goto out;
	}

	/* need to copy the pixbuf since we're modifying it */
	copy_pixbuf = gdk_pixbuf_copy (pixbuf);
	if (copy_pixbuf == NULL)
		goto out;

	/* compute scaling and translating for overlay */
	v_width = gdk_pixbuf_get_width (vendor_pixbuf);
	v_height = gdk_pixbuf_get_height (vendor_pixbuf);
	v_scale_width = v_width <= half_size ? v_width : half_size;
	v_scale_height = v_height <= half_size ? v_height : half_size;
	s_x = v_width <= half_size ? 1.0 : half_size / (gdouble)v_width;
	s_y = v_height <= half_size ? 1.0 : half_size / (gdouble)v_height;
	ofs_x = half_size + (half_size - v_scale_width);
	ofs_y = half_size + (half_size - v_scale_height);

	/* blend the vendor icon in the bottom right quarter */
	gdk_pixbuf_composite (vendor_pixbuf,
		copy_pixbuf,
		ofs_x, ofs_y, v_scale_width, v_scale_height,
		ofs_x, ofs_y, s_x, s_y,
		GDK_INTERP_BILINEAR,
		255);

	gtk_image_set_from_pixbuf (GTK_IMAGE (auth_dialog->_priv->icon), copy_pixbuf);

out:
	if (pixbuf != NULL)
		g_object_unref (pixbuf);
	if (copy_pixbuf != NULL)
		g_object_unref (copy_pixbuf);
	if (vendor_pixbuf != NULL)
		g_object_unref (vendor_pixbuf);
}

static GtkWidget *
add_row (GtkWidget *table, int row, const char *label_text, gboolean centered, GtkWidget *entry)
{
	GtkWidget *label;

	label = gtk_label_new_with_mnemonic (label_text);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, centered ? 0.5 : 0.0);

	gtk_table_attach (GTK_TABLE (table), label,
		0, 1, row, row + 1,
		GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (table), entry,
		1, 2, row, row + 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

	return label;
}

static GtkWidget *
create_stock_button (const gchar *stock, const gchar *labelstr)
{
	GtkWidget *button;
	GtkWidget *align, *hbox, *image, *label;

	button = gtk_button_new ();
	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (button), align);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
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
#if GTK_CHECK_VERSION(2,18,0)
	/* With GTK+ 2.18, GtkEntry uses a GtkEntryBuffer which cleans the
	 * memory for passwords. See trash_area() in gtkentrybuffer.c.
	 * The code below actually creates some bugs, like
	 * https://bugzilla.novell.com/show_bug.cgi?id=351917 so we shouldn't
	 * use it, if possible. */
	gtk_entry_set_text (GTK_ENTRY (entry), "");
#else
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
#endif
}

void
indicate_auth_error (GnomesuAuthDialog *auth_dialog)
{
	gint x, y, n;

	gtk_window_get_position (GTK_WINDOW (auth_dialog), &x, &y);

	for (n = 0; n < 10; n++) {
		gint diff = n % 2 == 0 ? -15 : 15;
		gtk_window_move (GTK_WINDOW (auth_dialog), x + diff, y);

		while (gtk_events_pending ()) {
			gtk_main_iteration ();
		}

		g_usleep (10000);
	}

	gtk_window_move (GTK_WINDOW (auth_dialog), x, y);
}

// ============================================================================
// GTK CLASS/OBJECT INITIALIZING
// ============================================================================

static gboolean
delete_event_handler (GtkWindow *window, gpointer user_data)
{
	return TRUE;
}

static void
gnomesu_auth_dialog_init (GnomesuAuthDialog *auth_dialog)
{
	GtkDialog *dialog = GTK_DIALOG (auth_dialog);
	GnomesuAuthDialogPrivate *priv;
	GtkWidget *action_area_parent;
	GtkWidget *left_action_area;
	GtkWidget *default_button;
	GtkWidget *image;
	GtkWidget *action_area;
	GtkWidget *dialog_vbox;

	action_area = gtk_dialog_get_action_area (dialog);
	dialog_vbox = gtk_dialog_get_content_area (dialog);

	priv = auth_dialog->_priv = g_new0 (GnomesuAuthDialogPrivate, 1);

	priv->watch = gdk_cursor_new (GDK_WATCH);

	gtk_dialog_add_button (dialog, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	default_button = gtk_dialog_add_button (dialog, _("C_ontinue"), GTK_RESPONSE_OK);
	image = gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image (GTK_BUTTON (default_button), image);

	gtk_widget_set_can_default (default_button, TRUE);
	gtk_dialog_set_default_response (dialog, GTK_RESPONSE_OK);
	gtk_window_set_default (GTK_WINDOW (dialog), default_button);

	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_box_set_spacing (GTK_BOX (dialog_vbox), 2); /* 2 * 5 + 2 = 12 */
	gtk_container_set_border_width (GTK_CONTAINER (action_area), 5);
	gtk_box_set_spacing (GTK_BOX (action_area), 6);

	GtkWidget *hbox, *main_vbox, *vbox;

	/* Reparent dialog->action_area into a hbox */
	g_object_ref (action_area);
	action_area_parent = gtk_widget_get_parent (action_area);
	gtk_container_remove (GTK_CONTAINER (action_area_parent), action_area);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
	gtk_box_pack_end (GTK_BOX (action_area_parent), hbox, FALSE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (action_area_parent), hbox, -1);

	/* Add another (left-aligned) button box to the dialog */
	left_action_area = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_container_set_border_width (GTK_CONTAINER (left_action_area), 6);
	/* gtk_button_box_set_spacing (GTK_BUTTON_BOX (left_action_area), 12); */
	gtk_box_set_spacing (GTK_BOX (left_action_area), 12);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (left_action_area), GTK_BUTTONBOX_START);
	priv->left_action_area = left_action_area;
	gtk_box_pack_start (GTK_BOX (hbox), left_action_area, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (hbox), action_area, TRUE, TRUE, 0);
	g_object_unref (action_area);
	gtk_widget_show_all (action_area_parent);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, TRUE, TRUE, 0);

	priv->icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_AUTHENTICATION, GTK_ICON_SIZE_DIALOG);
	gtk_misc_set_alignment (GTK_MISC (priv->icon), 0.5, 0.0);
	gtk_box_pack_start (GTK_BOX (hbox), priv->icon, FALSE, FALSE, 0);

	main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start (GTK_BOX (hbox), main_vbox, TRUE, TRUE, 0);

	/* main message */
	priv->message_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (priv->message_label), 0.0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (priv->message_label), TRUE);
	gtk_box_pack_start (GTK_BOX (main_vbox), GTK_WIDGET (priv->message_label),
				FALSE, FALSE, 0);

	/* secondary message */
	priv->message_label_secondary = gtk_label_new (NULL);
		gtk_label_set_markup (GTK_LABEL (priv->message_label_secondary), "");
	gtk_misc_set_alignment (GTK_MISC (priv->message_label_secondary), 0.0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (priv->message_label_secondary), TRUE);
	gtk_box_pack_start (GTK_BOX (main_vbox), GTK_WIDGET (priv->message_label_secondary),
				FALSE, FALSE, 0);

	/* password entry */
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);

	GtkWidget *table_alignment;
	GtkWidget *table;
	table_alignment = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
	gtk_box_pack_start (GTK_BOX (vbox), table_alignment, FALSE, FALSE, 0);
	table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_container_add (GTK_CONTAINER (table_alignment), table);
	priv->password_entry = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (priv->password_entry), FALSE);
	gtk_entry_set_activates_default (GTK_ENTRY (priv->password_entry), TRUE);
	priv->prompt_label = add_row (table, 0, "", TRUE, priv->password_entry);

	priv->details_expander = gtk_expander_new_with_mnemonic (_("<small><b>_Details</b></small>"));
	gtk_expander_set_use_markup (GTK_EXPANDER (priv->details_expander), TRUE);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), priv->details_expander, FALSE, FALSE, 0);

	GtkWidget *details_vbox;
	details_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_add (GTK_CONTAINER (priv->details_expander), details_vbox);

	table_alignment = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
	gtk_box_pack_start (GTK_BOX (details_vbox), table_alignment, FALSE, FALSE, 0);
	table = gtk_table_new (1, 3, FALSE);

	gint expander_size;
	gtk_widget_style_get (priv->details_expander,
		"expander-size", &expander_size,
		NULL);

	gtk_alignment_set_padding (GTK_ALIGNMENT (table_alignment), 6, 0, 2 * expander_size, 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_container_add (GTK_CONTAINER (table_alignment), table);

	/* Command Label */
	priv->command_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (priv->command_label), 0, 0);
	gtk_label_set_selectable (GTK_LABEL (priv->command_label), TRUE);

	GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), priv->command_label);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (gtk_bin_get_child (GTK_BIN (scroll))), GTK_SHADOW_NONE);

	gchar *msg = g_markup_printf_escaped ("<small><b>%s</b></small>", _("Command:"));
	priv->command_desc_label = add_row (table, 0, msg, FALSE, scroll);
	g_free (msg);

	gtk_widget_show_all (dialog_vbox);
	gtk_widget_grab_default (default_button);

	/* Configure */
	gnomesu_auth_dialog_set_desc (auth_dialog, NULL);
	gnomesu_auth_dialog_set_icon (auth_dialog, NULL);
	gnomesu_auth_dialog_set_command (auth_dialog, NULL);
	gnomesu_auth_dialog_set_prompt (auth_dialog, NULL);
	gnomesu_auth_dialog_set_mode (auth_dialog, GNOMESU_MODE_NORMAL);
}

static void
gnomesu_auth_dialog_response (GtkDialog *dialog, gint response_id)
{
	if (response_id != GTK_RESPONSE_OK)
		clear_entry (GNOMESU_AUTH_DIALOG (dialog)->_priv->password_entry);

	if (GTK_DIALOG_CLASS (gnomesu_auth_dialog_parent_class)->response)
		GTK_DIALOG_CLASS (gnomesu_auth_dialog_parent_class)->response (dialog, response_id);
}

static void
gnomesu_auth_dialog_finalize (GObject *object)
{
	GnomesuAuthDialog *auth_dialog = GNOMESU_AUTH_DIALOG (object);

	g_free (auth_dialog->_priv);

	G_OBJECT_CLASS (gnomesu_auth_dialog_parent_class)->finalize (object);
}

static void
gnomesu_auth_dialog_class_init (GnomesuAuthDialogClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gnomesu_auth_dialog_parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

	gobject_class->finalize = gnomesu_auth_dialog_finalize;
	GTK_DIALOG_CLASS (klass)->response = gnomesu_auth_dialog_response;
}

// ============================================================================
// PUBLIC API
// ============================================================================

GtkWidget *
gnomesu_auth_dialog_new ()
{
	GnomesuAuthDialog *auth_dialog;
	GtkWindow *window;

	auth_dialog = g_object_new (GNOMESU_TYPE_AUTH_DIALOG, NULL);
	window = GTK_WINDOW (auth_dialog);

	gtk_window_set_position (window, GTK_WIN_POS_CENTER);
	gtk_window_set_modal (window, TRUE);
	gtk_window_set_resizable (window, FALSE);
	gtk_window_set_keep_above (window, TRUE);
	gtk_window_set_resizable (window, FALSE);
	gtk_window_set_icon_name (window, GTK_STOCK_DIALOG_AUTHENTICATION);

	gchar *title = g_strdup (_("Password needed"));
	LGSD (replace_all) (&title, "needed", "Needed");
	gtk_window_set_title (window, title);
	g_free (title);

	g_signal_connect (auth_dialog, "delete-event",
		G_CALLBACK (delete_event_handler), NULL);

	return GTK_WIDGET (auth_dialog);
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
gnomesu_auth_dialog_set_desc_ps (GnomesuAuthDialog *dialog, const gchar *primary, const gchar *secondary)
{
	const gchar *_primary = primary ? primary : /*_("Administrator (root) privilege is required.");*/
		 _("The requested action needs further authentication.");
	const gchar *_secondary = secondary ? secondary : _("Please enter the root password to continue.");
	gchar *msg, *mod_primary = NULL;
	gssize p_len = strlen (_primary);

	/* Try to make the header string nicer to read and more HIGgy;
	   ugly hack, but beats breaking all the translations */
	if (p_len > 2) {
		mod_primary = g_strdup (_primary);
		if (mod_primary[p_len - 2] != '.' && mod_primary[p_len - 1] == '.') {
			mod_primary[p_len - 1] = '\0';
		}

		// LGSD (replace_all) (&mod_primary, " (root)", "");
	}

	msg = g_markup_printf_escaped ("<big><b>%s</b></big>", mod_primary ? mod_primary : _primary);
	gtk_label_set_markup (GTK_LABEL (dialog->_priv->message_label), msg);
	g_free (msg);
	g_free (mod_primary);

	gtk_label_set_text (GTK_LABEL (dialog->_priv->message_label_secondary), _secondary);

	/* Force both labels to be as wide as their parent; one of them will decide the width */
	GtkRequisition requisition;
	gtk_widget_size_request (gtk_widget_get_parent (dialog->_priv->message_label), &requisition);
	gtk_widget_set_size_request (dialog->_priv->message_label, requisition.width, -1);
	gtk_widget_set_size_request (dialog->_priv->message_label_secondary, requisition.width, -1);
}

void
gnomesu_auth_dialog_set_desc (GnomesuAuthDialog *dialog, const gchar *text)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (text) {
		GList *lines = split_lines_strip_markup (text, -1, 2);
		gchar *line1 = NULL;
		gchar *line2 = NULL;

		if (lines) {
			line1 = (gchar *)lines->data;
			if (lines->next) {
				line2 = (gchar *)lines->next->data;
			}

			g_list_free (lines);
		}

		gnomesu_auth_dialog_set_desc_ps (dialog, line1, line2);
		g_free (line1);
		g_free (line2);
	} else {
		gnomesu_auth_dialog_set_desc_ps (dialog, NULL, NULL);
	}
}

void
gnomesu_auth_dialog_set_icon (GnomesuAuthDialog *dialog, GdkPixbuf *pixbuf)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (pixbuf)
		g_object_ref (pixbuf);

	gnomesu_auth_dialog_set_icon_internal (dialog, pixbuf);
}

void
gnomesu_auth_dialog_set_command (GnomesuAuthDialog *dialog, const gchar *command)
{
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (command) {
		gchar *msg = g_markup_printf_escaped ("<small><i>%s</i></small>", command);
		gtk_label_set_markup (GTK_LABEL (dialog->_priv->command_label), msg);
		g_free (msg);

		gtk_widget_show (dialog->_priv->details_expander);
	} else {
		gtk_widget_hide (dialog->_priv->details_expander);
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

void
gnomesu_auth_dialog_set_mode (GnomesuAuthDialog *dialog, GnomesuAuthDialogMode mode)
{
	gboolean enabled = TRUE;
	gboolean redraw = TRUE;
	GdkWindow *window;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog));

	if (!gtk_widget_get_realized (GTK_WIDGET (dialog))) {
		gtk_widget_realize (GTK_WIDGET (dialog));
	}

	window = gtk_widget_get_window (GTK_WIDGET (dialog));

	switch (mode) {
	case GNOMESU_MODE_CHECKING:
		gdk_window_set_cursor (window, dialog->_priv->watch);
		enabled = FALSE;
		break;

	case GNOMESU_MODE_WRONG_PASSWORD:
	case GNOMESU_MODE_LAST_CHANCE:
		gdk_window_set_cursor (window, NULL);
		indicate_auth_error (dialog);
		break;

	default:
		gdk_window_set_cursor (window, NULL);
		redraw = FALSE;
		break;
	}

	gtk_widget_set_sensitive (dialog->_priv->password_entry, enabled);
	gtk_widget_set_sensitive (dialog->_priv->left_action_area, enabled);
	gtk_widget_set_sensitive (gtk_dialog_get_action_area (GTK_DIALOG (dialog)), enabled);

	if (enabled)
		gtk_widget_grab_focus (dialog->_priv->password_entry);
}

gchar *
gnomesu_auth_dialog_get_password (GnomesuAuthDialog *dialog)
{
	gchar *password;

	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOMESU_IS_AUTH_DIALOG (dialog), NULL);

	password = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->_priv->password_entry)));
	clear_entry (dialog->_priv->password_entry);
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
