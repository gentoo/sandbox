/* This is slightly wrong as libsbutil code is supposed to work both in
 * libsandbox and in sandbox, but egetcwd is only available in the former.
 * We'll worry about that if/when it becomes an issue for other programs.
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

_GL_INLINE_HEADER_BEGIN

extern char *egetcwd(char *buf, size_t size);

_GL_INLINE char *xgetcwd(void)
{
	char *ret = egetcwd(NULL, 0);
	if (ret == NULL && errno == ENOMEM)
		xalloc_die();
	return ret;
}

_GL_INLINE_HEADER_END
