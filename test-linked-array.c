#include "linked-array.h"

typedef struct
{
  gint negative;
  gint positive;
} Count;

static void
test_basic (void)
{
  LINKED_ARRAY_FIELD(Count, 32) linked_array;
  LINKED_ARRAY_FIELD(Count, 32) split;

  LINKED_ARRAY_INIT (&linked_array);
  LINKED_ARRAY_INIT (&split);

  for (gint i = 0; !LINKED_ARRAY_IS_FULL (&linked_array); i++)
    {
      Count count = { -i, i};
      g_assert (!LINKED_ARRAY_IS_FULL (&linked_array));
      LINKED_ARRAY_INSERT_VAL (&linked_array, i, count);
    }

  g_assert (LINKED_ARRAY_IS_FULL (&linked_array));

  LINKED_ARRAY_FOREACH (&linked_array, Count, count, {
    g_assert_cmpint (count->negative, ==, -count->positive);
  });

  LINKED_ARRAY_SPLIT (&linked_array, &split);

  g_assert_cmpint (LINKED_ARRAY_LENGTH (&linked_array), ==, 16);
  LINKED_ARRAY_FOREACH (&linked_array, Count, count, {
    g_assert_cmpint (count->negative, ==, -count->positive);
  });

  g_assert_cmpint (LINKED_ARRAY_LENGTH (&split), ==, 16);
  LINKED_ARRAY_FOREACH (&split, Count, count, {
    g_assert_cmpint (count->negative, ==, -count->positive);
  });
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/LinkedArray/basic", test_basic);
  return g_test_run ();
}
