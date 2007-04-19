/*
 * rctypes.h
 *
 * Misc types and macro's.
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

#ifndef __RCTYPES_H__
#define __RCTYPES_H__

/* Min/Max macro's */
#ifdef MAX
#  undef MAX
#endif
#define MAX(_a, _b)	(((_a) > (_b)) ? (_a) : (_b))
#ifdef MIN
#  undef MIN
#endif
#define MIN(_a, _b)	((_a) > (_b) ? (_b) : (_a))

typedef enum {
  FALSE,
  TRUE
} bool;

#endif /* __RCTYPES_H__ */
