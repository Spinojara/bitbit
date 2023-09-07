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

#ifndef EVALUATE_H
#define EVALUATE_H

#include <stdint.h>

#include "position.h"

#define VALUE_NONE (0x7FFF)
#define VALUE_INFINITE (0x7FFE)
#define VALUE_MATE (0x7F00)
#define VALUE_WIN (0x1000)
#define VALUE_MATE_IN_MAX_PLY (VALUE_MATE - 128)

enum { mg, eg };

typedef int32_t score_t;

extern const int material_value[7];

#define PHASE (256)
#define NORMAL_SCALE (256)

struct evaluationinfo {
	uint64_t mobility[2];
	uint64_t pawn_attack_span[2];

	uint64_t attacked[2][7];
	uint64_t attacked2[2];

	uint64_t king_ring[2];
	int king_attacks[2][7];

	int32_t material;
	int32_t material_value[2];

	uint64_t pinned[2];

	int king_square[2];

	score_t eval;
};


#define S(m, e) ((int32_t)((m) + ((uint32_t)(e) << 16)))

static inline int32_t score_mg(score_t eval) {
	union {
		uint16_t u;
		int16_t v;
	} ret = { .u = (uint16_t)eval };
	return (int32_t)ret.v;
}

static inline int32_t score_eg(score_t eval) {
	union {
		uint16_t u;
		int16_t v;
	} ret = { .u = (uint32_t)(eval + 0x8000) >> 16 };
	return (int32_t)ret.v;
}

int32_t evaluate_classical(const struct position *pos);

void evaluate_print(struct position *pos);

#endif
