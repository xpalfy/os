#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  backtrace();
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// set an alarm to call the handler function
// every period ticks
uint64
sys_sigalarm(void) {
  struct proc *my_proc = myproc();
  int period;
  if (argint(0, &period) < 0)
    return -1;
  uint64 p;
  if(argaddr(1, &p) < 0)
    return -1;
  my_proc->alarm_period = period;
  my_proc->alarm_handler = (void (*)()) p;
  my_proc->ticks_since_last_alarm = 0;
  my_proc->inalarm = 0;
  return 0;
}

uint64
sys_sigreturn(void) {
  struct proc* p = myproc();
  if (p->inalarm) {
    p->inalarm = 0;
    *p->trapframe = *p->alarmframe;
    p->ticks_since_last_alarm = 0;
  }
  return 0;
}
