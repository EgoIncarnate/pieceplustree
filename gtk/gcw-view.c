/* gcw-view.c
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

#include "gcw-view.h"

typedef struct
{
  void *data;
} GcwViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GcwView, gcw_view, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * gcw_view_new:
 *
 * Creates a new #GcwView.
 *
 * Returns: (type Gcw.View): A newly created #GcwView.
 *
 * Since: 0.2
 */
GtkWidget *
gcw_view_new (void)
{
  return g_object_new (GCW_TYPE_VIEW, NULL);
}

static void
gcw_view_finalize (GObject *object)
{
  GcwView *self = (GcwView *)object;
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  G_OBJECT_CLASS (gcw_view_parent_class)->finalize (object);
}

static void
gcw_view_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  GcwView *self = GCW_VIEW (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gcw_view_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  GcwView *self = GCW_VIEW (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gcw_view_class_init (GcwViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gcw_view_finalize;
  object_class->get_property = gcw_view_get_property;
  object_class->set_property = gcw_view_set_property;
}

static void
gcw_view_init (GcwView *self)
{
}
