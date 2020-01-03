# INTRODUCTION

xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).xv6 loosely follows the structure and style of v6, 
but is implemented for a modern x86-based multiprocessor using ANSI C.

## ``` PRIORITY BASED ``` SCHEDULER

Priority scheduling is a method of scheduling processes based on priority.
In this method, the scheduler chooses the tasks to work as per the priority, 
which is different from other types of scheduling, for example, a simple
round robin.

Priority scheduling involves priority assignment to every process, and
processes with higher priorities are carried out first, whereas tasks with
equal priorities are carried out on a round robin basis. An example of a 
general-priority-scheduling algorithm is the shortest-job-first (SJF) algorithm.

    

``` 
    struct proc *highP = 0;
    struct proc *p1 = 0;

    for (p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++)
    {
      if (highP)
      {
        if ((p1->state == RUNNABLE) && (highP->priority > p1->priority))
          highP = p1;
        else if ((p1->state == RUNNABLE) && (highP->priority == p1->priority))
        {
          if (p1->last_executed < highP->last_executed)
          {
            highP = p1;
          }
        }
      }
      else
      {
        if (p1->state == RUNNABLE)
        {
          highP = p1;
        }
      }
    }

    if (highP != 0)
      p = highP;
    if (p != 0 && p->state == RUNNABLE)
    {
      // cprintf("pid=%d having priority=%d is now running\n",p->pid,p->priority);
      p->last_executed = ticks;
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
```

## ``` FIRST-COME-FIRST-SERVE ``` SCHEDULER

In FCFS policy, the process which arrives first is executed first. It will work
best when incoming processes are short and there is no need for the processes to
execute in a specific order.

``` 
struct proc *minP = 0;
    struct proc *p1;
    for (p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++)
    {
      if (p1->state != RUNNABLE)
        continue;

      // ignore init and sh processes from FCFS
      // if (p1->pid > 1)
      // {
      if (minP != 0)
      {
        // here I find the process with the lowest creation time (the first one that was created)
        if (p1->ctime < minP->ctime)
          minP = p1;
      }
      else
        minP = p1;
      // }
    }
    // If I found the process which I created first and it is runnable I run it
    //(in the real FCFS I should not check if it is runnable, but for testing purposes I have to make this control, otherwise every time I launch
    // a process which does I/0 operation (every simple command) everything will be blocked
    if (minP != 0 && minP->state == RUNNABLE)
    {
      p = minP;
    }
    if (p != 0 && p->state == RUNNABLE)
    {
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
```

## ``` MULTILEVEL FEEDBACK QUEUE ``` SCHEDULING

In this policy 5 different types of queues are kept. The one having more CPU
burst time is pushed in the last priority queue which leaves behind interactive
processes in the top priority queues.If a process uses too much CPU time, it will
be moved to a lower-priority queue. Similarly, a process that waits too long in a 
lower-priority queue may be moved to a higher-priority queue.

``` 
    int priority; 
    p = 0;
    for (priority = 0; priority <= 4; priority++)
    {
      int i = ptable.queue_head[priority];
      while (i != ptable.queue_tail[priority])
      {
        if (ptable.queue[priority][i])
        {
          p = ptable.queue[priority][i];
          break;
        }
        i = (i + 1) % NPROC;
      }
      if (p != 0)
        break;
    }
    if (p != 0)
    {

      // cprintf("name %s pid  %d que %d ticks %d\n",p->name ,p->pid ,p->queue,ticks);
      if (p->state != RUNNABLE)
        panic("process is sleeping");

      dequeue(p->queue);
      //For ensuring ticks for each cycle
      for (int a = 0; a < slice[p->queue]; a++)
      {
        if (p->state == RUNNABLE)
        {
          p->num_run++; // increasing the run count of the process

          c->proc = p;
          // cprintf("Process with pid=%d executed with a=%d slice[p->queue]=%d\n", p->pid, a, slice[p->queue]);

          switchuvm(p);
          p->state = RUNNING;
          swtch(&(c->scheduler), p->context);
          switchkvm();
        }
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
        if (p->state != RUNNABLE)
        {
          break;
        }
      }
      if (p->state == RUNNABLE)
      {
        if (p->queue < 4)
        {
          p->queue++;
          // cprintf("Process with pid=%d moved down to queue=%d\n", p->pid, p->queue);
        }
        enqueue(p->queue, p);
      }
    }
    if (ticks > ageing_update + Ageing_const)
    {
      for (int que_number = 1; que_number < 5; que_number++)
      {
        for (int i = ptable.queue_head[que_number]; i != ptable.queue_tail[que_number]; i = (i + 1) % NPROC)
        {
          p = ptable.queue[que_number][i];

          if (ticks - p->last_executed > Ageing_const)
          {
            // cprintf("pid %d ticks %d in %d que \n", p->pid, p->ticks[p->queue], p->queue);
            dequeue(que_number);
            enqueue(p->queue - 1, p);
          }
        }
        ageing_update = ticks;
      }

```

