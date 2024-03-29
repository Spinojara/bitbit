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

#include <stdio.h>
#include <stdlib.h>

#include "move.h"
#include "position.h"
#include "evaluate.h"
#include "util.h"
#include "magicbitboard.h"
#include "attackgen.h"
#include "tables.h"
#include "option.h"

void store_information(struct position *pos, uint64_t piece_square[7][64]) {
	for (int color = 0; color < 2; color++) {
		for (int piece = PAWN; piece <= KING; piece++) {
			uint64_t b = pos->piece[color][piece];
			while (b) {
				int square = ctz(b);
				square = orient_horizontal(color, square);
				piece_square[piece][square]++;
				b = clear_ls1b(b);
			}
		}
	}
}

void print_information(uint64_t square[64], uint64_t total) {
	for (int r = 7; r >= 0; r--) {
		printf("+-------+-------+-------+-------+-------+-------+-------+-------+\n|");
		for (int f = 0; f < 8; f++) {
			int sq = make_square(f, r);
			printf(" %5.2f |", 100.f * square[sq] / (2 * total));
		}
		printf("\n");
	}
	printf("+-------+-------+-------+-------+-------+-------+-------+-------+\n");
	printf("\n");
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("provide a filename\n");
		return 1;
	}
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		printf("could not open %s\n", argv[1]);
		return 2;
	}

	magicbitboard_init();
	attackgen_init();
	bitboard_init();
	tables_init();
	position_init();
	struct position pos;

	uint64_t piece_square[7][64] = { 0 };

	move_t move;
	int16_t eval;
	size_t total = 0;
	size_t count = 0;
	size_t games = 0;
	size_t a = 0;
	size_t c = 0;
	while (1) {
		count++;
		if (count % 20000 == 0)
			printf("collecting data: %lu\r", count);
		move = 0;
		fread(&move, 2, 1, f);
		if (move) {
			do_move(&pos, &move);
		}
		else {
			fread(&pos, sizeof(struct partialposition), 1, f);
			if (!feof(f))
				games++;
		}
		
		fread(&eval, 2, 1, f);
		if (feof(f))
			break;

		if (eval == VALUE_NONE)
			continue;

#if 0
		const int material_values[] = { 0, 1, 3, 3, 5, 9, 0 };
		int material[2] = { 0 };
		for (int color = 0; color < 2; color++) {
			for (int piece = pawn; piece < king; piece++) {
				uint64_t b = pos.piece[color][piece];
				material[color] += material_values[piece] * popcount(b);
			}
		}

		int material_delta = abs(material[white] - material[black]);
		
		if (material_delta >= 3 && eval == 0 && pos.halfmove <= 0) {
			char fen[128];
			print_position(&pos);
			printf("%s\n", pos_to_fen(fen, &pos));
			printf("%d\n", (2 * pos.turn - 1) * evaluate_classical(&pos));
			printf("%d\n", pos.turn ? eval : -eval);
			if (eval == 0)
				c++;
			a++;
		}
#endif

		store_information(&pos, piece_square);
		total++;
	}
	printf("\033[2K");
	printf("total positions: %lu\n", total);
	printf("total games: %lu\n", games);
	printf("percent: %f\n", (double)c / a);
	for (int piece = PAWN; piece <= KING; piece++)
		print_information(piece_square[piece], total);
}
