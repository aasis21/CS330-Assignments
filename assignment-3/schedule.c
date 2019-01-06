#include <context.h>
#include <memory.h>
#include <schedule.h>
#include <apic.h>
#include <lib.h>
#include <idt.h>
static u64 numticks;

static void save_current_context(u64 *base)
{
  /*Your code goes in here*/

  struct exec_context *current = get_current_ctx();
  
  base = base - 1;

  // saving the current context
  current->regs.entry_ss = *(base + 20);
  current->regs.entry_rsp = *(base + 19);
  current->regs.entry_rflags = *(base + 18);
  current->regs.entry_cs = *(base + 17);
  current->regs.entry_rip = *(base + 16);
  current->regs.rbp = *(base + 15);
  current->regs.rax = *(base + 14);
  current->regs.rbx = *(base + 13);
  current->regs.rcx = *(base + 12);
  current->regs.rdx = *(base + 11);
  current->regs.rsi = *(base + 10);
  current->regs.rdi = *(base + 9);
  current->regs.r8 = *(base + 8);
  current->regs.r9 = *(base + 7);
  current->regs.r10 = *(base + 6);
  current->regs.r11 = *(base + 5);
  current->regs.r12 = *(base + 4);
  current->regs.r13 = *(base + 3);
  current->regs.r14 = *(base + 2);
  current->regs.r15 = *(base + 1);

  //printf("\nSAVE RAX [%x]\n", current->regs.rax);

}

static u64 *push_next_context_to_stack(u64 *base, struct exec_context *next)
{

  base = base - 1;

  *(base + 20) = next->regs.entry_ss;
  *(base + 19) = next->regs.entry_rsp;
  *(base + 18) = next->regs.entry_rflags;
  *(base + 17) = next->regs.entry_cs;
  *(base + 16) = next->regs.entry_rip;
  *(base + 15)= next->regs.rbp;
  *(base + 14) = next->regs.rax;
  *(base + 13) = next->regs.rbx;
  *(base + 12) = next->regs.rcx;
  *(base + 11) = next->regs.rdx;
  *(base + 10) = next->regs.rsi;
  *(base + 9) = next->regs.rdi;
  *(base + 8) = next->regs.r8;
  *(base + 7) = next->regs.r9;
  *(base + 6) = next->regs.r10;
  *(base + 5) = next->regs.r11;
  *(base + 4) = next->regs.r12;
  *(base + 3) = next->regs.r13;
  *(base + 2) = next->regs.r14;
  *(base + 1) = next->regs.r15;

  //  printf("\nRESTORE RAX [%x]\n", *(base +  14));



  return base + 1;
}


static struct exec_context *pick_next_context(struct exec_context *list)
{
  /*Your code goes in here*/
  struct exec_context *current = get_current_ctx();
  u32 curr_pid = current->pid;

  int new_pid = -1;
  for (int i = curr_pid + 1; i < MAX_PROCESSES; i++)
  {
    struct exec_context *new = get_ctx_by_pid(i);
    if (new->state == READY && new->pid != curr_pid && new->pid != 0)
    {
      new_pid = new->pid;
      break;
    }
  }
  if (new_pid == -1)
  {
    for (int i = 0; i < curr_pid; i++)
    {
      struct exec_context *new = get_ctx_by_pid(i);
      if (new->state == READY && new->pid != curr_pid && new->pid != 0)
      {
        new_pid = new->pid;
        break;
      }
    }
  }

  if (new_pid == -1 ){
    if(current->state == READY || current->state == RUNNING){
       new_pid = curr_pid;
    }else{
      new_pid = 0;
    }
   
  }

  struct exec_context *new_process = get_ctx_by_pid(new_pid);
  
  return new_process;
}



static void schedule_context(struct exec_context *next, u64 *next_context_on_stack)
{
  /*Your code goes in here. get_current_ctx() still returns the old context*/
  struct exec_context *current = get_current_ctx();
  if(next->pid != current->pid)
    printf("schedluing: old pid = %d  new pid  = %d\n", current->pid, next->pid); /*XXX: Don't remove*/

  if(current->state == RUNNING){
    current->state = READY;
    next->state = RUNNING;
  } else{
    next->state = RUNNING;    
  }

  /*These two lines must be executed*/
  set_tss_stack_ptr(next);
  set_current_ctx(next);
  
  // this func is used for handle timer tick,
  // all things have been set accordingly and context pushed on stack.
  // itretq and restoring of register will be done in handle timer tick
  return;

}



