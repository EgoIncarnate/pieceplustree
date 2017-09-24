/* piece-table.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "linked-array.h"
#include "piece-table.h"

#define PIECE_TREE_BRANCH_FANOUT (26)
#define PIECE_TREE_LEAF_FANOUT   (26)
#define PIECE_TREE_MAX_LENGTH    (G_MAXUINT64 >> 1)

#ifndef G_DISABLE_ASSERT
# define DEBUG_VALIDATE(a,b) piece_tree_node_validate(a,b)
#else
# define DEBUG_VALIDATE(a,b)
#endif

typedef struct _PieceTreeNodeAny    PieceTreeNodeAny;
typedef struct _PieceTreeNodeBranch PieceTreeNodeBranch;
typedef struct _PieceTreeNodeLeaf   PieceTreeNodeLeaf;
typedef union  _PieceTreeNode       PieceTreeNode;
typedef struct _PieceTreeChild      PieceTreeChild;
typedef struct _PieceTreeInsert     PieceTreeInsert;

#ifndef G_DISABLE_ASSERT
static void
piece_tree_node_validate (PieceTreeNode *node,
                          PieceTreeNode *parent);
#endif

typedef enum
{
  PIECE_TREE_NODE_BRANCH = 0,
  PIECE_TREE_NODE_LEAF   = 1,
} PieceTreeNodeKind;

struct _PieceTreeChild
{
  PieceTreeNode *node;
  guint64        length;
};

struct _PieceTreeNodeAny
{
  /* The parent node, always a BRANCH or %NULL */
  PieceTreeNode *parent;

  /* The kind of node, either BRANCH or LEAF */
  PieceTreeNodeKind kind : 1;
};

struct _PieceTreeNodeBranch
{
  /* Our parent node, alwyas a BRANCH or %NULL */
  PieceTreeNodeBranch *parent;

  /* Our node kind, always a BRANCH */
  PieceTreeNodeKind kind : 1;

  LINKED_ARRAY_FIELD(PieceTreeChild, PIECE_TREE_BRANCH_FANOUT) children;
};

struct _PieceTreeNodeLeaf
{
  /* Our parent node, always a branch */
  PieceTreeNodeBranch *parent;

  /* For this structure, kind will always be PIECE_TREE_NODE_LEAF. */
  PieceTreeNodeKind  kind : 1;

  /* This contains our entries pointing to the data in external bufers.
   * The data is either INITIAL (the original buffer contents) or CHANGE
   * (user edited content).
   */
  LINKED_ARRAY_FIELD(PieceTableEntry, PIECE_TREE_LEAF_FANOUT) entries;

  /* Pointer to the previous and next leaves along the bottom
   * of the tree. This is essentially the linked-leaves in a
   * B+ tree. It allows us to scan without touching the branches
   * when recreating the virtual buffer contents.
   */
  PieceTreeNodeLeaf *prev;
  PieceTreeNodeLeaf *next;
};

/*
 * This is a convenience union so that we can walk nodes without
 * knowing if we're pointing to a BRANCH or LEAF node (until we've
 * arrived at that node and checked).
 */
union _PieceTreeNode
{
  PieceTreeNodeAny    any;
  PieceTreeNodeBranch branch;
  PieceTreeNodeLeaf   leaf;
};

/*
 * This is our user facing structure. They can perform a series of mutations
 * on the table (which is backed by our piece tree), through the public API
 * functions in piece-table.h/
 */
struct _PieceTable
{
  PieceTreeNode root;
  guint64       length;
};

struct _PieceTreeInsert
{
  /* The position in our virtual buffer at which we want to insert */
  guint64 position;

  /* The kind of piece this is. INITIAL buffer or CHANGE buffer */
  PieceKind kind;

  /* The offset within INITIAL or CHANGE buffer */
  guint64 offset;

  /* The run length of the piece from INITIAL or CHANGE */
  guint64 length;
};

static guint64
piece_tree_node_length (PieceTreeNode *node)
{
  guint64 length = 0;

  g_assert (node != NULL);

  if (node->any.kind == PIECE_TREE_NODE_BRANCH)
    {
      LINKED_ARRAY_FOREACH (&node->branch.children, PieceTreeChild, child, {
        length += child->length;
      });
    }
  else
    {
      LINKED_ARRAY_FOREACH (&node->leaf.entries, PieceTableEntry, entry, {
        length += entry->length;
      });
    }

  return length;
}

