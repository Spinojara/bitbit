/* bitbit, a bitboard based chess engine written in c.
 * Copyright (C) 2022-2024 Isak Ellmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdio.h>

#include "move.h"
#include "position.h"

int write_uintx(FILE *f, uint64_t p, size_t x);

int read_uintx(FILE *f, void *p, size_t x);

int write_position(FILE *f, const struct position *pos);

int read_position(FILE *f, struct position *pos);

int write_move(FILE *f, move_t move);

int read_move(FILE *f, move_t *move);

int write_eval(FILE *f, int32_t eval);

int read_eval(FILE *f, int32_t *eval);

#endif