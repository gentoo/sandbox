{
	int pipefd[2];

	if (pipe2(pipefd, O_CLOEXEC) == -1)
		return -1;

	pid_t pid = fork();
	if (pid == -1) {
		int fork_errno = errno;
		close(pipefd[1]);
		close(pipefd[0]);
		errno = fork_errno;
		return -1;
	}

	if (pid == 0) {
		EXEC_FUNC;
		if (write(pipefd[1], &errno, sizeof(errno)) == -1)
			exit(126);
		exit(127);
	}

	close(pipefd[1]);

	int child_errno = 0;
	ssize_t read_result = read(pipefd[0], &child_errno, sizeof(child_errno));
	int read_errno = errno;

	close(pipefd[0]);
	kill(pid, SIGKILL);
	wait(NULL);

	errno = read_errno;
	if (read_result <= 0)
		return read_result;

	errno = child_errno;
	return -1;
}