static inline gboolean
piece_tree_node_is_root (PieceTreeNode *node)
{
  g_assert (node != NULL);

  return node->any.kind == PIECE_TREE_NODE_BRANCH &&
         node->any.parent == NULL;
}

static PieceTreeNode *
piece_tree_node_new (PieceTreeNodeKind kind)
{
  PieceTreeNode *node;

  g_assert (kind == PIECE_TREE_NODE_LEAF || kind == PIECE_TREE_NODE_BRANCH);

  node = g_slice_new (PieceTreeNode);
  node->any.kind = kind;
  node->any.parent = NULL;

  if (kind == PIECE_TREE_NODE_BRANCH)
    LINKED_ARRAY_INIT (&node->branch.children);
  else
    {
      LINKED_ARRAY_INIT (&node->leaf.entries);
      node->leaf.prev = NULL;
      node->leaf.next = NULL;
    }

  return node;
}

static void
piece_tree_node_free (PieceTreeNode *node)
{
  if (node->any.kind == PIECE_TREE_NODE_BRANCH)
    {
      LINKED_ARRAY_FOREACH (&node->branch.children, PieceTreeChild, child, {
        piece_tree_node_free (child->node);
      });
    }

  g_slice_free (PieceTreeNode, node);
}

static inline gboolean
piece_table_entry_chain_head (PieceTableEntry  *entry,
                              PieceTreeInsert *insert)
{
  g_assert (entry != NULL);
  g_assert (insert != NULL);

  if (entry->kind == insert->kind &&
      (insert->offset + insert->length) == entry->offset)
    {
      entry->offset = insert->offset;
      entry->length += insert->length;
      return TRUE;
    }

  return FALSE;
}

static inline gboolean
piece_table_entry_chain_tail (PieceTableEntry *entry,
                              PieceTreeInsert *insert)
{
  g_assert (entry != NULL);
  g_assert (insert != NULL);

  if (entry->kind == insert->kind &&
      (entry->offset + entry->length) == insert->offset)
    {
      entry->length += insert->length;
      return TRUE;
    }

  return FALSE;
}

/**
 * piece_table_get_first_leaf:
 * @self: A #PieceTable
 *
 * This finds the leftmost leaf in the tree and returns it. This can
 * be used to begin scanning the all leaves (using next pointers) so
 * that you may avoid touching intermediate branches.
 *
 * Returns: (not nullable): A #PieceTreeNodeLeaf
 */
static PieceTreeNodeLeaf *
piece_table_get_first_leaf (PieceTable *self)
{
  PieceTreeNodeLeaf *ret = NULL;
  PieceTreeNode *iter;

  g_assert (self != NULL);

  iter = &self->root;

  for (;;)
    {
      if (iter->any.kind == PIECE_TREE_NODE_LEAF)
        {
          ret = &iter->leaf;
          break;
        }

      g_assert (iter->any.kind == PIECE_TREE_NODE_BRANCH);
      g_assert (!LINKED_ARRAY_IS_EMPTY (&iter->branch.children));

      iter = LINKED_ARRAY_PEEK_HEAD (&iter->branch.children).node;
    }

  g_assert (ret->prev == NULL);

  return ret;
}

static inline gboolean
piece_tree_node_needs_split (PieceTreeNode *node)
{
  /*
   * We want to split the tree node if there is not enough space to
   * split a single entry into two AND add a new entry. That means we
   * need two empty slots before we ever perform an insert.
   */

  if (node->any.kind == PIECE_TREE_NODE_BRANCH)
    return LINKED_ARRAY_LENGTH (&node->branch.children) >= (LINKED_ARRAY_CAPACITY (&node->branch.children) - 2);
  else
    return LINKED_ARRAY_LENGTH (&node->leaf.entries) >= (LINKED_ARRAY_CAPACITY (&node->leaf.entries) - 2);
}

/**
 * piece_tree_node_search:
 * @node: A #PieceTreeNode
 * @position: the position, adjusted relative to @node
 * @relative_position: (out): The position adjusted to be relative
 *   to the resulting node.
 *
 */
