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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <glib.h>
#include <glade/glade.h>

/* These functions are private and should not be used by applications */

G_BEGIN_DECLS

#define LGSD(x) __libgnomesu_ ## x

#define glt_add(list, item) list = g_list_append (list, (gpointer) item)
#define glt_addv(list, vec) list = LGSD(g_list_addv) (list, vec)
#define glt_to_vector LGSD(g_list_to_vector)
#define strf g_strdup_printf


gchar * LGSD(create_command)	(gchar **argv);
void    LGSD(replace_all)	(gchar **str, gchar *from, gchar *to);
gchar **LGSD(generate_env)	(gchar *user);

GList  *LGSD(g_list_addv)	(GList *list, gchar **argv);
gchar **LGSD(g_list_to_vector)  (GList *list, guint *size);
guint   LGSD(count_args)	(gchar **argv);

GladeXML *LGSD(load_glade) (gchar *basename);

void LGSD(libgnomesu_init) (void);


G_END_DECLS

#endif /* _UTILS_H_ */
