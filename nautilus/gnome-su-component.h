/* 
 * Gnome SuperUser
 * Copyright (C) 2003  Hongli Lai
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

#ifndef GNOME_SU_COMPONENT_H
#define GNOME_SU_COMPONENT_H

#include <gtk/gtk.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS


typedef struct _GnomeSuComponent GnomeSuComponent;
typedef struct _GnomeSuComponentClass GnomeSuComponentClass;


#define TYPE_GNOME_SU_COMPONENT		(gnome_su_component_get_type ())
#define GNOME_SU_COMPONENT(obj)		(GTK_CHECK_CAST ((obj), TYPE_GNOME_SU_COMPONENT, GnomeSuComponent))
#define GNOME_SU_COMPONENT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), TYPE_GNOME_SU_COMPONENT, GnomeSuComponentClass))
#define IS_GNOME_SU_COMPONENT(obj)	(GTK_CHECK_TYPE ((obj), TYPE_GNOME_SU_COMPONENT))
#define IS_GNOME_SU_COMPONENT_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), TYPE_GNOME_SU_COMPONENT))


struct _GnomeSuComponent
{
	BonoboObject parent;
};

struct _GnomeSuComponentClass
{
	BonoboObjectClass parent;

	POA_Bonobo_Listener__epv epv;
};


GType gnome_su_component_get_type (void);


G_END_DECLS

#endif /* GNOME_SU_COMPONENT_H */
