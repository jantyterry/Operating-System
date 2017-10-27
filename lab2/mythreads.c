#include <stdio.h>
#include <stdlib.h>

#ifndef _mythreads
#include "mythreads.h"
#define _mythreads
#endif

#ifndef _listingh
#include "listing.h"
#define _listingh
#endif

#include <sys/time.h>
#include <string.h>
#include "assert.h"

int count_t = 1;
int interruptsAreDisabled = 0;

static void interruptDisable(){
interruptsAreDisabled = 1;
}

static void interruptEnable(){
interruptsAreDisabled = 0;
}

/* Functions for Thread Management */

//Applications will call threadInit to initialize the library
extern void threadInit() {
  // Adding main thread structure 
  threads *main_thr = malloc(sizeof(threads));
  getcontext(&main_thr->context);
  main_thr->threadID = 1;
  main_thr->finish = 0;

  List *mnode = malloc(sizeof(List));
  mnode->thr = main_thr;
  mnode->next = mnode;

  // this is where the list is initialize
  head = mnode;
  curr = mnode;
}


// handling user thread
void running_thr(threads *thr) {
  interruptEnable();
  thr->result = thr->func(thr->argument);
  thr->finish = 1;
  interruptDisable();
}


/*Applications will call threadCreate to create new
threads. When an application calls this function, a new thread will
be created */
extern int threadCreate(thFuncPtr funcPtr, void *argPtr) {
  interruptDisable();

  // Creating user thread function
  threads *thr = (threads *) malloc(sizeof(threads));
  getcontext(&thr->context);
  // allocate and initialize a new call stack
  thr->context.uc_stack.ss_sp = malloc(STACK_SIZE);
  thr->context.uc_stack.ss_size = STACK_SIZE;
  thr->context.uc_stack.ss_flags = 0;
  thr->context.uc_link = &head->thr->context;

  //modify the context to handle arguments
  thr->func = funcPtr;
  thr->argument = argPtr;
  thr->finish = 0;
  thr->threadID = ++count_t;
  makecontext(&thr->context, (void (*)(void)) running_thr, 1, thr);

  List *Addnode = (List *) malloc(sizeof(List));
  Addnode->thr = thr;
  // Add node to the list structure
  enqueue(Addnode);
  
  swapping_thr();

  interruptEnable();
  return Addnode->thr->threadID;
}

// continously swaping thread
void swapping_thr() {
  List *oldnode = curr;
  // move onto the next thread
  do {
    curr = curr->next;
  } while (curr->thr->finish);

  // swap contexts with the next thread
  if (oldnode != curr) {
    swapcontext(&oldnode->thr->context, &curr->thr->context);
  } else {
  }
}

/* Calls to threadYield cause the currently running thread
to “yield” the processor to the next runnable thread */
extern void threadYield() {
  interruptDisable();
  swapping_thr(); 
  interruptEnable();
}


//This function waits until the thread corresponding to id exits
extern void threadJoin(int thread_id, void **result) {
  interruptDisable();

  // checking to find the location of the thread
  List *temp = head;
  while (temp->thr->threadID != thread_id) {
    temp = temp->next;
  }

  // swapping threads until they are finish
  while (!temp->thr->finish) {
    swapping_thr();
  }

  // update result to the threads
  if (result)
    *result = temp->thr->result;

  interruptEnable();
}


//exits the current thread
extern void threadExit(void *result) {
  interruptDisable();
  // updating
  curr->thr->result = result;
  // mark as complete
  curr->thr->finish = 1;
  // removing from list structure
  dequeue(curr);
  
  swapping_thr();
  interruptEnable();
}

/* Functions for Synchronization */

// continuously search and grap locks
void grabbing_locks(int lockNum) {
  
  int foundlock = 0;
  while (!foundlock) {
    if (!locks_buff[lockNum]) {
      // lock found
      locks_buff[lockNum] = 1;
      foundlock = 1;
    } else {
      // continue swapping threads then grab when it is ok
      threadYield();
    }
  }
}


//This function blocks (waits) until it is able to acquire
//the specified lock
extern void threadLock(int lockNum) {
  interruptDisable();
  grabbing_locks(lockNum);
  interruptEnable();
}

//This function unlocks the specified lock
extern void threadUnlock(int lockNum) {
  // give up the use of the lock
  locks_buff[lockNum] = 0;
}

/*This function atomically unlocks the specified mutex
lock and causes the current thread to block (wait) until the specified
condition is signaled (by a call to threadSignal)
*/
extern void threadWait(int lockNum, int conditionNum) {
  interruptDisable();

  threadUnlock(lockNum);
  List *oldnode = curr;
  dequeue(curr);
  
  while (!conditions_buff[lockNum][conditionNum]) {
  	threadYield();
  }

  enqueue(oldnode);
  grabbing_locks(lockNum);

  interruptEnable();
}

// This function unblocks a single thread waiting on
// the specified condition variable
extern void threadSignal(int lockNum, int conditionNum) {
  interruptDisable();
  conditions_buff[lockNum][conditionNum]++;
  interruptEnable();
}


