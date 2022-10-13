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

int mulCycle=0;
int lsCycle=0;
int orderArray[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int orderLen=0;
int intINS=5;
int mulIns=7;
int lsIns=9;
int lsStalled=0;
int intCycle=0;
int mulStalled=0;
int intStalled=0;
int proceed = 0;

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
    
    //printf("\nENABLE_DEBUG_MESSAGES:%d\t stalled:%d\n",ENABLE_DEBUG_MESSAGES,stalled);
    // if(cpu->fetch.has_insn == FALSE){
    //     printf("cpu->fetch.has_insn == FALSE\n");
    // }
    // if(cpu->fetch.has_insn == TRUE){
    //     printf("cpu->fetch.has_insn == TRUE\n");
    // }

    if(cpu->fetch.has_insn == FALSE){
        printf("Fetch          : EMPTY\n");
    }
    
        
    if(stalled==0 && proceed==1){
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

            if(lsStalled==1){
                if(strcmp(current_ins->opcode_str,"LOAD")==0|| strcmp(current_ins->opcode_str,"STORE")==0 ||
                strcmp(current_ins->opcode_str,"LDR")==0 || strcmp(current_ins->opcode_str,"STR")==0){
                    lsStalled=1;
                }
            }
            if(mulStalled==1){
                if(strcmp(current_ins->opcode_str,"MUL")==0){
                    mulStalled=1;
                }
            }
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
            strcmp(cpu->fetch.opcode_str,"OR")==0 || strcmp(cpu->fetch.opcode_str,"EXOR")==0 ||  
            strcmp(cpu->fetch.opcode_str,"LOAD")==0 || strcmp(cpu->fetch.opcode_str,"MOVC")==0){
                //printf("\nCurrent rd:%d\tfetch rd:%d",current_ins->rd,cpu->fetch.rd);

                validCheck[current_ins->rd]=1;

            }

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
            //printf("\nENABLE_DEBUG_MESSAGES: %d\n",ENABLE_DEBUG_MESSAGES);
            if (ENABLE_DEBUG_MESSAGES)
            {    
                print_stage_content("Fetch", &cpu->fetch);
            }   

            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->fetch.has_insn = FALSE;
            }
        }
    }else{
        printf("Fetch          : STALLED\n");

    }
    
}
/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{   
    if(cpu->decode.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("Decode/RF      : EMPTY\n");
    }else{
        print_stage_content("Decode/RF", &cpu->decode);
    }
//------------------
// printf("\n---------------------------\nArray: ");
// for(int i = 0; i < arrLen; i++){
//     printf(" %d ",regArray[i]);
// }
// printf("\nArray length: %d",arrLen);
// printf("\n-----------------------------\n");
//printf("Z flag : %d\n---------------------------\n", cpu->zero_flag);
//------------------
// printf("\n----------------------------\nORDER Array before decode: ");
// for(int i = 0; i <= orderLen; i++){
//     printf(" %d ",orderArray[i]);
// }
// printf("\nArray length: %d",orderLen);
// printf("\n------------------------------\n");
    //else

if(cpu->fetch.has_insn && (strcmp(cpu->fetch.opcode_str,"LOAD")==0 || 
    strcmp(cpu->fetch.opcode_str,"STORE")==0 || strcmp(cpu->fetch.opcode_str,"STR")==0 || 
    strcmp(cpu->fetch.opcode_str,"LDR")==0)){
        if(lsStalled==0){
            proceed=1;
        }else if(lsStalled==1){
            proceed=0;
        }
    }else if(strcmp(cpu->fetch.opcode_str,"MUL")==0){
            if(mulStalled==0){
                proceed=1;
            }else{
                proceed=0;
            }
        }
    else{
        if(intStalled==0){
            proceed=1;
        }else{
            proceed=0;
        }
    }

    if (cpu->decode.has_insn && proceed==1)
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
        //printf("\nStalled:%d\t lsStalled:%d\t mulStalled:%d\t intStalled:%d\n ",stalled,lsStalled,mulStalled,intStalled);
        // if(cpu->MUL_FU.has_insn==TRUE){
        //     //printf("\nMUL have an instruction:%s,R%d,R%d,R%d\ncycle: %d\n",
        //     //cpu->decode.opcode_str, cpu->decode.rd, cpu->decode.rs1, cpu->decode.rs2,mulCycle);
        //     mulCycle+=1;
        //     //stalled=1;
        // }
        // if (cpu->LOAD_STORE_FU.has_insn==TRUE)
        // {
        //     printf("\nLOAD/STORE have cycle: %d\n",lsCycle);
        //     lsCycle+=1;
        //     //stalled=1;
        // }
        // if(cpu->INT_FU.has_insn==TRUE){
        //     intCycle=1;
        // }
        

        /* Copy data from decode latch to execute latch*/
       
        if(stalled==0){
            if(mulStalled==0){
                if(strcmp(cpu->decode.opcode_str,"MUL")==0){
                    //printf("\nMUL Order Length: %d\n",orderLen);
                    orderArray[orderLen]=mulIns;
                    orderLen+=1;
                    cpu->MUL_FU = cpu->decode;
                    //print_stage_content("Decode/RF", &cpu->decode);
                    cpu->decode.has_insn = FALSE;
                }
            }
            if(lsStalled==0){ 
                if(strcmp(cpu->decode.opcode_str,"LOAD")==0 || strcmp(cpu->decode.opcode_str,"STORE")==0 ||
                strcmp(cpu->decode.opcode_str,"LDR")==0 || strcmp(cpu->decode.opcode_str,"STR")==0){
                    //printf("\nLS Order Length: %d\n",orderLen);
                    orderArray[orderLen]=lsIns;
                    orderLen+=1;
                    cpu->LOAD_STORE_FU = cpu->decode;
                    //print_stage_content("Decode/RF", &cpu->decode);
                    cpu->decode.has_insn = FALSE;
                }
            }
            if(intStalled==0){
                if(strcmp(cpu->decode.opcode_str,"ADD")==0 || strcmp(cpu->decode.opcode_str,"SUB")==0 ||
                strcmp(cpu->decode.opcode_str,"DIV")==0 || strcmp(cpu->decode.opcode_str,"AND")==0 ||
                strcmp(cpu->decode.opcode_str,"OR")==0 || strcmp(cpu->decode.opcode_str,"EXOR")==0 ||
                strcmp(cpu->decode.opcode_str,"MOVC")==0 || strcmp(cpu->decode.opcode_str,"BZ")==0 ||
                strcmp(cpu->decode.opcode_str,"BNZ")==0 || strcmp(cpu->decode.opcode_str,"ADDL")==0 ||
                strcmp(cpu->decode.opcode_str,"NOP")==0 || strcmp(cpu->decode.opcode_str,"SUBL")==0 ||
                strcmp(cpu->decode.opcode_str,"CMP")==0 || strcmp(cpu->decode.opcode_str,"HALT")==0){
                    ////printf("\nint Order Length: %d\n",orderLen);
                    //if(lsCycle==3 || lsCycle==0){
                        orderArray[orderLen]=intINS;
                        orderLen+=1;
                        cpu->INT_FU = cpu->decode;
                        //print_stage_content("Decode/RF", &cpu->decode);
                        cpu->decode.has_insn = FALSE;
                    //}
                }
            }
            // printf("\nStalled:%d\t lsStalled:%d\t mulStalled:%d\t intStalled:%d\n ",stalled,lsStalled,mulStalled,intStalled);
            // if(strcmp(cpu->decode.opcode_str,"HALT")==0 && lsStalled==0 && mulStalled==0 && intStalled==0){
            //     cpu->writeback = cpu->decode;
            //     cpu->decode.has_insn=FALSE;
            // }
        }
        // if(ENABLE_DISPLAY!=0){
        //     if (ENABLE_DEBUG_MESSAGES || stalled==0)
        //     {
        //         print_stage_content("Decode/RF", &cpu->decode);
        //     }else{
        //         printf("Decode/RF      : EMPTY\n");
        //         print_stage_content("Fetch", &cpu->fetch);
        //     }
        // }

      
    }
    //print_stage_content("Decode", &cpu->decode); 
    if(lsStalled==1){
        if (cpu->LOAD_STORE_FU.has_insn==TRUE)
        {
            //printf("\nLOAD/STORE have cycle: %d\n",lsCycle);
            lsCycle+=1;
            //stalled=1;
        }
    }
    if(mulStalled==1){
        if(cpu->MUL_FU.has_insn==TRUE){
            //printf("\nMUL have cycle: %d\n",mulCycle);
            mulCycle+=1;
            //stalled=1;
        }
    }
    if(cpu->INT_FU.has_insn==TRUE){
        intCycle=1;
    }

    
    //printf("lsStalled=%d && mulStalled=%d && intStalled=%d && orderLen=%d && regArray=%d\n",lsStalled,mulStalled,intStalled,orderLen,arrLen);    
    if(strcmp(cpu->decode.opcode_str,"HALT")==0 && lsStalled==0 && mulStalled==0 && intStalled==0 && orderLen==0 && arrLen==0){
        cpu->writeback = cpu->decode;
        cpu->decode.has_insn=FALSE;
    }
