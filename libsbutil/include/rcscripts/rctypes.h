/*
 * rctypes.h
 *
 * Misc types and macro's.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
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

#endif /* __RCTYPES_H__ */