static void schedule(u64 *context_on_stack)
{ 

  // this schedule to be used by handle timer tick
  // other palces schedule written using helper function explicitly

  save_current_context(context_on_stack);

  struct exec_context *list = get_ctx_list();

  struct exec_context *next = pick_next_context(list);

  struct exec_context *curr = get_current_ctx();

  u64 *next_context_on_stack = push_next_context_to_stack(context_on_stack, next);

  schedule_context(next, next_context_on_stack);

}

static void do_sleep_and_alarm_account()
{
  /*All processes in sleep() must decrement their sleep count*/
  struct exec_context *process = get_ctx_list();
  for (int i = 0; i < MAX_PROCESSES; i++)
  {
    if(process[i].state == WAITING && process[i].ticks_to_sleep > 1)
    {
      process[i].ticks_to_sleep--;
     
    }
    else if ( process[i].state == WAITING && process[i].ticks_to_sleep == 1)
    {
      process[i].ticks_to_sleep--;
      process[i].state = READY;
    }
  }

  // Alarm for all process can be handled here same way as above
  // we can set the alarm sig bit in ctx to 1, when the process
  // will get scheduled, alarm  will be called.
  // Since alarm only tested with init so, no need to it it this way.

}

/*The five functions above are just a template. You may change the signatures as you wish*/
void handle_timer_tick()
{
  /*
   This is the timer interrupt handler. 
   You should account timer ticks for alarm and sleep
   and invoke schedule
 */

  asm volatile("push %%rax;"
               "push %%rbx;"
               "push %%rcx;"
               "push %%rdx;"
               "push %%rsi;"
               "push %%rdi;"
               "push %%r8;"
               "push %%r9;"
               "push %%r10;"
               "push %%r11;"
               "push %%r12;"
               "push %%r13;"
               "push %%r14;"
               "push %%r15;"
               :
               :
               : "memory");

  printf("Got a tick. #ticks = %u\n", numticks++); /*XXX Do not modify this line*/
  
  do_sleep_and_alarm_account();

  do_alarm_and_schedule_and_general_stuff();

  ack_irq(); /*acknowledge the interrupt, before calling iretq */
  asm volatile("mov %%rbp, %%rsp;"
               "sub $112, %%rsp;"
               "pop %%r15;"
               "pop %%r14;"
               "pop %%r13;"
               "pop %%r12;"
               "pop %%r11;"
               "pop %%r10;"
               "pop %%r9;"
               "pop %%r8;"
               "pop %%rdi;"
               "pop %%rsi;"
               "pop %%rdx;"
               "pop %%rcx;"
               "pop %%rbx;"
               "pop %%rax;"
               "pop %%rbp;"
               "iretq;" 
               :
               :
               : "memory");
}

void do_alarm_and_schedule_and_general_stuff(){
  struct exec_context *ctx = get_current_ctx();
  
  u64 *local_rbp;
  asm volatile("mov %%rbp, %0;"
                : "=r"(local_rbp)
                :
                : "memory");
  
  u64 *rbp = *local_rbp;

  //handling alarm for init process.

  if (ctx->ticks_to_alarm > 1)
  {
    ctx->ticks_to_alarm--;
  }
  else if (ctx->ticks_to_alarm == 1)
  {
    ctx->ticks_to_alarm = 0;
    invoke_sync_signal(SIGALRM, rbp + 4, rbp + 1);
  }

  schedule(rbp - 14);
  
}

