{
	/*  0 = success
	 * -1 = exec failed
	 * -2 = vfork failed
	 * -3 = vfork is broken (no shared memory)
	 */
	volatile int exec_result = -3;
	volatile int exec_errno = errno;

	pid_t pid = vfork();
	if (pid == -1)
		return -2;

	if (pid == 0) {
		exec_result = 0;
		exec_result = EXEC_FUNC;
		exec_errno = errno;
		_exit(127);
	}

	errno = exec_errno;
	return exec_result;
}
