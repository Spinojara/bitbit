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

#include "interrupt.h"

#include <signal.h>

#include "util.h"

volatile int interrupt = 0;

void sigint_handler(int num) {
	UNUSED(num);
	interrupt = 1;
	signal(SIGINT, sigint_handler);
}

void interrupt_init(void) {
	signal(SIGINT, sigint_handler);
}

void interrupt_term(void) {
	signal(SIGINT, SIG_DFL);
}