//------------------
// printf("\n----------------------------\nArray before execute: ");
// for(int i = 0; i < arrLen; i++){
//     printf(" %d ",regArray[i]);
// }
// printf("\nArray length: %d",arrLen);
// printf("\n------------------------------\n");
//printf("Z flag after decode: %d\n---------------------------\n", cpu->zero_flag);
//------------------
// printf("\n----------------------------\nORDER Array before execute: ");
// for(int i = 0; i <= orderLen; i++){
//     printf(" %d ",orderArray[i]);
// }
// printf("\nArray length: %d",orderLen);
// printf("\n------------------------------\n");

}

static void
Apex_MUL_FU(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES){
    if(cpu->MUL_FU.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("MUL_FU         : EMPTY\n");
    } else if(mulStalled==1  && mulCycle!=2){
        print_stage_content("MUL_FU", &cpu->MUL_FU);
    }else
    if (cpu->MUL_FU.has_insn || mulCycle==2)
    {
        /* MUL_FU logic based on instruction type */
        
        /* Copy data from MUL_FU latch to writeback latch*/
        if (orderArray[0]==7)
        {
            if(mulCycle>=2){
                mulCycle=0;
                cpu->writeback = cpu->MUL_FU;
                cpu->MUL_FU.has_insn = FALSE;
                mulStalled=0;

            }else{
                mulStalled=1;
                cpu->MUL_FU.has_insn = TRUE;
            }
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MUL_FU", &cpu->MUL_FU);
        }
    }
    
}

