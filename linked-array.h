/* linked-array.h
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

#ifndef LINKED_ARRAY_H
#define LINKED_ARRAY_H

#include <glib.h>
#include <string.h>

#include "iqueue.h"

G_BEGIN_DECLS

/**
 * LINKED_ARRAY_FIELD:
 * @TYPE: The type of the structure used by elements in the array
 * @N_ITEMS: The maximum number of items in the array
 *
 * This creates a new inline structure that can be embedded within
 * other super-structures.
 *
 * @N_ITEMS must be <= 254 or this macro will fail.
 */
#define LINKED_ARRAY_FIELD(TYPE,N_ITEMS)        \
  struct {                                      \
    TYPE items[N_ITEMS <= 0xFE ? N_ITEMS : -1]; \
    IQUEUE_NODE(guint8, N_ITEMS) q;             \
  }

/**
 * LINKED_ARRAY_INIT:
 * @FIELD: A pointer to a LinkedArray
 *
 * This will initialize a node that has been previously registered
 * using %LINKED_ARRAY_FIELD(). You must call this macro before
 * using the LinkedArray structure.
 */
#define LINKED_ARRAY_INIT(FIELD)            \
  G_STMT_START {                            \
    memset ((FIELD), 0, sizeof (*(FIELD))); \
    IQUEUE_INIT(&(FIELD)->q);               \
  } G_STMT_END

/**
 * LINKED_ARRAY_LENGTH:
 * @FIELD: A pointer to the LinkedArray field.
 *
 * This macro will evaluate to the number of items inserted into
 * the LinkedArray.
 */
#define LINKED_ARRAY_LENGTH(FIELD) (IQUEUE_LENGTH(&(FIELD)->q))

/**
 * LINKED_ARRAY_CAPACITY:
 * @FIELD: A pointer to the LinkedArray field.
 *
 * This macro will evaluate to the number of elements in the LinkedArray.
 * This is dependent on how the LinkedArray was instantiated using
 * the %LINKED_ARRAY_FIELD() macro.
 */
#define LINKED_ARRAY_CAPACITY(FIELD) (G_N_ELEMENTS((FIELD)->items))

/**
 * LINKED_ARRAY_IS_FULL:
 * @FIELD: A pointer to the LinkedArray field.
 *
 * This macro will evaluate to 1 if the LinkedArray is at capacity.
 * Otherwise, the macro will evaluate to 0.
 */
#define LINKED_ARRAY_IS_FULL(FIELD) (LINKED_ARRAY_LENGTH(FIELD) == LINKED_ARRAY_CAPACITY(FIELD))

/**
 * LINKED_ARRAY_IS_EMPTY:
 * @FIELD: A LinkedArray field
 *
 * This macro will evaluate to 1 if the LinkedArray contains zero children.
 */
#define LINKED_ARRAY_IS_EMPTY(FIELD) (LINKED_ARRAY_LENGTH(FIELD) == 0)

/**
 * LINKED_ARRAY_INSERT_VAL:
 * @FIELD: A pointer to a LinkedArray field.
 * @POSITION: the logical position at which to insert
 * @ELEMENT: The element to insert
 *
 * This will insert a new item into the array. It is invalid API use
 * to call this function while the LinkedArray is at capacity. Check
 * LINKED_ARRAY_IS_FULL() befure using this function to be certain.
 */
#define LINKED_ARRAY_INSERT_VAL(FIELD,POSITION,ELEMENT)     \
  G_STMT_START {                                            \
    guint8 _pos;                                            \
                                                            \
    g_assert (POSITION >= 0);                               \
    g_assert (POSITION <= LINKED_ARRAY_LENGTH(FIELD));      \
                                                            \
    _pos = IQUEUE_LENGTH(&(FIELD)->q);                      \
    (FIELD)->items[_pos] = ELEMENT;                         \
    IQUEUE_INSERT(&(FIELD)->q, POSITION, _pos);             \
  } G_STMT_END

#define LINKED_ARRAY_REMOVE_INDEX(FIELD,POSITION)                  \
  ({                                                               \
    typeof((FIELD)->items[0]) _ele;                                \
    guint8 _pos;                                                   \
    guint8 _len;                                                   \
                                                                   \
    _pos = IQUEUE_POP_NTH(&(FIELD)->q, POSITION);                  \
    _ele = (FIELD)->items[_pos];                                   \
    _len = IQUEUE_LENGTH(&(FIELD)->q);                             \
                                                                   \
    /* We must preserve our invariant of having no empty gaps      \
     * in the array so that se can place new items always at the   \
     * end (to avoid scanning for an empty spot).                  \
     * Therefore we move our tail item into the removed slot and   \
     * adjust the iqueue positions (which are all O(1).            \
     */                                                            \
                                                                   \
    if (_pos < _len)                                               \
      {                                                            \
        (FIELD)->items[_pos] = (FIELD)->items[_len];               \
        _IQUEUE_MOVE(&(FIELD)->q, _len, _pos);                     \
      }                                                            \
                                                                   \
    _ele;                                                          \
  })

