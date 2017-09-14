/* piece-table.h
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

#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _PieceTable      PieceTable;
typedef struct _PieceTableEntry PieceTableEntry;

typedef enum
{
  PIECE_INITIAL = 0,
  PIECE_CHANGE  = 1,
} PieceKind;

struct _PieceTableEntry
{
  PieceKind kind : 1;
  guint64   offset : 63;
  guint64   length;
};

PieceTable *piece_table_new        (void);
void        piece_table_free       (PieceTable    *self);
guint64     piece_table_get_length (PieceTable    *self);
void        piece_table_insert     (PieceTable    *self,
                                    guint64        position,
                                    PieceKind      kind,
                                    guint64        offset,
                                    guint64        length);
void        piece_table_delete     (PieceTable    *self,
                                    guint64        position,
                                    guint64        length);
void        piece_table_copy       (PieceTable    *self,
                                    guint64        from,
                                    guint64        to,
                                    guint64        length);
void        piece_table_foreach    (PieceTable    *self,
                                    GFunc          func,
                                    gpointer       user_data);
void        piece_table_validate   (PieceTable    *self);

G_END_DECLS

#endif /* PIECE_TABLE_H */