static PieceTreeNode *
piece_tree_node_search (PieceTreeNode *node,
                        guint64        position,
                        guint64       *relative_position)
{
  PieceTreeChild *last_child = NULL;

  g_assert (node != NULL);
  g_assert (relative_position != NULL);

  if (node->any.kind == PIECE_TREE_NODE_LEAF)
    {
      *relative_position = position;
      return node;
    }

  g_assert (node->any.kind == PIECE_TREE_NODE_BRANCH);
  g_assert (!LINKED_ARRAY_IS_EMPTY (&node->branch.children));

  LINKED_ARRAY_FOREACH (&node->branch.children, PieceTreeChild, child, {
    /* We always want to prefer the left if the item can land between two
     * nodes. That allows us to only have to look right when trying to sink
     * a new PieceTableEntry.
     *
     * The practical effect here is that instead of < we use <=.
     */
    if (position <= child->length)
      return piece_tree_node_search (child->node, position, relative_position);

    position -= child->length;
    last_child = child;
  });

  /* It belongs in the last node (and we've already subtracted the
   * relative length.
   */
  return piece_tree_node_search (last_child->node, position, relative_position);
}

static void
piece_tree_node_split_root (PieceTreeNode *node)
{
  PieceTreeNode *left;
  PieceTreeNode *right;
  PieceTreeChild child;

  g_assert (node->any.kind == PIECE_TREE_NODE_BRANCH);
  g_assert (node->any.parent == NULL);
  g_assert (!LINKED_ARRAY_IS_EMPTY (&node->branch.children));

  left = piece_tree_node_new (PIECE_TREE_NODE_BRANCH);
  right = piece_tree_node_new (PIECE_TREE_NODE_BRANCH);

  LINKED_ARRAY_SPLIT2 (&node->branch.children, &left->branch.children, &right->branch.children);
  LINKED_ARRAY_FOREACH (&left->branch.children, PieceTreeChild, child, {
    child->node->any.parent = left;
  });
  LINKED_ARRAY_FOREACH (&right->branch.children, PieceTreeChild, child, {
    child->node->any.parent = right;
  });

  g_assert (LINKED_ARRAY_IS_EMPTY (&node->branch.children));

  child.node = right;
  child.length = piece_tree_node_length (right);
  LINKED_ARRAY_PUSH_HEAD (&node->branch.children, child);
  right->any.parent = node;

  child.node = left;
  child.length = piece_tree_node_length (left);
  LINKED_ARRAY_PUSH_HEAD (&node->branch.children, child);
  left->any.parent = node;

  g_assert_cmpint (LINKED_ARRAY_LENGTH (&node->branch.children), ==, 2);

  DEBUG_VALIDATE (node, NULL);
  DEBUG_VALIDATE (left, node);
  DEBUG_VALIDATE (right, node);
}

static void
piece_tree_node_split_internal_node (PieceTreeNode *left)
{
  PieceTreeNode *parent;
  PieceTreeNode *right;
  guint64 right_length = 0;
  guint64 left_length = 0;
  guint i = 0;

  g_assert (left != NULL);
  g_assert (left->any.kind == PIECE_TREE_NODE_BRANCH);
  g_assert (left->any.parent != NULL);
  g_assert (left->any.parent->any.kind == PIECE_TREE_NODE_BRANCH);

  /*
   * This operation should not change the height of the tree. Only
   * splitting the root node can change the height of the tree. So
   * here we add a new right node, and update the parent to point to
   * it right after our node.
   *
   * Since no new items are added, lengths do not change and we do
   * not need to update lengths up the hierarchy except for our two
   * effected nodes (and their direct parent).
   */

  parent = left->any.parent;

  /* Create a new node to split half the items into */
  right = piece_tree_node_new (PIECE_TREE_NODE_BRANCH);
  right->any.parent = parent;

  LINKED_ARRAY_SPLIT (&left->branch.children, &right->branch.children);
  LINKED_ARRAY_FOREACH (&right->branch.children, PieceTreeChild, child, {
    child->node->any.parent = right;
  });

  right_length = piece_tree_node_length (right);
  left_length = piece_tree_node_length (left);

  LINKED_ARRAY_FOREACH (&parent->branch.children, PieceTreeChild, child, {
    i++;

    if (child->node == left)
      {
        PieceTreeChild right_child;

        child->length = left_length;

        right_child.node = right;
        right_child.length = right_length;
        LINKED_ARRAY_INSERT_VAL (&parent->branch.children, i, right_child);

        DEBUG_VALIDATE (left, parent);
        DEBUG_VALIDATE (right, parent);

        return;
      }
  });

  g_assert_not_reached ();
}

