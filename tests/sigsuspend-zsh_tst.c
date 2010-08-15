/*
 * Based on zsh's sigsuspend() configure test.  Idea here is:
 *  - set a handler for SIGCHLD
 *  - fork a child
 *  - have parent do sigsuspend() and wait for SIGCHLD
 *  - have child exit notifying the parent
 *
 * If sandbox is mucking with SIGCHLD generation, this will often hang. #289963
 */

#include "tests.h"

static int child;

void handler(int sig) { child = sig; }

int main()
{
	struct sigaction act;
	sigset_t set;

	/* Register the SIGCHLD handler */
	act.sa_handler = &handler;
	sigfillset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCHLD, &act, 0);

	/* Mask all signals */
	sigfillset(&set);
	sigprocmask(SIG_SETMASK, &set, 0);

	/* Fork the child */
	pid_t pid = fork();
	if (pid == -1)
		abort();

	if (pid == 0) {
		/* Have child just exit to automatically send up SIGCHLD */
		return 0;
	} else {
		/* Have parent enable all signals and wait for SIGCHLD */
		sigemptyset(&set);
		sigsuspend(&set);
		return child == SIGCHLD ? 0 : 1;
	}
}
