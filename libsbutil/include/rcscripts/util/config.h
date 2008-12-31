/*
 * config.h
 *
 * Miscellaneous config related macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
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

#endif /* __RC_CONFIG_H__ */
