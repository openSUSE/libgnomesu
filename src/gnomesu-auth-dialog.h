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

#ifndef _GNOMESU_AUTH_DIALOG_H_
#define _GNOMESU_AUTH_DIALOG_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GNOMESU_TYPE_AUTH_DIALOG		(gnomesu_auth_dialog_get_type ())
#define GNOMESU_AUTH_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOMESU_TYPE_AUTH_DIALOG, GnomesuAuthDialog))
#define GNOMESU_AUTH_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GNOMESU_TYPE_AUTH_DIALOG, GnomesuAuthDialogClass))
#define GNOMESU_IS_AUTH_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOMESU_TYPE_AUTH_DIALOG))
#define GNOMESU_IS_AUTH_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), GNOMESU_TYPE_AUTH_DIALOG))
#define GNOMESU_AUTH_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GNOMESU_TYPE_AUTH_DIALOG, GnomesuAuthDialogClass))


typedef struct _GnomesuAuthDialog GnomesuAuthDialog;
typedef struct _GnomesuAuthDialogClass GnomesuAuthDialogClass;


struct _GnomesuAuthDialog {
	GtkDialog parent;

	/*<private>*/
	GtkWidget *left_action_area;
	GtkWidget *icon;
	GtkWidget *desc_label;
	GtkWidget *command_desc_label;
	GtkWidget *command_label;
	GtkWidget *input;
	GtkWidget *mode_label;
};

struct _GnomesuAuthDialogClass {
	GtkDialogClass parent_class;
};


GType      gnomesu_auth_dialog_get_type	(void) G_GNUC_CONST;
GtkWidget *gnomesu_auth_dialog_new	(void);

void gnomesu_auth_dialog_set_desc	(GnomesuAuthDialog *dialog, const gchar *text);
void gnomesu_auth_dialog_set_icon	(GnomesuAuthDialog *dialog, GdkPixbuf *pixbuf);
void gnomesu_auth_dialog_set_command	(GnomesuAuthDialog *dialog, const gchar *command);

typedef enum {
	GNOMESU_MODE_NORMAL,
	GNOMESU_MODE_CHECKING,
	GNOMESU_MODE_WRONG_PASSWORD,
	GNOMESU_MODE_LAST_CHANCE
} GnomesuAuthDialogMode;

void   gnomesu_auth_dialog_set_mode	(GnomesuAuthDialog *dialog, GnomesuAuthDialogMode mode);

gchar *gnomesu_auth_dialog_get_password	(GnomesuAuthDialog *dialog);
void   gnomesu_free_password		(gchar **password);


GtkWidget *gnomesu_auth_dialog_add_button (GnomesuAuthDialog *dialog,
						const char *stock_id,
						const char *label,
						gint response_id);
void       gnomesu_auth_dialog_add_custom_button (GnomesuAuthDialog *dialog,
						GtkWidget *button,
						gint response_id);


G_END_DECLS

#endif /* _GNOMESU_AUTH_DIALOG_H_ */
