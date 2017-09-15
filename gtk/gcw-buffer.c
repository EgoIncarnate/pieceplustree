/* gcw-buffer.c
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

#include "gcw-buffer.h"
#include "gcw-private.h"

typedef struct
{
  /*
   * The priority of the buffer. This is adjusted at runtime based
   * on the focus widget or other operations that need priority.
   */
  gint priority;
} GcwBufferPrivate;

enum {
  PROP_0,
  N_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (GcwBuffer, gcw_buffer, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];

static void
gcw_buffer_finalize (GObject *object)
{
  G_OBJECT_CLASS (gcw_buffer_parent_class)->finalize (object);
}

static void
gcw_buffer_get_property (GObject    *object,
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
gcw_buffer_set_property (GObject      *object,
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
gcw_buffer_class_init (GcwBufferClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gcw_buffer_finalize;
  object_class->get_property = gcw_buffer_get_property;
  object_class->set_property = gcw_buffer_set_property;
}

static void
gcw_buffer_init (GcwBuffer *self)
{
}

/**
 * gcw_buffer_new:
 *
 * Creates a new #GcwBuffer.
 *
 * Returns: The newly created #GcwBuffer.
 *
 * Since: 0.2
 */
GcwBuffer *
gcw_buffer_new (void)
{
  return g_object_new (GCW_TYPE_BUFFER, NULL);
}

/**
 * _gcw_buffer_lower_priority:
 *
 * Lowers the priority of the buffer.
 *
 * This helps ensure that long running background operations are
 * not taking up important CPU cycles while another view has
 * focus (or intense operations pending).
 */
void
_gcw_buffer_lower_priority (GcwBuffer *self)
{
  GcwBufferPrivate *priv = gcw_buffer_get_instance_private (self);

  g_assert (GCW_IS_BUFFER (buffer));

  priv->priority--;
}

/**
 * _gcw_buffer_raise_priority:
 *
 * Raise the priority of the buffer.
 *
 * This helps ensure that a buffer is given higher priority
 * to complete necessary work. You might want the focused buffer
 * to have higher priority than the others so that interacivity
 * is maintained.
 */
void
_gcw_buffer_raise_priority (GcwBuffer *self)
{
  GcwBufferPrivate *priv = gcw_buffer_get_instance_private (self);

  g_assert (GCW_IS_BUFFER (buffer));

  priv->priority++;
}
