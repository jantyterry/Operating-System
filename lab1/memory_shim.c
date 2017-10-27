#define _GNU_SOURCE

void __attribute__ ((constructor)) init(void);
void __attribute__ ((destructor)) cleanup(void);

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

//implementing structure link-list to store different information
typedef struct List {
	int size;
	void *block_value;
	struct List *next;
}List;

//Setting pointer to struct to NULL value;
struct List *headptr = NULL;


// implement these functions
void free ( void * ptr );
void * malloc ( size_t size );


// setting up original versions
void (* original_free) (void * ptr) = NULL;
void *(* original_malloc ) ( size_t size ) = NULL;


//Initializing the library
void init(void){
/*
Call the original versions
Using dlsym with RTDL_NEXT pseudo handle grabs the next library in the specified load order
*/
	if(original_malloc == NULL){
		original_malloc = dlsym ( RTLD_NEXT, "malloc");
	}

	if(original_free == NULL){
		original_free = dlsym (RTLD_NEXT, "free");
	}

}


void *malloc(size_t size){
	struct List *branch; 
	void *mem = original_malloc(size);

	//Accessing the malloc through struct pointer	
	branch = (struct List*) original_malloc(sizeof(struct List));
	branch->size = size;
	branch->block_value = mem;
	branch->next = headptr;
	headptr = branch;

return mem;
}

/*The free function delete memory that is no longer in use, hence delete the information */
void free(void *ptr){
	
	struct List *lp = NULL;
	struct List *prevlp = NULL;
	original_free(ptr);

	lp = headptr;
	while(lp != NULL){
		if(lp->block_value == ptr){
			if(prevlp == NULL){
				headptr = lp->next;
			}
			else{
				prevlp->next = lp->next;
			}
			
			original_free(lp);
			return;
		}
		prevlp = lp;
		lp = lp->next;
		
	}
return;
}

// Called when the library is unloaded
void cleanup(void){
	struct List *lp = NULL;
	int l_size = 0;
	int count = 0;

	lp = headptr;
	while(lp != NULL){
	count = count + 1;
	l_size = l_size + lp->size;
	fprintf(stderr, "LEAK \t %d \n", lp->size);
	lp = lp->next;
	}

	fprintf(stderr, "TOTAL \t %d \t %d \n", count, l_size);
}

