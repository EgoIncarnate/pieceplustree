/* iqueue.h
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

#ifndef IQUEUE_H
#define IQUEUE_H

#include <glib.h>
#include <string.h>

G_BEGIN_DECLS

#define IQUEUE_NODE(Type, N_Items) \
  struct {                         \
    Type length;                   \
    Type head;                     \
    Type tail;                     \
    struct {                       \
      Type prev;                   \
      Type next;                   \
    } items[N_Items];              \
  }

#define IQUEUE_INVALID(Node) ((typeof((Node)->head))-1)
#define IQUEUE_LENGTH(Node) ((Node)->length)
#define IQUEUE_EMPTY(Node) ((Node)->head == IQUEUE_INVALID(Node))
#define IQUEUE_PEEK_HEAD(Node) ((Node)->head)
#define IQUEUE_PEEK_TAIL(Node) ((Node)->tail)
#define IQUEUE_IS_VALID(Node, ID) ((ID) != IQUEUE_INVALID(Node))

#define IQUEUE_INIT(Node)                \
  G_STMT_START {                         \
    (Node)->length = 0;                  \
    (Node)->head = IQUEUE_INVALID(Node); \
    (Node)->tail = IQUEUE_INVALID(Node); \
  } G_STMT_END

#ifndef G_DISABLE_ASSERT
# define _IQUEUE_VALIDATE(Node)                                                    \
  G_STMT_START {                                                                  \
    typeof((Node)->head) count = 0;                                               \
                                                                                  \
    g_assert_cmpint((Node)->items[(Node)->tail].next, ==, IQUEUE_INVALID(Node));  \
    g_assert_cmpint((Node)->items[(Node)->head].prev , ==, IQUEUE_INVALID(Node)); \
                                                                                  \
    for (typeof((Node)->head) _iter = (Node)->head;                               \
         IQUEUE_IS_VALID(Node, _iter);                                            \
         _iter = (Node)->items[_iter].next)                                       \
    {                                                                             \
      count++;                                                                    \
    }                                                                             \
                                                                                  \
    g_assert_cmpint(count, ==, (Node)->length);                                   \
  } G_STMT_END
#else
# define _IQUEUE_VALIDATE(Node) G_STMT_START { } G_STMT_END
#endif

#define IQUEUE_PUSH_HEAD(Node, ID)                 \
  G_STMT_START {                                   \
    (Node)->items[ID].prev = IQUEUE_INVALID(Node); \
    (Node)->items[ID].next = (Node)->head;         \
    if (IQUEUE_IS_VALID(Node, (Node)->head))       \
      (Node)->items[(Node)->head].prev = ID;       \
    (Node)->head = ID;                             \
    if (!IQUEUE_IS_VALID(Node, (Node)->tail))      \
      (Node)->tail = ID;                           \
    (Node)->length++;                              \
    _IQUEUE_VALIDATE(Node);                        \
  } G_STMT_END

#define IQUEUE_PUSH_TAIL(Node, ID)                 \
  G_STMT_START {                                   \
    (Node)->items[ID].prev = (Node)->tail;         \
    (Node)->items[ID].next = IQUEUE_INVALID(Node); \
    if (IQUEUE_IS_VALID (Node, (Node)->tail))      \
      (Node)->items[(Node)->tail].next = ID;       \
    (Node)->tail = ID;                             \
    if (!IQUEUE_IS_VALID(Node, (Node)->head))      \
      (Node)->head = ID;                           \
    (Node)->length++;                              \
    _IQUEUE_VALIDATE(Node);                        \
  } G_STMT_END

#define IQUEUE_INSERT(Node, Nth, Val)                                      \
  G_STMT_START {                                                           \
    g_assert_cmpint (IQUEUE_LENGTH(Node), <, G_N_ELEMENTS((Node)->items)); \
                                                                           \
    if ((Nth) == 0)                                                        \
      {                                                                    \
        IQUEUE_PUSH_HEAD(Node, Val);                                       \
      }                                                                    \
    else if ((Nth) == (Node)->length)                                      \
      {                                                                    \
        IQUEUE_PUSH_TAIL(Node, Val);                                       \
      }                                                                    \
    else                                                                   \
      {                                                                    \
        typeof((Node)->head) ID;                                           \
        typeof((Node)->head) _nth;                                         \
                                                                           \
        g_assert_cmpint (IQUEUE_LENGTH(Node), >, 0);                       \
        g_assert (IQUEUE_IS_VALID(Node, (Node)->head));                    \
        g_assert (IQUEUE_IS_VALID(Node, (Node)->tail));                    \
                                                                           \
        for (ID = (Node)->head, _nth = 0;                                  \
             _nth < (Nth) && IQUEUE_IS_VALID(Node, ID);                    \
             ID = (Node)->items[ID].next, ++_nth)                          \
          { /* Do Nothing */ }                                             \
                                                                           \
        g_assert (IQUEUE_IS_VALID(Node, ID));                              \
        g_assert (IQUEUE_IS_VALID(Node, (Node)->items[ID].prev));          \
        g_assert (IQUEUE_IS_VALID(Node, (Node)->items[ID].prev));          \
                                                                           \
        (Node)->items[Val].prev = (Node)->items[ID].prev;                  \
        (Node)->items[Val].next = ID;                                      \
        (Node)->items[(Node)->items[ID].prev].next = Val;                  \
        (Node)->items[ID].prev = Val;                                      \
                                                                           \
        (Node)->length++;                                                  \
                                                                           \
        _IQUEUE_VALIDATE(Node);                                            \
      }                                                                    \
  } G_STMT_END

