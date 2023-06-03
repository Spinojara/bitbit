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

#include "evaluate.h"

#include <string.h>

#include "init.h"

mevalue psqtable[2][7][64];

mevalue piece_value[6] = { S(pawn_mg, pawn_eg), S(knight_mg, knight_eg), S(bishop_mg, bishop_eg), S(rook_mg, rook_eg), S(queen_mg, queen_eg), S(0, 0) };

/* from white's perspective, files a to d on a regular board */
mevalue white_psqtable[6][64] = {
	{ /* pawn */
		S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
		S( 98, 26), S( 53, 64), S( 64, 62), S( 83, 45), S( 53, 54), S( 21, 57), S( -9, 60), S( 27, 12),
		S( 28, 21), S(  4, 34), S( 11, 34), S( 29, 24), S( 31, 20), S( 32, 27), S( 18, 30), S( 35, 12),
		S( -8, -1), S(-18, -2), S(-24, -5), S( -8,-17), S(-20,-12), S( -2, -1), S( -8, -5), S(  1,-13),
		S(-19,-20), S(-26,-13), S(-11,-17), S( -5,-18), S(-13,-18), S(  0,-10), S(-22,-15), S(-15,-23),
		S(-18,-23), S(-18,-22), S(-12,-14), S( -2,-22), S(  7,-13), S( -6, -9), S( 25,-24), S( -9,-26),
		S(-24, -8), S( -7,-14), S(-11, -6), S( -3,-15), S(-16,  0), S( 20, -5), S( 26,-19), S( -9,-26),
		S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
	}, { /* knight */
		S(-108,-32), S(-26, -7), S(-41, 18), S(-14,  4),
		S( -3, -9), S(-12,  4), S( 19,  0), S(  7, 12),
		S(-12,-14), S( 11, -6), S( 40,  5), S( 58, -2),
		S( 23,-13), S(  0, -1), S( 15,  6), S(  8,  4),
		S( -3,-14), S(-12, -8), S( -4, -2), S( -2, -4),
		S(-26,-26), S(  0,-24), S(-17,-26), S(-13,-13),
		S(-29,-25), S(-16,-20), S(-23,-35), S(-10,-27),
		S(-56,-32), S(-19,-35), S(-20,-41), S(-22,-32),
	}, { /* bishop */
		S( -8,  6), S( -9, -6), S(-34,-14), S(-24,-17),
		S(-40, -3), S(-12, -9), S( -9,-11), S( -8,-14),
		S(  6,-13), S( 31,-23), S( 29,-14), S( 26,-15),
		S(-14,-12), S(  6,-10), S(  7,-14), S( 16, -3),
		S(  9,-23), S(-12,-21), S(-14,-11), S( 12,-13),
		S( 10,-26), S( 11,-23), S( -2,-16), S( -7,-12),
		S( 14,-30), S( 12,-30), S(  8,-29), S(-14,-20),
		S(-15,-20), S( -1,-29), S(-25,-21), S(  0,-30),
	}, { /* rook */
		S( 29,-19), S(  7, -6), S(  5, -5), S( 13, -4),
		S(  3, -4), S(  3, -8), S( 26,-10), S( 47,-11),
		S( -7, -9), S( 18,-14), S( 27,-16), S( 29,-15),
		S(-22,-12), S(-17,-16), S(  8,-16), S( 13,-15),
		S(-28,-21), S(-21,-19), S(-26,-16), S(-20,-22),
		S(-41,-35), S(-29,-35), S(-27,-34), S(-16,-39),
		S(-58,-42), S(-31,-41), S(-13,-43), S( -4,-45),
		S( -7,-44), S(-20,-41), S( -7,-38), S(  5,-40),
	}, { /* queen */
		S(-17,-41), S(  3,-42), S( -5,-29), S(-15,-31),
		S( -6,-27), S(-45,-11), S( -9,-14), S(-20,  0),
		S(  6,-36), S( -3,-21), S(-10,  3), S( -6, -1),
		S( -4,-31), S( -7, -9), S(-16, -5), S(-21,  6),
		S( -2,-43), S( -8,-16), S(-12,-18), S(-18, -4),
		S( -2,-55), S( 11,-37), S(  1,-28), S( -5,-24),
		S(-24,-61), S(  8,-59), S( 13,-59), S(  5,-45),
		S(  4,-69), S(-23,-77), S(-13,-79), S( -4,-56),
	}, { /* king */
		S(-86,-98), S(-78, -6), S(-76, -8), S(-72,  0),
		S(-44, -9), S(-93,-12), S(-64, -5), S(-55,  0),
		S(-34, -6), S(-57,  6), S(-50,  3), S(-51,  5),
		S(-15,-28), S(-19,-11), S(-30, -2), S(-36,  2),
		S(-30,-37), S(-26,-18), S( -8, -6), S(-22,  4),
		S(  7,-36), S( -7,-19), S(-13, -8), S(-19,  1),
		S( 23,-39), S( -9,-16), S( 13,-15), S(  8,-13),
		S( 18,-51), S( 31,-38), S( 25,-38), S( 54,-50),
	}
};

