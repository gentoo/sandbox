/*
 * sb_method.c
 *
 * Util functions for sandbox method settings.
 *
 * Copyright 2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

sandbox_method_t parse_sandbox_method(const char *method)
{
	if (method == NULL || streq(method, "") || streq(method, "any"))
		return SANDBOX_METHOD_ANY;

	if (streq(method, "preload"))
		return SANDBOX_METHOD_PRELOAD;

	return SANDBOX_METHOD_ANY;
}

const char *str_sandbox_method(sandbox_method_t method)
{
	switch (method) {
		case SANDBOX_METHOD_PRELOAD:
			return "preload";
		case SANDBOX_METHOD_ANY:
			return "any";
		default:
			return "";
	}
}
