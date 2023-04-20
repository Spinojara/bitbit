/* bitbit, a bitboard based chess engine written in c.
 * Copyright (C) 2022 Isak Ellmer
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NNUE_H
#define NNUE_H

#include <stdint.h>

#include "bitboard.h"
#include "position.h"
#include "move.h"

#define FV_SCALE (16)

struct index {
	int size;
	uint16_t values[30];
};

enum {
	PS_W_PAWN   =  0 * 64,
	PS_B_PAWN   =  1 * 64,
	PS_W_KNIGHT =  2 * 64,
	PS_B_KNIGHT =  3 * 64,
	PS_W_BISHOP =  4 * 64,
	PS_B_BISHOP =  5 * 64,
	PS_W_ROOK   =  6 * 64,
	PS_B_ROOK   =  7 * 64,
	PS_W_QUEEN  =  8 * 64,
	PS_B_QUEEN  =  9 * 64,
	PS_END      = 10 * 64,
};

static uint32_t piece_to_index[2][13] = {
	{ 0, PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, 0,
	     PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, 0, },
	{ 0, PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, 0,
	     PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, 0, },
};

/* should use horizontal symmetry */
static inline int orient(int turn, int square) {
	return turn ? square : 8 * (7 - square / 8) + square % 8;
}

static inline uint16_t make_index(int turn, int square, int piece, int king_square) {
	return orient(turn, square) + piece_to_index[turn][piece] + PS_END * king_square;
}

static inline void append_active_indices(struct position *pos, struct index *active, int turn) {
	active->size = 0;
	int king_square = ctz(pos->piece[turn][king]);
	king_square = orient(turn, king_square);
	uint64_t b;
	int square;
	for (int color = 0; color < 2; color++) {
		for (int piece = 1; piece < 6; piece++) {
			b = pos->piece[color][piece];
			while (b) {
				square = ctz(b);
				active->values[active->size++] = make_index(turn, square, piece + 6 * (1 - color), king_square);
				b = clear_ls1b(b);
			}
		}
	}
}

void update_accumulator(struct position *pos, int16_t (*accumulation)[2][K_HALF_DIMENSIONS], int turn);

void do_refresh_accumulator(struct position *pos, move *m, int turn);

void do_accumulator(struct position *pos, move *m);

void undo_refresh_accumulator(struct position *pos, move *m, int turn);

void undo_accumulator(struct position *pos, move *m);

int nnue_init(int argc, char **argv);

int16_t evaluate_nnue(struct position *pos);

int16_t evaluate_accumulator(struct position *pos);

#endif
