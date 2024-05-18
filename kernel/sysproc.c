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
  int n;
  uint ticks0;

  argint(0, &n);
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

// системный вызов для проверки и обнуления атрибута A
uint64
sys_pgaccess(void)
{
    uint64 page_addr, res_addr;
    int size, npages;

    // получить аргументы системного вызова
    argaddr(0, &page_addr);
    argint(1, &size);
    argaddr(2, &res_addr);

    // вычислить количество страниц
    npages = (size + PGSIZE - 1) / PGSIZE; pagetable_t cur_pt = myproc()->pagetable;
    // массив для хранения результатов доступа к страницам
    char accessed[npages];
    memset(accessed, 0, sizeof(accessed));
    for (int i = 0; i < npages; ++i)
    {
        pte_t *pte = walk(cur_pt, page_addr, 0);
        if (pte && (*pte & PTE_A))
        {
            // установить бит доступа для страницы
            accessed[i] = 1;
            // обнулить атрибут A
            *pte &= ~PTE_A;
        }
        page_addr += PGSIZE;
    }

    // скопировать результаты в пространство пользователя
    if (copyout(cur_pt, res_addr, (char *)accessed, sizeof(accessed)) < 0)
        return -1;

    return 0;
}
