#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <string.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define syscall_num 328
#define MAXREAD 2048

char *copy_file(char *new_location, const char *old_location);

char arguments[MAXREAD];
int system_call[syscall_num];
FILE *fpt;


int main(int argc, char **argv) {

if(argc < 3){/* handle input error */
	printf("ERROR: Not enough input argument, failed\n");
	exit(1);
}

//Handle output file
int n = argc - 1; 

pid_t child;
child = fork(); /* Fork a child process */

if(child < 0){
	printf("ERRPR: forking child process failed\n");
	exit(1);
}

if (child == 0) {
/*doing all childish behaviour */

	//copy user program to arguments
	copy_file(arguments, argv[1]);
	
	//adding space between every arguments from user handle
	int i = 2;
	while(i < n){
		strcat(arguments, " ");
		strcat(arguments, argv[i]);
		i++;
	}

	//array to store splitted arguments
	char *commands[MAXREAD];
	
	//get the first space in argument array
	char *stoken = strtok(arguments, " ");

	//pointer to commands
	char **pointer = commands;

	//walk through other spaces in argument
	while(stoken) {
		//writing the split aguments to commands array
		*(pointer++) = stoken;

		//terminate
		stoken = strtok(NULL, " ");
	}

	ptrace(PTRACE_TRACEME);
	child = getpid();

	//stop myself --allow the parent to get ready to trace me
	kill(child, SIGSTOP);
	
	//do something that will result in a system call
	execvp(commands[0], commands);
}

else{
	int status;
	int syscall_count;
	//I'm the parent...keep tabs on that child process
	
	//Now wait for the child to stop itself
	waitpid(child, &status, 0);
	
	//this option makes it easier to distinguish normal traps from
        //system calls
	ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

	while(1){
	
	
	do{
            //Request: I want to wait for a system call
            ptrace(PTRACE_SYSCALL, child, 0, 0);

            //actually wait for child status to change
            waitpid(child, &status, 0);

            //there are differented reasons that a child's
            //status might change. Check to see if the child
            //exited
            if (WIFEXITED(status)) {
                fpt = fopen(argv[n], "w");
		// Writing out the system calls to a file 
		int i = 0;
		while(i < syscall_num){
			if(system_call[i]){
				fprintf(fpt, "%d \t %d \n", i, system_call[i]);
			}
			i++;
		}
		
		fclose(fpt);
		return 1;
            }
	}

	while (!(WIFSTOPPED(status) && WSTOPSIG(status) & 0x80));

        //read out the saved value of the RAX register,
        //which contains the system call number
        syscall_count = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX, NULL);
        system_call[syscall_count]++;
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        waitpid(child, &status, 0);

	}
    }
}

char *copy_file(char *new_location, const char *old_location){
	unsigned i;
	for(i=0; old_location[i] != '\0'; ++i){
		new_location[i] = old_location[i];
	}
	new_location[i] = '\0';
	return new_location;
}

