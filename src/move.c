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

#include "move.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "bitboard.h"
#include "util.h"
#include "movegen.h"
#include "attackgen.h"

void do_move(struct position *pos, move_t *m) {
	assert(*m);
	int source_square = move_from(m);
	int target_square = move_to(m);
	assert(pos->mailbox[source_square]);

	uint64_t from = bitboard(source_square);
	uint64_t to = bitboard(target_square);
	uint64_t from_to = from | to;

	move_set_castle(m, pos->castle);
	move_set_en_passant(m, pos->en_passant);
	move_set_halfmove(m, pos->halfmove);

	pos->en_passant = 0;
	pos->halfmove++;

	pos->castle = castle(source_square, target_square, pos->castle);

	if (pos->mailbox[target_square]) {
		pos->piece[other_color(pos->turn)][uncolored_piece(pos->mailbox[target_square])] ^= to;
		move_set_captured(m, uncolored_piece(pos->mailbox[target_square]));
		pos->piece[other_color(pos->turn)][all] ^= to;
		pos->halfmove = 0;
	}

	if (source_square + 16 == target_square && pos->mailbox[source_square] == white_pawn)
		pos->en_passant = target_square - 8;
	if (source_square - 16 == target_square && pos->mailbox[source_square] == black_pawn)
		pos->en_passant = target_square + 8;

	if (uncolored_piece(pos->mailbox[source_square]) == pawn)
		pos->halfmove = 0;

	pos->piece[pos->turn][uncolored_piece(pos->mailbox[source_square])] ^= from_to;
	pos->piece[pos->turn][all] ^= from_to;

	pos->mailbox[target_square] = pos->mailbox[source_square];
	pos->mailbox[source_square] = empty;

	if (move_flag(m) == 1) {
		int direction = 2 * pos->turn - 1;
		pos->piece[other_color(pos->turn)][pawn] ^= bitboard(target_square - direction * 8);
		pos->piece[other_color(pos->turn)][all] ^= bitboard(target_square - direction * 8);
		pos->mailbox[target_square - direction * 8] = empty;
	}
	else if (move_flag(m) == 2) {
		pos->piece[pos->turn][pawn] ^= to;
		pos->piece[pos->turn][move_promote(m) + 2] ^= to;
		pos->mailbox[target_square] = colored_piece(move_promote(m) + 2, pos->turn);
	}
	else if (move_flag(m) == 3) {
		switch (target_square) {
		case g1:
			pos->piece[white][rook] ^= 0xA0;
			pos->piece[white][all] ^= 0xA0;
			pos->mailbox[h1] = empty;
			pos->mailbox[f1] = white_rook;
			break;
		case c1:
			pos->piece[white][rook] ^= 0x9;
			pos->piece[white][all] ^= 0x9;
			pos->mailbox[a1] = empty;
			pos->mailbox[d1] = white_rook;
			break;
		case g8:
			pos->piece[black][rook] ^= 0xA000000000000000;
			pos->piece[black][all] ^= 0xA000000000000000;
			pos->mailbox[h8] = empty;
			pos->mailbox[f8] = black_rook;
			break;
		case c8:
			pos->piece[black][rook] ^= 0x900000000000000;
			pos->piece[black][all] ^= 0x900000000000000;
			pos->mailbox[a8] = empty;
			pos->mailbox[d8] = black_rook;
			break;
		}
	}

	if (!pos->turn)
		pos->fullmove++;
	pos->turn = other_color(pos->turn);
}

void undo_move(struct position *pos, const move_t *m) {
	assert(*m);
	int source_square = move_from(m);
	int target_square = move_to(m);
	assert(pos->mailbox[target_square]);

	uint64_t from = bitboard(source_square);
	uint64_t to = bitboard(target_square);
	uint64_t from_to = from | to;

	pos->castle = move_castle(m);
	pos->en_passant = move_en_passant(m);
	pos->halfmove = move_halfmove(m);
	pos->turn = other_color(pos->turn);

	if (move_flag(m) == 1) {
		int direction = 2 * pos->turn - 1;
		pos->piece[other_color(pos->turn)][pawn] ^= bitboard(target_square - direction * 8);
		pos->piece[other_color(pos->turn)][all] ^= bitboard(target_square - direction * 8);
		pos->mailbox[target_square - direction * 8] = colored_piece(pawn, other_color(pos->turn));
	}
	else if (move_flag(m) == 2) {
		pos->piece[pos->turn][pawn] ^= to;
		pos->piece[pos->turn][uncolored_piece(pos->mailbox[target_square])] ^= to;
		/* later gets updated as a normal move_t would */
		pos->mailbox[target_square] = colored_piece(pawn, pos->turn);
	}
	else if (move_flag(m) == 3) {
		switch (target_square) {
		case g1:
			pos->piece[white][rook] ^= 0xA0;
			pos->piece[white][all] ^= 0xA0;
			pos->mailbox[h1] = white_rook;
			pos->mailbox[f1] = empty;
			break;
		case c1:
			pos->piece[white][rook] ^= 0x9;
			pos->piece[white][all] ^= 0x9;
			pos->mailbox[a1] = white_rook;
			pos->mailbox[d1] = empty;
			break;
		case g8:
			pos->piece[black][rook] ^= 0xA000000000000000;
			pos->piece[black][all] ^= 0xA000000000000000;
			pos->mailbox[h8] = black_rook;
			pos->mailbox[f8] = empty;
			break;
		case c8:
			pos->piece[black][rook] ^= 0x900000000000000;
			pos->piece[black][all] ^= 0x900000000000000;
			pos->mailbox[a8] = black_rook;
			pos->mailbox[d8] = empty;
			break;
		}
	}

	pos->piece[pos->turn][uncolored_piece(pos->mailbox[target_square])] ^= from_to;
	pos->piece[pos->turn][all] ^= from_to;

	pos->mailbox[source_square] = pos->mailbox[target_square];
	pos->mailbox[target_square] = empty;

	if (move_capture(m)) {
		pos->piece[other_color(pos->turn)][move_capture(m)] ^= to;
		pos->piece[other_color(pos->turn)][all] ^= to;
		pos->mailbox[target_square] = colored_piece(move_capture(m), other_color(pos->turn));
	}

	if (!pos->turn)
		pos->fullmove--;
}

