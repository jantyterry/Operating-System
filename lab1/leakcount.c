#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *copy_file(char *new_location, const char *old_location);

int main(int argc, char *argv[]){

if(argc < 1){/* handle input error */
	printf("ERRPR: Not enough input argument, failed\n");
	exit(1);
}

FILE *fpt;
int length = 0;

//Getting the length of the input arguments 
for(int i = 1; i < argc; i++){
	length = length + strlen(argv[i]);
}

/*
Using LD_PRELOAD to modify the order of library loading to request
my specified library be loaded prior to other libraries
*/
const char *mylib = "LD_PRELOAD=./memory_shim.so";
length = length + strlen(mylib);

char Buffer[length];
//write preload to buffer
copy_file(Buffer, mylib);

//add a tab space between preload and the command 
//by user
int k = 1;
while(k < argc){
	strcat(Buffer,"\t");
	strcat(Buffer, argv[k]);
	k++;
}

//reading the buffer
fpt = popen(Buffer, "r");
if (fpt == NULL)
  {
  printf("Unable to open for reading");
  exit(0);
  }

pclose(fpt);

return 0;
}

char *copy_file(char *new_location, const char *old_location){
	unsigned i;
	for(i=0; old_location[i] != '\0'; ++i){
		new_location[i] = old_location[i];
	}
	new_location[i] = '\0';
	return new_location;
}


