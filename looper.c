#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>


/*
==========task0b=============
==========notes==============
SIG_DFL default handler - what makes the real signal "execution"
SIGSTTP stop
SIGINT interrupt
SIGNCONT continue

*/
void handler(int sig)
{
	printf("\nRecieved Signal : %s\n", strsignal(sig));
	//if im stopping prepare for continue
	if (sig == SIGTSTP)
	{
		//changed from SIG_DFL  to handler
		signal(SIGCONT, handler);
	}
	//if im waking up(continue) make sure that prepare to be stoped again
	else if (sig == SIGCONT)
	{
		//changed from SIG_DFL  to handler
		signal(SIGTSTP, handler);
	}
	//now we mark the os that default handler will 'execute" the signal and not our handler
	signal(sig, SIG_DFL);
	//raise sending sig to the calling process(boomerang),
	//in the last line we marked default handler so now the signal will finally be executed
	raise(sig);
}


/*
==========notes==============
1)when we write signal we mark to the os to stop infinite loop when we get one of the signals
->then we transfer signal to handler
->when handler job is done we going back to sleep in infi loop(unless we kill the program)
*/
int main(int argc, char **argv)
{

	printf("Starting the program\n");
	signal(SIGINT, handler);
	signal(SIGTSTP, handler);
	signal(SIGCONT, handler);

	
	while (1)
	{
		sleep(1);
	}

	return 0;
}