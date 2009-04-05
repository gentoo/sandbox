/* lock.c - providing locking mechanisms
 *
 * Copyright 2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "libsandbox.h"

#if defined(HAVE_PTHREAD_H)

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void sb_lock(void)
{
	pthread_mutex_lock(&lock);
}

void sb_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

#elif defined(HAVE___SYNC_LOCK_TEST_AND_SET)

static int lock = 0;

void sb_lock(void)
{
	while (__sync_lock_test_and_set(&lock, 1))
		continue;
}

void sb_unlock(void)
{
	__sync_lock_release(&lock);
}

#else
# error no locking mech
#endif
