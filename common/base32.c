#include "base32.h"
#include <string.h>
#include <ctype.h>

static const char base32_alphabet[32] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', '2', '3', '4', '5', '6', '7'
};

int base32_encode(char *dst, size_t size, const unsigned char *data, size_t len)
{
	size_t i = 0;
	const unsigned char *p = data;
	const char *end = &dst[size];
	char *q = dst;

	do {
		size_t j, k;
		unsigned char x[5];
		char s[8];

		switch (len - i) {
		case 4:
			k = 7;
			break;
		case 3:
			k = 5;
			break;
		case 2:
			k = 3;
			break;
		case 1:
			k = 2;
			break;
		default:
			k = 8;
		}

		for (j = 0; j < 5; j++)
			x[j] = i < len ? p[i++] : 0;

		/*
		  +-------+-----------+--------+
		  | target| source	  | source |
		  | byte  | bits	  | byte   |
		  +-------+-----------+--------+
		  |		0 | 7 6 5 4 3 | 0	   |
		  |		1 | 2 1 0 7 6 | 0-1	   |
		  |		2 | 5 4 3 2 1 | 1	   |
		  |		3 | 0 7 6 5 4 | 1-2	   |
		  |		4 | 3 2 1 0 7 | 2-3	   |
		  |		5 | 6 5 4 3 2 | 3	   |
		  |		6 | 1 0 7 6 5 | 3-4	   |
		  |		7 | 4 3 2 1 0 | 4	   |
		  +-------+-----------+--------+
		*/

		s[0] = (x[0] >> 3);
		s[1] = ((x[0] & 0x07) << 2) | (x[1] >> 6);
		s[2] = (x[1] >> 1) & 0x1f;
		s[3] = ((x[1] & 0x01) << 4) | (x[2] >> 4);
		s[4] = ((x[2] & 0x0f) << 1) | (x[3] >> 7);
		s[5] = (x[3] >> 2) & 0x1f;
		s[6] = ((x[3] & 0x03) << 3) | (x[4] >> 5);
		s[7] = x[4] & 0x1f;

		for (j = 0; j < k && q != end; j++) {
			*q++ = base32_alphabet[(unsigned char) s[j]];
		}

		if (end == q) {
			break;
		}

	} while (i < len);

	return q - dst;
}

static char base32_map[(unsigned char) -1];


int base32_decode(unsigned char *dst, size_t size, const unsigned char *data, size_t len)
{
	const char *end = &dst[size];
	const unsigned char *p = data;
	char s[8];
	char *q = dst;
	int pad = 0;
	size_t i, si;

	if (0 == base32_map[0]) {
		for (i = 0; i < sizeof(base32_map)/sizeof(base32_map[0]); i++) {
			const char *x;

			x = (const char *)memchr(base32_alphabet, toupper(i),
					   sizeof base32_alphabet);
			base32_map[i] = x ? (x - base32_alphabet) : (unsigned char) -1;
		}
	}

	memset(&s[0], 0, sizeof s);
	si = 0;
	i = 0;

	while (i < len) {
		unsigned char c;

		c = p[i++];
		if ('=' == c) {
			pad++;
			c = 0;
		} else {
			c = base32_map[c];
			if ((unsigned char) -1 == c) {
				return -1;
			}
		}

		s[si++] = c;

		if (sizeof(s)/sizeof(s[0]) == si || pad > 0 || i == len) {
			unsigned char b[5];
			size_t bi;
			if (sizeof(s)/sizeof(s[0]) - si > 0) {
				memset(&s[si], 0, sizeof(s)/sizeof(s[0]) - si);
			}
			si = 0;

			b[0] =
				((s[0] << 3) & 0xf8) |
				((s[1] >> 2) & 0x07);
			b[1] =
				((s[1] & 0x03) << 6) |
				((s[2] & 0x1f) << 1) |
				((s[3] >> 4) & 1);
			b[2] =
				((s[3] & 0x0f) << 4) |
				((s[4] >> 1) & 0x0f);
			b[3] =
				((s[4] & 1) << 7) |
				((s[5] & 0x1f) << 2) |
				((s[6] >> 3) & 0x03);
			b[4] =
				((s[6] & 0x07) << 5) |
				(s[7] & 0x1f);

			for (bi = 0; bi < sizeof(b)/sizeof(b[0]) && q != end; bi++) {
				*q++ = b[bi];
			}
		}

		if (end == q) {
			break;
		}
	}

	return q - dst;
}