static void
piece_tree_node_split_leaf (PieceTreeNode *left)
{
  PieceTreeNode *parent;
  PieceTreeNode *right;
  guint64 right_length;
  guint i;

  g_assert (left != NULL);
  g_assert (left->any.kind == PIECE_TREE_NODE_LEAF);
  g_assert (left->any.parent != NULL);

  parent = left->any.parent;

  g_assert (parent != left);
  g_assert (parent->any.kind == PIECE_TREE_NODE_BRANCH);
  g_assert (!LINKED_ARRAY_IS_EMPTY (&parent->branch.children));
  g_assert (!LINKED_ARRAY_IS_FULL (&parent->branch.children));

  DEBUG_VALIDATE (parent, parent->any.parent);
  DEBUG_VALIDATE (left, parent);

  right = piece_tree_node_new (PIECE_TREE_NODE_LEAF);
  right->any.parent = parent;

  right->leaf.prev = &left->leaf;
  right->leaf.next = left->leaf.next;

  left->leaf.next = &right->leaf;

  LINKED_ARRAY_SPLIT (&left->leaf.entries, &right->leaf.entries);

  right_length = piece_tree_node_length (right);

  i = 0;
  LINKED_ARRAY_FOREACH (&parent->branch.children, PieceTreeChild, child, {
    ++i;

    if (child->node == left)
      {
        PieceTreeChild right_child;

        right_child.node = right;
        right_child.length = right_length;
        child->length -= right_length;

        LINKED_ARRAY_INSERT_VAL (&parent->branch.children, i, right_child);

        return;
      }
  });

  g_assert_not_reached ();
}

static void
piece_tree_node_split (PieceTreeNode *node)
{
  g_assert (node != NULL);

  /* First, work our way up to the root and ensure that the parent
   * has also been split if necessary.
   */
  if (node->any.parent != NULL &&
      piece_tree_node_needs_split (node->any.parent))
    piece_tree_node_split (node->any.parent);

  if (node->any.kind == PIECE_TREE_NODE_BRANCH)
    {
      if (piece_tree_node_is_root (node))
        piece_tree_node_split_root (node);
      else
        piece_tree_node_split_internal_node (node);
    }
  else if (node->any.kind == PIECE_TREE_NODE_LEAF)
    piece_tree_node_split_leaf (node);
  else
    g_assert_not_reached ();
}

/*
 * piece_table_insert_full:
 * @self: A PieceTable
 * @node: The current node being inspected
 * @insert: our insert request
 *
 * This is a recursive function used to walk the tree and locate the target
 * leaf to insert into.
 *
 * So that we don't have to update any node other than the parent branches,
 * we pass the calculated offset as we walk down (and then update them as
 * we walk back up the tree).
 *
 * Since the size of the PieceTreeNodeBranch and PieceTreeNodeLeaf are the
 * same, we can change their type if we have to split the nodes because it
 * is out of space.
 */
