#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "userdata.h"

int main(int argc, char *argv[])
{
    struct proc_stat *p=malloc(sizeof(struct proc_stat));
    int pid;
    pid=getpinfo(p);
    printf(1, "process %s with id %d\nruntime : %d ticks\nnum_run : %d\ncurrent_queue : %d\nticks : %d %d %d %d %d\n", argv[1], p->pid, p->runtime, p->num_run, p->current_queue, p->ticks[0], p->ticks[1], p->ticks[2], p->ticks[3], p->ticks[4]);

    for (int i = 0; i < 500; i++)
        printf(1, "%d", i);
    printf(1, "\n");
    pid=getpinfo(p);
    printf(1, "process %s with id %d\nruntime : %d ticks\nnum_run : %d\ncurrent_queue : %d\nticks : %d %d %d %d %d\n", argv[1], p->pid, p->runtime, p->num_run, p->current_queue, p->ticks[0], p->ticks[1], p->ticks[2], p->ticks[3], p->ticks[4]);

    for (int i = 0; i < 500; i++)
        printf(1, "%d", i);
    printf(1, "\n");
    pid=getpinfo(p);
    printf(1, "process %s with id %d\nruntime : %d ticks\nnum_run : %d\ncurrent_queue : %d\nticks : %d %d %d %d %d\n", argv[1], p->pid, p->runtime, p->num_run, p->current_queue, p->ticks[0], p->ticks[1], p->ticks[2], p->ticks[3], p->ticks[4]);
    printf(1,"%d\n",pid);
    exit();
    return 0;
}
