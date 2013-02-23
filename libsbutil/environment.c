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

static bool env_is_in(const char *env, const char *values[], bool *set)
{
	size_t i = 0;
	const char *val;

	if (unlikely(!env))
		return (*set = false);

	val = getenv(env);
	*set = (val != NULL);
	if (unlikely(!*set))
		return false;

	while (values[i])
		if (!strcasecmp(val, values[i++]))
			return true;

	return false;
}

bool is_env_set_on(const char *env, bool *set)
{
	static const char *values[] = {
		"1", "true", "yes", NULL,
	};
	return env_is_in(env, values, set);
}
bool is_env_on(const char *env)
{
	bool set;
	return is_env_set_on(env, &set);
}

bool is_env_set_off(const char *env, bool *set)
{
	static const char *values[] = {
		"0", "false", "no", NULL,
	};
	return env_is_in(env, values, set);
}
bool is_env_off(const char *env)
{
	bool set;
	return is_env_set_off(env, &set);
}
