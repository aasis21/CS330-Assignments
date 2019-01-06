#include<init.h>
#include<lib.h>
#include<context.h>
#include<memory.h>


void init_zero(u32 pfn){
    u64 * virtual = (u64 *)osmap(pfn);
    for(int i=0; i<512; i++){
        virtual[i] = 0;
    }
    return;
}

u32 os_pfn_alloc_wrapper(u32 region){
    u32 p_addr = os_pfn_alloc(region);
    init_zero(p_addr);
    return p_addr;
}

/*System Call handler*/
u64 virtual_to_physical_for_shrink(u64 l4_physical, u64 virtual_addr){

    u64 virtual_addr_usr_context =  virtual_addr;

    u64  l4_entry =  ( virtual_addr_usr_context << 16 ) >> 55;
    u64  l3_entry =  ( virtual_addr_usr_context << 25 ) >> 55;
    u64  l2_entry =  ( virtual_addr_usr_context << 34 ) >> 55;
    u64  l1_entry =  ( virtual_addr_usr_context << 43 ) >> 55;


    u64 p_addr_l4 = l4_physical;

    u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
    u64 pt_entry_l4 =  l4_virtual[l4_entry];
    if( (pt_entry_l4 & 1)==0 ){
        return 0;
    }

    u64 * l3_virtual = (u64 *)osmap(l4_virtual[l4_entry] >> 12);
    u64 pt_entry_l3 =  l3_virtual[l3_entry];
    if( (pt_entry_l3 & 1)==0 ){
        return 0;
    }


    u64 * l2_virtual = (u64 *)osmap(l3_virtual[l3_entry] >> 12);
    u64 pt_entry_l2 =  l2_virtual[l2_entry];
    if( (pt_entry_l2 & 1)==0 ){
        return 0;
    }


    u64 * l1_virtual = (u64 *)osmap(l2_virtual[l2_entry] >> 12);
    u64 pt_entry_l1 = l1_virtual[l1_entry];
    if( (pt_entry_l1 & 1)==0 ){
        return 0;
    }
    l1_virtual[l1_entry] = (l1_virtual[l1_entry] >> 1) << 1;
    return pt_entry_l1 >> 12;

}

