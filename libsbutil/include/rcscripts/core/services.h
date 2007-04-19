/*
 * services.h
 *
 * Functions dealing with services.
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

#ifndef __RC_SERVICES_H__
#define __RC_SERVICES_H__

typedef enum {
  rc_service_coldplugged,
  rc_service_starting,
  rc_service_started,
  rc_service_inactive,
  rc_service_wasinactive,
  rc_service_stopping
} rc_service_state_t;

bool rc_service_test_state (const char *service, rc_service_state_t state);

#endif /* __RC_SERVICES_H__ */
