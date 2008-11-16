/* sb_printf.c
 * Minimal printf implementation that is designed to work without calling back
 * into the C library or libsandbox.  Goes straight to kernel so that this
 * should be usable from anywhere.  All state is on stack, so also avoids
 * signal, threaded, and other fun async behavior.
 *
 * The following conversion specifiers are supported:
 *	c - character
 *	s - string
 *	i - integer
 *	u - unsigned integer
 *	X - HEX number
 *	x - hex number
 *	p - pointer
 *
 * The following modifiers are supported:
 *	z - size_t
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

void sb_vfdprintf(int fd, const char *format, va_list args)
{
	const char *fmt = format;
	const char *format_end = format + strlen(format);
	const char *conv = fmt;

	while (fmt < format_end) {
		conv = memchr(fmt, '%', format_end - fmt);

		if (conv != fmt) {
			size_t out_bytes = (conv ? conv : format_end) - fmt;
			sb_write(fd, fmt, out_bytes);
			if (!conv)
				return;
		}

		char buf[50];
		size_t idx;
		int i;
		unsigned int u, x;
		char hex_base;
		size_t hex_pad = 0;

		unsigned int modifiers = 0;
 eat_more:
		switch (conv[1]) {
			default:
				sb_fdprintf(fd, "{invalid conversion specifier in string: %s}", format);
				break;

			case '%':
				if (modifiers)
					sb_fdprintf(fd, "{invalid modifier in string: %s}", format);
				sb_write(fd, conv, 1);
				break;
			case 'z':
				++conv;
				if (modifiers & 0x1)
					sb_fdprintf(fd, "{invalid modifier in string: %s}", format);
				modifiers |= 0x1;
				goto eat_more;

			case 'c': {
				char c = va_arg(args, int);
				sb_write(fd, &c, 1);
				break;
			}
			case 's': {
				char *s = va_arg(args, char *);
				sb_write(fd, s, strlen(s));
				break;
			}

			case 'i': {
				i = va_arg(args, int);
				u = i;
				if (i < 0) {
					sb_write(fd, "-", 1);
					u = i * -1;
				}
 out_uint:
				idx = 0;
				do {
					buf[idx++] = '0' + (u % 10);
				} while (u /= 10);
				while (idx--)
					sb_write(fd, buf+idx, 1);
				break;
			}
			case 'u': {
				u = va_arg(args, unsigned int);
				goto out_uint;
				break;
			}

			case 'X': {
				hex_base = 'A';
				goto out_hex;
			}
			case 'x': {
				hex_base = 'a';
 out_hex:
				x = va_arg(args, unsigned int);
 out_ptr:
				idx = 0;
				do {
					u = x % 16;
					buf[idx++] = (u < 10 ? '0' + u : hex_base + u - 10);
				} while (x /= 16);
				while (idx < hex_pad)
					buf[idx++] = '0';
				buf[idx++] = 'x';
				buf[idx++] = '0';
				while (idx--)
					sb_write(fd, buf+idx, 1);
				break;
			}
			case 'p': {
				void *p = va_arg(args, void *);
				hex_base = 'a';
				hex_pad = sizeof(p) * 2;
				x = (unsigned long)p;
				goto out_ptr;
			}
		}

		fmt = conv + 2;
	}
}

void sb_fdprintf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vfdprintf(fd, format, args);
	va_end(args);
}

void sb_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vfdprintf(STDOUT_FILENO, format, args);
	va_end(args);
}
