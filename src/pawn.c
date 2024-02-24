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

#include "pawn.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "bitboard.h"
#include "util.h"
#include "option.h"
#include "texeltune.h"

CONST score_t supported_pawn     = S( 12, 14);
CONST score_t backward_pawn[4]   = { S(  0,  3), S(-12,-11), S( -8, -4), S( -6, -2), };
CONST score_t isolated_pawn[4]   = { S( -8,  0), S( -5, -9), S(  0, -5), S(-11, -7), };
CONST score_t doubled_pawn[4]    = { S(-17,-36), S( 21,-23), S( -1, -9), S(-10,-19), };
CONST score_t connected_pawn[7]  = { S(  0,  0), S(  2,  3), S(  5,  4), S(  5,  4), S( 10,  9), S( 29, 37), S( 40, 77), };
CONST score_t passed_pawn[7]     = { S(  0,  0), S( 50, 38), S( 30, 43), S(-40, 77), S(-26,114), S(199,174), S(428,289), };
CONST score_t passed_blocked[7]  = { S(  0,  0), S(  2, -1), S(-11,  1), S(-12, -8), S(  4,-20), S(  6,-30), S(-71,-70), };
CONST score_t passed_file[4]     = { S( -7,  1), S(-23, -3), S(-28,-15), S(-26,-35), };
CONST score_t distance_us[7]     = { S(  0,  0), S(  5, -6), S( 12, -6), S( 23,-15), S( 15,-17), S(-15,-19), S(-33,-20), };
CONST score_t distance_them[7]   = { S(  0,  0), S(-12,  1), S(-15,  2), S(-12,  9), S( -4, 18), S( -4, 29), S( 10, 32), };

/* Mostly inspiration from stockfish. */
score_t evaluate_pawns(const struct position *pos, struct evaluationinfo *ei, int us) {
	const int them = other_color(us);
	const unsigned up = us ? N : S;

	UNUSED(ei);
	score_t eval = 0;

	const int up_sq = us ? 8 : -8;
	const int down_sq = -up_sq;

	uint64_t ourpawns = pos->piece[us][PAWN];
	uint64_t theirpawns = pos->piece[them][PAWN];
	uint64_t b = pos->piece[us][PAWN];
	uint64_t neighbours, doubled, stoppers, support, phalanx, lever, leverpush, blocker;
	int backward, passed;
	int square;
	uint64_t squareb;
	while (b) {
		square = ctz(b);
		squareb = bitboard(square);

		int r = rank_of(orient_horizontal(us, square));
		int f = file_of(square);
		int rf = min(f, 7 - f);
		
		/* uint64_t */
		doubled    = ourpawns & bitboard(square + down_sq);
		neighbours = ourpawns & adjacent_files(square);
		stoppers   = theirpawns & passed_files(square, us);
		blocker    = theirpawns & bitboard(square + up_sq);
		support    = neighbours & rank(square + down_sq);
		phalanx    = neighbours & rank(square);
		lever      = theirpawns & shift(shift(squareb, E) | shift(squareb, W), up);
		leverpush  = theirpawns & shift_twice(shift(squareb, E) | shift(squareb, W), up);

		/* int */
		backward   = !(neighbours & passed_files(square + up_sq, them)) && (leverpush | blocker);
		passed     = !(stoppers ^ lever) || (!(stoppers ^ lever ^ leverpush) && popcount(phalanx) >= popcount(leverpush));
		passed    &= !(passed_files(square, us) & file(square) & ourpawns);

		if (support | phalanx) {
			eval += connected_pawn[r] * (2 + (phalanx != 0)) + supported_pawn * popcount(support);
			if (TRACE) trace.supported_pawn[us] += popcount(support);
			if (TRACE) trace.connected_pawn[us][r] += 2 + (phalanx != 0);
		}

		if (passed) {
			eval += passed_pawn[r] + passed_file[rf];
			if (TRACE) trace.passed_pawn[us][r]++;
			if (TRACE) trace.passed_file[us][rf]++;

			int i = distance(square + up_sq, ctz(pos->piece[us][KING]));
			eval += i * distance_us[r];
			if (TRACE) trace.distance_us[us][r] += i;
			int j = distance(square + up_sq, ctz(pos->piece[them][KING]));
			eval += j * distance_them[r];
			if (TRACE) trace.distance_them[us][r] += j;

			if (pos->mailbox[square + up_sq]) {
				eval += passed_blocked[r];
				if (TRACE) trace.passed_blocked[us][r]++;
			}
		}
		else if (!neighbours) {
			eval += isolated_pawn[rf];
			if (TRACE) trace.isolated_pawn[us][rf]++;
		}
		if (doubled) {
			eval += doubled_pawn[rf];
			if (TRACE) trace.doubled_pawn[us][rf]++;
		}
		
		if (backward) {
			eval += backward_pawn[rf];
			if (TRACE) trace.backward_pawn[us][rf]++;
		}

		b = clear_ls1b(b);
	}

	return eval;
}
