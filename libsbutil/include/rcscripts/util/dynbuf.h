/*
 * dynbuf.h
 *
 * Dynamic allocated buffers.
 *
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Header$
 */

#ifndef __RC_DYNBUF_H__
#define __RC_DYNBUF_H__

typedef struct
{
  char *data;			/* Actual data */
  size_t length;		/* Length of data block */
  size_t rd_index;		/* Current read index */
  size_t wr_index;		/* Current write index */
  bool file_map;		/* File mapped as dynbuf */
} rc_dynbuf_t;

rc_dynbuf_t *rc_dynbuf_new (void);

rc_dynbuf_t *rc_dynbuf_new_mmap_file (const char *name);

void rc_dynbuf_free (rc_dynbuf_t *dynbuf);

int rc_dynbuf_write (rc_dynbuf_t *dynbuf, const char *buf, size_t length);

int rc_dynbuf_write_fd (rc_dynbuf_t *dynbuf, int fd, size_t length);

int rc_dynbuf_sprintf (rc_dynbuf_t *dynbuf, const char *format, ...);

int rc_dynbuf_read (rc_dynbuf_t *dynbuf, char *buf, size_t length);

int rc_dynbuf_read_fd (rc_dynbuf_t *dynbuf, int fd, size_t length);

char *rc_dynbuf_read_line (rc_dynbuf_t *dynbuf);

int rc_dynbuf_replace_char (rc_dynbuf_t *dynbuf, const char old, const char new);

bool rc_dynbuf_read_eof (rc_dynbuf_t *dynbuf);

inline bool rc_check_dynbuf (rc_dynbuf_t *dynbuf);
inline bool __rc_check_arg_dynbuf (rc_dynbuf_t *dynbuf, const char *file,
				 const char *func, size_t line);

#define rc_check_arg_dynbuf(_dynbuf) \
 __rc_check_arg_dynbuf (_dynbuf, __FILE__, __FUNCTION__, __LINE__)

#endif /* __RC_DYNBUF_H__ */

