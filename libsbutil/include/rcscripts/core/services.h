/*
 * services.h
 *
 * Functions dealing with services.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
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
