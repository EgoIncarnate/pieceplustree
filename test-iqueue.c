#include "iqueue.h"

static void
test_iqueue_head (void)
{
  IQUEUE_NODE (guint8, 32) node;
  guint8 id;

  IQUEUE_INIT (&node);

  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_HEAD (&node, 10);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_HEAD (&node, 12);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_HEAD (&node, 14);
  g_assert_cmpint (3, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (id, ==, 14);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (id, ==, 12);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (id, ==, 10);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (id, ==, (guint8)-1);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
}

static void
test_iqueue_tail (void)
{
  IQUEUE_NODE (guint8, 32) node;
  guint8 id;

  IQUEUE_INIT (&node);

  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_TAIL (&node, 10);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_TAIL (&node, 12);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_TAIL (&node, 14);
  g_assert_cmpint (3, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (id, ==, 14);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (id, ==, 12);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (id, ==, 10);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (id, ==, (guint8)-1);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
}

static void
test_iqueue_head_tail (void)
{
  IQUEUE_NODE (guint8, 32) node;
  guint8 id;

  IQUEUE_INIT (&node);

  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));

  IQUEUE_PUSH_HEAD (&node, 10);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (node.head, ==, 10);
  g_assert_cmpint (node.tail, ==, 10);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (id, ==, 10);
  g_assert_cmpint (node.head, ==, 0xFF);
  g_assert_cmpint (node.tail, ==, 0xFF);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);


  IQUEUE_PUSH_HEAD (&node, 10);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (node.head, ==, 10);
  g_assert_cmpint (node.tail, ==, 10);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  IQUEUE_PUSH_HEAD (&node, 12);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (node.head, ==, 12);
  g_assert_cmpint (node.tail, ==, 10);
  g_assert_cmpint (node.items[12].prev, ==, 0xFF);
  g_assert_cmpint (node.items[12].next, ==, 10);
  g_assert_cmpint (node.items[10].prev, ==, 12);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  id = IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (id, ==, 10);
  g_assert_cmpint (node.head, ==, 12);
  g_assert_cmpint (node.tail, ==, 12);
  g_assert_cmpint (node.items[12].prev, ==, 0xFF);
  g_assert_cmpint (node.items[12].next, ==, 0xFF);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  IQUEUE_PUSH_TAIL (&node, 10);
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (node.head, ==, 12);
  g_assert_cmpint (node.tail, ==, 10);
  g_assert_cmpint (node.items[12].prev, ==, 0xFF);
  g_assert_cmpint (node.items[12].next, ==, 10);
  g_assert_cmpint (node.items[10].prev, ==, 12);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (id, ==, 12);
  g_assert_cmpint (node.head, ==, 10);
  g_assert_cmpint (node.tail, ==, 10);
  g_assert_cmpint (node.items[12].prev, ==, 0xFF);
  g_assert_cmpint (node.items[12].next, ==, 0xFF);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (id, ==, 10);
  g_assert_cmpint (node.head, ==, 0xFF);
  g_assert_cmpint (node.tail, ==, 0xFF);
  g_assert_cmpint (node.items[12].prev, ==, 0xFF);
  g_assert_cmpint (node.items[12].next, ==, 0xFF);
  g_assert_cmpint (node.items[10].prev, ==, 0xFF);
  g_assert_cmpint (node.items[10].next, ==, 0xFF);

  id = IQUEUE_POP_HEAD (&node);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
  g_assert_cmpint (id, ==, IQUEUE_INVALID (&node));
}

static void
test_iqueue_foreach (void)
{
  IQUEUE_NODE (guint8, 32) node;
  guint count = 0;

  IQUEUE_INIT (&node);
  IQUEUE_PUSH_TAIL (&node, 1);
  IQUEUE_PUSH_TAIL (&node, 2);
  IQUEUE_PUSH_TAIL (&node, 3);
  IQUEUE_PUSH_TAIL (&node, 4);
  IQUEUE_PUSH_TAIL (&node, 5);
  g_assert_cmpint (5, ==, IQUEUE_LENGTH (&node));

  IQUEUE_FOREACH (&node, iter, {
    g_assert_cmpint (iter, ==, ++count);
  });

  g_assert_cmpint (1, ==, IQUEUE_PEEK_HEAD (&node));
  g_assert_cmpint (5, ==, IQUEUE_PEEK_TAIL (&node));
  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 4));

  IQUEUE_POP_TAIL (&node);
  IQUEUE_POP_TAIL (&node);
  IQUEUE_POP_TAIL (&node);
  IQUEUE_POP_TAIL (&node);
  IQUEUE_POP_TAIL (&node);
  g_assert_cmpint (0, ==, IQUEUE_LENGTH (&node));
}

static void
test_iqueue_insert (void)
{
  IQUEUE_NODE (guint8, 32) node;

  IQUEUE_INIT (&node);

  IQUEUE_INSERT (&node, 0, 3);

  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (1, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 1, 5);

  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (2, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 1, 4);

  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (3, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 0, 1);

  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (4, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 1, 2);

  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 4));
  g_assert_cmpint (5, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 3, 9);

  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (9, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 4));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 5));
  g_assert_cmpint (6, ==, IQUEUE_LENGTH (&node));

  IQUEUE_INSERT (&node, 4, 11);

  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (9, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (11, ==, IQUEUE_NTH (&node, 4));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 5));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 6));
  g_assert_cmpint (7, ==, IQUEUE_LENGTH (&node));
}

