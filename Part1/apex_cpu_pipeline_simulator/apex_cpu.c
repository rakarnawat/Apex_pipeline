/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"


int regArray[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int arrLen = 0; 
int stalled=0;
int prevInstValue=0;
int canProceed = 0;
int argValue=0;
int branchValue=0;
int neverTrue=0;
int validCheck[] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


int ENABLE_DEBUG_MESSAGES = 0;
int ENABLE_DISPLAY = 0;
int ENABLE_SIMULATE = 0;
int ENABLE_SHOW_MEM = 0;

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        {
           printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_SUB:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_MUL:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_DIV:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_AND:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_OR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break; 
        }
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2, stage->imm);
            break;
        }

        case OPCODE_BZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_ADDL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }
        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs3, stage->rs1, stage->rs2);
            break;
        }
        case OPCODE_LDR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break;
        }
        
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{
    APEX_Instruction *current_ins;
    
    
    if(stalled==0){
        if (cpu->fetch.has_insn)
        {
            /* This fetches new branch target instruction from next cycle */
            if (cpu->fetch_from_next_cycle == TRUE)
            {
                cpu->fetch_from_next_cycle = FALSE;

                /* Skip this cycle*/
                return;
            }

            /* Store current PC in fetch latch */
            cpu->fetch.pc = cpu->pc;

            /* Index into code memory using this pc and copy all instruction fields
            * into fetch latch  */
            current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
            strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
            cpu->fetch.opcode = current_ins->opcode;
            cpu->fetch.rd = current_ins->rd;
            cpu->fetch.rs1 = current_ins->rs1;
            cpu->fetch.rs2 = current_ins->rs2;
            if(strcmp(current_ins->opcode_str,"STR")==0){
            cpu->fetch.rs3 = current_ins->rs3;}
            cpu->fetch.imm = current_ins->imm;

            /* Update PC for next instruction */
            cpu->pc += 4;


            if(strcmp(cpu->fetch.opcode_str,"ADD")==0 || strcmp(cpu->fetch.opcode_str,"ADDL")==0 ||
            strcmp(cpu->fetch.opcode_str,"SUB")==0 || strcmp(cpu->fetch.opcode_str,"SUBL")==0 || 
            strcmp(cpu->fetch.opcode_str,"MUL")==0 || strcmp(cpu->fetch.opcode_str,"LDR")==0 || 
            strcmp(cpu->fetch.opcode_str,"DIV")==0 || strcmp(cpu->fetch.opcode_str,"AND")==0 ||  
            strcmp(cpu->fetch.opcode_str,"OR")==0 || strcmp(cpu->fetch.opcode_str,"XOR")==0 ||  
            strcmp(cpu->fetch.opcode_str,"LOAD")==0 || strcmp(cpu->fetch.opcode_str,"MOVC")==0){
                //printf("\nCurrent rd:%d\tfetch rd:%d",current_ins->rd,cpu->fetch.rd);

                validCheck[current_ins->rd]=1;

            }
            // printf("\nValid Check:");
            // for(int i=0; i<16;i++){
            //     printf(" %d ",validCheck[i]);
            // }
            // printf("\n");

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
            
            

            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->fetch.has_insn = FALSE;
            }
        }
    }
    if (ENABLE_DEBUG_MESSAGES && cpu->fetch.has_insn == TRUE && stalled==0)
    {    
        print_stage_content("Fetch", &cpu->fetch);
    }
    else{
        printf("Fetch          : EMPTY\n");
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{   if(cpu->decode.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("Decode/RF      : EMPTY\n");
    }
    
//------------------
// printf("\n---------------------------\nArray: ");
// for(int i = 0; i < arrLen; i++){
//     printf(" %d ",regArray[i]);
// }
// printf("\nArray length: %d",arrLen);
// printf("\n-----------------------------\n");
// printf("Z flag : %d\n---------------------------\n", cpu->zero_flag);
//------------------

    if (cpu->decode.has_insn)
    {
        if(arrLen==0){canProceed=1;}
        else{canProceed=0;}
        //printf("canProceed:%d\n",canProceed);
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            
            case OPCODE_ADD:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }
                        else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                //printf("canProceed--: %d\n",canProceed);
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_SUB:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }
                        else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
                
            }
            case OPCODE_MUL:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_DIV:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_AND:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_OR:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                            canProceed = 0;
                            stalled = 1;
                            break;
                        }else{
                            canProceed = 1;
                            stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_XOR:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_MOVC:
            {
                //printf("MOVC rd: %d\n",cpu->decode.rd);
                if(canProceed==0){
                    for(int i=0; i<arrLen; i++){
                        if(regArray[i]==cpu->decode.rd){
                            stalled=1;
                            canProceed = 0;
                            break;
                        }else{
                            canProceed = 1;    
                        }
                    }    
                }
                if(canProceed == 1){
                    stalled=0;
                    regArray[arrLen] = cpu->decode.rd;
                    arrLen+=1;
                    /* MOVC doesn't have register operands */
                    //break;
                }
                break;
            }
            case OPCODE_LOAD:
            {
                
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //break;
                }
                break;
            }
            case OPCODE_STORE:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        //if(regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                    
                }
                if(canProceed==1){
                    stalled=0;
                    //regArray[arrLen]=cpu->decode.rd;
                    //arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_ADDL:
            {

                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //break;
                }
                break;
            }
            case OPCODE_SUBL:
            {
                
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //break;
                }
                break;
            }
            case OPCODE_NOP:
            {
                stalled=0;
                break;
            }
            case OPCODE_CMP:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    //regArray[arrLen]=cpu->decode.rd;
                    //arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            case OPCODE_STR:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2
                        || regArray[i]==cpu->decode.rs3){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    //regArray[arrLen]=cpu->decode.rd;
                    //arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
                    //break;
                }
                break;
            }
            case OPCODE_LDR:
            {
                if(canProceed==0){
                    for(int i=0; i<arrLen;i++){
                        if(regArray[i]==cpu->decode.rs1 || regArray[i]==cpu->decode.rs2){
                        canProceed = 0;
                        stalled = 1;
                        break;
                        }else{
                        canProceed = 1;
                        stalled = 0;
                        }
                    }
                }
                if(canProceed==1){
                    stalled=0;
                    regArray[arrLen]=cpu->decode.rd;
                    arrLen+=1;
                }
                //printf("stalled:%d\n",stalled);
                if(stalled==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //break;
                }
                break;
            }
            
        }

        /* Copy data from decode latch to execute latch*/
        if(stalled==0){
        cpu->execute = cpu->decode;
        cpu->decode.has_insn = FALSE;
        }
        
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
        
    }

