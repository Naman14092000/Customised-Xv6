#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc, char *argv[])
{
	int pid;
	int status;
	int *a=malloc(sizeof(int));
	int *b=malloc(sizeof(int));
	// printf(1,"address1=%d address2=%d\n",a,b);
	pid = fork();
	if (pid == 0)
	{
		sleep(100);
		exec(argv[1], argv);
		printf(1, "exec %s failed\n", argv[1]);
	}
	else
	{
		status = waitx(a, b);
		printf(1, "Wait Time = %d\n Run Time = %d with PID = %d \n", *a, *b, status);
	}
	exit();
}