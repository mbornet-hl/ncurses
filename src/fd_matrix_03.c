/*
 *	@(#)	[MB] fd_matrix_03.c	Version 1.1 du 23/07/08 - 
 */

#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
	int			 _n, _p, _i, _j;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s n p\n", argv[0]);
		exit(1);
	}

	_n			= atoi(argv[1]);
	_p			= atoi(argv[2]);


	for (_i = 1; _i <= _n; _i++) {
		for (_j = 1; _j <= _p; _j++) {
			printf("(%2d, %2d)   ", _i, _j);
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