//------------------
// printf("\n----------------------------\nArray before execute: ");
// for(int i = 0; i < arrLen; i++){
//     printf(" %d ",regArray[i]);
// }
// printf("\nArray length: %d",arrLen);
// printf("\n------------------------------\n");
// printf("Z flag after decode: %d\n---------------------------\n", cpu->zero_flag);
//------------------


}


/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{   
    if(cpu->execute.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("Execute        : EMPTY\n");
    } else
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
               
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####ADD setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####ADD setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_SUB:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####SUB setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####SUB setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_MUL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####MUL setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####MUL setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_DIV:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value / cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####DIV setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####DIV setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_AND:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value & cpu->execute.rs2_value;

                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####AND setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####AND setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_OR:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####OR setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####OR setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_XOR:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.rs2_value: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####XOR setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####XOR setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.imm: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####MOVC setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####MOVC setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_LOAD:
            {
                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }
            case OPCODE_STORE:
            {
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                //cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
                
                break;
            }
            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BNZ:
            {
                
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.imm: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####ADDL setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####ADDL setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;
                //printf("\n------------------------------\ncpu->execute.result_buffer: %d\ncpu->execute.rs1_value: %d\ncpu->execute.imm: %d\n---------------------------------------\n",cpu->execute.result_buffer,cpu->execute.rs1_value,cpu->execute.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                //printf("\n#####SUBL setting zero flag to : %d ########\n", cpu->zero_flag);
                break;
            }
            case OPCODE_NOP:
            {
                //do noting
                break;
            }
            case OPCODE_CMP:
            {
                //cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;
                if(cpu->execute.rs1_value==cpu->execute.rs2_value){
                    //set z flag to 1
                    cpu->execute.result_buffer=0;

                    //cpu->zero_flag = TRUE;
                    //cpu->p_flag = FALSE;

                }else{
                    cpu->execute.result_buffer=1;
                    //set p flag to 1
                    //cpu->zero_flag = FALSE;
                    //cpu->p_flag = TRUE;

                }
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                //printf("\n#####CMP setting zero flag to : %d ########\n", cpu->zero_flag);
                break;
            }
            case OPCODE_STR:
            {
                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
                //cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;
                break;
            }
            case OPCODE_LDR:
            {
                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
                break;
            }
            
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{
    if(cpu->memory.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("Memory         : EMPTY\n");
    }
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            {
                /* No work for ADD */
                break;
            }
            case OPCODE_SUB:
            {
                break;
            }
            case OPCODE_MUL:
            {
                break;
            }
            case OPCODE_DIV:
            {
                break;
            }
            case OPCODE_AND:
            {
                break;
            }
            case OPCODE_OR:
            {
                break;
            }
            case OPCODE_XOR:
            {
                break;
            }
            case OPCODE_MOVC:
            {
                break;
            }
            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
                break;
            }
            case OPCODE_STORE:
            {
                /* Store to data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                break;
            }
            //case OPCODE_BZ:{}
            //case OPCODE_BNZ:{}
            case OPCODE_ADDL:
            {
                break;
            }
            case OPCODE_SUBL:
            {
                break;
            }
            case OPCODE_NOP:
            {
                break;
            }
            case OPCODE_CMP:
            {
                cpu->memory.result_buffer = cpu->zero_flag;
                break;
            }
            case OPCODE_STR:
            {
                /* Store to data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs3_value;
                break;
            }
            case OPCODE_LDR:
            {
                /* Read from data memory */
                cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{
    if(cpu->writeback.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("Writeback      : EMPTY\n");
    }
    if (cpu->writeback.has_insn)
    {

        
            if(
            strcmp(cpu->writeback.opcode_str,"ADD")==0  || strcmp(cpu->writeback.opcode_str,"ADDL")==0 ||
            strcmp(cpu->writeback.opcode_str,"SUB")==0  || strcmp(cpu->writeback.opcode_str,"SUBL")==0 || 
            strcmp(cpu->writeback.opcode_str,"MUL")==0  || strcmp(cpu->writeback.opcode_str,"LDR")==0 || 
            strcmp(cpu->writeback.opcode_str,"DIV")==0  || strcmp(cpu->writeback.opcode_str,"AND")==0 ||  
            strcmp(cpu->writeback.opcode_str,"OR")==0   || strcmp(cpu->writeback.opcode_str,"XOR")==0 ||  
            strcmp(cpu->writeback.opcode_str,"LOAD")==0 || strcmp(cpu->writeback.opcode_str,"MOVC")==0){
                //printf("\nCurrent rd:%d\tfetch rd:%d",cpu->writeback.rd,cpu->writeback.rd);

                validCheck[cpu->writeback.rd]=0;

            }
            // printf("\nValid Check:");
            // for(int i=0; i<16;i++){
            //     printf(" %d ",validCheck[i]);
            // }
            // printf("\n");
   

        
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
//                 //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                        //break;
                   }
               }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_SUB:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_MUL:
            {
                
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_MOVC: 
            {   
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled =0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
                
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                 stalled=0;
                // //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_STORE:
            {
                //cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                 stalled=0;
                // //prevInstValue=0;
                // for(int i=0; i<arrLen;i++){
                //     if(regArray[i]==cpu->writeback.rd){
                //         for(int j=i;j<arrLen;j++ ){
                //             regArray[j]=regArray[j+1];
                //         }
                //         arrLen-=1;
                //         break;
                //     }
                // }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_BZ:
            {
                break;
            }
            case OPCODE_BNZ:
            {
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_NOP:
            {
                break;
            }
            case OPCODE_CMP:
            {
                //cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                // for(int i=0; i<arrLen;i++){
                //     if(regArray[i]==cpu->writeback.rs1 || regArray[i]==cpu->writeback.rs2){
                //         for(int j=i;j<arrLen;j++ ){
                //             regArray[j]=regArray[j+1];
                //         }
                //         arrLen-=1;
                //         i=i-1;
                //     }
                // }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_STR:
            {
                //cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                 stalled=0;
                // //prevInstValue=0;
                // for(int i=0; i<arrLen;i++){
                //     if(regArray[i]==cpu->writeback.rd){
                //         for(int j=i;j<arrLen;j++ ){
                //             regArray[j]=regArray[j+1];
                //         }
                //         arrLen-=1;
                //         break;
                //     }
                // }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_LDR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
//--------------------------------------------------------------------------------------
                // //printf("writeback rd: %d\n",cpu->writeback.rd);
                stalled=0;
                //prevInstValue=0;
                for(int i=0; i<arrLen;i++){
                    if(regArray[i]==cpu->writeback.rd){
                        for(int j=i;j<arrLen;j++ ){
                            regArray[j]=regArray[j+1];
                        }
                        arrLen-=1;
                        break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
   
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename, const char *argv2, const int argv3)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }
//-------------------------------------------------------------------------
    // printf("\n----------------------\n");
    // printf("\nargv2: %s, argv3: %d",argv2,argv3);
    // printf("\n----------------------\n");
//-------------------------------------------------------------------------
    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
//--------------------------------------------------------------------------    
    if(strcmp(argv2,"display")==0){
        cpu->single_step = argv3;
        ENABLE_DISPLAY = 1;
    }else if(strcmp(argv2,"simulate")==0){
        cpu->single_step = argv3;
        ENABLE_SIMULATE = 1;
    }else if(strcmp(argv2,"show_mem")==0){
        cpu->single_step = argv3;
        argValue=argv3;

        //printf("argv: %d\n,",argv3);
        cpu->show_mem = 1;
        ENABLE_SHOW_MEM = 1;
    }else{
        cpu->single_step = 1;
    }
//--------------------------------------------------------------------------
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,"APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2","imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    //printf("\ncpu->single_step : %d\n",cpu->single_step);

    while (TRUE)
    {
        if(ENABLE_SHOW_MEM){
            ENABLE_DEBUG_MESSAGES = 0;

        }
        else if(ENABLE_SIMULATE){
            ENABLE_DEBUG_MESSAGES = 0;
        }
        else{
            ENABLE_DEBUG_MESSAGES = 1;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu, ENABLE_DEBUG_MESSAGES))
        {
            if(cpu->show_mem == 1){
                int regNum=4000;
                int count=0;
                while(regNum!=argValue){
                            regNum+=4;
                            count+=1;
                            if(count>=16){
                                cpu->regs[count]=0;
                            }
                            if(regNum>=argValue){
                                break;
                            }
                            
                        }
                //printf("cpu->single_step: %d ", argv3 );
                printf("-------------------------------------------------------------------------------\n");
                printf("Memory Location: %d, is associated with register R%d and contains value: %d\n",argValue,count,cpu->regs[count]);
                printf("-------------------------------------------------------------------------------\n");
                break;
            }
            else{
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
            }
        }

        APEX_memory(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_execute(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_decode(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_fetch(cpu, ENABLE_DEBUG_MESSAGES);

        if(neverTrue==1){
            print_reg_file(cpu);
        }


        //printf("\n+++++given value: %d +++++++\n",cpu->single_step);
        if (cpu->single_step==1 && (ENABLE_DISPLAY ==1 || ENABLE_SIMULATE==1 )){
            user_prompt_val= 'q';
            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
                break;
            }
        }
        if (cpu->single_step==1 && ENABLE_DISPLAY==0)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
                break;
            }
        }else if(cpu->single_step>1){
            cpu->single_step-=1;
        }

        cpu->clock++;
    }
if(cpu->show_mem == 0){
    char* space = " ";
    char* status= "";
    printf("\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");
    for(int i=0;i<16;i++)
    {
        if(validCheck[i]==0){
            status="VALID";
        }else{
            status="INVALID";
        }
        printf("\n |%-2s REG[%-2d] %-2s |%-2s Value = %-3d %-2s|  status= %-7s |",
        space,i,space,space,cpu->regs[i],space,status);
    }
    printf("\n\n================ STATE OF DATA MEMORY ================\n");

    for(int i=0;i<100;i++)
    {
        printf(" | MEM[%-2d]%-2s | Value=%-3d %-2s| \n",i,space,cpu->data_memory[i],space);
    }
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
