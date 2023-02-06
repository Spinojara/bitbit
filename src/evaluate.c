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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "bitboard.h"
#include "move_gen.h"
#include "move.h"
#include "util.h"
#include "transposition_table.h"
#include "init.h"
#include "position.h"
#include "interrupt.h"
#include "attack_gen.h"

uint64_t nodes = 0;

uint64_t mvv_lva_lookup[13 * 13];

int eval_table[2][13][64];

/* <https://www.chessprogramming.org/King_Safety> */
int safety_table[100] = {
	  0,   0,   1,   2,   3,   5,   7,   9,  12,  15,
	 18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
	 68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
	140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
	260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
	377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
	494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

int piece_value[13] = { 0, 100, 300, 315, 500, 900, 0, -100, -300, -315, -500, -900, 0 };

/* <https://www.chessprogramming.org/Simplified_Evaluation_Function> */
int white_side_eval_table[2][6][64] = { { /* early game */
	{
		  0,   0,   0,   0,   0,   0,   0,   0,
		 50,  50,  50,  50,  50,  50,  50,  50,
		 10,  10,  20,  30,  30,  20,  10,  10,
		  5,   5,  10,  25,  25,  10,   5,   5,
		  0,   0,   0,  20,  20,   0,   0,   0,
		  5,  -5, -10,   0,   0, -10,  -5,   5,
		  5,  10,  10, -20, -20,  10,  10,   5,
		  0,   0,   0,   0,   0,   0,   0,   0
	}, {
		-50, -40, -30, -30, -30, -30, -40, -50,
		-40, -20,   0,   0,   0,   0, -20, -40,
		-30,   0,  10,  15,  15,  10,   0, -30,
		-30,   5,  15,  20,  20,  15,   5, -30,
		-30,   0,  15,  20,  20,  15,   0, -30,
		-30,   5,  10,  15,  15,  10,   5, -30,
		-40, -20,   0,   5,   5,   0, -20, -40,
		-50, -40, -30, -30, -30, -30, -40, -50
	}, {
		-20, -10, -10, -10, -10, -10, -10, -20,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-10,   0,   5,  10,  10,   5,   0, -10,
		-10,   5,   5,  10,  10,   5,   5, -10,
		-10,   0,  10,  10,  10,  10,   0, -10,
		-10,  10,  10,  10,  10,  10,  10, -10,
		-10,   5,   0,   0,   0,   0,   5, -10,
		-20, -10, -10, -10, -10, -10, -10, -20
	}, {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  5,  10,  10,  10,  10,  10,  10,   5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		  0,   0,   0,   5,   5,   0,   0,   0
	}, {
		-20, -10, -10,  -5,  -5, -10, -10, -20,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-10,   0,   5,   5,   5,   5,   0, -10,
		 -5,   0,   5,   5,   5,   5,   0,  -5,
		  0,   0,   5,   5,   5,   5,   0,  -5,
		-10,   5,   5,   5,   5,   5,   0, -10,
		-10,   0,   5,   0,   0,   0,   0, -10,
		-20, -10, -10,  -5,  -5, -10, -10, -20
	}, {
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		 20,  20,   0,   0,   0,   0,  20,  20,
		 20,  30,  10,   0,   0,  10,  30,  20
	} }, /* end game */ { {
		  0,   0,   0,   0,   0,   0,   0,   0,
		 50,  50,  50,  50,  50,  50,  50,  50,
		 10,  10,  20,  30,  30,  20,  10,  10,
		  5,   5,  10,  25,  25,  10,   5,   5,
		  0,   0,   0,  20,  20,   0,   0,   0,
		  5,  -5, -10,   0,   0, -10,  -5,   5,
		  5,  10,  10, -20, -20,  10,  10,   5,
		  0,   0,   0,   0,   0,   0,   0,   0
	}, {
		-50, -40, -30, -30, -30, -30, -40, -50,
		-40, -20,   0,   0,   0,   0, -20, -40,
		-30,   0,  10,  15,  15,  10,   0, -30,
		-30,   5,  15,  20,  20,  15,   5, -30,
		-30,   0,  15,  20,  20,  15,   0, -30,
		-30,   5,  10,  15,  15,  10,   5, -30,
		-40, -20,   0,   5,   5,   0, -20, -40,
		-50, -40, -30, -30, -30, -30, -40, -50
	}, {
		-20, -10, -10, -10, -10, -10, -10, -20,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-10,   0,   5,  10,  10,   5,   0, -10,
		-10,   5,   5,  10,  10,   5,   5, -10,
		-10,   0,  10,  10,  10,  10,   0, -10,
		-10,  10,  10,  10,  10,  10,  10, -10,
		-10,   5,   0,   0,   0,   0,   5, -10,
		-20, -10, -10, -10, -10, -10, -10, -20
	}, {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  5,  10,  10,  10,  10,  10,  10,   5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		 -5,   0,   0,   0,   0,   0,   0,  -5,
		  0,   0,   0,   5,   5,   0,   0,   0
	}, {
		-20, -10, -10,  -5,  -5, -10, -10, -20,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-10,   0,   5,   5,   5,   5,   0, -10,
		 -5,   0,   5,   5,   5,   5,   0,  -5,
		  0,   0,   5,   5,   5,   5,   0,  -5,
		-10,   5,   5,   5,   5,   5,   0, -10,
		-10,   0,   5,   0,   0,   0,   0, -10,
		-20, -10, -10,  -5,  -5, -10, -10, -20
	}, {
		-50, -40, -30, -20, -20, -30, -40, -50,
		-30, -20, -10,   0,   0, -10, -20, -30,
		-30, -10,  20,  30,  30,  20, -10, -30,
		-30, -10,  30,  40,  40,  30, -10, -30,
		-30, -10,  30,  40,  40,  30, -10, -30,
		-30, -10,  20,  30,  30,  20, -10, -30,
		-30, -30,   0,   0,   0,   0, -30, -30,
		-50, -30, -30, -30, -30, -30, -30, -50
	} }
};

uint64_t mvv_lva_calc(int attacker, int victim) {
	int a = (attacker - 1) % 6;
	int v = (victim - 1) % 6;
	int lookup_t[6 * 6] = {
		 2, 15, 16, 17, 21,  0,
		 0,  3,  7, 14, 20,  0,
		 0,  1,  4, 13, 19,  0,
		 0,  0,  0,  5, 18,  0,
		 0,  0,  0,  0,  6,  0,
		 8,  9, 10, 11, 12,  0,
	};
	if (!lookup_t[v + 6 * a])
		return 0xFFFFFFFFFFFF0000;
	return lookup_t[v + 6 * a] + 0xFFFFFFFFFFFFFF00;
}

void evaluate_init(void) {
	memset(eval_table, 0, sizeof(eval_table));
	for (int i = 0; i < 13; i++) {
		for (int j  = 0; j < 64; j++) {
			for (int k = 0; k < 2; k++) {
				if (i == 0)
					eval_table[k][i][j] = 0;
				else if (i < 7)
					eval_table[k][i][j] = white_side_eval_table[k][i - 1][(7 - j / 8) * 8 + (j % 8)] +
						           piece_value[i];
				else
					eval_table[k][i][j] = -white_side_eval_table[k][i - 7][j] +
							   piece_value[i];
				init_status("populating evaluation lookup table");
			}
		}
		for (int j = 0; j < 13; j++) {
			mvv_lva_lookup[j + 13 * i] = mvv_lva_calc(i, j);
			init_status("populating mvv lva lookup table");
		}
	}
}

void print_pv(struct position *pos, move *pv_move, int ply) {
	char str[8];
	if (ply > 255 || !move_str_pgn(str, pos, pv_move))
		return;
	printf(" %s", str);
	do_move(pos, pv_move);
	print_pv(pos, pv_move + 1, ply + 1);
	undo_move(pos, pv_move);
	*pv_move = *pv_move & 0xFFFF;
}

int is_threefold(struct position *pos, struct history *history) {
	int count;
	struct history *t;
	for (t = history, count = 0; t; t = t->previous)
		if (pos_are_equal(pos, t->pos))
			count++;
	return count >= 2;
}

static inline uint64_t mvv_lva(int attacker, int victim) {
	return mvv_lva_lookup[victim + 13 * attacker];
}

static inline void store_killer_move(move *m, uint8_t ply, move killer_moves[][2]) {
	if (!killer_moves)
		return;
	killer_moves[ply][1] = killer_moves[ply][0];
	killer_moves[ply][0] = *m & 0xFFFF;
}

static inline void store_history_move(struct position *pos, move *m, uint8_t depth, uint64_t history_moves[13][64]) {
	history_moves[pos->mailbox[move_from(m)]][move_to(m)] += (uint64_t)1 << depth;
}

static inline void store_pv_move(move *m, uint8_t ply, move pv_moves[256][256]) {
	if (!pv_moves)
		return;
	pv_moves[ply][ply] = *m & 0xFFFF;
	memcpy(pv_moves[ply] + ply + 1, pv_moves[ply + 1] + ply + 1, sizeof(move) * (255 - ply));
}

int contains_pv_move(move *move_list, uint8_t ply, move pv_moves[256][256]) {
	if (!pv_moves)
		return 0;
	for (move *ptr = move_list; *ptr; ptr++)
		if ((*ptr & 0xFFFF) == (pv_moves[0][ply] & 0xFFFF))
			return 1;
	return 0;
}

uint64_t evaluate_move(struct position *pos, move *m, uint8_t ply, struct transposition *e, int pv_flag, move pv_moves[256][256], move killer_moves[][2], uint64_t history_moves[13][64]) {
	/* pv */
	if (pv_flag && pv_moves && (pv_moves[0][ply] & 0xFFFF) == (*m & 0xFFFF))
		return 0xFFFFFFFFFFFFFFFF;

	/* transposition table */
	if (e && *m == transposition_move(e))
		return 0xFFFFFFFFFFFFFFFE;

	/* attack */
	if (pos->mailbox[move_to(m)])
		return mvv_lva(pos->mailbox[move_from(m)], pos->mailbox[move_to(m)]);

	/* promotions */
	if (move_flag(m) == 2)
		return 0xFFFFFFFFFFFFF000 + move_promote(m);

	/* killer */
	if (killer_moves) {
		if (killer_moves[ply][0] == *m)
			return 0xFFFFFFFFFFFF0002;
		if (killer_moves[ply][1] == *m)
			return 0xFFFFFFFFFFFF0001;
	}

	/* history */
	if (history_moves)
		return history_moves[pos->mailbox[move_from(m)]][move_to(m)];
	return 0;
}

/* 1. pv
 * 2. tt
 * 3. mvv lva winning
 * 4. promotions
 * 5. killer
 * 6. mvv lva losing
 * 7. history
 */
void evaluate_moves(struct position *pos, move *move_list, uint8_t depth, struct transposition *e, int pv_flag, move pv_moves[256][256], move killer_moves[][2], uint64_t history_moves[13][64]) {
	uint64_t evaluation_list[MOVES_MAX];
	int i;
	for (i = 0; move_list[i]; i++)
		evaluation_list[i] = evaluate_move(pos, move_list + i, depth, e, pv_flag, pv_moves, killer_moves, history_moves);

	merge_sort(move_list, evaluation_list, 0, i - 1, 0);
}

int pawn_structure(struct position *pos) {
	int eval = 0, i;
	uint64_t t;

	/* doubled pawns */
	for (i = 0; i < 8; i++) {
		if ((t = (pos->piece[white][pawn] & file(i))))
			eval -= (popcount(t) - 1) * 50;
		if ((t = (pos->piece[black][pawn] & file(i))))
			eval += (popcount(t) - 1) * 50;
	}

	/* isolated pawns */
	for (i = 0; i < 8; i++) {
		if ((pos->piece[white][pawn] & file(i)) &&
				!(pos->piece[white][pawn] & adjacent_files(i)))
			eval -= 35;
		if ((pos->piece[black][pawn] & file(i)) &&
				!(pos->piece[black][pawn] & adjacent_files(i)))
			eval += 35;
	}

	/* passed pawns */
	int square;
	uint64_t b;
	for (i = 0; i < 8; i++) {
		/* asymmetric because of bit scan */
		if ((b = (pos->piece[white][pawn] & file(i)))) {
			while (b) {
				square = ctz(b);
				b = clear_ls1b(b);
			}
			if (!(pos->piece[black][pawn] & passed_files_white(square)))
				eval += 20 * (square / 8);
		}
		if ((b = (pos->piece[black][pawn] & file(i)))) {
			square = ctz(b);
			if (!(pos->piece[white][pawn] & passed_files_black(square)))
				eval -= 20 * (7 - square / 8);
		}
	}

	return eval;
}

int center_control(struct position *pos) {
	int eval = 0;
	uint64_t center = 0x3C3C000000;
	eval += 15 * popcount((shift_west(shift_north(pos->piece[white][pawn])) | shift_east(shift_north(pos->piece[white][pawn]))) & center);
	eval -= 15 * popcount((shift_west(shift_south(pos->piece[black][pawn])) | shift_east(shift_south(pos->piece[black][pawn]))) & center);
	return eval;
}

int king_safety(struct position *pos) {
	int king_white;
	int king_black;
	int eval = 0;
	/* half open files near king are bad */
	king_white = ctz(pos->piece[white][king]);
	if (file_left(king_white) && !(file_left(king_white) & pos->piece[white][pawn]))
		eval -= 30;
	if (!(file(king_white) & pos->piece[white][pawn]))
		eval -= 30;
	if (file_right(king_white) && !(file_right(king_white) & pos->piece[white][pawn]))
		eval -= 30;
	king_black = ctz(pos->piece[black][king]);
	if (file_left(king_black) && !(file_left(king_black) & pos->piece[black][pawn]))
		eval += 30;
	if (!(file(king_black) & pos->piece[black][pawn]))
		eval += 30;
	if (file_right(king_black) && !(file_right(king_black) & pos->piece[black][pawn]))
		eval += 30;

	/* safety table */
	int attack_units;
	uint64_t squares;
	attack_units = 0;
	squares = king_squares(ctz(pos->piece[white][king]), 1);
	attack_units += 5 * popcount(squares & pos->piece[black][queen]);
	attack_units += 3 * popcount(squares & pos->piece[black][rook]);
	attack_units += 2 * popcount(squares & (pos->piece[black][bishop] | pos->piece[black][knight]));
	eval -= safety_table[MIN(attack_units, 99)];
	attack_units = 0;
	squares = king_squares(ctz(pos->piece[black][king]), 0);
	attack_units += 5 * popcount(squares & pos->piece[white][queen]);
	attack_units += 3 * popcount(squares & pos->piece[white][rook]);
	attack_units += 2 * popcount(squares & (pos->piece[white][bishop] | pos->piece[white][knight]));
	eval += safety_table[MIN(attack_units, 99)];

	/* pawns close to king */
	eval += 15 * popcount(king_attacks(king_white, 0) & pos->piece[white][pawn]);
	eval -= 15 * popcount(king_attacks(king_black, 0) & pos->piece[black][pawn]);
	return eval;
}

int16_t evaluate_static(struct position *pos) {
	nodes++;
	int eval = 0, i;
	int total_piece_count[] = { 0, 0, 3, 3, 5, 9, 0, 0, 3, 3, 5, 9, 0 };

	double early_game = 0;
	int queen_flag = 0;
	for (i = 0; i < 64; i++) {
		/* don't count queens from promotion */
		if (queen_flag == 2 && total_piece_count[pos->mailbox[i]] == 9)
			continue;
		if (total_piece_count[pos->mailbox[i]] == 9)
			queen_flag++;
		early_game += total_piece_count[pos->mailbox[i]];
	}

	if (early_game > 40)
		early_game = 1;
	else if (early_game < 10)
		early_game = 0;
	else
		early_game = (early_game - 10) / 40;

	/* piece square tables */
	for (i = 0; i < 64; i++)
		eval += early_game * eval_table[0][pos->mailbox[i]][i] + (1 - early_game) * eval_table[1][pos->mailbox[i]][i];
	/* encourage trading when ahead, discourage when behind */
	eval = nearint((0.99 + 0.011 / (early_game + 0.1)) * eval);

	/* bishop pair */
	if (pos->piece[white][bishop] && clear_ls1b(pos->piece[white][bishop]))
		eval += 30;
	if (pos->piece[black][bishop] && clear_ls1b(pos->piece[black][bishop]))
		eval -= 30;

	/* king safety */
	eval += early_game * king_safety(pos);

	/* center control */
	eval += early_game * center_control(pos);

	/* piece mobility */
	eval += 8 * (mobility(pos, white) - mobility(pos, black));

	/* pawn structure */
	eval += pawn_structure(pos);

	return pos->turn ? eval : -eval;
}

int16_t quiescence(struct position *pos, int16_t alpha, int16_t beta, clock_t clock_stop) {
	if (interrupt)
		return 0;
	if (nodes % 4096 == 0)
		if (clock_stop && clock() > clock_stop)
			interrupt = 1;

	int16_t evaluation;
	evaluation = evaluate_static(pos);
	if (evaluation >= beta)
		return beta;
	if (evaluation > alpha)
		alpha = evaluation;

	move move_list[MOVES_MAX];
	generate_quiescence(pos, move_list);
	if (!move_list[0])
		return evaluation;

	evaluate_moves(pos, move_list, 0, NULL, 0, NULL, NULL, NULL);
	for (move *ptr = move_list; *ptr; ptr++) {
		do_move(pos, ptr);
		evaluation = -quiescence(pos, -beta, -alpha, clock_stop);
		undo_move(pos, ptr);
		if (evaluation >= beta)
			return beta;
		if (evaluation > alpha)
			alpha = evaluation;
	}
	return alpha;
}

int16_t evaluate_recursive(struct position *pos, uint8_t depth, uint8_t ply, int16_t alpha, int16_t beta, int null_move, clock_t clock_stop, int *pv_flag, move pv_moves[][256], move killer_moves[][2], uint64_t history_moves[13][64]) {
	int16_t evaluation;

	if (interrupt)
		return 0;
	if (nodes % 4096 == 0)
		if (clock_stop && clock() > clock_stop)
			interrupt = 1;

	struct transposition *e = attempt_get(pos);
	if (e && transposition_open(e))
		return 0;
	if (e && transposition_depth(e) >= depth && !(*pv_flag)) {
		/* pv */
		if (transposition_type(e) == 0)
			return transposition_evaluation(e);
		/* cut */
		else if (transposition_type(e) == 1)
			alpha = transposition_evaluation(e);
		/* all */
		else
			beta = transposition_evaluation(e);
	}

	if (depth <= 0) {
		evaluation = mate(pos);
		/* stalemate */
		if (evaluation == 1)
			return 0;
		/* checkmate */
		if (evaluation == 2)
			return -0x7F00;
		return quiescence(pos, alpha, beta, clock_stop);
	}

	/* null move pruning */
	uint64_t checkers = generate_checkers(pos, pos->turn);
	if (!null_move && !(*pv_flag) && !checkers && depth >= 3 && has_big_piece(pos)) {
		int t = pos->en_passant;
		do_null_move(pos, 0);
		evaluation = -evaluate_recursive(pos, depth - 3, ply + 1, -beta, -beta + 1, 1, clock_stop, pv_flag, NULL, NULL, history_moves);
		do_null_move(pos, t);
		if (evaluation >= beta)
			return beta;
	}

	move move_list[MOVES_MAX];
	generate_all(pos, move_list);

	if (!move_list[0])
		return checkers ? -0x7F00 : 0;

	if (*pv_flag && !contains_pv_move(move_list, ply, pv_moves))
		*pv_flag = 0;

	evaluate_moves(pos, move_list, depth, e, *pv_flag, pv_moves, killer_moves, history_moves);

	if (pos->halfmove >= 100)
		return 0;
	
	if (e)
		transposition_set_open(e);

	uint16_t m = 0;
	for (move *ptr = move_list; *ptr; ptr++) {
		do_move(pos, ptr);
		if (ptr == move_list || *pv_flag) {
			/* -beta - 1 to open the window and search for mate in n */
			evaluation = -evaluate_recursive(pos, depth - 1, ply + 1, -beta - 1, -alpha, 0, clock_stop, pv_flag, pv_moves, killer_moves, history_moves);
		}
		else {
			/* late move reduction */
			if (depth >= 3 && !checkers && ptr - move_list >= 2 && move_flag(ptr) != 2 && !move_capture(ptr)) {
				uint8_t r = ptr - move_list >= 4 ? depth / 3 : 1;
				evaluation = -evaluate_recursive(pos, depth - 1 - MIN(r, depth - 1), ply + 1, -alpha - 1, -alpha, 0, clock_stop, pv_flag, pv_moves, killer_moves, history_moves);
			}
			else {
				evaluation = alpha + 1;
			}
			if (evaluation > alpha) {
				evaluation = -evaluate_recursive(pos, depth - 1, ply + 1, -alpha - 1, -alpha, 0, clock_stop, pv_flag, pv_moves, killer_moves, history_moves);
				if (evaluation > alpha && evaluation < beta)
					evaluation = -evaluate_recursive(pos, depth - 1, ply + 1, -beta - 1, -alpha, 0, clock_stop, pv_flag, pv_moves, killer_moves, history_moves);
			}
		}
		evaluation -= (evaluation > 0x4000);
		undo_move(pos, ptr);
		if (evaluation >= beta) {
			/* quiet */
			if (!pos->mailbox[move_to(ptr)])
				store_killer_move(ptr, ply, killer_moves);
			if (e)
				transposition_set_closed(e);
			/* type cut */
			attempt_store(pos, beta, depth, 1, *ptr);
			return beta;
		}
		if (evaluation > alpha) {
			/* quiet */
			if (!pos->mailbox[move_to(ptr)])
				store_history_move(pos, ptr, depth, history_moves);
			alpha = evaluation;
			store_pv_move(ptr, ply, pv_moves);
			m = *ptr;
		}
	}
	if (e)
		transposition_set_closed(e);
	/* type pv or all */
	attempt_store(pos, alpha, depth, m ? 0 : 2, m & 0xFFFF);
	return alpha;
}

int16_t evaluate(struct position *pos, uint8_t depth, move *m, int verbose, int max_duration, struct history *history) {
	int16_t evaluation = 0, saved_evaluation;
	move move_list[MOVES_MAX];
	generate_all(pos, move_list);

	if (m)
		*m = 0;

	if (!move_list[0]) {
		uint64_t checkers = generate_checkers(pos, pos->turn);
		if (!checkers)
			evaluation = 0;
		else 
			evaluation = pos->turn ? -0x7F00 : 0x7F00;
		if (verbose) {
			if (evaluation)
				printf("%c#\n", pos->turn ? '-' : '+');
			else
				printf("=\n");
		}
		return evaluation;
	}

	if (depth == 0) {
		evaluation = evaluate_static(pos);
		if (!pos->turn)
			evaluation = -evaluation;
		if (verbose)
			printf("0 %+.2f\n", (double)evaluation / 100);
		return evaluation;
	}

	clock_t clock_stop;
	if (max_duration >= 0)
		clock_stop = clock() + CLOCKS_PER_SEC * max_duration;
	else
		clock_stop = 0;

	move pv_moves[256][256];
	move killer_moves[256][2];
	uint64_t history_moves[13][64];
	memset(pv_moves, 0, sizeof(pv_moves));
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));

	saved_evaluation = 0;
	int16_t alpha, beta;
	int pv_flag;
	for (int d = 1; d <= depth; d++) {
		nodes = 0;
		pv_flag = 1;
		alpha = -0x7F00;
		beta = 0x7F00;

		evaluate_moves(pos, move_list, 0, NULL, pv_flag, pv_moves, killer_moves, history_moves);

		for (move *ptr = move_list; *ptr; ptr++) {
			do_move(pos, ptr);
			evaluation = -evaluate_recursive(pos, d - 1, 1, -beta, -alpha, 0, clock_stop, &pv_flag, pv_moves, killer_moves, history_moves);
			evaluation -= (evaluation > 0x4000);
			if (is_threefold(pos, history))
				evaluation = 0;
			undo_move(pos, ptr);
			if (evaluation > alpha) {
				store_pv_move(ptr, 0, pv_moves);
				alpha = evaluation;
			}
		}
		if (interrupt)
			break;
		evaluation = pos->turn ? alpha : -alpha;
		saved_evaluation = evaluation;
		if (verbose) {
			if (evaluation < -0x4000)
				printf("%i -m%i", d, 0x7F00 + evaluation);
			else if (evaluation > 0x4000)
				printf("%i +m%i", d, 0x7F00 - evaluation);
			else
				printf("%i %+.2f", d, (double)evaluation / 100);
			printf(" nodes %" PRIu64 " pv", nodes);
			print_pv(pos, pv_moves[0], 0);
			printf("\n");
			fflush(stdout);
		}
		if (m)
			*m = pv_moves[0][0];
		/* stop searching if mate is found */
		if (evaluation < -0x4000 && 2 * (0x7F00 + evaluation) + pos->turn - 1 <= d)
			break;
		if (evaluation > 0x4000 && 2 * (0x7F00 - evaluation) - pos->turn <= d)
			break;
	}
	return saved_evaluation;
}
