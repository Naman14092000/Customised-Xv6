#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "userdata.h"
int main(int argc, char *argv[])
{
    int pid;
    int k, n;
    int x, z;
    int *rtime = malloc(sizeof(int));
    int *wtime = malloc(sizeof(int));

    if (argc != 2)
    {
        printf(2, "usage: %s n\n", argv[0]);
    }
    n = atoi(argv[1]);
    if (fork() == 0)
    {
        for (k = 0; k < n; k++)
        {
            pid = fork();
            if (pid < 0)
            {
                exit();
            }
            else if (pid == 0)
            {
                chpr(80 - k);
                sleep(1000);
                // printf(1,"child=%d\n",getpid());
                for (z = 0; z < 100000000; z++)
                {
                    x = (x + 25 * 65); // useless calculations to consume CPU time
                    x = 0;
                    printf(1, "");
                }
// #ifdef MLFQ
//                 struct proc_stat *p = malloc(sizeof(struct proc_stat));
//                 int g = getpinfo(p);
//                 printf(1, "process %d with id %d\nruntime : %d ticks\nnum_run : %d\ncurr_q : %d\nticks : %d %d %d %d %d\n", k, p->pid, p->runtime, p->num_run, p->current_queue, p->ticks[0], p->ticks[1], p->ticks[2], p->ticks[3], p->ticks[4]);
//                 printf(1, "pid=%d\n", g);
// #endif
                exit();
            }
        }

        for (k = 0; k < n; k++)
        {

            int pid = waitx(wtime, rtime);

            printf(1, "CHILD %d :-\n Child with pid=%d wtime=%d rtime=%d\n", k, pid, *wtime, *rtime);
        }
        exit();
    }
    else
    {
        int pid = waitx(wtime, rtime);
        printf(1, "time=%d\n", (*rtime) + (*wtime));
    }

    exit();
}