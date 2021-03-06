#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "userdata.h"

struct
{
  struct spinlock lock;
  struct proc proc[NPROC];
  int queue_head[5];
  int queue_tail[5];
  struct proc *queue[5][NPROC];
  // int priCount[5];
} ptable;

int slice[] = {1, 2, 4, 8, 16};
static struct proc *initproc;

int nextpid = 1;
int flag = 0;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

#define Ageing_const 1000
void enqueue(int queue_number, struct proc *new)
{
  if (queue_number > 4 || queue_number < 0)
    panic("out of bound quenumber");
  int tail_end = ptable.queue_tail[queue_number];
  ptable.queue[queue_number][tail_end] = new;
  new->queue = queue_number;
  ptable.queue_tail[queue_number] = (ptable.queue_tail[queue_number] + 1) % NPROC;
  return;
}
void dequeue(int queue_number)
{
  if (queue_number > 4 || queue_number < 0)
    panic("out of bound quenumber");
  ptable.queue_head[queue_number] = (ptable.queue_head[queue_number] + 1) % NPROC;
}
void pinit(void)
{
  initlock(&ptable.lock, "ptable");
  // ptable.priCount[0] = -1;
  // ptable.priCount[1] = -1;
  // ptable.priCount[2] = -1;
  // ptable.priCount[3] = -1;
  // ptable.priCount[4] = -1;
}

// Must be called with interrupts disabled
int cpuid()
{
  return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *
mycpu(void)
{
  int apicid, i;

  if (readeflags() & FL_IF)
    panic("mycpu called with interrupts enabled\n");

  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i)
  {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc(void)
{
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if (p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->priority = 60;
  p->queue = 0;

  release(&ptable.lock);

  p->ctime = ticks;
  p->etime = ticks;
  p->retime = ticks;
  p->rtime = 0;
  p->wtime = 0;
  p->last_executed = 0;
  p->num_run = 0;
  for (int i = 0; i < 5; i++)
  {
    p->ticks[i] = 0;
  }
  // Allocate kernel stack.
  if ((p->kstack = kalloc()) == 0)
  {
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe *)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint *)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context *)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  initproc = p;
  if ((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0; // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  p->modtime = 0;
  enqueue(0, p);
  // p->retime = ticks;
  // cprintf("pid=%d added from userinit\n", p->pid);
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if (n > 0)
  {
    if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  else if (n < 0)
  {
    if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if ((np = allocproc()) == 0)
  {
    return -1;
  }

  // Copy process state from proc.
  if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0)
  {
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for (i = 0; i < NOFILE; i++)
    if (curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);
  np->state = RUNNABLE;
  np->modtime = 0;
  enqueue(0, np);
  // cprintf("Ticks1=%d\n", ticks);
  // ptable.priCount[0]++;
  // ptable.que[0][ptable.priCount[0]] = np;
  // cprintf("pid=%d added from fork\n", np->pid);
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if (curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for (fd = 0; fd < NOFILE; fd++)
  {
    if (curproc->ofile[fd])
    {
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }
  curproc->etime = ticks;

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);
  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->parent == curproc)
    {
      p->parent = initproc;
      if (p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}
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
// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(void)
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
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
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

// Modified wait for a child process to exit and return its pid.
// Return -1 if this process has no children.

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

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void)
{
  struct proc *p = 0;
  if (!flag)
  {
    for (int i = 0; i < 5; i++)
    {
      ptable.queue_head[i] = 0;
      ptable.queue_tail[i] = 0;
    }
    flag++;
  }
  struct cpu *c = mycpu();
  c->proc = 0;
  int ageing_update = ticks; // captures the last updated ageing time

  for (;;)
  {
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

#ifdef DEFAULT
    // cprintf("hi\n");
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
      if (p->state != RUNNABLE)
        continue;
      else
        break;
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
#else
#ifdef PBS
    // cprintf("hii\n");

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
#else
#ifdef FCFS
    // cprintf("hiii\n");

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

#else
#ifdef MLFQ

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
    }

#endif
#endif
#endif
#endif
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.

void sched(void)
{
  int intena;
  struct proc *p = myproc();

  if (!holding(&ptable.lock))
    panic("sched ptable.lock");
  if (mycpu()->ncli != 1)
    panic("sched locks");
  if (p->state == RUNNING)
    panic("sched running");
  if (readeflags() & FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  acquire(&ptable.lock); //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first)
  {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  if (p == 0)
    panic("sleep");

  if (lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if (lk != &ptable.lock)
  {                        //DOC: sleeplock0
    acquire(&ptable.lock); //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if (lk != &ptable.lock)
  { //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if (p->state == SLEEPING && p->chan == chan)
    {
      p->state = RUNNABLE;
      enqueue(p->queue, p); // changing here
      // ptable.priCount[p->queue]++;
      // ptable.que[p->queue][ptable.priCount[p->queue]] = p;
      // cprintf("Process with pid=%d added from wakeup1\n", p->pid);
    }
}

// Wake up all processes sleeping on chan.
void wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->pid == pid)
    {
      p->killed = 1;
      // Wake process from sleep if necessary.
      if (p->state == SLEEPING)
      {
        p->state = RUNNABLE;
        enqueue(p->queue, p);
        // ptable.priCount[p->queue]++;
        // ptable.que[p->queue][ptable.priCount[p->queue]] = p;
        // cprintf("pid=%d added from kill into queue\n", p->pid, p->queue);
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// this will change a process's priority

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

void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [EMBRYO] "embryo",
      [SLEEPING] "sleep ",
      [RUNNABLE] "runble",
      [RUNNING] "run   ",
      [ZOMBIE] "zombie"};
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s %d", p->pid, state, p->name, p->queue);
    if (p->state == SLEEPING)
    {
      getcallerpcs((uint *)p->context->ebp + 2, pc);
      for (i = 0; i < 10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}