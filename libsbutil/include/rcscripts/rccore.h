/*
 * rccore.h
 *
 * Core includes.
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

#ifndef __RCCORE_H__
#define __RCCORE_H__

#include "rcscripts/rcdefines.h"
#include "rcscripts/rcutil.h"

#include "rcscripts/core/services.h"

/* Initialize needed variables, etc.  Should be called before anything else
 * from the rccore library is used.  Return 0 on success, else -1 and sets
 * errno.
 */
int rc_init (void);

#endif /* __RCCORE_H__ */
