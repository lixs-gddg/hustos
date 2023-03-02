/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "elf.h"

#include "spike_interface/spike_utils.h"

//added for lab1_challenge1_backtrace
extern elf_ctx elfloader;
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

ssize_t sys_user_print_backtrace(int depth)
{
  uint64 user_fp=current->trapframe->regs.s0;
  uint64 user_ra=*(uint64 *)(user_fp-8)-8;
  uint64 ptr_ra;
  int curdepth=0;
  for(curdepth=0;curdepth<depth;curdepth++,user_ra=*(uint64 *)(user_fp)-8)
  {
    ptr_ra=*(uint64 *)user_ra;
    if(ptr_ra==0) break;
    int symbol_index=-1;
    for(int j=0;j<elfloader.symtab_length;j++)
    {
      if((elfloader.symtab[j].st_info&0x0F)==STT_FUNC && elfloader.symtab[j].st_value < ptr_ra && elfloader.symtab[j].st_value+elfloader.symtab[j].st_size > ptr_ra)
      {
        symbol_index =j;
        break;
      }
    }
    if(symbol_index!=-1)  sprint("%s\n", &elfloader.strtab[elfloader.symtab[symbol_index].st_name]);
    else sprint("fail to backtrace symbol:%lx\n",ptr_ra);
    user_fp=user_ra-8;
  }
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_user_print_backtrace:
      return sys_user_print_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
