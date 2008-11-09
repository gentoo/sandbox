/*
 * rccore.h
 *
 * Core includes.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
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
