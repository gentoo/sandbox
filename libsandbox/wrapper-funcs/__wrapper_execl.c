int WRAPPER_NAME(const char *path, const char *arg, ...)
{
	int argc = 1;
	va_list ap;

	va_start(ap, arg);
	while (va_arg(ap, char *))
		++argc;
	va_end(ap);

	char *argv[argc + 1];
	argv[0] = (char *)arg;

	va_start(ap, arg);
	for (int i = 1; i <= argc; ++i)
		argv[i] = va_arg(ap, char *);
#ifdef EXEC_ENVP
	char **envp = va_arg(ap, char **);
#endif
	va_end(ap);

#ifdef EXEC_ENVP
	return EXEC_FUNC(path, argv, envp);
#else
	return EXEC_FUNC(path, argv);
#endif
}
#undef EXEC_ENVP
#undef EXEC_FUNC