static void
piece_table_insert_full (PieceTable      *self,
                         PieceTreeInsert *insert)
{
  PieceTableEntry to_insert;
  PieceTreeNode *target;
  PieceTreeNode *node;
  PieceTreeNode *parent;
  guint64 real_position;
  guint i;

  g_assert (self != NULL);
  g_assert (insert != NULL);
  g_assert (insert->length > 0);

  to_insert.kind = insert->kind;
  to_insert.offset = insert->offset;
  to_insert.length = insert->length;

  real_position = insert->position;
  target = piece_tree_node_search (&self->root, insert->position, &insert->position);

  g_assert (target->any.kind == PIECE_TREE_NODE_LEAF);
  g_assert (target->any.parent != NULL);
  g_assert (insert->position <= piece_tree_node_length (target));

  /* We should only hit this if we have an empty tree. */
  if G_UNLIKELY (LINKED_ARRAY_IS_EMPTY (&target->leaf.entries))
    {
      g_assert (insert->position == 0);
      LINKED_ARRAY_PUSH_HEAD (&target->leaf.entries, to_insert);
      goto inserted;
    }

  if (piece_tree_node_needs_split (target))
    {
      /* Split the target into two and then re-locate our position as
       * we might need to be in another node.
       *
       * TODO: Potentially optimization here to look at prev/next to
       *       locate which we need. Complicated though since we don't
       *       have real offsets.
       */
      piece_tree_node_split (target);

      insert->position = real_position;
      target = piece_tree_node_search (&self->root, insert->position, &insert->position);

      g_assert (target->any.kind == PIECE_TREE_NODE_LEAF);
      g_assert (target->any.parent != NULL);
      g_assert_cmpint (insert->position, <=, piece_tree_node_length (target));
    }

  i = 0;
  LINKED_ARRAY_FOREACH (&target->leaf.entries, PieceTableEntry, entry, {
    /*
     * If this insert request would happen immediately after this entry,
     * we want to see if we can chain it to this entry or the beginning
     * of the next entry.
     *
     * Note: We coudld also follow the the B+tree style linked-leaf to
     *       the next leaf and compare against it's first item. But that is
     *       out of scope for this prototype.
     */

      if (insert->position == 0)
        {
          if (!piece_table_entry_chain_head (entry, insert))
            LINKED_ARRAY_INSERT_VAL (&target->leaf.entries, i, to_insert);
          goto inserted;
        }
      else if (insert->position == entry->length)
        {
          PieceTableEntry *next = LINKED_ARRAY_FOREACH_PEEK (&target->leaf.entries);

          /* Try to chain to the end of this entry or the beginning of the next */
          if (!piece_table_entry_chain_tail (entry, insert) &&
              (next == NULL || !piece_table_entry_chain_head (next, insert)))
            LINKED_ARRAY_INSERT_VAL (&target->leaf.entries, i + 1, to_insert);
          goto inserted;
        }
      else if (insert->position < entry->length)
        {
          PieceTableEntry split;

          split.kind = entry->kind;
          split.offset = entry->offset + insert->position;
          split.length = entry->length - insert->position;

          entry->length = insert->position;

          LINKED_ARRAY_INSERT_VAL (&target->leaf.entries, i + 1, to_insert);
          LINKED_ARRAY_INSERT_VAL (&target->leaf.entries, i + 2, split);

          goto inserted;
        }

      insert->position -= entry->length;

      i++;
    });

  g_assert_not_reached ();

inserted:

  /*
   * Now update each of the parent nodes in the tree so that they have
   * an apprporiate length along with the child pointer. This allows them
   * to calculate offsets while walking the tree (without dereferncing the
   * child node) at the cost of us walking back up the tree.
   */
  for (parent = target->any.parent, node = target;
       parent != NULL;
       node = parent, parent = node->any.parent)
    {
      LINKED_ARRAY_FOREACH (&parent->branch.children, PieceTreeChild, child, {
        if (child->node == node)
          {
            child->length += insert->length;
            break;
          }
      });
    }

  self->length += insert->length;
}

/**
 * piece_table_new:
 *
 * Creates a new #PieceTable.
 *
 * The PieceTable is backed by an N-ary B+ tree.
 */
PieceTable *
piece_table_new (void)
{
  PieceTable *self;
  PieceTreeNode *leaf;
  PieceTreeChild child;

  self = g_slice_new0 (PieceTable);
  self->length = 0;

  /* The B+Tree has a root node (a branch) and a single leaf
   * as a child to simplify how we do splits/rotations/etc.
   */
  leaf = piece_tree_node_new (PIECE_TREE_NODE_LEAF);
  leaf->any.parent = &self->root;

  child.node = leaf;
  child.length = 0;

  self->root.any.kind = PIECE_TREE_NODE_BRANCH;
  LINKED_ARRAY_INIT (&self->root.branch.children);
  LINKED_ARRAY_PUSH_HEAD (&self->root.branch.children, child);

  return self;
}

void
piece_table_free (PieceTable *self)
{
  if (self != NULL)
    {
      g_assert (self->root.any.kind == PIECE_TREE_NODE_BRANCH);
      g_assert (!LINKED_ARRAY_IS_EMPTY (&self->root.branch.children));

      LINKED_ARRAY_FOREACH (&self->root.branch.children, PieceTreeChild, child, {
        piece_tree_node_free (child->node);
      });

      g_slice_free (PieceTable, self);
    }
}

