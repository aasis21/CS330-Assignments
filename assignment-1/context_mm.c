#include<context.h>
#include<memory.h>
#include<lib.h>



u64 page_table_entry(u32 pfn,int u_s, int write, int present){
    u64 entry = 0 ;
    if(write){
        int a = 2;
        entry = entry | a;
    }
    if(present) {
        entry =  entry | 1;
    }
    if(u_s){
        entry = entry | 4;
    }
    entry = entry | ( (u64)pfn << 12 );

    return entry;
}

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

void prepare_context_mm(struct exec_context *ctx)
{

   // printf("------------all segments -------------\n");
   // printf("Code  : start=%x , end=%x , flags:%x \n", ctx->mms[MM_SEG_CODE].start,ctx->mms[MM_SEG_CODE].end,ctx->mms[MM_SEG_CODE].access_flags);
   // printf("Data  : start=%x , end=%x , flags:%x \n", ctx->mms[MM_SEG_DATA].start,ctx->mms[MM_SEG_DATA].end,ctx->mms[MM_SEG_DATA].access_flags);
   // printf("Stack : start=%x , end=%x , flags:%x \n", ctx->mms[MM_SEG_STACK].start,ctx->mms[MM_SEG_STACK].end,ctx->mms[MM_SEG_STACK].access_flags);
   // printf("---------all segments info end----------\n");

    u32 p_addr_l4 = os_pfn_alloc_wrapper(OS_PT_REG);
    u64 * l4_virtual = (u64 *)osmap(p_addr_l4);
    ctx->pgd = p_addr_l4;

   //do stack page table entry

    u64 stack_start = ctx->mms[MM_SEG_STACK].end - (1 << 12);

    int stack_access_flag = ( ctx->mms[MM_SEG_STACK].access_flags & 2) ;

    u64  stack_l4_entry =  ( stack_start << 16 ) >> 55;
    u64  stack_l3_entry =  ( stack_start << 25 ) >> 55;
    u64  stack_l2_entry =  ( stack_start << 34 ) >> 55;
    u64  stack_l1_entry =  ( stack_start << 43 ) >> 55;


    u32 p_addr_l3_stack = 0;
    if( (l4_virtual[stack_l4_entry] & 1) == 0){
        p_addr_l3_stack = os_pfn_alloc_wrapper(OS_PT_REG);
        l4_virtual[stack_l4_entry] = page_table_entry(p_addr_l3_stack,1,stack_access_flag,1);
    }else{
        p_addr_l3_stack = l4_virtual[stack_l4_entry] >> 12;
    }

    u64 * l3_stack_virtual = (u64 *)osmap(p_addr_l3_stack);

    u32 p_addr_l2_stack = 0;
    if( (l3_stack_virtual[stack_l3_entry] & 1) == 0){
        p_addr_l2_stack = os_pfn_alloc_wrapper(OS_PT_REG);
        l3_stack_virtual[stack_l3_entry] = page_table_entry(p_addr_l2_stack,1,stack_access_flag,1);
    }else{
        p_addr_l2_stack = l3_stack_virtual[stack_l3_entry] >> 12;
    }

    u64 * l2_stack_virtual = (u64 *)osmap(p_addr_l2_stack);

    u32 p_addr_l1_stack = 0;
    if( (l2_stack_virtual[stack_l2_entry] & 1) == 0){
        p_addr_l1_stack = os_pfn_alloc_wrapper(OS_PT_REG);
        l2_stack_virtual[stack_l2_entry] = page_table_entry(p_addr_l1_stack,1,stack_access_flag,1);
    }else{
        p_addr_l1_stack = l2_stack_virtual[stack_l2_entry] >> 12;
    }

    u32 p_addr_dt_stack = os_pfn_alloc(USER_REG);
    u64 * l1_stack_virtual = (u64 *)osmap(p_addr_l1_stack);
    l1_stack_virtual[stack_l1_entry] = page_table_entry(p_addr_dt_stack,1,stack_access_flag,1);


    //do code page table entry

    u64 code_start = ctx->mms[MM_SEG_CODE].start;

    int code_access_flag = ( ctx->mms[MM_SEG_CODE].access_flags & 2) ;

    u64  code_l4_entry =  ( code_start << 16 ) >> 55;
    u64  code_l3_entry =  ( code_start << 25 ) >> 55;
    u64  code_l2_entry =  ( code_start << 34 ) >> 55;
    u64  code_l1_entry =  ( code_start << 43 ) >> 55;


    u32 p_addr_l3_code = 0;
    if( (l4_virtual[code_l4_entry] & 1 ) == 0){
        p_addr_l3_code = os_pfn_alloc_wrapper(OS_PT_REG);
        l4_virtual[code_l4_entry] = page_table_entry(p_addr_l3_code,1,code_access_flag,1);
    }else{
        p_addr_l3_code = l4_virtual[code_l4_entry] >> 12;
    }

    u64 * l3_code_virtual = (u64 *)osmap(p_addr_l3_code);

    u32 p_addr_l2_code = 0;
    if( (l3_code_virtual[code_l3_entry] & 1) == 0){
        p_addr_l2_code = os_pfn_alloc_wrapper(OS_PT_REG);
        l3_code_virtual[code_l3_entry] = page_table_entry(p_addr_l2_code,1,code_access_flag,1);
    }else{
        p_addr_l2_code = l3_code_virtual[code_l3_entry] >> 12;
    }

    u64 * l2_code_virtual = (u64 *)osmap(p_addr_l2_code);

    u32 p_addr_l1_code = 0;
    if( (l2_code_virtual[code_l2_entry] & 1 )== 0){
        p_addr_l1_code = os_pfn_alloc_wrapper(OS_PT_REG);
        l2_code_virtual[code_l2_entry] = page_table_entry(p_addr_l1_code,1,code_access_flag,1);
    }else{
        p_addr_l1_code = l2_code_virtual[code_l2_entry] >> 12;
    }

    u32 p_addr_dt_code = os_pfn_alloc(USER_REG);
    u64 * l1_code_virtual = (u64 *)osmap(p_addr_l1_code);
    l1_code_virtual[code_l1_entry] = page_table_entry(p_addr_dt_code,1,0,1);


    // do data page table entry

    u64 data_start = ctx->mms[MM_SEG_DATA].start;

    int data_access_flag = ( ctx->mms[MM_SEG_DATA].access_flags & 2) ;


    u64  data_l4_entry =  ( data_start << 16 ) >> 55;
    u64  data_l3_entry =  ( data_start << 25 ) >> 55;
    u64  data_l2_entry =  ( data_start << 34 ) >> 55;
    u64  data_l1_entry =  ( data_start << 43 ) >> 55;

    u32 p_addr_l3_data = 0;
    if( (l4_virtual[data_l4_entry] & 1) == 0){
        p_addr_l3_data = os_pfn_alloc_wrapper(OS_PT_REG);
        l4_virtual[data_l4_entry] = page_table_entry(p_addr_l3_data,1,data_access_flag,1);
    }else{
        p_addr_l3_data = l4_virtual[data_l4_entry] >> 12;
    }

    u64 * l3_data_virtual = (u64 *)osmap(p_addr_l3_data);

    u32 p_addr_l2_data = 0;
    if( (l3_data_virtual[data_l3_entry] & 1) == 0){
        p_addr_l2_data = os_pfn_alloc_wrapper(OS_PT_REG);
        l3_data_virtual[data_l3_entry] = page_table_entry(p_addr_l2_data,1,data_access_flag,1);
    }else{
        p_addr_l2_data = l3_data_virtual[data_l3_entry] >> 12;
    }

    u64 * l2_data_virtual = (u64 *)osmap(p_addr_l2_data);

    u32 p_addr_l1_data = 0;
    if( (l2_data_virtual[data_l2_entry] & 1 )== 0){
        p_addr_l1_data = os_pfn_alloc_wrapper(OS_PT_REG);
        l2_data_virtual[data_l2_entry] = page_table_entry(p_addr_l1_data,1,data_access_flag,1);
    }else{
        p_addr_l1_data = l2_data_virtual[data_l2_entry] >> 12;
    }

    u32 p_addr_dt_data = ctx->arg_pfn;
    u64 * l1_data_virtual = (u64 *)osmap(p_addr_l1_data);
    l1_data_virtual[data_l1_entry] = page_table_entry(p_addr_dt_data,1,data_access_flag,1);



    return;

}
void cleanup_context_mm(struct exec_context *ctx)
{
   u32 p_addr_l4 = ctx->pgd;
   u64 * l4_virtual = (u64 *)osmap(p_addr_l4);

   // free stack info

   u64 stack_start =  ctx->mms[MM_SEG_STACK].end - ( 1 << 12 )  ;

   u64  stack_l4_entry =  ( stack_start << 16 ) >> 55;
   u64  stack_l3_entry =  ( stack_start << 25 ) >> 55;
   u64  stack_l2_entry =  ( stack_start << 34 ) >> 55;
   u64  stack_l1_entry =  ( stack_start << 43 ) >> 55;

   u64 * l3_stack_virtual = (u64 *)osmap(l4_virtual[stack_l4_entry] >> 12);
   u64 l3_stack_physical = l4_virtual[stack_l4_entry] >> 12;

   u64 * l2_stack_virtual = (u64 *)osmap(l3_stack_virtual[stack_l3_entry] >> 12);
   u64 l2_stack_physical = l3_stack_virtual[stack_l3_entry] >> 12;

   u64 * l1_stack_virtual = (u64 *)osmap(l2_stack_virtual[stack_l2_entry] >> 12);
   u64 l1_stack_physical = l2_stack_virtual[stack_l2_entry] >> 12;

   u64 dt_stack_physical = l1_stack_virtual[stack_l1_entry] >> 12;

   // free code info

   u64 code_start =  ctx->mms[MM_SEG_CODE].start ;

   u64  code_l4_entry =  ( code_start << 16 ) >> 55;
   u64  code_l3_entry =  ( code_start << 25 ) >> 55;
   u64  code_l2_entry =  ( code_start << 34 ) >> 55;
   u64  code_l1_entry =  ( code_start << 43 ) >> 55;

   u64 * l3_code_virtual = (u64 *)osmap(l4_virtual[code_l4_entry] >> 12);
   u64 l3_code_physical = l4_virtual[code_l4_entry] >> 12;

   u64 * l2_code_virtual = (u64 *)osmap(l3_code_virtual[code_l3_entry] >> 12);
   u64 l2_code_physical = l3_code_virtual[code_l3_entry] >> 12;

   u64 * l1_code_virtual = (u64 *)osmap(l2_code_virtual[code_l2_entry] >> 12);
   u64 l1_code_physical = l2_code_virtual[code_l2_entry] >> 12;

   u64 dt_code_physical = l1_code_virtual[code_l1_entry] >> 12;

   // free data info

   u64 data_start =  ctx->mms[MM_SEG_DATA].start;

   u64  data_l4_entry =  ( data_start << 16 ) >> 55;
   u64  data_l3_entry =  ( data_start << 25 ) >> 55;
   u64  data_l2_entry =  ( data_start << 34 ) >> 55;
   u64  data_l1_entry =  ( data_start << 43 ) >> 55;

   u64 * l3_data_virtual = (u64 *)osmap(l4_virtual[data_l4_entry] >> 12);
   u64 l3_data_physical = l4_virtual[data_l4_entry] >> 12;

   u64 * l2_data_virtual = (u64 *)osmap(l3_data_virtual[data_l3_entry] >> 12);
   u64 l2_data_physical = l3_data_virtual[data_l3_entry] >> 12;

   u64 * l1_data_virtual = (u64 *)osmap(l2_data_virtual[data_l2_entry] >> 12);
   u64 l1_data_physical = l2_data_virtual[data_l2_entry] >> 12;

   u64 dt_data_physical = l1_data_virtual[data_l1_entry] >> 12;

   // free all the used pfn in the context

   os_pfn_free(USER_REG, dt_code_physical);
   os_pfn_free(USER_REG, dt_stack_physical);
   os_pfn_free(USER_REG, dt_data_physical);

   os_pfn_free(OS_PT_REG, l1_stack_physical);
   os_pfn_free(OS_PT_REG, l1_code_physical);
   os_pfn_free(OS_PT_REG, l1_data_physical);

   os_pfn_free(OS_PT_REG, l2_stack_physical);
   if(l2_stack_physical != l2_code_physical){
       os_pfn_free(OS_PT_REG, l2_code_physical);
   }
   if(l2_stack_physical != l2_data_physical && l2_code_physical != l2_data_physical ){
       os_pfn_free(OS_PT_REG, l2_data_physical);
   }

   os_pfn_free(OS_PT_REG, l3_stack_physical);
   if(l3_stack_physical != l3_code_physical){
       os_pfn_free(OS_PT_REG, l3_code_physical);
   }
   if(l3_stack_physical != l3_data_physical && l3_code_physical != l3_data_physical ){
       os_pfn_free(OS_PT_REG, l3_data_physical);
   }

   os_pfn_free(OS_PT_REG, p_addr_l4);

   return;
}
