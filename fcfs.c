#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc, char *argv[])
{
    for (int i = 0; i < 10; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            chpr(10-i);
            sleep(100);
            printf(1, "%d %d\n", i, pid);
            exit();
        }
    }
    while (1)
        ;
}