/**
 * LINKED_ARRAY_FOREACH:
 * @FIELD: A pointer to a LinkedArray
 * @Element: The type of the elements in @FIELD
 * @Name: the name for a pointer of type @Element
 * @LABlock: a {} tyle block to execute for each item. You may use
 *    "break" to exit the foreach.
 *
 * Calls @Block for every element stored in @FIELD. A pointer to
 * each element will be provided as a variable named @Name.
 */
#define LINKED_ARRAY_FOREACH(FIELD, Element, Name, LABlock) \
  G_STMT_START {                                            \
    for (typeof((FIELD)->q.head) _aiter = (FIELD)->q.head;  \
         _aiter != IQUEUE_INVALID(&(FIELD)->q);             \
         _aiter = (FIELD)->q.items[_aiter].next)            \
      {                                                     \
        Element * Name = &(FIELD)->items[_aiter];           \
        LABlock                                             \
      }                                                     \
  } G_STMT_END

#define LINKED_ARRAY_FOREACH_PEEK(FIELD)                          \
  (((FIELD)->q.items[_aiter].next != IQUEUE_INVALID(&(FIELD)->q)) \
    ? &(FIELD)->items[(FIELD)->q.items[_aiter].next] : NULL)

#define LINKED_ARRAY_SPLIT(FIELD, SPLIT)                    \
  G_STMT_START {                                            \
    guint8 mid;                                             \
                                                            \
    LINKED_ARRAY_INIT(SPLIT);                               \
                                                            \
    mid = LINKED_ARRAY_LENGTH(FIELD) / 2;                   \
                                                            \
    for (guint8 i = 0; i < mid; i++)                        \
      {                                                     \
        typeof((FIELD)->items[0]) ele;                      \
        ele = LINKED_ARRAY_POP_TAIL(FIELD);                 \
        LINKED_ARRAY_PUSH_HEAD(SPLIT, ele);                 \
      }                                                     \
  } G_STMT_END

#define LINKED_ARRAY_SPLIT2(FIELD, LEFT, RIGHT)             \
  G_STMT_START {                                            \
    guint8 mid;                                             \
                                                            \
    LINKED_ARRAY_INIT(LEFT);                                \
    LINKED_ARRAY_INIT(RIGHT);                               \
                                                            \
    mid = LINKED_ARRAY_LENGTH(FIELD) / 2;                   \
                                                            \
    for (guint8 i = 0; i < mid; i++)                        \
      {                                                     \
        typeof((FIELD)->items[0]) ele;                      \
        ele = LINKED_ARRAY_POP_TAIL(FIELD);                 \
        LINKED_ARRAY_PUSH_HEAD(RIGHT, ele);                 \
      }                                                     \
                                                            \
    while (!LINKED_ARRAY_IS_EMPTY(FIELD))                   \
      {                                                     \
        typeof((FIELD)->items[0]) ele;                      \
        ele = LINKED_ARRAY_POP_TAIL(FIELD);                 \
        LINKED_ARRAY_PUSH_HEAD(LEFT, ele);                  \
      }                                                     \
  } G_STMT_END

#define LINKED_ARRAY_PEEK_HEAD(FIELD) ((FIELD)->items[IQUEUE_PEEK_HEAD(&(FIELD)->q)])
#define LINKED_ARRAY_POP_HEAD(FIELD) LINKED_ARRAY_REMOVE_INDEX(FIELD, 0)
#define LINKED_ARRAY_POP_TAIL(FIELD) LINKED_ARRAY_REMOVE_INDEX(FIELD, LINKED_ARRAY_LENGTH(FIELD)-1)

#define LINKED_ARRAY_PUSH_HEAD(FIELD, ele)                    \
  G_STMT_START {                                              \
    guint8 _pos = IQUEUE_LENGTH(&(FIELD)->q);                 \
    g_assert_cmpint (_pos, <, G_N_ELEMENTS ((FIELD)->items)); \
    (FIELD)->items[_pos] = ele;                               \
    IQUEUE_PUSH_HEAD(&(FIELD)->q, _pos);                      \
  } G_STMT_END

G_END_DECLS

#endif /* LINKED_ARRAY_H */
