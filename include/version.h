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

#ifndef VERSION_H
#define VERSION_H

char compiler[] =
#if __clang__
"clang "
MACRO_VALUE(__clang_major__)"."
MACRO_VALUE(__clang_minor__)"."
MACRO_VALUE(__clang_patchlevel__)
#elif _MSC_VER
"MSVC "
MACRO_VALUE(_MSC_FULL_VER)"."
MACRO_VALUE(_MSC_BUILD)
#elif __GNUC__
"gcc "
MACRO_VALUE(__GNUC__)"."
MACRO_VALUE(__GNUC_MINOR__)"."
MACRO_VALUE(__GNUC_PATCHLEVEL__)
#else
"unknown"
#endif
#if __MINGW64__
" (MinGW-w64)"
#elif __MINGW32__
" (MinGW-w32)"
#endif
;

char environment[] =
#if __APPLE__
"Apple"
#elif __CYGWIN__
"Cygwin"
#elif _WIN64
"Microsoft Windows 64-bit"
#elif _WIN32
"Microsoft Windows 32-bit"
#elif __linux__
"Linux"
#elif __unix__
"Unix"
#else
"unknown"
#endif
;

/* MSVC does not allow a macro based transformation. */
char *date(char *str) {
#ifdef __DATE__
	/* Year. */
	str[0] = __DATE__[9];
	str[1] = __DATE__[10];
	/* First digit of month. */
	/* Oct, Nov, Dev. */
	str[2] = (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0';
	/* second digit of month */
	str[3] =
	(__DATE__[2] == 'l') ? '7' : /* Jul. */
	(__DATE__[2] == 'g') ? '8' : /* Aug. */
	(__DATE__[1] == 'u') ? '6' : /* Jun. */
	(__DATE__[2] == 'n') ? '1' : /* Jan. */
	(__DATE__[2] == 'y') ? '5' : /* May. */
	(__DATE__[1] == 'a') ? '3' : /* Mar. */
	(__DATE__[2] == 'b') ? '2' : /* Feb. */
	(__DATE__[0] == 'A') ? '4' : /* Apr. */
	(__DATE__[2] == 'p') ? '9' : /* Sep. */
	(__DATE__[2] == 't') ? '0' : /* Oct. */
	(__DATE__[2] == 'v') ? '1' : /* Nov. */
	'2'; /* Dec. */

	/* First digit of day. */
	str[4] = (__DATE__[4] == ' ') ? '0' : __DATE__[4];
	/* Second digit of day. */
	str[5] = __DATE__[5];
	str[6] = '\0';
	return str;
#else
	sprintf(str, "unknown");
	return str;
#endif
}

char simd[] =
#if defined(AVX2)
"avx2"
#elif defined(SSE4)
"sse4"
#elif defined(SSE2)
"sse2"
#else
"none"
#endif
;

#endif
