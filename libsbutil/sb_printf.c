/* sb_printf.c
 * Minimal printf implementation that is designed to work without calling back
 * into the C library or libsandbox.  Goes straight to kernel so that this
 * should be usable from anywhere.  All state is on stack, so also avoids
 * signal, threaded, and other fun async behavior.
 *
 * The following conversion specifiers are supported:
 *	c - character
 *	s - string
 *	d - integer
 *	i - integer
 *	u - unsigned integer
 *	X - HEX number
 *	x - hex number
 *	p - pointer
 *
 * The following modifiers are supported:
 *	z - size_t
 *	* - width is an argument
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

#define MOD_SIZE_T (1 << 0)
#define MOD_STAR   (1 << 1)

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
		unsigned int u, x;
		char hex_base;
		size_t padding = 0;

		unsigned int modifiers = 0;
 eat_more:
		switch (conv[1]) {
			default:
				sb_fdprintf(fd, "{invalid conversion specifier in string: %s}", format);
				break;

			case '%':
				if (modifiers) {
 inv_modifier:
					sb_fdprintf(fd, "{invalid modifier in string: %s}", format);
				}
				sb_write(fd, conv, 1);
				break;
			case 'z':
				++conv;
				if (modifiers & MOD_SIZE_T)
					goto inv_modifier;
				modifiers |= MOD_SIZE_T;
				goto eat_more;
			case '*':
				++conv;
				if (modifiers & MOD_STAR)
					goto inv_modifier;
				modifiers |= MOD_STAR;
				padding = va_arg(args, int);
				goto eat_more;

			case 'c': {
				char c = va_arg(args, int);
				sb_write(fd, &c, 1);
				break;
			}
			case 's': {
				char *s = va_arg(args, char *);
				if (!s)
					s = "(null)";
				size_t len = strlen(s);
				while (len < padding--)
					sb_write(fd, " ", 1);
				sb_write(fd, s, len);
				break;
			}

			case 'd':
			case 'i': {
				long i;
				if (modifiers & MOD_SIZE_T)
					i = va_arg(args, ssize_t);
				else
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
				if (modifiers & MOD_SIZE_T)
					x = va_arg(args, size_t);
				else
					x = va_arg(args, unsigned int);
 out_ptr:
				idx = 0;
				do {
					u = x % 16;
					buf[idx++] = (u < 10 ? '0' + u : hex_base + u - 10);
				} while (x /= 16);
				while (idx < padding)
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
				padding = sizeof(p) * 2;
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

/* All output goes to stderr as any unexpected interjection into stdout
 * could easily break shell scripts and such that are piping or evaluating
 * or some such thing.
 */
void sb_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vfdprintf(STDERR_FILENO, format, args);
	va_end(args);
}

static bool nocolor, init_color = false;
void sb_efunc(const char *color, const char *hilight, const char *format, ...)
{
	save_errno();

	int fd = STDERR_FILENO;

	if (!init_color) {
		nocolor = is_env_on(ENV_NOCOLOR);
		init_color = true;
	}

	if (!nocolor)
		sb_fdprintf(fd, "%s%s%s", color, hilight, COLOR_NORMAL);
	else
		sb_fdprintf(fd, "%s", hilight);

	va_list args;
	va_start(args, format);
	sb_vfdprintf(fd, format, args);
	va_end(args);

	restore_errno();
}