void print_move(const move_t *m) {
	char move_from_str[3];
	char move_to_str[3];
	algebraic(move_from_str, move_from(m));
	algebraic(move_to_str, move_to(m));
	printf("%s%s", move_from_str, move_to_str);
	if (move_flag(m) == 2)
		printf("%c", "nbrq"[move_promote(m)]);
}

/* str needs to be at least 6 bytes */
char *move_str_algebraic(char *str, const move_t *m) {
	algebraic(str, move_from(m));
	algebraic(str + 2, move_to(m));
	str[4] = str[5] = '\0';
	if (move_flag(m) == 2)
		str[4] = "nbrq"[move_promote(m)];
	return str;
}

/* str needs to be at least 8 bytes */
/* m can be illegal */
char *move_str_pgn(char *str, const struct position *pos, const move_t *m) {
	if (!is_legal(pos, m))
		return NULL;
	int f = file_of(move_from(m));
	int r = rank_of(move_from(m));
	int i = 0;

	str[i] = '\0';
	switch (uncolored_piece(pos->mailbox[move_from(m)])) {
	case pawn:
		break;
	case knight:
		str[i++] = 'N';
		break;
	case bishop:
		str[i++] = 'B';
		break;
	case rook:
		str[i++] = 'R';
		break;
	case queen:
		str[i++] = 'Q';
		break;
	/* king */
	case king:
		if (f == 4 && file_of(move_to(m)) == 6) {
			sprintf(str, "O-O");
			i = 3;
		}
		else if (f == 4 && file_of(move_to(m)) == 2) {
			sprintf(str, "O-O-O");
			i = 5;
		}
		else {
			str[i++] = 'K';
		}
		break;
	}

	uint64_t attackers = 0;
	if (uncolored_piece(pos->mailbox[move_from(m)]) == pawn) {
		if (is_capture(pos, m) || move_flag(m) == 1)
			attackers = rank(move_from(m));
		else
			attackers = bitboard(move_from(m));
	}
	else {
		move_t move_list[MOVES_MAX];
		generate_all(pos, move_list);
		for (move_t *ptr = move_list; *ptr; ptr++) {
			if (move_to(ptr) == move_to(m) && pos->mailbox[move_from(ptr)] == pos->mailbox[move_from(m)])
				attackers |= bitboard(move_from(ptr));
		}
	}
	assert(attackers);

	if (popcount(attackers & file(move_from(m))) > 1) {
		if (popcount(attackers & rank(move_from(m))) > 1) {
			str[i++] = "abcdefgh"[f];
			str[i++] = "12345678"[r];
		}
		else {
			str[i++] = "12345678"[r];
		}
	}
	else if (popcount(attackers) > 1) {
		str[i++] = "abcdefgh"[f];
	}

	if (pos->mailbox[move_to(m)] || move_flag(m) == 1)
		str[i++] = 'x';
	if (str[0] != 'O') {
		algebraic(str + i, move_to(m));
		i += 2;
	}

	if (move_flag(m) == 2) {
		str[i++] = '=';
		str[i++] = "NBRQ"[move_promote(m)];
	}
	
	struct position pos_t;
	move_t m_t;
	m_t = *m;
	memcpy(&pos_t, pos, sizeof(pos_t));
	do_move(&pos_t, &m_t);
	int ma = mate(&pos_t);
	uint64_t checkers = generate_checkers(&pos_t, pos_t.turn);

	if (ma == 2)
		str[i++] = '#';
	else if (checkers)
		str[i++] = '+';
	str[i] = '\0';
	return str;
}

/* str can be illegal */
move_t string_to_move(const struct position *pos, const char *str) {
	if (!str)
		return 0;
	move_t move_list[MOVES_MAX];
	generate_all(pos, move_list);
	char str_t[8];
	for (move_t *move_ptr = move_list; *move_ptr; move_ptr++) {
		move_str_algebraic(str_t, move_ptr);
		if (strcmp(str_t, str) == 0)
			return *move_ptr;
		move_str_pgn(str_t, pos, move_ptr);
		if (strcmp(str_t, str) == 0)
			return *move_ptr;
	}
	return 0;
}

int is_legal(const struct position *pos, const move_t *m) {
	move_t move_list[MOVES_MAX];
	generate_all(pos, move_list);
	for (move_t *move_ptr = move_list; *move_ptr; move_ptr++)
		if ((*m & 0xFFFF) == (*move_ptr & 0xFFFF))
			return 1;
	return 0;
}

void do_null_move(struct position *pos, int en_passant) {
	pos->en_passant = en_passant;
	pos->turn = other_color(pos->turn);
}