mevalue pawn_shelter[4][7] = {
	{
		S( -8,  2), S( 24,-27), S( 47,-22), S( 29,-18), S( 15,  2), S( 25, 14), S( 36, 23),
	}, {
		S(-33,  2), S( 16,-23), S(  6,-25), S(-17,-21), S(-18, -3), S(-14, 10), S(-30, 12),
	}, {
		S(-20, -4), S( 22,-14), S( -2,-11), S(-15,-14), S(-15, -7), S( -9, -2), S(  1, 13),
	}, {
		S(-48,-12), S(-23,-19), S(-31,-19), S(-38,-10), S(-40,  0), S(-72, -2), S(-45,  6),
	}
};

mevalue unblocked_storm[4][7] = {
	{
		S(-19,-32), S( 28,196), S( 10,148), S(-49, 28), S(-20,-20), S( -3,-30), S(-20,-26),
	}, {
		S(-24,-21), S( 40,134), S(-27, 93), S(-24,  9), S(-21,-12), S(  8,-20), S( -8,-20),
	}, {
		S( -7,-20), S(-38, 85), S(-52, 64), S(-30, 14), S(-10, -8), S( -5,-13), S( -1,-20),
	}, {
		S(-17,-20), S( 13,107), S(-33, 65), S(-21,  3), S( -9,-15), S(  8,-14), S( -9, -5),
	}
};

mevalue blocked_storm[7] = {
	S(  0,  0), S(-26,-18), S( -6,-21), S(  4,-30), S( -6,-42), S( -9,-54), S(  4,  0),
};

mevalue mobility_bonus[4][28] = {
	{	/* idx 0 to 8 */
		S(-77,-104), S(-42,-81), S(-14,-39), S(-11,-17), S(  0, -8), S(  7, -4), S( 16,  2), S( 25,  2), S( 33,-11),
	}, {	/* idx 0 to 13 */
		S(-59,-88), S(-22,-65), S( -9,-25), S( -3, -2), S(  5, 10), S( 11, 26), S( 16, 30), S( 22, 35), S( 26, 34),
		S( 28, 33), S( 29, 36), S( 45, 20), S( 61, 29), S( 68,  1),
	}, {	/* idx 0 to 14 */
		S(-72,-92), S(-35,-74), S( -8,-46), S( -3,-18), S( -5,-11), S( -8, -4), S(-12,  5), S( -5,  5), S( -2, 12),
		S(  4, 16), S( 14, 20), S( 28, 18), S( 33, 17), S( 39, 14), S( 66,  1),
	}, {	/* idx 0 to 27 */
		S(-11,-20), S(-21,-28), S(-13,-32), S(  3,-30), S(  7,-36), S( 11,-36), S( 11,-28), S( 11,-13), S( 10,-11),
		S(  9,  5), S( 10, 20), S( 12, 19), S( 13, 26), S( 17, 30), S( 17, 29), S( 23, 26), S( 19, 30), S( 21, 29),
		S( 21, 31), S( 23, 27), S( 36, 23), S( 42, 15), S( 54, 10), S( 57,-10), S( 59,  1), S( 60,-18), S( 38, 25),
		S( 35,-21),
	}
};

void tables_init(void) {
	memset(psqtable, 0, sizeof(psqtable));
	for (int turn = 0; turn < 2; turn++) {
		for (int piece = pawn; piece <= king; piece++) {
			for (int square = 0; square < 64; square++) {
				int x = square % 8;
				int y = square / 8;
				int factor = (piece == pawn) ? 8 : 4;
				if (x >= 4 && piece != pawn)
					x = 7 - x;
				if (turn == white)
					y = 7 - y;
				psqtable[turn][piece][square] = white_psqtable[piece - 1][factor * y + x] +
					piece_value[piece - 1];
				init_status("populating evaluation lookup table");
			}
		}
	}
}
