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

#include "movepicker.h"

#include <assert.h>

#include "moveorder.h"
#include "bitboard.h"
#include "movegen.h"
#include "util.h"

static inline int good_capture(struct position *pos, move_t *move, int threshold) {
	return (is_capture(pos, move) && see_geq(pos, move, threshold)) || (move_flag(move) == MOVE_EN_PASSANT && 0 >= threshold);
}

static inline int promotion(struct position *pos, move_t *move, int threshold) {
	UNUSED(pos);
	return move_flag(move) == MOVE_PROMOTION && move_promote(move) + 2 >= threshold;
}

int find_next(struct movepicker *mp, int (*filter)(struct position *, move_t *, int), int threshold) {
	for (int i = 0; mp->move[i]; i++) {
		if (filter(mp->pos, &mp->move[i], threshold)) {
			move_t move = mp->move[i];
			mp->move[i] = mp->move[0];
			mp->move[0] = move;
			return 1;
		}
	}
	return 0;
}

void sort_moves(struct movepicker *mp) {
	if (!mp->move[0])
		return;
	for (int i = 1; mp->move[i]; i++) {
		int64_t eval = mp->eval[i];
		move_t move = mp->move[i];
		int j;
		for (j = i - 1; j >= 0 && mp->eval[j] < eval; j--) {
			mp->eval[j + 1] = mp->eval[j];
			mp->move[j + 1] = mp->move[j];
		}
		mp->eval[j + 1] = eval;
		mp->move[j + 1] = move;
	}
}

void evaluate_nonquiet(struct movepicker *mp) {
	for (int i = 0; mp->move[i]; i++) {
		move_t *move = &mp->move[i];
		int square_from = move_from(move);
		int square_to = move_to(move);
		int attacker = mp->pos->mailbox[square_from];
		int victim = mp->pos->mailbox[square_to];
		mp->eval[i] = victim ? mvv_lva(attacker, victim) : 0;
	}
}

void evaluate_quiet(struct movepicker *mp) {
	for (int i = 0; mp->move[i]; i++) {
		move_t *move = &mp->move[i];
		int from = move_from(move);
		int to = move_to(move);
		int attacker = mp->pos->mailbox[from];
		mp->eval[i] = mp->si->history_moves[attacker][to];
	}
}

void filter_moves(struct movepicker *mp) {
	for (int i = 0; mp->move[i]; i++) {
		if (mp->move[i] == mp->ttmove || mp->move[i] == mp->killer1 || mp->move[i] == mp->killer2) {
			mp->move[i] = mp->end[-1];
			*--mp->end = 0;
		}
	}
}

/* Need to check for duplicates with tt and killer moves somehow. */
move_t next_move(struct movepicker *mp) {
	int i;
	switch (mp->stage) {
	case STAGE_TT:
		mp->stage++;
		if (mp->ttmove)
			return mp->ttmove;
		/* fallthrough */
	case STAGE_GENNONQUIET:
		mp->stage++;
		mp->end = movegen(mp->pos, mp->pstate, mp->move, MOVETYPE_NONQUIET);
		filter_moves(mp);
		/* fallthrough */
	case STAGE_SORTNONQUIET:
		evaluate_nonquiet(mp);
		sort_moves(mp);
		mp->stage++;
		/* fallthrough */
	case STAGE_GOODCAPTURE:
		if (find_next(mp, &good_capture, 100))
			return *mp->move++;
		mp->stage++;
		/* fallthrough */
	case STAGE_PROMOTION:
		if (find_next(mp, &promotion, QUEEN))
			return *mp->move++;
		mp->stage++;
		/* fallthrough */
	case STAGE_KILLER1:
		mp->stage++;
		if (mp->killer1)
			return mp->killer1;
		/* fallthrough */
	case STAGE_KILLER2:
		mp->stage++;
		if (mp->killer2)
			return mp->killer2;
		/* fallthrough */
	case STAGE_OKCAPTURE:
		if (find_next(mp, &good_capture, 0))
			return *mp->move++;
		mp->stage++;
		/* fallthrough */
	case STAGE_GENQUIET:
		if (mp->quiescence && !mp->pstate->checkers)
			return 0;
		/* First put all the bad leftovers at the end. */
		for (i = 0; mp->move[i]; i++)
			mp->bad[-i] = mp->move[i];
		mp->bad[-i] = 0;

		mp->stage++;
		mp->end = movegen(mp->pos, mp->pstate, mp->move, MOVETYPE_QUIET);
		filter_moves(mp);
		/* fallthrough */
	case STAGE_SORTQUIET:
		evaluate_quiet(mp);
		sort_moves(mp);
		mp->stage++;
		/* fallthrough */
	case STAGE_QUIET:
		if (*mp->move)
			return *mp->move++;
		mp->stage++;
		/* fallthrough */
	case STAGE_BAD:
		if (*mp->bad)
			return *mp->bad--;
		mp->stage++;
		/* fallthrough */
	case STAGE_DONE:
		return 0;
	default:
		assert(0);
		return 0;
	}
}

void movepicker_init(struct movepicker *mp, int quiescence, struct position *pos, const struct pstate *pstate, move_t ttmove, move_t killer1, move_t killer2, const struct searchinfo *si) {
	mp->quiescence = quiescence;

	mp->move = mp->moves;
	mp->move[0] = 0;
	mp->eval = mp->evals;
	mp->eval[0] = 0;
	mp->bad = &mp->moves[MOVES_MAX - 1];

	mp->pos = pos;
	mp->pstate = pstate;
	mp->si = si;

	mp->ttmove = (!quiescence || mp->pstate->checkers || is_capture(mp->pos, &ttmove) || move_flag(&ttmove) == MOVE_PROMOTION) &&
		pseudo_legal(mp->pos, mp->pstate, &ttmove) ?
		ttmove : 0;

	mp->killer1 = pseudo_legal(mp->pos, mp->pstate, &killer1) ? killer1 : 0;
	mp->killer2 = pseudo_legal(mp->pos, mp->pstate, &killer2) ? killer2 : 0;

	mp->stage = STAGE_TT;
}
