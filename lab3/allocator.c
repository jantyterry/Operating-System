#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include "listing.h"


//manipulating the page from os
void greatsize(pageH * page, nodeH * node){
		page->freelist = page->freelist->next;
		node->next = page->usedlist;
		page->usedlist = node;
}


//When size is too large, request page of virtual memory
void * addPage(size_t nodesize, size_t pagesize){

	int updatep;
	updatep = 1;
	updatep = updatep + (int)(pagesize/PAGESIZE);

	pagesize = updatep;
	pagesize *= PAGESIZE;

	// ask the OS to map a page of virtual memory
	// initialized to zero
	void * page = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	//Allocation Failure
	if(page == MAP_FAILED){
		return NULL;
	}
	
	//initiallizing page header
	pageH *head = page;
	head->size = nodesize;
	head->total = pagesize;
	head->next = NULL;
	head->usedlist = NULL;
	head->freelist = NULL;
	
	//checking page size, then adjust it
	//point it to the head
	//return the page

	int idx = sizeof(pageH);
	nodeH * tmp = NULL;
	if(pagesize>PAGESIZE){
		tmp = (nodeH*)(((char*)page)+idx);
		tmp->next = NULL;
		head->freelist = tmp;
	}
	else{
		if(nodesize<=LIST_LIMIT){
			while(idx<(PAGESIZE-(nodeH_SIZE+nodesize))){
				tmp = (nodeH*)(((char*)page)+idx);
				tmp->next = head->freelist;
				head->freelist = tmp;
				idx+= (nodeH_SIZE+nodesize);
			}
		}
		else{
				tmp = (nodeH*)(((char*)page)+idx);
				tmp->next = NULL;
				head->freelist = tmp;
		}
	}
		head->next = pageHead;
		pageHead = head;
	return page;	
}


/*
 malloc - returns a pointer to an allocated block  
 of size bytes.  It first searches through different
 size free lists for a free block
 */

void * malloc(size_t size){
	
	if(size == 0){
		return NULL;
	}

	pageH * page;
	nodeH * node;
	
	//searches through different size lists 
	if(size<=8){
		size = 8;
	}
	else if(size<=16){
		size = 16;
	}
	else if(size<=32){
		size = 32;
	}
	else if(size<=64){
		size = 64;
	}
	else if(size<=128){
		size = 128;
	}
	else if(size<=256){
		size = 256;
	}
	else if(size<=512){
		size = 512;
	}
	else if(size<=1024){
		size = 1024;
	}
	else if(size > 1024){
		//request page
		page = addPage(size, size+sizeof(pageH)+nodeH_SIZE);

		//Check if the page is of NULL
		if(page == NULL){
			return NULL;
		}
		node = page->freelist;
		greatsize(page, node);
		
		return ((char*)page->usedlist+nodeH_SIZE);
	}

	pageH * tmp = pageHead;
	while(tmp!=NULL && (tmp->size!=size || tmp->freelist==NULL)){
		tmp = tmp->next;
	}
	if(tmp == NULL){
		//request page
		page = addPage(size, size+sizeof(pageH)+nodeH_SIZE);
	}
	else page = tmp;
	if(page==NULL){
		return NULL;
	}
	node = page->freelist;
	greatsize(page, node);
	
	return (void*)((char *)page->usedlist+nodeH_SIZE);
}


/* free - It frees the block pointed to by ptr and returns 
   nothing. Then, it adds freed block to apprpriate size
   free list.
 */

void free(void *ptr){
	if(ptr == (void *)0||ptr==NULL) {
		return;
	}
	ptr = ((char*)ptr) - nodeH_SIZE;

	nodeH * node = ptr;
	
	//frees the block pointed to by ptr 
	
	pageH * page = (void *)((uintptr_t) node & ~(uintptr_t)hexa);
	nodeH * tmp = page->usedlist;
	if(tmp!=node){
		while(tmp->next!= NULL && tmp->next!=node){
			tmp = tmp->next;
		}
		tmp->next = node->next;
		node->next = page->freelist;
		page->freelist = node;
	}
	else{
		page->usedlist = node->next;
		node->next = page->freelist;
		page->freelist = node;
	}

	/* Add free block to appropriate list */	

	if(page->usedlist == NULL){
		if(pageHead == page){
			pageHead = page->next;
		}
		else {	
			pageH * ph = pageHead;
			while(ph->next!= page){
				ph = ph->next;
			}
			ph->next = ph->next->next;
		}
		// unmap the page
		munmap(page, page->total);
	}


}

/* realloc - Returns a pointer to an allocated region
   of size bytes with the contents of pointer
   ptr up to the size bytes. The action is to
   simply free current allocated block and copy size bytes
   at ptr to a new free block and free ptr.
*/


void* realloc(void* ptr, size_t size){

//Returns a pointer to an allocated region
//of size bytes with the contents of pointer

	if(ptr == NULL){
		return malloc(size);
	}
	else{
		pageH * page = (void *)((uintptr_t) ptr & ~(uintptr_t)hexa);
		if(page->size == size){
			return ptr;
		}
	// copy size block
		int grabsize;
		if(page->size < size){
			grabsize = page->size;
		}
		else{
			grabsize = size;
		}
	// copy size bytes at ptr to a new free block and free ptr.

		void * ph = malloc(size);
		
		memcpy(ph, ptr, grabsize);
		free(ptr);
		return ph;
	}
}

//memory setting, copies the character c 
//to the first n characters point to dst
void *memoryset(void *dst, int c, size_t n)
{
    if (n) {
        char *d = dst;

         do {
             *d++ = c;
         } while (--n);
     }
     return dst;
}


/*
calloc - Allocates memory for an array of number elements 
of size bytes each and returns a pointer to the allocated 
memory. The memory is set to zero before returning.
*/

void* calloc(size_t number, size_t size){

	//returns a pointer to the allocated memory
	void * ph = malloc(number*size);
	
	if(ph != NULL) {
		memoryset(ph, 0, number*size);
	}
return ph;
}

//init - iniallizing the memory allocator library
//first function that is called	
void init(void){
	// used for initializing the memory to zero 
	fd = open("/dev/zero", O_RDWR);

	nodeH_SIZE = sizeof(nodeH);
	pageH_SIZE = sizeof(pageH);
	pageHead = NULL;	
}	