void do_exit()
{
  /*You may need to invoke the scheduler from here if there are
    other processes except swapper in the system. Make sure you make 
    the status of the current process to UNUSED before scheduling 
    the next process. If the only process alive in system is swapper, 
    invoke do_cleanup() to shutdown gem5 (by crashing it, huh!)
    */

   struct exec_context *current = get_current_ctx();
   current->state = UNUSED;

   int curr_pid =  current->pid;
   int p_flag = 0;

   for(int i=0; i< MAX_PROCESSES; i++){
     struct exec_context *new = get_ctx_by_pid(i);
     if (new->state != UNUSED && new->pid != curr_pid && new->pid != 0){
        p_flag = 1;
     }
   }
   if(p_flag == 1){

     // scheduling

      os_pfn_free(OS_PT_REG, current->os_stack_pfn);
      
      struct exec_context *list = get_ctx_list();
      struct exec_context *next = pick_next_context(list);
      next->state=RUNNING;

      // changing OS STACK
      set_tss_stack_ptr(next);
      set_current_ctx(next);
      
      u64 *context_on_stack;
      
      asm volatile("mov %%rsp, %0;"
      "sub $160, %%rsp;"
      : "=r"(context_on_stack)
      :
      : "memory");
      
      if(next->pid != current->pid)
        printf("schedluing: old pid = %d  new pid  = %d\n", current->pid, next->pid); /*XXX: Don't remove*/

      context_on_stack = context_on_stack - 20;

      push_next_context_to_stack(context_on_stack, next);
     
      asm volatile("mov %0, %%rsp;"
      "pop %%r15;"
      "pop %%r14;"
      "pop %%r13;"
      "pop %%r12;"
      "pop %%r11;"
      "pop %%r10;"
      "pop %%r9;"
      "pop %%r8;"
      "pop %%rdi;"
      "pop %%rsi;"
      "pop %%rdx;"
      "pop %%rcx;"
      "pop %%rbx;"
      "pop %%rax;"
      "pop %%rbp;"
      "iretq;"
      :
      : "r"(context_on_stack)
      : "memory");
      return;

   }else{
     os_pfn_free(OS_PT_REG, current->os_stack_pfn);
     do_cleanup();
   }

  
}

/*system call handler for sleep*/
long do_sleep(u32 ticks)
{ 

  //printf(">>>>>>>>>>>>>>>>>>> INSIDE SLEEP SYSCALL\n");
  struct exec_context *current = get_current_ctx();
  current->state = WAITING;
  current->ticks_to_sleep = ticks;

  u64 *sleep_rbp;
  asm volatile("mov %%rbp, %0;"
               :"=r"(sleep_rbp)
               :
               :"memory");

  u64 *base = *sleep_rbp;

  u64 rax;
  asm volatile("mov %%rax, %0;"
               :"=r"(rax)
               :
               :"memory");


  // save the current context
  current->regs.entry_ss = *(base + 20);
  current->regs.entry_rsp = *(base + 19);
  current->regs.entry_rflags = *(base + 18);
  current->regs.entry_cs = *(base + 17);
  current->regs.entry_rip = *(base + 16);
  current->regs.rbp = *(base + 10);

  current->regs.rax = rax;
  current->regs.rbx = *(base + 15);
  current->regs.rcx = *(base + 14);
  current->regs.rdx = *(base + 13);
  current->regs.rsi = *(base + 12);
  current->regs.rdi = *(base + 11);

  current->regs.r8 = *(base + 9);
  current->regs.r9 = *(base + 8);
  current->regs.r10 = *(base + 7);
  current->regs.r11 = *(base + 6);
  current->regs.r12 = *(base + 5);
  current->regs.r13 = *(base + 4);
  current->regs.r14 = *(base + 3);
  current->regs.r15 = *(base + 2);

  struct exec_context *list = get_ctx_list();

  struct exec_context *next = pick_next_context(list);

  // load new context
  *(base + 20) = next->regs.entry_ss;
  *(base + 19) = next->regs.entry_rsp;
  *(base + 18) = next->regs.entry_rflags;
  *(base + 17) = next->regs.entry_cs;
  *(base + 16) = next->regs.entry_rip;
  *(base + 15) = next->regs.rbx;
  *(base + 14) = next->regs.rcx;
  *(base + 13) = next->regs.rdx;
  *(base + 12) = next->regs.rsi;
  *(base + 11) = next->regs.rdi;
  *(base + 10) = next->regs.rbp;
  *(base + 9) = next->regs.r8;
  *(base + 8) = next->regs.r9;
  *(base + 7) = next->regs.r10;
  *(base + 6) = next->regs.r11;
  *(base + 5) = next->regs.r12;
  *(base + 4) = next->regs.r13;
  *(base + 3) = next->regs.r14;
  *(base + 2) = next->regs.r15;

  
  next->state = RUNNING;
  if(next->pid != current->pid)
    printf("schedluing: old pid = %d  new pid  = %d\n", current->pid, next->pid); /*XXX: Don't remove*/

  /*These two lines must be executed*/
  set_tss_stack_ptr(next);
  set_current_ctx(next);

  asm volatile("mov %0, %%rax;"
               :
               :"m"(next->regs.rax)
               :"memory");

  //return next->regs.rax;
  

}

