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

#include "gcw-private.h"
#include "gcw-view.h"

typedef struct
{
  GcwBuffer *buffer;
} GcwViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GcwView, gcw_view, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_BUFFER,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static GcwBuffer *
gcw_view_real_get_buffer (GcwView *self)
{
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  g_assert (GCW_IS_VIEW (self));

  return priv->buffer;
}

static void
gcw_view_real_set_buffer (GcwView   *self,
                          GcwBuffer *buffer)
{
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  g_assert (GCW_IS_VIEW (self));
  g_assert (GCW_IS_BUFFER (buffer));

  if (buffer != priv->buffer)
    {
      if (priv->buffer != NULL)
        _gcw_view_disconnect (self);
      g_assert (priv->buffer == NULL);
      priv->buffer = g_object_ref (buffer);
      _gcw_view_connect (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_BUFFER]);
    }
}

static gboolean
gcw_view_focus_in_event (GtkWidget     *widget,
                         GdkEventFocus *focus)
{
  GcwView *self = GCW_VIEW (widget);
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  if (priv->buffer != NULL)
    _gcw_buffer_raise_priority (priv->buffer);

  return GTK_WIDGET_CLASS (gcw_view_parent_class)->focus_in_event (widget, focus);
}

static gboolean
gcw_view_focus_out_event (GtkWidget     *widget,
                          GdkEventFocus *focus)
{
  GcwView *self = GCW_VIEW (widget);
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  if (priv->buffer != NULL)
    _gcw_buffer_lower_priority (priv->buffer);

  return GTK_WIDGET_CLASS (gcw_view_parent_class)->focus_in_event (widget, focus);
}

static void
gcw_view_destroy (GtkWidget *widget)
{
  GcwView *self = GCW_VIEW (widget);
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  gcw_view_set_buffer (self, NULL);

  GTK_WIDGET_CLASS (gcw_view_parent_class)->destroy (widget);
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
    case PROP_BUFFER:
      g_value_set_object (value, gcw_view_get_buffer (self));
      break;

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
    case PROP_BUFFER:
      gcw_view_set_buffer (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gcw_view_class_init (GcwViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = gcw_view_get_property;
  object_class->set_property = gcw_view_set_property;

  widget_class->destroy = gcw_view_destroy;
  widget_class->focus_in_event = gcw_view_focus_in_event;
  widget_class->focus_out_event = gcw_view_focus_out_event;

  klass->get_buffer = gcw_view_real_get_buffer;
  klass->set_buffer = gcw_view_real_set_buffer;

  properties [PROP_BUFFER] =
    g_param_spec_object ("buffer",
                         "Buffer",
                         "The buffer to be shown in the view",
                         GCW_TYPE_BUFFER,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gcw_view_init (GcwView *self)
{
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  priv->buffer = gcw_buffer_new ();

  _gcw_view_connect (self);
}

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

/**
 * gcw_view_get_buffer:
 * @self: A #GcwBuffer
 *
 * Gets the currently attached #GcwBuffer.
 *
 * Returns: (transfer none) (not nullable): A #GcwBuffer.
 *
 * Since: 0.2
 */
GcwBuffer *
gcw_view_get_buffer (GcwView *self)
{
  g_return_val_if_fail (GCW_IS_VIEW (self), NULL);

  return GCW_VIEW_GET_CLASS (self)->get_buffer (self);
}

/**
 * gcw_view_set_buffer:
 * @self: a #GcwView
 *
 * Clears any previous state from an attached buffer and makes
 * @buffer the currently viewed buffer.
 *
 * Since: 0.2
 */
void
gcw_view_set_buffer (GcwView   *self,
                     GcwBuffer *buffer)
{
  g_autoptr(GcwBuffer) freeme = NULL;

  g_return_if_fail (GCW_IS_VIEW (self));
  g_return_if_fail (!buffer || GCW_IS_BUFFER (buffer));

  if (buffer == NULL)
    buffer = freeme = gcw_buffer_new ();

  GCW_VIEW_GET_CLASS (self)->set_buffer (self, buffer);
}

void
_gcw_view_connect (GcwView *self)
{
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  g_assert (GCW_IS_VIEW (self));
  g_assert (priv->buffer != NULL);

}

void
_gcw_view_disconnect (GcwView *self)
{
  GcwViewPrivate *priv = gcw_view_get_instance_private (self);

  g_assert (GCW_IS_VIEW (self));
  g_assert (priv->buffer != NULL);

  g_clear_object (&priv->buffer);
}
