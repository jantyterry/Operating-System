#include <stdio.h>
#include <stdlib.h>

#ifndef _listingh
#include "listing.h"
#define _listh
#endif

void enqueue(List *node){
	List *temp = curr->next;
	curr->next = node;
	node->next = temp;
}

void dequeue(List *node){
	List *temp2 = head;
	while(temp2->next != node){
		temp2 = temp2->next;
	}
	temp2 = temp2->next;
}



