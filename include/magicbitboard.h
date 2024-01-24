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

#ifndef MAGIC_BITBOARD_H
#define MAGIC_BITBOARD_H

#include <stdint.h>

int magicbitboard_init(void);

extern uint64_t bishop_attacks_lookup[64 * 512];
extern uint64_t rook_attacks_lookup[64 * 4096];

extern uint64_t bishop_magic[64];
extern uint64_t rook_magic[64];

extern uint64_t bishop_mask[64];
extern uint64_t rook_mask[64];

extern uint64_t bishop_full_mask[64];
extern uint64_t rook_full_mask[64];

static inline int bishop_index(int square, uint64_t b) {
	return (((b & bishop_mask[square]) * bishop_magic[square]) >> (64 - 9)) + 512 * square;
}

static inline int rook_index(int square, uint64_t b) {
	return (((b & rook_mask[square]) * rook_magic[square]) >> (64 - 12)) + 4096 * square;
}

#endif
