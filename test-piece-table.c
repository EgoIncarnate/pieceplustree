#include <string.h>

#include "piece-table.h"

static void
collect_entries (gpointer data,
                 gpointer user_data)
{
  const PieceTableEntry *entry = data;
  GArray *ar = user_data;

  g_array_append_vals (ar, entry, 1);
}

static void
compare_entries (PieceTable            *table,
                 const PieceTableEntry *entries,
                 guint                  n_entries)
{
  g_autoptr(GArray) ar = g_array_new (FALSE, FALSE, sizeof *entries);

  piece_table_foreach (table, collect_entries, ar);

  if (ar->len != n_entries)
    {
      g_print ("\n");

      for (guint i = 0; i < MAX (ar->len, n_entries); i++)
        {
          if (i < ar->len)
            {
              const PieceTableEntry *e1 = &g_array_index(ar, PieceTableEntry, i);
              g_print ("> %u: %u: %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT"\n",
                       i, e1->kind, (guint64)e1->offset, (guint64)e1->length);
            }
          else
            g_print ("> %u: .....\n", i);

          if (i < n_entries)
            {
              const PieceTableEntry *e2 = &entries[i];
              g_print ("< %u: %u: %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT"\n",
                       i, e2->kind, (guint64)e2->offset, (guint64)e2->length);
            }
          else
            g_print ("< %u: .....\n", i);
        }
    }

  g_assert_cmpint (ar->len, ==, n_entries);

  for (guint i = 0; i < n_entries; i++)
    {
      PieceTableEntry *ar_entry = &g_array_index (ar, PieceTableEntry, i);

#if 0
      g_print ("%d: %d %d %d\n",
               i,
               (gint)ar_entry->kind,
               (gint)ar_entry->offset,
               (gint)ar_entry->length);
#endif

      g_assert_cmpint (entries[i].kind, ==, ar_entry->kind);
      g_assert_cmpint (entries[i].offset, ==, ar_entry->offset);
      g_assert_cmpint (entries[i].length, ==, ar_entry->length);
    }
}

static void
test_basic (void)
{
  PieceTable *table;

  table = piece_table_new ();

  piece_table_insert (table, 0, PIECE_CHANGE, 0, 0);
  piece_table_insert (table, 0, PIECE_INITIAL, 0, 1024);
  g_assert_cmpint (1024, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_INITIAL, 0, 1024 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 512, PIECE_CHANGE, 0, 1024);
  g_assert_cmpint (2048, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_INITIAL, 0, 512 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 512 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 0, PIECE_CHANGE, 91, 100);
  g_assert_cmpint (2148, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 91, 100 },
      { PIECE_INITIAL, 0, 512 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 512 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 0, PIECE_CHANGE, 90, 1);
  g_assert_cmpint (2149, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 90, 101 },
      { PIECE_INITIAL, 0, 512 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 512 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 2149, PIECE_INITIAL, 1024, 100);
  g_assert_cmpint (2249, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 90, 101 },
      { PIECE_INITIAL, 0, 512 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 612 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 2249, PIECE_CHANGE, 0, 100);
  g_assert_cmpint (2349, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 90, 101 },
      { PIECE_INITIAL, 0, 512 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 612 },
      { PIECE_CHANGE, 0, 100 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_insert (table, 2349, PIECE_CHANGE, 10, 15);
  g_assert_cmpint (2364, ==, piece_table_get_length (table));

  piece_table_insert (table, 2349, PIECE_CHANGE, 10, 15);
  g_assert_cmpint (2379, ==, piece_table_get_length (table));

  piece_table_insert (table, 256, PIECE_CHANGE, 9000, 1);
  g_assert_cmpint (2380, ==, piece_table_get_length (table));

  piece_table_insert (table, 2380-15, PIECE_CHANGE, 5, 5);
  g_assert_cmpint (2385, ==, piece_table_get_length (table));

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 90, 101 },
      { PIECE_INITIAL, 0, 155 },
      { PIECE_CHANGE, 9000, 1 },
      { PIECE_INITIAL, 155, 357 },
      { PIECE_CHANGE, 0, 1024 },
      { PIECE_INITIAL, 512, 612 },
      { PIECE_CHANGE, 0, 100 },
      { PIECE_CHANGE, 10, 15 },
      { PIECE_CHANGE, 5, 20 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_validate (table);
  piece_table_free (table);
}

static void
test_inserts_at_head (void)
{
  PieceTable *table;

  table = piece_table_new ();

  for (guint i = 10000; i > 0; i--)
    piece_table_insert (table, 0, PIECE_CHANGE, i, 1);

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 1, 10000 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_validate (table);
  piece_table_free (table);
}

static void
test_inserts_at_tail (void)
{
  PieceTable *table;

  table = piece_table_new ();

  for (guint i = 0; i < 10000; i++)
    piece_table_insert (table, i, PIECE_CHANGE, i, 1);

  {
    static const PieceTableEntry entries[] = {
      { PIECE_CHANGE, 0, 10000 },
    };
    compare_entries (table, entries, G_N_ELEMENTS (entries));
  }

  piece_table_validate (table);
  piece_table_free (table);
}

static void
test_root_split (void)
{
  PieceTable *table = piece_table_new ();

  for (guint i = 0; i < 10000; i++)
    piece_table_insert (table, 0, PIECE_CHANGE, i, 1);

  piece_table_validate (table);
  piece_table_free (table);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/PieceTable/basic", test_basic);
  g_test_add_func ("/PieceTable/inserts_at_head", test_inserts_at_head);
  g_test_add_func ("/PieceTable/inserts_at_tail", test_inserts_at_tail);
  g_test_add_func ("/PieceTable/test_root_split", test_root_split);
  return g_test_run ();
}
