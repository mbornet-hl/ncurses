# ============================================================================
# Copyright (C) 2023-2024, Martial Bornet
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#	@(#)	[MB] fd_Makefile	Version 1.3 du 24/07/22 - 
#
# ============================================================================

CC			= gcc
LDFLAGS		= -lncurses -lm

all			: sources bin
			@ ls -l

sources		: test_01.c rectangle.c

bin			: matrix_01 matrix_02 matrix_03 matrix_04 rectangle test_01

matrix_01		: fd_matrix_01.c
			$(CC) -o matrix_01 fd_matrix_01.c $(LDFLAGS)

matrix_02		: fd_matrix_02.c
			$(CC) -o matrix_02 fd_matrix_02.c $(LDFLAGS)

matrix_03		: fd_matrix_03.c
			$(CC) -o matrix_03 fd_matrix_03.c $(LDFLAGS)

matrix_04		: fd_matrix_04.c
			$(CC) -o matrix_04 fd_matrix_04.c $(LDFLAGS)

rectangle		: rectangle.c
			$(CC) -o rectangle rectangle.c $(LDFLAGS)

rectangle.c	: fd_rectangle.c
			@ ln -s fd_rectangle.c rectangle.c

test_01.c		: fd_test_01.c
			@ ln -s fd_test_01.c test_01.c

test_01		: test_01.c
			$(CC) -o test_01 test_01.c $(LDFLAGS)

clean		: test_01
			@ rm -f $<
