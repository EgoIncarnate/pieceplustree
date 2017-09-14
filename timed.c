#include "piece-table.h"

#define N_INSERTS 1000000

gint
main (gint argc,
      gchar *argv[])
{
  PieceTable *table = piece_table_new ();
  GTimer *t;
  guint64 *positions = g_new (guint64, N_INSERTS);
  guint64 *lengths = g_new (guint64, N_INSERTS);

  positions[0] = 0;
  lengths[0] = g_random_int_range (1, 32);

  g_print ("Generating commands to run before starting timer.\n");

  for (guint i = 1; i < N_INSERTS; i++)
    {
      positions[i] = g_random_int_range (0, i);
      lengths[i] = g_random_int_range (1, 32);
    }

  g_print ("Starting timer\n");
  t = g_timer_new ();

  for (guint i = 0; i < N_INSERTS; i++)
    {
      piece_table_insert (table, *positions++, PIECE_INITIAL, 0, *lengths++);
    }

  g_print ("Done. %lf seconds\n", g_timer_elapsed (t, NULL));
  g_timer_destroy (t);

  piece_table_free (table);

  return 0;
}
