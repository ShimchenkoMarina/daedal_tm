/** # Copyright (C) Eta Scale AB. Licensed under the Eta Scale Open Source License. See the LICENSE file for details.
 *
 * # This is a small example benchmark */

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"
#include "thread.h"
#include "tm.h"

void  fakeCallBegin(){
	asm volatile("": : : "memory");
};

void fakeCallEnd(){
	asm volatile("": : : "memory");
};

enum param_types {
    PARAM_THREAD  = (unsigned char)'t',
};

#define PARAM_DEFAULT_THREAD  (1L)

long global_params[256];

//Artificial example
//Global variables that should be captured
int G1, G2;
typedef struct
{
	int num;
	int* list;
}global_distance;
global_distance d;

static void
displayUsage (const char* appName)
{
    printf("Usage: %s [options]\n", appName);
    printf("    t <UINT>   Number of [t]hreads      (%li)\n", PARAM_DEFAULT_THREAD);
    puts("");
    exit(1);
}

static void
setDefaultParams( void )
{
    global_params[PARAM_THREAD]  = PARAM_DEFAULT_THREAD;
}

static void
parseArgs (long argc, char* const argv[])
{
    long i;
    long opt;

    opterr = 0;

    setDefaultParams();

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
            case 't':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case '?':
            default:
                opterr++;
                break;
        }
    }

    for (i = optind; i < argc; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if (opterr) {
        displayUsage(argv[0]);
    }
}

int __attribute__((noinline)) do_smth_with_ptr(int *ptr1)
{
	int *ptr2 = &G2;
	if (*ptr1 > *ptr2)
	{
		printf("More\n");
		return *ptr2;
	}
	else return (*ptr1+ *ptr2);
}

void __attribute__((noinline)) 
increment_number(void* ptr)
//increment_number(void* argptr,int* ptr)
{
  int *ptr1 = &G1;
  int value;
  global_distance *d_ptr;
  /*long * number = (long *) argptr;
  for (long i=0; i<500000; i++) {
    TM_BEGIN(0);
    (*number)++;
    TM_END(0);
  }*/
  int * ptr_temp = (int*) ptr;
 for (int j = 0; j < 10; ++j)
 {
  TM_BEGIN(0);
	d_ptr = &d;
	*ptr_temp += (*d_ptr).list[(*d_ptr).num - 2]; 
  	value = do_smth_with_ptr(&G1);
	#pragma clang loop vectorize_width(1337)
	for (int i = 0; i < value; i++)
	{
		*ptr1 += (*ptr_temp)++;	
	}
  TM_END(0);
  }
}


MAIN (argc,argv)
{
  TIMER_T start;
  TIMER_T stop;
  GOTO_REAL();

  /* Initialization */
  parseArgs(argc, (char** const)argv);
  SIM_GET_NUM_CPU(global_params[PARAM_THREAD]);
  long numThread = global_params[PARAM_THREAD];

  TM_STARTUP(numThread);
  P_MEMORY_STARTUP(numThread);
  thread_startup(numThread);


  long number = 0;

//Artificial example
//Initializing of global variables that should be captured
G1 = 1;
G2 = 2;
d.num = 4;
d.list = malloc(sizeof(int)*d.num);
for(int i = 0; i < d.num; ++i)
{
	d.list[i] = i*G1;
}
int *ptr = &G2;
int *ptr_again = &G1;
  
  TIMER_READ(start);


  //thread_start(increment_number, (void*) &number);
  thread_start(increment_number,  (void *) ptr);

  TIMER_READ(stop);

  printf("Number %li\n", number);
  printf("Artificial computation %d\n", *ptr);
  printf("Artificial computation %d\n", *ptr_again);
  printf("Time = %lf\n", TIMER_DIFF_SECONDS(start, stop));
  fflush(stdout);
  free(d.list);

  TM_SHUTDOWN();
  P_MEMORY_SHUTDOWN();

  GOTO_SIM();

  thread_shutdown();

  MAIN_RETURN(0);
}
