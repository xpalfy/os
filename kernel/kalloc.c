#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; 

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    char name[9] = {0};
    snprintf(name, 8, "kmem-%d", i);
    initlock(&kmem[i].lock, name);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  push_off();
  int cpu = cpuid();
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
  pop_off();
}

void *
ksteal(int cpu) {
  struct run *r;
  for (int i = 1; i < NCPU; i++) {
    int next_cpu = (cpu + i) % NCPU;
    acquire(&kmem[next_cpu].lock);
    r = kmem[next_cpu].freelist;
    if (r) {
      kmem[next_cpu].freelist = r->next;
    }
    release(&kmem[next_cpu].lock);
    if (r) {
      break;
    }
  }
  return r;
}

void *
kalloc(void)
{
  struct run *r;

  push_off();

  int cpu = cpuid();
  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if (r) {
    kmem[cpu].freelist = r->next;
  }
  release(&kmem[cpu].lock);

  if (r == 0) {
    r = ksteal(cpu);
  }

  if(r)
    memset((char*)r, 5, PGSIZE); 

  pop_off();
  return (void*)r;
}