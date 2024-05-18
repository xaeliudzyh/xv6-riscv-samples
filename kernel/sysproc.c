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
// системный вызов для проверки доступа к страницам памяти и обнуления флага PTE_A
uint64 sys_pgaccess(void) {
    uint64 start_addr;
    uint64 user_buffer;
    int num_pages;

    // получение аргументов системного вызова
    argaddr(0, &start_addr), argint(1, &num_pages), argaddr(2, &user_buffer);
    // получение таблицы страниц текущего процесса
    pagetable_t pagetable = myproc()->pagetable;
    // массив для хранения результатов проверки доступа
    char access_status[num_pages];
    // цикл по всем страницам
    for (int page_idx = 0; page_idx < num_pages; page_idx++)
    {
        // получение PTE для текущей страницы
        pte_t *pte_entry = walk(pagetable, start_addr + page_idx * PGSIZE, 0);
        // проверка установленного флага PTE_A (доступ)
        if (*pte_entry & PTE_A)
        {
            access_status[page_idx] = 1;  // страница была доступна
            // обнуление флага PTE_A после проверки
            *pte_entry &= ~PTE_A;
        }
        else
            access_status[page_idx] = 0;  // доступ к странице не был зафиксирован
    }

    // копирование результатов проверки в пользовательское пространство
    if (copyout(pagetable, user_buffer, access_status, sizeof(access_status)) < 0)
        // ошибка копирования в пространство пользователя
        return -1;

    // успешное выполнение системного вызова
    return 0;
}