/*
  system call handler for clone, create thread like 
  execution contexts
*/
long do_clone(void *th_func, void *user_stack)
{
  
      struct exec_context *new = get_new_ctx();
      struct exec_context *current = get_current_ctx();
      
      new->state = NEW; 
      new->type = current->type;
      new->used_mem = current->used_mem;
      new->pgd = current->pgd;
      new->os_stack_pfn =  os_pfn_alloc(OS_PT_REG);
      new->os_rsp = osmap(new->os_stack_pfn);


      for(int i = 0; i < MAX_MM_SEGS; i++){
        new->mms[i].start = current->mms[i].start; 
        new->mms[i].end = current->mms[i].end; 
        new->mms[i].next_free = current->mms[i].next_free; 
        new->mms[i].access_flags = current->mms[i].access_flags;
       // printf("MEM [%x] [%x] [%x]\n", new->mms[i].start , new->mms[i].end, new->mms[i].next_free ); 
      }
      // new->name 

      int counter = sizeof(current->name)/sizeof(char);

      memcpy(new->name, current->name, sizeof(current->name));
      
      for(int i = 0; i < counter; i++ ){
        if(new->name[i] == '\0'){
          counter = i;
          break;
        } 
      }

      new->name[counter] = '0' + new->pid % 10;
      
      if (new->pid < 10){
        new->name[counter + 1] = '\0';
      }else{
        new->name[counter + 1] = '0' + new->pid / 10;
        new->name[counter + 2] = '\0';
      }

      //printf("\n NAME OF PROCESS [%s] \n",new->name);

      // new  user reg
      new->regs.entry_ss = 0x2b;
      new->regs.entry_rsp = user_stack;
      new->regs.entry_rflags = current->regs.entry_rflags;
      new->regs.entry_cs = 0x23;
      new->regs.entry_rip = th_func;
      new->regs.rbp = user_stack;
      new->regs.rax = current->regs.rax;
      new->regs.rbx = current->regs.rbx;
      new->regs.rcx = current->regs.rcx;
      new->regs.rdx = current->regs.rdx;
      new->regs.rsi = current->regs.rsi;
      new->regs.rdi = current->regs.rdi;
      new->regs.r8 =  current->regs.r8;
      new->regs.r9 = current->regs.r9;
      new->regs.r10 = current->regs.r10;
      new->regs.r11 = current->regs.r11;
      new->regs.r12 = current->regs.r12;
      new->regs.r13 = current->regs.r13;
      new->regs.r14 = current->regs.r14;
      new->regs.r15 = current->regs.r15;



      new->pending_signal_bitmap = 0;
      
      for(int i=0; i< MAX_SIGNALS; i++){
        new->sighandlers[i] = current->sighandlers[i];
      }
      new->ticks_to_sleep = 0;
      new->alarm_config_time = 0;
      new->ticks_to_alarm = 0;
      new->state = READY;

      return new->pid;

    
}

long invoke_sync_signal(int signo, u64 *ustackp, u64 *urip)
{
  /*If signal handler is registered, manipulate user stack and RIP to execute signal handler*/
  /*ustackp and urip are pointers to user RSP and user RIP in the exception/interrupt stack*/
  printf("Called signal [%x] with ustackp=%x urip=%x\n", signo, *ustackp, *urip);

  struct exec_context *ctx = get_current_ctx();
  if (ctx->sighandlers[signo] != NULL)
  {
    // getting the user mode stack pointer
    u64 *user_stack_pointer = *ustackp;
    // putting urip on top of stack.
    *(user_stack_pointer - 1) = *urip;

    // updating the top of user mode stack
    *ustackp = *ustackp - 8;

    // setting the urip to custom handler
    *urip = ctx->sighandlers[signo];
  }
  else
  {
    /*Default behavior is exit( ) if sighandler is not registered for SIGFPE or SIGSEGV.
        Ignore for SIGALRM*/
    if (signo != SIGALRM)
      do_exit();
  }
}
/*system call handler for signal, to register a handler*/
long do_signal(int signo, unsigned long handler)
{
  //printf("DO SIGNAL -[%x]\n", handler);
  struct exec_context *ctx = get_current_ctx();
  ctx->sighandlers[signo] = handler;
  return 0;
}

/*system call handler for alarm*/
long do_alarm(u32 ticks)
{
  struct exec_context *ctx = get_current_ctx();
  u32 return_value = ctx->ticks_to_alarm;
  ctx->ticks_to_alarm = ticks;
  ctx->alarm_config_time = ticks;
  return return_value;
}
