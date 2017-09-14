/* gcw-view.h
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

#include "gcw-buffer.h"

G_BEGIN_DECLS

#define GCW_TYPE_VIEW (gcw_view_get_type())

G_DECLARE_DERIVABLE_TYPE (GcwView, gcw_view, GCW, VIEW, GtkWidget)

struct _GcwViewClass
{
  GtkWidgetClass parent;

  GcwBuffer *(*get_buffer) (GcwView   *self);
  void       (*set_buffer) (GcwView   *self,
                            GcwBuffer *buffer);

	gpointer _reserved[64];
};

GtkWidget *gcw_view_new        (void);
GcwBuffer *gcw_view_get_buffer (GcwView   *self);
void       gcw_view_set_buffer (GcwView   *self,
                                GcwBuffer *buffer);

G_END_DECLS
