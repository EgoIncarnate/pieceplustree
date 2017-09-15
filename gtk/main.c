/* main.c
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

int
main (int argc,
      char *argv[])
{
  GtkWidget *window;
  GtkWidget *scroller;
  GtkWidget *view;

  gtk_init ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), scroller);

  view = gcw_view_new ();
  gtk_container_add (GTK_CONTAINER (scroller), view);

  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_window_present (GTK_WINDOW (window));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_main ();
}
