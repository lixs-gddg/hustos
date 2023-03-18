#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"
typedef struct trapframe_t {
  // space to store context (all common registers)
  /* offset:0   */ riscv_regs regs;

  // process's "user kernel" stack
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  /* offset:264 */ uint64 epc;

  // kernel page table. added @lab2_1
  /* offset:272 */ uint64 kernel_satp;
}trapframe;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
  uint64 kstack;
  // user page table
  pagetable_t pagetable;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;

  //add for lab2_challenge2
  uint64 heap_sz; //the first address that heap can used;
  uint64 heap_occurpied_start;
  uint64 heap_occurpied_last;
}process;

// switch to run user app
void switch_to(process*);

// add for lab2_challeng2
void expandprocess(uint64 n); 
void init_malloc();
uint64 mem_alignment();
uint64 my_malloc(int n);
void my_free(uint64 addr);
// current running process
extern process* current;

// address of the first free page in our simple heap. added @lab2_2
extern uint64 g_ufree_page;

#endif
