#include <ucontext.h>
#ifndef _mythreads
#include "mythreads.h"
#define _mythreads
#endif

typedef struct threads {
int finish;
int threadID;
ucontext_t context;
void *argument;
void *result;
thFuncPtr func;
}threads;


typedef struct List {
threads *thr;
struct List *next;
}List;

List *head;
List *curr;


int conditions_buff[NUM_LOCKS][CONDITIONS_PER_LOCK];
int locks_buff[NUM_LOCKS];


void enqueue(List *node);
void dequeue(List *node);
void running_thr(threads *thr);
void swapping_thr();
void grabbing_locks(int lockNum);