static void
test_iqueue_move (void)
{
  IQUEUE_NODE (guint8, 32) node;

  IQUEUE_INIT (&node);

  IQUEUE_INSERT (&node, 0, 5);
  IQUEUE_INSERT (&node, 0, 4);
  IQUEUE_INSERT (&node, 0, 3);
  IQUEUE_INSERT (&node, 0, 2);
  IQUEUE_INSERT (&node, 0, 1);

  g_assert_cmpint (1, ==, IQUEUE_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_NTH (&node, 1));
  g_assert_cmpint (3, ==, IQUEUE_NTH (&node, 2));
  g_assert_cmpint (4, ==, IQUEUE_NTH (&node, 3));
  g_assert_cmpint (5, ==, IQUEUE_NTH (&node, 4));

  g_assert_cmpint (node.head, ==, 1);
  g_assert_cmpint (node.items[1].prev, ==, 0xFF);
  g_assert_cmpint (node.items[1].next, ==, 2);
  g_assert_cmpint (node.items[2].prev, ==, 1);
  g_assert_cmpint (node.items[2].next, ==, 3);
  g_assert_cmpint (node.items[3].next, ==, 4);
  g_assert_cmpint (node.items[4].next, ==, 5);

  /* this is testing internal move helpers, this does
   * not use public virtual positions, but raw bucket
   * positions.
   */

  _IQUEUE_MOVE (&node, 1, 31);

  g_assert_cmpint (node.head, ==, 31);
  g_assert_cmpint (node.items[2].prev, ==, 31);
  g_assert_cmpint (node.items[2].next, ==, 3);
  g_assert_cmpint (node.items[3].prev, ==, 2);
  g_assert_cmpint (node.items[3].next, ==, 4);
  g_assert_cmpint (node.items[4].next, ==, 5);
}

static void
test_iqueue_pop_nth (void)
{
  IQUEUE_NODE (guint8, 32) node;

  IQUEUE_INIT (&node);

  IQUEUE_INSERT (&node, 0, 5);
  IQUEUE_INSERT (&node, 0, 4);
  IQUEUE_INSERT (&node, 0, 3);
  IQUEUE_INSERT (&node, 0, 2);
  IQUEUE_INSERT (&node, 0, 1);

  g_assert_cmpint (1, ==, IQUEUE_POP_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_POP_NTH (&node, 0));
  g_assert_cmpint (3, ==, IQUEUE_POP_NTH (&node, 0));
  g_assert_cmpint (4, ==, IQUEUE_POP_NTH (&node, 0));
  g_assert_cmpint (5, ==, IQUEUE_POP_NTH (&node, 0));

  IQUEUE_INSERT (&node, 0, 5);
  IQUEUE_INSERT (&node, 1, 4);
  IQUEUE_INSERT (&node, 2, 3);
  IQUEUE_INSERT (&node, 3, 2);
  IQUEUE_INSERT (&node, 4, 1);

  g_assert_cmpint (1, ==, IQUEUE_POP_NTH (&node, 4));
  g_assert_cmpint (2, ==, IQUEUE_POP_NTH (&node, 3));
  g_assert_cmpint (3, ==, IQUEUE_POP_NTH (&node, 2));
  g_assert_cmpint (4, ==, IQUEUE_POP_NTH (&node, 1));
  g_assert_cmpint (5, ==, IQUEUE_POP_NTH (&node, 0));

  IQUEUE_INSERT (&node, 0, 0);
  IQUEUE_INSERT (&node, 1, 1);
  IQUEUE_INSERT (&node, 2, 2);

  g_assert_cmpint (1, ==, IQUEUE_POP_NTH (&node, 1));
  g_assert_cmpint (0, ==, IQUEUE_POP_NTH (&node, 0));
  g_assert_cmpint (2, ==, IQUEUE_POP_NTH (&node, 0));
}

static void
test_iqueue_full_foreach (void)
{
  IQUEUE_NODE (guint8, 32) node;
  guint count;

  IQUEUE_INIT (&node);

  for (guint8 i = 0; i < 32; i++)
    IQUEUE_PUSH_TAIL (&node, i);

  count = 0;
  IQUEUE_FOREACH (&node, idx, { count += idx; });
  g_assert_cmpint (496, ==, count);

  for (guint8 i = 0; i < 32; i++)
    {
      guint8 v = IQUEUE_POP_NTH (&node, i);
      IQUEUE_INSERT (&node, i, v);
    }

  count = 0;
  IQUEUE_FOREACH (&node, idx, { count += idx; });
  g_assert_cmpint (496, ==, count);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/IQueue/head", test_iqueue_head);
  g_test_add_func ("/IQueue/tail", test_iqueue_tail);
  g_test_add_func ("/IQueue/head_tail", test_iqueue_head_tail);
  g_test_add_func ("/IQueue/foreach", test_iqueue_foreach);
  g_test_add_func ("/IQueue/insert", test_iqueue_insert);
  g_test_add_func ("/IQueue/move", test_iqueue_move);
  g_test_add_func ("/IQueue/pop_nth", test_iqueue_pop_nth);
  g_test_add_func ("/IQueue/full_foreach", test_iqueue_full_foreach);
  return g_test_run ();
}
