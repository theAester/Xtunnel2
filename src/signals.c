#include "signals.h"

static int _exitsig = 0;

void siginthand(int s){
	printf("exiting...\n");
	_exitsig = 1;
}

int setup_sigint() {
	struct sigaction sa;
	sa.sa_handler = siginthand;
	sa.sa_flags = 0;
	return sigaction(SIGINT, &sa, NULL);
}