static void
Apex_LOAD_STORE_FU(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES){
    if(cpu->LOAD_STORE_FU.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("LOAD_STORE_FU  : EMPTY\n");
    }else if(lsStalled==1 && lsCycle!=3){
        print_stage_content("LOAD_STORE_FU", &cpu->LOAD_STORE_FU);
    }else
    if(cpu->LOAD_STORE_FU.has_insn || lsCycle==3){
        
        //printf("\norderArray[0]: %d\n",orderArray[0]);
        if (orderArray[0]==9 || lsCycle<3)
        {
            //printf("cycle count is: %d\n",lsCycle);
            if(lsCycle==3){
                //printf("\nin load store to writeback\n");
                /* Copy data from LOAD_STORE latch to LOAD_STORE_FU latch*/
                cpu->writeback = cpu->LOAD_STORE_FU;
                lsCycle=0;
                lsStalled=0;
                cpu->LOAD_STORE_FU.has_insn = FALSE;
            }else{
                lsStalled=1;
                cpu->LOAD_STORE_FU.has_insn = TRUE;
            }

        }   
        if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("LOAD_STORE_FU", &cpu->LOAD_STORE_FU);
                //printf("\nlsCycle :%d\n",lsCycle);
            }
        
    }
}

static void
APEX_INT_FU(APEX_CPU *cpu, int ENABLE_DEBUG_MESSAGES)
{   if(orderArray[0]==5){stalled=0;}
    if(cpu->INT_FU.has_insn == FALSE && ENABLE_DEBUG_MESSAGES==1){
        printf("INT_FU         : EMPTY\n");
    } else if(intStalled==1 && intCycle!=1){
        print_stage_content("INT_FU", &cpu->MUL_FU);
    }else
    if (cpu->INT_FU.has_insn || intCycle==1)
    {
        switch (cpu->INT_FU.opcode)
        {
            
            case OPCODE_BZ:
            {
            if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->INT_FU.pc + cpu->INT_FU.imm;
                    
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
                    cpu->pc = cpu->INT_FU.pc + cpu->INT_FU.imm;
                    
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
        }

        if(orderArray[0]==5){
            /* Copy data from INT_FU latch to writeback latch*/
            cpu->writeback = cpu->INT_FU;
            intStalled=0;
            intCycle=0;
            cpu->INT_FU.has_insn = FALSE;   
        }else{
            intStalled=1;
            cpu->INT_FU.has_insn = TRUE;
        }
        if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("INT_FU", &cpu->INT_FU);
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
    // if(cpu->writeback.has_insn == TRUE){printf("\nwriteback has insc\t");}
    // else if(cpu->writeback.has_insn == FALSE){printf("\nNO writeback insc\t");}
    //printf(" ENABLE_DEBUG_MESSAGES:%d\n",ENABLE_DEBUG_MESSAGES);
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
            strcmp(cpu->writeback.opcode_str,"OR")==0   || strcmp(cpu->writeback.opcode_str,"EXOR")==0 ||  
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
                   cpu->writeback.result_buffer = cpu->writeback.rs1_value + cpu->writeback.rs2_value;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
               
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####ADD setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####ADD setting zero flag to : %d ########\n", cpu->zero_flag);
                }
             
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_SUB:
            {
                
                cpu->writeback.result_buffer = cpu->writeback.rs1_value - cpu->writeback.rs2_value;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####SUB setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####SUB setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_MUL:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value * cpu->writeback.rs2_value;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                
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
                mulStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==mulIns){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_DIV:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value / cpu->writeback.rs2_value;
                ////printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####DIV setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####DIV setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_AND:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value & cpu->writeback.rs2_value;

                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####AND setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####AND setting zero flag to : %d ########\n", cpu->zero_flag);
                }
                
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_OR:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value | cpu->writeback.rs2_value;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####OR setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####OR setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_XOR:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value ^ cpu->writeback.rs2_value;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.rs2_value: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.rs2_value);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####XOR setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####XOR setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_MOVC: 
            {   
                cpu->writeback.result_buffer = cpu->writeback.imm;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.imm: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####MOVC setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####MOVC setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
                
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_LOAD:
            {
                cpu->writeback.memory_address = cpu->writeback.rs1_value + cpu->writeback.imm;
                /* Read/LOAD from data writeback */
                cpu->writeback.result_buffer = cpu->data_memory[cpu->writeback.memory_address];
                
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
                lsStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==lsIns){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_STORE:
            {
                cpu->writeback.memory_address = cpu->writeback.rs2_value + cpu->writeback.imm;
                /* Store to data writeback */
                cpu->data_memory[cpu->writeback.memory_address] = cpu->writeback.rs1_value;
                
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
                lsStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==lsIns){

                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_BZ:
            {
                intStalled=0;
                
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
                break;
            }
            case OPCODE_BNZ:
            {
                
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value + cpu->writeback.imm;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.imm: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    //printf("\n#####ADDL setting zero flag to : %d ########\n", cpu->zero_flag);
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    //printf("\n#####ADDL setting zero flag to : %d ########\n", cpu->zero_flag);
                }
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->writeback.result_buffer = cpu->writeback.rs1_value - cpu->writeback.imm;
                //printf("\n------------------------------\ncpu->writeback.result_buffer: %d\ncpu->writeback.rs1_value: %d\ncpu->writeback.imm: %d\n---------------------------------------\n",cpu->writeback.result_buffer,cpu->writeback.rs1_value,cpu->writeback.imm);
  
                /* Set the zero flag based on the result buffer */
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                //printf("\n#####SUBL setting zero flag to : %d ########\n", cpu->zero_flag);
                
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_NOP:
            {
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
                break;
            }
            case OPCODE_CMP:
            {
                    //cpu->writeback.result_buffer = cpu->writeback.rs1_value - cpu->writeback.rs2_value;
                if(cpu->writeback.rs1_value==cpu->writeback.rs2_value){
                    //set z flag to 1
                    cpu->writeback.result_buffer=0;

                    //cpu->zero_flag = TRUE;
                    //cpu->p_flag = FALSE;

                }else{
                    cpu->writeback.result_buffer=1;
                    //set p flag to 1
                    //cpu->zero_flag = FALSE;
                    //cpu->p_flag = TRUE;

                }
                if (cpu->writeback.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                //printf("\n#####CMP setting zero flag to : %d ########\n", cpu->zero_flag);
                //cmp
                cpu->writeback.result_buffer = cpu->zero_flag;
                
            
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
                intStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==intINS){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_STR:
            {
                cpu->writeback.memory_address = cpu->writeback.rs1_value + cpu->writeback.rs2_value;
                /* Store/STR to data writeback */
                cpu->data_memory[cpu->writeback.memory_address] = cpu->writeback.rs3_value;
                
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
                lsStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==lsIns){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
                    }
                }
//-----------------------------------------------------------------------------------------
                break;
            }
            case OPCODE_LDR:
            {
                   
                cpu->writeback.memory_address = cpu->writeback.rs1_value + cpu->writeback.rs2_value;
                
                /* Read/LDR from data writeback */
                cpu->writeback.result_buffer = cpu->data_memory[cpu->LOAD_STORE_FU.memory_address];
             
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
                lsStalled=0;
                for(int i=0; i<orderLen;i++){
                    if(orderArray[i]==lsIns){
                        for(int j=i;j<orderLen;j++ ){
                            orderArray[j]=orderArray[j+1];
                        }
                        orderLen-=1;
                        break;
                        //break;
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

        Apex_LOAD_STORE_FU(cpu, ENABLE_DEBUG_MESSAGES);
        Apex_MUL_FU(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_INT_FU(cpu, ENABLE_DEBUG_MESSAGES);
        //APEX_memory(cpu, ENABLE_DEBUG_MESSAGES);
        //APEX_execute(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_decode(cpu, ENABLE_DEBUG_MESSAGES);
        APEX_fetch(cpu, ENABLE_DEBUG_MESSAGES);

        if(neverTrue==0){
            print_reg_file(cpu);
        }


        //printf("\n+++++given value: %d +++++++\n",cpu->single_step);
        if (cpu->single_step==1 && (ENABLE_DISPLAY ==1 || ENABLE_SIMULATE==1 )){
            user_prompt_val= 'Q';
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
    printf("\n\n============== STATE OF DATA MEMORY =============\n");

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