#define IQUEUE_POP_HEAD(Node) IQUEUE_POP_NTH((Node), 0)
#define IQUEUE_POP_TAIL(Node) IQUEUE_POP_NTH((Node), (Node)->length - 1)

#define IQUEUE_POP_NTH(Node, Nth)                                                  \
  ({                                                                               \
    typeof((Node)->head) _pos = IQUEUE_INVALID(Node);                              \
                                                                                   \
    if (Nth == 0)                                                                  \
      _pos = (Node)->head;                                                         \
    else if (Nth == (((Node)->length) - 1))                                        \
      _pos = (Node)->tail;                                                         \
    else if ((Node)->length > 0)                                                   \
      _pos = IQUEUE_NTH (Node, Nth);                                               \
                                                                                   \
   if (_pos != IQUEUE_INVALID(Node))                                               \
      {                                                                            \
        if ((Node)->items[_pos].prev != IQUEUE_INVALID(Node))                      \
          (Node)->items[(Node)->items[_pos].prev].next = (Node)->items[_pos].next; \
        if ((Node)->items[_pos].next != IQUEUE_INVALID(Node))                      \
          (Node)->items[(Node)->items[_pos].next].prev = (Node)->items[_pos].prev; \
        if ((Node)->head == _pos)                                                  \
          (Node)->head = (Node)->items[_pos].next;                                 \
        if ((Node)->tail == _pos)                                                  \
          (Node)->tail = (Node)->items[_pos].prev;                                 \
                                                                                   \
        (Node)->items[_pos].prev = IQUEUE_INVALID((Node));                         \
        (Node)->items[_pos].next = IQUEUE_INVALID((Node));                         \
                                                                                   \
        (Node)->length--;                                                          \
      }                                                                            \
                                                                                   \
    _pos;                                                                          \
  })

#define IQUEUE_NTH(Node, Nth)                         \
  ({                                                  \
    typeof((Node)->head) _iter;                       \
    typeof((Node)->head) _nth;                        \
    if (Nth == 0)                                     \
      _iter = (Node)->head;                           \
    else if (Nth == (((Node)->length) - 1))           \
      _iter = (Node)->tail;                           \
    else                                              \
      for (_iter = (Node)->head, _nth = 0;            \
           _nth < (Nth);                              \
           _iter = (Node)->items[_iter].next, ++_nth) \
        { /* Do Nothing */ }                          \
    _iter;                                            \
  })

#define IQUEUE_FOREACH(Node, name, IQBlock)        \
  G_STMT_START {                                   \
    for (typeof((Node)->head) name = (Node)->head; \
         name != IQUEUE_INVALID(Node);             \
         name = (Node)->items[name].next)          \
      {                                            \
        IQBlock                                    \
      }                                            \
  } G_STMT_END

#define _IQUEUE_MOVE(Node, Old, New)                     \
  G_STMT_START {                                         \
    (Node)->items[New] = (Node)->items[Old];             \
    if ((Node)->items[New].prev != IQUEUE_INVALID(Node)) \
      (Node)->items[(Node)->items[New].prev].next = New; \
    if ((Node)->items[New].next != IQUEUE_INVALID(Node)) \
      (Node)->items[(Node)->items[New].next].prev = New; \
    if ((Node)->head == Old)                             \
      (Node)->head = New;                                \
    if ((Node)->tail == Old)                             \
      (Node)->tail = New;                                \
  } G_STMT_END

G_END_DECLS

#endif /* IQUEUE_H */