void
piece_table_insert (PieceTable *self,
                    guint64     position,
                    PieceKind   kind,
                    guint64     offset,
                    guint64     length)
{
  PieceTreeInsert insert;

  g_return_if_fail (self != NULL);
  g_return_if_fail (position >= 0);
  g_return_if_fail (kind == PIECE_INITIAL || kind == PIECE_CHANGE);
  g_return_if_fail (offset >= 0);
  g_return_if_fail (length >= 0);
  g_return_if_fail (length <= (PIECE_TREE_MAX_LENGTH - self->length));

  if (length == 0)
    return;

  insert.kind = kind;
  insert.offset = offset;
  insert.length = length;
  insert.position = position;

  piece_table_insert_full (self, &insert);
}

/**
 * piece_table_foreach:
 * @self: A #PieceTable
 * @func: (scope call) (closure user_data): A callback for each entry
 * @user_data: closure data for @func
 *
 * This function will invoke @func for each entry in the piece table.
 * The first parameter @func will be a #PieceTableEntry and must not
 * be modified.
 *
 * It is a programming error to modify the table from @func.
 */
void
piece_table_foreach (PieceTable *self,
                     GFunc       func,
                     gpointer    user_data)
{
  PieceTreeNodeLeaf *leaf;

  g_return_if_fail (self != NULL);
  g_return_if_fail (func != NULL);

  for (leaf = piece_table_get_first_leaf (self);
       leaf != NULL;
       leaf = leaf->next)
    {
      g_assert (leaf->next == NULL || leaf->next->prev == leaf);

      LINKED_ARRAY_FOREACH (&leaf->entries, PieceTableEntry, entry, {
        func (entry, user_data);
      });
    }
}

guint64
piece_table_get_length (PieceTable *self)
{
  g_return_val_if_fail (self != NULL, 0L);

  return self->length;
}

#ifndef G_DISABLE_ASSERT
static void
piece_tree_node_validate (PieceTreeNode *node,
                          PieceTreeNode *parent)
{
  g_assert (node != NULL);
  g_assert (node->any.parent == parent);
  g_assert (!parent || parent->any.kind == PIECE_TREE_NODE_BRANCH);
  g_assert (!parent || !LINKED_ARRAY_IS_EMPTY (&parent->branch.children));

  /* Make sure our parent contains a pointer to us */
  if (parent != NULL)
    {
      LINKED_ARRAY_FOREACH (&parent->branch.children, PieceTreeChild, child, {
        if (child->node == node)
          goto found;
      });
      g_assert_not_reached ();
    }
found:

  for (PieceTreeNode *iter = node->any.parent;
       iter != NULL;
       iter = iter->any.parent)
    g_assert (iter->any.kind == PIECE_TREE_NODE_BRANCH);

  if (node->any.kind == PIECE_TREE_NODE_BRANCH)
    {
      LINKED_ARRAY_FOREACH (&node->branch.children, PieceTreeChild, child, {
        g_assert (child->node != NULL);
        g_assert_cmpint (child->length, ==, piece_tree_node_length (child->node));
        g_assert (child->node->any.parent == node);

        //piece_tree_node_validate (child->node, node);
      });
    }
  else if (node->any.kind == PIECE_TREE_NODE_LEAF)
    {
      LINKED_ARRAY_FOREACH (&node->leaf.entries, PieceTableEntry, entry, {
        g_assert (entry->length > 0);
      });

#if 0
      if (node->leaf.next != NULL)
        {
          g_assert (node->leaf.next->kind == PIECE_TREE_NODE_LEAF);
          g_assert (node->leaf.next->prev == &node->leaf);
        }

      if (node->leaf.prev != NULL)
        {
          g_assert (node->leaf.prev->kind == PIECE_TREE_NODE_LEAF);
          g_assert (node->leaf.prev->next == &node->leaf);
        }
#endif
    }
  else
    g_assert_not_reached ();
}
#endif

void
piece_table_validate (PieceTable *self)
{
#ifndef G_DISABLE_ASSERT
  PieceTreeNodeLeaf *left;
  guint64 length = 0;

  g_assert (self != NULL);
  g_assert (self->root.any.kind == PIECE_TREE_NODE_BRANCH);

  piece_tree_node_validate (&self->root, NULL);

  g_assert (!LINKED_ARRAY_IS_EMPTY (&self->root.branch.children));

  left = piece_table_get_first_leaf (self);
  g_assert (left->prev == NULL);

  length = piece_tree_node_length (&self->root);
  g_assert_cmpint (self->length, ==, length);
#endif
}
