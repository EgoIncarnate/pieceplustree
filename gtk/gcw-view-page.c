/* gcw-view-page.c
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

#include "gcw-view-page.h"

typedef struct
{
  gpointer dummy;
} GcwViewPagePrivate;

enum {
  PROP_0,
  N_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (GcwViewPage, gcw_view_page, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];

static void
gcw_view_page_finalize (GObject *object)
{
  G_OBJECT_CLASS (gcw_view_page_parent_class)->finalize (object);
}

static void
gcw_view_page_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gcw_view_page_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gcw_view_page_class_init (GcwViewPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gcw_view_page_finalize;
  object_class->get_property = gcw_view_page_get_property;
  object_class->set_property = gcw_view_page_set_property;
}

static void
gcw_view_page_init (GcwViewPage *self)
{
}

/**
 * gcw_view_page_new:
 *
 * Creates a new #GcwViewPage.
 *
 * #GcwView renders content into a series of areas called "pages".
 *
 * These pages are then moved around based on the scroll area of the
 * #GcwView. Often times, they'll be pushed into the GPU as a texture
 * to allow easy placement based on render nodes.
 *
 * Returns: A newly created #GcwViewPage
 *
 * Since: 0.2
 */
GcwViewPage *
gcw_view_page_new (void)
{
  return g_object_new (GCW_TYPE_VIEW_PAGE, NULL);
}
