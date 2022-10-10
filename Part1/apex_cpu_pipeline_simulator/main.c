/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);
    if(strcmp(argv[2],"single_step")==0){
        argv[3]="";
        argc = 4;
    }
    if (argc != 4 )
    {
        printf("FOUR ARGUMENTS ARE REQUIRED!\n");
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }


    //printf("\n-------------------------------\n");
    //printf("++argv0: %s ++\n",argv[0]);
    //printf("++argv1: %s ++\n",argv[1]);
    //printf("++argv2: %s ++\n",argv[2]);
    //printf("++argv3: %s ++\n",argv[3]);
    //printf("\n-------------------------------\n");
    

    cpu = APEX_cpu_init(argv[1], argv[2], atoi(argv[3]));
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }


    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}