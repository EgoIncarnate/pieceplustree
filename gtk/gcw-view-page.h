/* gcw-view-page.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GCW_TYPE_VIEW_PAGE (gcw_view_page_get_type())

G_DECLARE_DERIVABLE_TYPE (GcwViewPage, gcw_view_page, GCW, VIEW_PAGE, GObject)

struct _GcwViewPageClass
{
  GObjectClass parent_class;

  gpointer _reserved[32];
};

GcwViewPage *gcw_view_page_new (void);

G_END_DECLS