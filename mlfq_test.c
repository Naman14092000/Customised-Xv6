#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "userdata.h"

int main(void)
{
    struct proc_stat *p=malloc(sizeof(struct proc_stat));

    for (int i = 0; i < 3; i++)
    {
        int pid = fork();

        if (pid == 0)
        {
            while (1)
            {
                // for(int i=0;i<100000000;i++)
                // {
                    // printf(1,"");
                // }
                int pid=getpinfo(p);
                printf(1, "process %d with id %d\nruntime : %d ticks\nnum_run : %d\ncurr_q : %d\nticks : %d %d %d %d %d\n", i, p->pid, p->runtime, p->num_run, p->current_queue, p->ticks[0], p->ticks[1], p->ticks[2], p->ticks[3], p->ticks[4]);
                sleep(30);
            }
        }
    }
    while (1)
        ;
}