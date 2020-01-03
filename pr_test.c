#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(void)
{
    for (int i = 0; i < 10; i++)
    {
        int pid = fork();

        if (pid == 0)
        {
            chpr(80-i);
            sleep(i*100);
            while(1)
                printf(1, "%d ", i);
        }
    }
    while(1);
}