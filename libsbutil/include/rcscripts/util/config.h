/*
 * config.h
 *
 * Miscellaneous config related macro's and functions.
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

#ifndef __RC_CONFIG_H__
#define __RC_CONFIG_H__

#include <stdio.h>

/* The following return a pointer on success, or NULL with errno set on error.
 * If it returned NULL, but errno is not set, then there was no error, but
 * there is nothing to return. */
/* rc_get_cnf_entry() will get the last config entry (ala bash variable
 * assignment rules) if 'sep' equals NULL, else it will concat all values
 * together seperated by 'sep'. */
char *rc_get_cnf_entry (const char *pathname, const char *entry,
			const char *sep);
char ** rc_get_list_file (char **list, char *filename);

#endif /* __RC_CONFIG_H__ */
