#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include <string.h>

static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}

//added for lab1_challenge2_errorline
//to print the line information of the error
char path[1024];//to save the path of the file;
char code[16384];// to save the code of the error;
struct stat ustat;
void print_line_info(addr_line line)
{
  //read the path of the code file.
  strcpy(path,current->dir[current->file[line.file].dir]);
  strcpy(path+strlen(path),"/");
  strcpy(path+strlen(path),current->file[line.file].file);
  sprint("Runtime error at %s:%d\n", path, line.line);
  //used htif to read the code file.
  spike_file_t *f=spike_file_open(path,O_RDONLY, 0);
  spike_file_stat(f,&ustat);
  spike_file_read(f,code,ustat.st_size);
  spike_file_close(f);
  int curline=0;
  int off=0;
  int index=0;
  for(;curline<line.line;curline++,off++)
  {
    index=off;
    while(off<ustat.st_size && code[off]!='\n') off++;
  }
  code[off]='\0';
  sprint("%s",code+index);
} 


//added for lab1_challenge2_errorline
//to print the the error
void print_error_line()
{
  uint64 mepc =read_csr(mepc);
  for(int i=0;i<current->line_ind;i++)
  {
    if(mepc==current->line[i].addr)
    {
      print_line_info(current->line[i]);
      return;
    }
  }
  sprint("Could not find the error\n");
}


//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  switch (mcause) {
    case CAUSE_MTIMER:
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      print_error_line();
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      print_error_line();
      handle_load_access_fault();
    case CAUSE_STORE_ACCESS:
      print_error_line();
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      // TODO (lab1_2): call handle_illegal_instruction to implement illegal instruction
      // interception, and finish lab1_2.
      //panic( "call handle_illegal_instruction to accomplish illegal instruction interception for lab1_2.\n" );
      print_error_line();
      handle_illegal_instruction();
      break;
    case CAUSE_MISALIGNED_LOAD:
      print_error_line();
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      print_error_line();
      handle_misaligned_store();
      break;

    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
