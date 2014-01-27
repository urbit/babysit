/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "estrtol.h"

long
estrtol(const char *s, int base)
{
	char *end;
	long n;

	errno = 0;
	n = strtol(s, &end, base);
	if(*end != '\0') {
		if(base == 0)
			fprintf(stderr, "%s: not an integer\n", s);
		else
			fprintf(stderr, "%s: not a base %d integer\n", s, base);
		exit(1);
	}
	if(errno != 0) {
		fprintf(stderr, "%s:", s);
		exit(1);
	}

	return n;
}

