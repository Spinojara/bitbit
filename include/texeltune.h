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

#ifndef TEXELTUNE_H
#define TEXELTUNE_H

#include "evaluate.h"

#ifdef TRACE
#undef TRACE
#define TRACE 1
#define CONST
#else
#define TRACE 0
#define CONST const
#endif

struct trace {
	int mobility[2][4][28];
	int pawn_shelter[2][28];
	int unblocked_storm[2][28];
	int unblockable_storm[2][28];
	int blocked_storm[2][28];

	int king_on_open_file[2];
	int knight_outpost[2];
	int knight_outpost_attack[2];
	int bishop_outpost[2];
	int bishop_outpost_attack[2];
	int bishop_long_diagonal[2];
	int knight_behind_pawn[2];
	int bishop_behind_pawn[2];
	int defended_knight[2];
	int defended_bishop[2];
	int knight_far_from_king[2];
	int bishop_far_from_king[2];
	int knight_pair[2];
	int bishop_pair[2];
	int rook_pair[2];
	int pawn_blocking_bishop[2];
	int rook_open[2];
	int rook_semi[2];
	int rook_closed[2];
	int rook_blocked[2];
	int bad_queen[2];
	int king_attack_pawn[2];
	int king_defend_pawn[2];

	int pawn_threat[2];
	int push_threat[2];
	int minor_threat[2][7];
	int rook_threat[2][7];

	int weak_squares[2];
	int knight_attack[2];
	int bishop_attack[2];
	int rook_attack[2];
	int queen_attack[2];
	int discovery[2];
	int checks[2][12];
	int enemy_no_queen[2];

	int supported_pawn[2];
	int backward_pawn[2][4];
	int isolated_pawn[2][4];
	int doubled_pawn[2][4];
	int connected_pawn[2][7];
	int passed_pawn[2][7];
	int passed_blocked[2][7];
	int passed_file[2][4];
	int distance_us[2][7];
	int distance_them[2][7];

	int king_danger[2];
	int material;

	int eval;

	int p;
	int s;
};

extern struct trace trace;

extern CONST score_t king_on_open_file;
extern CONST score_t knight_outpost;
extern CONST score_t knight_outpost_attack;
extern CONST score_t bishop_outpost;
extern CONST score_t bishop_outpost_attack;
extern CONST score_t bishop_long_diagonal;
extern CONST score_t knight_behind_pawn;
extern CONST score_t bishop_behind_pawn;
extern CONST score_t defended_knight;
extern CONST score_t defended_bishop;
extern CONST score_t knight_far_from_king;
extern CONST score_t bishop_far_from_king;
extern CONST score_t knight_pair;
extern CONST score_t bishop_pair;
extern CONST score_t rook_pair;
extern CONST score_t pawn_blocking_bishop;
extern CONST score_t rook_open;
extern CONST score_t rook_semi;
extern CONST score_t rook_closed;
extern CONST score_t rook_blocked;
extern CONST score_t bad_queen;
extern CONST score_t king_attack_pawn;
extern CONST score_t king_defend_pawn;
extern CONST score_t tempo_bonus;

extern CONST score_t pawn_threat;
extern CONST score_t push_threat;
extern CONST score_t minor_threat[7];
extern CONST score_t rook_threat[7];

extern CONST int weak_squares;
extern CONST int enemy_no_queen;
extern CONST int knight_attack;
extern CONST int bishop_attack;
extern CONST int rook_attack;
extern CONST int queen_attack;
extern CONST int discovery;
extern CONST int checks[12];

extern CONST int phase_max;
extern CONST int phase_min;
extern CONST int phase_knight;
extern CONST int phase_bishop;
extern CONST int phase_rook;
extern CONST int phase_queen;

extern CONST score_t supported_pawn;
extern CONST score_t backward_pawn[4];
extern CONST score_t isolated_pawn[4];
extern CONST score_t doubled_pawn[4];
extern CONST score_t connected_pawn[7];
extern CONST score_t passed_pawn[7];
extern CONST score_t passed_blocked[7];
extern CONST score_t passed_file[4];
extern CONST score_t distance_us[7];
extern CONST score_t distance_them[7];

#endif
