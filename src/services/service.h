/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003,2004,2005  Hongli Lai
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

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS


typedef gboolean (*GnomeSuServiceDetectFunc) (const gchar *exe, const gchar *user);
typedef gboolean (*GnomeSuServiceSpawnAsyncFunc2) (const gchar *user, const gchar **argv, GPid *pid,
						GdkPixbuf *icon, const gchar *title, gboolean show_command);


struct _GnomeSuService
{
	GnomeSuServiceDetectFunc detect;
	GnomeSuServiceSpawnAsyncFunc2 spawn_async2;
};
typedef struct _GnomeSuService GnomeSuService;

typedef GnomeSuService * (*GnomeSuServiceConstructor) (void);


G_END_DECLS

#endif /* _SERVICE_H_ */
