/*
 * environment.c
 *
 * Environment utility functions.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

static const char * const true_values[] = {
	"1", "true", "yes", NULL,
};

static const char * const false_values[] = {
	"0", "false", "no", NULL,
};

static bool val_is_in(const char *val, const char * const values[])
{
	size_t i = 0;

	while (values[i])
		if (!strcasecmp(val, values[i++]))
			return true;

	return false;
}

static bool env_is_in(const char *env, const char * const values[], bool *set)
{
	const char *val;

	if (unlikely(!env))
		return (*set = false);

	val = getenv(env);
	*set = (val != NULL);
	if (unlikely(!*set))
		return false;

	return val_is_in(val, values);
}

bool is_val_on(const char *val)
{
	return val_is_in(val, true_values);
}
bool is_val_off(const char *val)
{
	return val_is_in(val, false_values);
}

bool is_env_set_on(const char *env, bool *set)
{
	return env_is_in(env, true_values, set);
}
bool is_env_on(const char *env)
{
	bool set;
	return is_env_set_on(env, &set);
}

bool is_env_set_off(const char *env, bool *set)
{
	return env_is_in(env, false_values, set);
}
bool is_env_off(const char *env)
{
	bool set;
	return is_env_set_off(env, &set);
}