## ``` waitx ``` SYSTEM CALL

``` waitx ``` system call returns the number of context switches performed from
 **RUNNING** to **WAITING** and number of context switches performed during the
 **RUNNING** state and total number of context switches.
 

``` 
 int waitx(int *wtime, int *rtime)
 {
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  acquire(&ptable.lock);
  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
      if (p->parent != curproc)
        continue;
      havekids = 1;
      if (p->state == ZOMBIE)
      {
        // Found one.
        pid = p->pid;
        // cprintf("process with pid=%d creation time=%d end time=%d runtime=%d waittime=%d exited successfully\n", p->pid, p->ctime, p->etime, p->rtime, p->wtime);
        int g = (p->etime) - (p->ctime);
        // cprintf("%d\n\n", g);
        int h = g - (p->rtime);
        // cprintf("%d\n\n", h);
        // *wtime = p->etime - p->ctime - p->rtime;
        *rtime = p->rtime;
        // cprintf("Process with wtime=%d waddress=%d rtime=%d raddress=%d\n\n", *wtime, wtime, *rtime, rtime);
        *wtime = h;
        // cprintf("Process with wtime=%d rtime=%d\n\n", *wtime, *rtime);
        // cprintf("process with pid=%d creation time=%d end time=%d runtime=%d waittime=%d exited successfully\n", p->pid, p->ctime, p->etime, p->rtime, p->wtime);
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        p->ctime = 0;
        p->etime = 0;
        p->rtime = 0;
        p->wtime = 0;
        p->modtime = 0;
        // cprintf("ticks2=%d\n", ticks);
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || curproc->killed)
    {
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock); //DOC: wait-sleep
  }
 }

 ```

## ``` chpr ``` SYSTEM CALL

``` chpr ``` system call sets the priority of the process and
returns the old priority of the process.

``` 
int chpr(int priority)
{
  struct proc *p;
  int pid;
  pid = myproc()->pid;
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->pid == pid)
    {
      // cprintf("Process with pid %d has changed its priority from %d to %d\n", pid, p->priority, priority);
      p->priority = priority;
      break;
    }
  }
  release(&ptable.lock);

  return pid;
}
```

## ``` getpinfo ``` SYSTEM CALL

``` getpinfo ``` system call returns some basic information about each process: its process ID, 
total run time, how many times it has been chosen to run, which queue it is currently
on 0, 1, 2, 3 or 4 (check Task 2 part c) and ticks received in each queue.

``` 
int getpinfo(struct proc_stat *s)
{
  struct proc *p = myproc();
  if (p != 0)
  {
    s->pid = p->pid;
    s->runtime = p->rtime;
    s->num_run = p->num_run;
    s->current_queue = p->queue;
    for (int i = 0; i < 5; i++)
    {
      s->ticks[i] = p->ticks[i];
    }
    return 1;
  }
  else
  {
    return 0;
  }
}
```

## In defs.h
* Declared function for made syscalls waitx, setpriority, getpinfo.
* Added struct proc_stat.

## In proc.c
* Added function for syscalls waitx, setpriority, getpinfo.
* Added scheduling algorithms.
* Added required functions and struct to implement queue.
* Added iniatialise function in userinit function.
* Initialised struct proc variables in allocproc.
* Changed necessary variables for schduler implementation.
* Added queue no value in printf of procdump.

## In proc.h
* Added useful variables in struct proc.

## In syscall.h
* Added declaration of sys_waitx, sys_set_priority, sys_getpinfo.
* Added them in static int array syscalls.

## In syscall.h
* Added #define for SYS_waitx, SYS_set_priority, SYS_getpinfo.

## In sysproc.h
* Added functions sys_waitx, sys_set_priority, sys_getpinfo.

## In trap.c 
* Added necesarry variable for scheduler in trap function.

## In user.h
* Added declaration of function for syscalls waitx, setpriority, getpinfo.
* Declared struct proc_stat.

## In user.S
* Added definition for calling syscall waitx, setpriority, getpinfo.

## New files added
* waitx_test.c
    * This is tester for waitx it prints time status of command.
    * write time [Command Name] to run command.

* benchmark.c
    * Run it by command benchmark <n>.
    * Open OS by setting appropriate value of scheduler variable. Then run benchmark.
    * It shows pid of creation time as it creates them and wait time and run time when ends them.

* userdata.h
    * contains struct proc_stat.

* fcfs.c
    * This is test for fcfs Scheduling.

* mlfq_test.c
    * This is test for MLFQ Scheduling.
* pinfo_test.c
    * This is test for getpinfo.
* REPORT.pdf
    * This gives detailed comparison between different scheduling algorithm.