long do_syscall(int syscall, u64 param1, u64 param2, u64 param3, u64 param4)
{
    struct exec_context *current = get_current_ctx();
    printf("[GemOS] System call invoked. syscall no  = %d\n", syscall);
    switch(syscall)
    {
          case SYSCALL_EXIT:
                              printf("[GemOS] exit code = %d\n", (int) param1);
                              do_exit();
                              break;
          case SYSCALL_GETPID:
                              printf("[GemOS] getpid called for process %s, with pid = %d\n", current->name, current->id);
                              return current->id;
          case SYSCALL_WRITE:
                             {
                                 if(param2 > 1024){
                                     return -1;
                                 }

                                 {
                                        u64 virtual_addr_usr_context =  param1 ;

                                        u64  l4_entry =  ( virtual_addr_usr_context << 16 ) >> 55;
                                        u64  l3_entry =  ( virtual_addr_usr_context << 25 ) >> 55;
                                        u64  l2_entry =  ( virtual_addr_usr_context << 34 ) >> 55;
                                        u64  l1_entry =  ( virtual_addr_usr_context << 43 ) >> 55;


                                        u64 p_addr_l4 = current->pgd;

                                        u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
                                        u64 pt_entry_l4 =  l4_virtual[l4_entry];
                                        if( (pt_entry_l4 & 1)==0 ){
                                            return -1;
                                        }

                                        u64 * l3_virtual = (u64 *)osmap(l4_virtual[l4_entry] >> 12);
                                        u64 pt_entry_l3 =  l3_virtual[l3_entry];

                                        if( (pt_entry_l3 & 1)==0 ){
                                            return -1;
                                        }


                                        u64 * l2_virtual = (u64 *)osmap(l3_virtual[l3_entry] >> 12);
                                        u64 pt_entry_l2 =  l2_virtual[l2_entry];

                                        if( (pt_entry_l2 & 1)==0 ){
                                            return -1;
                                        }


                                        u64 * l1_virtual = (u64 *)osmap(l2_virtual[l2_entry] >> 12);
                                        u64 pt_entry_l1 = l1_virtual[l1_entry];
                                        if( (pt_entry_l1 & 1)==0 ){
                                            return -1;
                                        }


                                }

                                {
                                       u64 virtual_addr_usr_context =  param1 + param2;

                                       u64  l4_entry =  ( virtual_addr_usr_context << 16 ) >> 55;
                                       u64  l3_entry =  ( virtual_addr_usr_context << 25 ) >> 55;
                                       u64  l2_entry =  ( virtual_addr_usr_context << 34 ) >> 55;
                                       u64  l1_entry =  ( virtual_addr_usr_context << 43 ) >> 55;


                                       u64 p_addr_l4 = current->pgd;

                                       u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
                                       u64 pt_entry_l4 =  l4_virtual[l4_entry];
                                       if( (pt_entry_l4 & 1)==0 ){
                                           return -1;
                                       }

                                       u64 * l3_virtual = (u64 *)osmap(l4_virtual[l4_entry] >> 12);
                                       u64 pt_entry_l3 =  l3_virtual[l3_entry];
                                       if( (pt_entry_l3 & 1)==0 ){
                                                  return -1;
                                       }


                                       u64 * l2_virtual = (u64 *)osmap(l3_virtual[l3_entry] >> 12);
                                       u64 pt_entry_l2 =  l2_virtual[l2_entry];
                                       if( (pt_entry_l2 & 1)==0 ){
                                           return -1;
                                       }


                                       u64 * l1_virtual = (u64 *)osmap(l2_virtual[l2_entry] >> 12);
                                       u64 pt_entry_l1 = l1_virtual[l1_entry];
                                       if( (pt_entry_l1 & 1)==0 ){
                                           return -1;
                                       }


                               }

                                printf("%s", param1);
                                return param2;

                             }
          case SYSCALL_EXPAND:
                             {
                                    u32 size = param1;
                                    int flag ;
                                    if(param2 == MAP_RD){
                                        flag = MM_SEG_RODATA;
                                    }else if(param2 == MAP_WR){
                                        flag = MM_SEG_DATA;
                                    } else{
                                         return 0;
                                    }

                                    if(current->mms[flag].next_free + (size << 12) <= (current->mms[flag].end) + 1 ){
                                            unsigned long to_return = current->mms[flag].next_free;
                                            current->mms[flag].next_free += (size << 12) ;
                                            //printf("EXPANFD : O - %x  N - %x S- %x \n", to_return, current->mms[flag].next_free, current->mms[flag].start  );
                                            return to_return;
                                    }else{
                                        return 0;
                                    }


                             }
          case SYSCALL_SHRINK:
                             {
                                        /*Your code goes here*/

                                        u32 size = param1;

                                        int flag;

                                        if(param2 == MAP_RD){
                                            flag = MM_SEG_RODATA;
                                        }else if(param2 == MAP_WR){
                                            flag = MM_SEG_DATA;
                                        } else{
                                             return 0;
                                        }

                                        if(current->mms[flag].next_free - (size << 12) >= (current->mms[flag].start) ){
                                                u64 shrink_start = current->mms[flag].next_free - (1 << 12);
                                                u64 shrink_end = current->mms[flag].next_free - (size << 12);

                                                current->mms[flag].next_free = shrink_end ;

                                                for(u64 i = shrink_start ; i >=shrink_end; i = i - (1 << 12) ){

                                                    u64 to_free = virtual_to_physical_for_shrink(current->pgd, i);
                                                    if(to_free != 0){
                                                        os_pfn_free(USER_REG, to_free);
                                                        asm volatile("invlpg (%0)" ::"r" (i) : "memory");
                                                    }

                                                }
                                                //printf("SHRINK : E - %x  S - %x NF - %x  \n", shrink_end, shrink_start , current->mms[flag].next_free );
                                                return shrink_end;
                                        }else{
                                            return 0;
                                        }

                             }

          default:
                              return -1;

    }
    return 0;   /*GCC shut up!*/
}

extern int handle_div_by_zero(void)
{
    /*Your code goes in here*/
    u64 *stack_base_pointer;
    asm volatile("mov %%rbp, %0;"
               :"=r" (stack_base_pointer)
               :
               :"memory"
    );
    printf("[GEMOS] [DivideByZero] RIP [%x] \n",*(stack_base_pointer + 1));
    do_exit();
    return 0;
}

