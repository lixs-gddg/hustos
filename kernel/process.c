/*
 * Utility functions for process management. 
 *
 * Note: in Lab1, only one process (i.e., our user application) exists. Therefore, 
 * PKE OS at this stage will set "current" to the loaded user application, and also
 * switch to the old "current" process after trap handling.
 */

#include "riscv.h"
#include "strap.h"
#include "config.h"
#include "process.h"
#include "elf.h"
#include "string.h"
#include "vmm.h"
#include "pmm.h"
#include "memlayout.h"
#include "spike_interface/spike_utils.h"
#include "limits.h"

//Two functions defined in kernel/usertrap.S
extern char smode_trap_vector[];
extern void return_to_user(trapframe *, uint64 satp);

// current points to the currently running user-mode application.
process* current = NULL;

// points to the first free page in our simple heap. added @lab2_2
uint64 g_ufree_page = USER_FREE_ADDRESS_START;

//
// switch to a user-mode process
//
void switch_to(process* proc) {
  assert(proc);
  current = proc;

  // write the smode_trap_vector (64-bit func. address) defined in kernel/strap_vector.S
  // to the stvec privilege register, such that trap handler pointed by smode_trap_vector
  // will be triggered when an interrupt occurs in S mode.
  write_csr(stvec, (uint64)smode_trap_vector);

  // set up trapframe values (in process structure) that smode_trap_vector will need when
  // the process next re-enters the kernel.
  proc->trapframe->kernel_sp = proc->kstack;      // process's kernel stack
  proc->trapframe->kernel_satp = read_csr(satp);  // kernel page table
  proc->trapframe->kernel_trap = (uint64)smode_trap_handler;

  // SSTATUS_SPP and SSTATUS_SPIE are defined in kernel/riscv.h
  // set S Previous Privilege mode (the SSTATUS_SPP bit in sstatus register) to User mode.
  unsigned long x = read_csr(sstatus);
  x &= ~SSTATUS_SPP;  // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE;  // enable interrupts in user mode

  // write x back to 'sstatus' register to enable interrupts, and sret destination mode.
  write_csr(sstatus, x);

  // set S Exception Program Counter (sepc register) to the elf entry pc.
  write_csr(sepc, proc->trapframe->epc);

  // make user page table. macro MAKE_SATP is defined in kernel/riscv.h. added @lab2_1
  uint64 user_satp = MAKE_SATP(proc->pagetable);

  // return_to_user() is defined in kernel/strap_vector.S. switch to user mode with sret.
  // note, return_to_user takes two parameters @ and after lab2_1.
  return_to_user(proc->trapframe, user_satp);
}

//add for lab2_challenge2
// to expand n byte for current process; 
void expandprocess(uint64 n)
{
  if(n<0) panic("fail in expanding space for process!\n");
  user_vm_malloc(current->pagetable,current->heap_sz,current->heap_sz+n);
  current->heap_sz+=n;
}

int is_init_malloc=0;

//to init the malloc function;

void init_malloc()
{
  current->heap_sz = USER_FREE_ADDRESS_START;
  uint64 addr = current->heap_sz;
  expandprocess(sizeof(MCB));
  pte_t *pte = page_walk(current->pagetable, addr, 0);
  MCB *first_mcb = (MCB *) PTE2PA(*pte);
  current->heap_occurpied_start = (uint64) first_mcb;
  first_mcb->next_mcb = NULL;
  first_mcb->size = 0;
  current->heap_occurpied_last = (uint64)first_mcb;
  is_init_malloc = 1;
}

//to memory alignment
uint64 mem_alignment(uint64 addr)
{
  uint64 amo = (8 - (addr%8))%8;
  return addr+amo;
}

//to find the MCB for the addr
MCB *find_MCB(pagetable_t pagetable, uint64 addr)
{
  pte_t *pte = page_walk(pagetable, addr, 1);
  uint64 pageoffset=addr & 0xfff;
  return (MCB *)mem_alignment(PTE2PA(*pte) + pageoffset);
}

//to malloc memory for process.
uint64 my_malloc(int n)
{
  if(is_init_malloc==0)
  {
    init_malloc();
  }
  MCB *cur = (MCB *)current->heap_occurpied_start;
  MCB *last = (MCB *)current->heap_occurpied_last;
  MCB *g=(MCB *)current->heap_occurpied_start;
  int flag=0;
  int min_n=INT_MAX;
  //find the most suitable space for the process.
  for(cur=(MCB *)current->heap_occurpied_start;cur!=last->next_mcb;cur=cur->next_mcb)
  {
    if(cur->size>=n && cur->is_occupied==0)
    {
      flag=1;
      if(cur->size<min_n)
      {
        min_n=cur->size;
        g=cur;
      }
      if(cur->size==n)//the most suitable one.
      {
        cur->is_occupied=1;
        return cur->start_address+sizeof(MCB);
      }
    }
  }
  //find the already exist MCB which can be used for process.
  if(flag)
  {
   g->is_occupied=1;
   return g->start_address+sizeof(MCB);
  }
  else
  {
    uint64 va = current->heap_sz;
    expandprocess((uint64) (sizeof(MCB) + n + 8));
    MCB *new_mcb=find_MCB(current->pagetable,va);
    new_mcb->is_occupied = 1;
    new_mcb->start_address = va;
    new_mcb->size = n;
    new_mcb->next_mcb = last->next_mcb;
    last->next_mcb = new_mcb;
    current->heap_occurpied_last=(uint64)new_mcb;
    return va + sizeof(MCB);
  }
}

void my_free(uint64 firstaddr)
{
  firstaddr = ((uint64)firstaddr - sizeof(MCB));
  MCB *cur=find_MCB(current->pagetable,firstaddr);
  if(cur->is_occupied == 0)  
    panic("This memory has been freed before! \n");
  cur->is_occupied = 0;
}