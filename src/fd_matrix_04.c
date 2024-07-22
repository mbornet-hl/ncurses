/*
 *	@(#)	[MB] fd_matrix_04.c	Version 1.1 du 23/07/08 - 
 */

#include <stdio.h>
#include <stdlib.h>

/******************************************************************************

						VALUE

******************************************************************************/
int value(int x, int y)
{
	int			 _val;

	if (x == y) {
		_val			= 15 * 10;
	}
	else {
		_val			= (15 - abs(x - y)) * 10;
	}

	return _val;
}

/******************************************************************************

						MAIN

******************************************************************************/
int main(int argc, char *argv[])
{
	int			 _n, _p, _i, _j;
	char			 _buf[32];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s n p\n", argv[0]);
		exit(1);
	}

	_n			= atoi(argv[1]);
	_p			= atoi(argv[2]);


	for (_i = 1; _i <= _n; _i++) {
		for (_j = 1; _j <= _p; _j++) {
			sprintf(_buf, "(%d, %d)", _i, _j);
			printf("%-10s ", _buf);
		}
		printf("\n");

		for (_j = 1; _j <= _p; _j++) {
			printf("%10f ", (double) value(_i, _j));
		}
		printf("\n");

		printf("\n");
	}

	return 0;
}