extern int handle_page_fault(void)
{
    /*Your code goes in here*/

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
                 :"memory"
    );

    u64* rsp;
    asm volatile("mov %%rsp, %0;"
               :"=r" (rsp)
               :
               :"memory"
    );


    u64* rbp;
    asm volatile("mov %%rbp, %0;"
               :"=r" (rbp)
               :
               :"memory"
    );

    u64 old_rbp = *(rbp);
    u64 error = *(rbp + 1);
    u64 rip = *(rbp+2);
    u64 virtual_addr;
    asm volatile("mov %%CR2, %0;"
                 :"=r"(virtual_addr)
                 :
                 :"memory"
    );


    int exit_or_restore = 0;
    // 0 means exit
    // 1 means restore

    if( (error & 1) == 1){
        printf("[GEMOS] [PageFaultHandler] Protection violation : RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
        do_exit();
    }

    struct exec_context *ctx = get_current_ctx();

    if(virtual_addr >= ctx->mms[MM_SEG_RODATA].start && virtual_addr <= ctx->mms[MM_SEG_RODATA].end ){
        // RO DATA
        if( (error & 2) == 2 ){
            printf("[GEMOS] [PageFaultHandler] Trying to write on Read Only Data: RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
            do_exit();
        }
        else if(virtual_addr <  ctx->mms[MM_SEG_DATA].next_free ){
            int write_fault = 0;

            {
                u64  l4_entry =  ( virtual_addr << 16 ) >> 55;
                u64  l3_entry =  ( virtual_addr << 25 ) >> 55;
                u64  l2_entry =  ( virtual_addr << 34 ) >> 55;
                u64  l1_entry =  ( virtual_addr << 43 ) >> 55;

                u64 p_addr_l4 = ctx->pgd;
                u64 * l4_virtual = (u64 *)osmap(p_addr_l4);

                if((l4_virtual[l4_entry] & 1) == 1){
                    u64 p_addr_l3 = l4_virtual[l4_entry] >> 12;
                    u64 * l3_virtual = (u64 *)osmap(p_addr_l3);
                    if((l3_virtual[l3_entry] & 1) == 1){
                        u64 p_addr_l2 = l3_virtual[l3_entry] >> 12;
                        u64 * l2_virtual = (u64 *)osmap(p_addr_l2);
                        if((l2_virtual[l2_entry] & 1) == 1){
                            u64 p_addr_l1 = l2_virtual[l2_entry] >> 12;
                            u64 * l1_virtual = (u64 *)osmap(p_addr_l1);
                            if( (l1_virtual[l1_entry] & 2) == 2){
                                write_fault = 1;
                            }
                        }
                    }
                }
            }

            if(write_fault == 1){
                printf("[GEMOS] [PageFaultHandler] Write access on Read Only Data: RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
            }else{

                u64 p_addr_l4 = ctx->pgd;
                u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
                u64  l4_entry =  ( virtual_addr << 16 ) >> 55;
                u64  l3_entry =  ( virtual_addr << 25 ) >> 55;
                u64  l2_entry =  ( virtual_addr << 34 ) >> 55;
                u64  l1_entry =  ( virtual_addr << 43 ) >> 55;

                u64 p_addr_l3;
                if( (l4_virtual[l4_entry] & 1) == 0){
                    p_addr_l3 = os_pfn_alloc_wrapper(OS_PT_REG);
                    l4_virtual[l4_entry] = (p_addr_l3 << 12) + 5;
                }else{
                    p_addr_l3 = l4_virtual[l4_entry] >> 12;
                }

                u64 * l3_virtual = (u64 *)osmap(p_addr_l3);

                u32 p_addr_l2;
                if( (l3_virtual[l3_entry] & 1) == 0){
                    p_addr_l2 = os_pfn_alloc_wrapper(OS_PT_REG);
                    l3_virtual[l3_entry] = (p_addr_l2 << 12) + 5 ;
                }else{
                    p_addr_l2 = l3_virtual[l3_entry] >> 12;
                }

                u64 * l2_virtual = (u64 *)osmap(p_addr_l2);

                u32 p_addr_l1;
                if( (l2_virtual[l2_entry] & 1) == 0){
                    p_addr_l1 = os_pfn_alloc_wrapper(OS_PT_REG);
                    l2_virtual[l2_entry] = (p_addr_l1 << 12) + 5 ;
                }else{
                    p_addr_l2 = l2_virtual[l2_entry] >> 12;
                }

                u32 p_addr_dt = os_pfn_alloc(USER_REG);
                u64 * l1_virtual = (u64 *)osmap(p_addr_l1);
                l1_virtual[l1_entry] = (p_addr_dt << 12) + 5 ;

                exit_or_restore = 1;

            }

        }else{
            printf("[GEMOS] [PageFaultHandler] This RODATA VA is not legitimate : RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
        }

    }else if(virtual_addr >= ctx->mms[MM_SEG_DATA].start && virtual_addr <= ctx->mms[MM_SEG_DATA].end){

        // DATA
        // check legitmacy

        if(virtual_addr <  ctx->mms[MM_SEG_DATA].next_free ){
            u64 p_addr_l4 = ctx->pgd;
            u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
            u64  l4_entry =  ( virtual_addr << 16 ) >> 55;
            u64  l3_entry =  ( virtual_addr << 25 ) >> 55;
            u64  l2_entry =  ( virtual_addr << 34 ) >> 55;
            u64  l1_entry =  ( virtual_addr << 43 ) >> 55;

            u64 p_addr_l3;
            if( (l4_virtual[l4_entry] & 1) == 0){
                p_addr_l3 = os_pfn_alloc_wrapper(OS_PT_REG);
                l4_virtual[l4_entry] = (p_addr_l3 << 12) + 7;
            }else{
                p_addr_l3 = l4_virtual[l4_entry] >> 12;
            }
            u64 * l3_virtual = (u64 *)osmap(p_addr_l3);

            u32 p_addr_l2;
            if( (l3_virtual[l3_entry] & 1) == 0){
                p_addr_l2 = os_pfn_alloc_wrapper(OS_PT_REG);
                l3_virtual[l3_entry] = (p_addr_l2 << 12) + 7;
            }else{
                p_addr_l2 = l3_virtual[l3_entry] >> 12;
            }

            u64 * l2_virtual = (u64 *)osmap(p_addr_l2);

            u32 p_addr_l1;
            if( (l2_virtual[l2_entry] & 1) == 0){
                p_addr_l1 = os_pfn_alloc_wrapper(OS_PT_REG);
                l2_virtual[l2_entry] = (p_addr_l1 << 12) + 7;
            }else{
                p_addr_l1 = l2_virtual[l2_entry] >> 12;
            }

            u32 p_addr_dt = os_pfn_alloc(USER_REG);
            u64 * l1_virtual = (u64 *)osmap(p_addr_l1);
            l1_virtual[l1_entry] = (p_addr_dt << 12) + 7;

            exit_or_restore = 1;


        }else{
            printf("[GEMOS] [PageFaultHandler] This DATA VA is not legitimate : RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
        }


    }else if(virtual_addr >= ctx->mms[MM_SEG_STACK].start && virtual_addr <= ctx->mms[MM_SEG_STACK].end){
        // stack

            u64 p_addr_l4 = ctx->pgd;
            u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
            u64  l4_entry =  ( virtual_addr << 16 ) >> 55;
            u64  l3_entry =  ( virtual_addr << 25 ) >> 55;
            u64  l2_entry =  ( virtual_addr << 34 ) >> 55;
            u64  l1_entry =  ( virtual_addr << 43 ) >> 55;


            u64 p_addr_l3;
            if( (l4_virtual[l4_entry] & 1) == 0){
                p_addr_l3 = os_pfn_alloc_wrapper(OS_PT_REG);
                l4_virtual[l4_entry] = (p_addr_l3 << 12) + 7;
            }else{
                p_addr_l3 = l4_virtual[l4_entry] >> 12;
            }

            u64 * l3_virtual = (u64 *)osmap(p_addr_l3);

            u32 p_addr_l2;
            if( (l3_virtual[l3_entry] & 1) == 0){
                p_addr_l2 = os_pfn_alloc_wrapper(OS_PT_REG);
                l3_virtual[l3_entry] = (p_addr_l2 << 12) + 7;
            }else{
                p_addr_l2 = l3_virtual[l3_entry] >> 12;
            }

            u64 * l2_virtual = (u64 *)osmap(p_addr_l2);

            u32 p_addr_l1;
            if( (l2_virtual[l2_entry] & 1) == 0){
                p_addr_l1 = os_pfn_alloc_wrapper(OS_PT_REG);
                l2_virtual[l2_entry] = (p_addr_l1 << 12) + 7;
            }else{
                p_addr_l2 = l2_virtual[l2_entry] >> 12;
            }

            u32 p_addr_dt = os_pfn_alloc(USER_REG);
            u64 * l1_virtual = (u64 *)osmap(p_addr_l1);
            l1_virtual[l1_entry] = (p_addr_dt << 12) + 7;

            exit_or_restore = 1;



    }else{
         printf("[GEMOS] [PageFaultHandler] This VA is not legitimate : RIP [%x] ERROR CODE [%x], VA [%x]\n",rip, error ,virtual_addr);
    }

    if(exit_or_restore == 0){
        do_exit();
    }else{
        // do  restore
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
                     "mov %%rbp, %%rsp;"
                     "pop %%rbp;"
                     "add $8, %%rsp;"
                     "iretq"
                     :
                     :"m"(rsp)
        );

    }


    return 0;
}
