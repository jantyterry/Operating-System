//Jamar Robinson Christopher Cornwall

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "fatfile.h"


unsigned long getsector_num(unsigned short FAT_entry)
{
	unsigned long sector;
	sector = (33 + FAT_entry - 2) *  SEC_SIZE;
	return sector;
}

void myMemCpy(void *dest, void *src, size_t n)
{
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
 int i;
   // Copy contents of src[] to dest[]
   for (i = 0; i<n; i++)
       cdest[i] = csrc[i];
}

void *memoryset(void *dst, int c, size_t n)
{
	if(n) {
		char *d = dst;
		do {
			*d++ = c;
		} while(--n);
	}
	return dst;
}




unsigned short search_second_cluster(unsigned short cluster, int deleted) {
unsigned long offset;
secondclust = 0;
segment = 0;	
if (cluster < 0x002 || cluster >= 0xFF0)
		return FIRSTCLUSTER;
 
	if (deleted)
	 {
		cluster++;
	}

	offset = BOOTSECTOR + (cluster/2)*3;
	
	myMemCpy(&segment, &file[offset], 3);
	

	if ((cluster & 1) != 0) 
	{
		
		secondclust = ((0xFFF000 & segment) >> 12);
	} 
	else
	{
		secondclust = (0x000FFF & segment);		
	}

	if (deleted && secondclust != FIRSTCLUSTER)
		return FIRSTCLUSTER;

	return secondclust;
}





void init_outfile (unsigned long offset)
{
	memoryset(filename, 0, sizeof filename);
	memoryset(ext, 0, sizeof ext);
	myMemCpy(&size, &file[offset+FILESIZE], 4);	
	myMemCpy(&cluster, &file[offset+LOGICALCLUSTER], 2);
}

//Traverse filename and check on deleted files
int traversefileext (unsigned long offset, int cursor, int deleted, int extcursor)
	{

		for ( i = 0; i < FILENAMELENGTH; i++) {
			if (file[offset+i] == '\xE5') 
			{
				deleted = 1;
				filename[cursor++] = '_';
			}
			 else if (file[offset+i] != '\x20') 
			{
				filename[cursor++] = file[offset+i];
			}
		}

	
		//Taking ext information
		for ( i = FILENAMELENGTH; i < 12; i++) {
			if (file[offset+i] != '\x20') {
				ext[extcursor++] = file[offset+i];
			}
		}

return deleted;
	}	
	



void searchroot(unsigned long cluster_entry, char *direct1) 
{
	unsigned long offset;

	for (offset = cluster_entry; offset <= ( cluster_entry + SEC_SIZE); offset += 32) 
	{
		
		if (file[offset] == 0)
			return;
		deleted = 0;
		init_outfile(offset);
		
		cursor = 0;
		extcursor = 0;
		directsize = strlen(direct1)+14;
	
		deleted = traversefileext ( offset, cursor, deleted, extcursor);
	

		if ((file[offset+11] >> 4) & 1)
		 {
			
			if (file[offset] != '.') 
			{
				char outputdir[directsize];
				strcpy(outputdir, direct1);
				strcat(outputdir, filename);
				strcat(outputdir, "/");
				searchroot(getsector_num(cluster), outputdir);
			}
		} 

		else 
		{
			char filename_out[11];
			sprintf(filename_out, "file%d.%s", filenum++, ext);
		

			outputfile = open(filename_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
			if(outputfile == -1) 
			{
				fprintf(stderr, "Output file not created%s\n", filename_out);
				perror("");
				exit(1);
			}

			printf("FILE\t");

			if (deleted)
				printf("DELETED\t%s%s.%s\t%lu\n", direct1, filename, ext, size);
			else
				printf("NORMAL\t%s%s.%s\t%lu\n", direct1, filename, ext, size);


			while (cluster >= 0x002) 
			{
				 outputsize= size;
				if (size >= SEC_SIZE)
				outputsize= SEC_SIZE;
				else
				outputsize = size;

		
				write(outputfile, &file[getsector_num(cluster)],outputsize);
				size =  size - outputsize;
				cluster = search_second_cluster(cluster, deleted);
			}
			close(outputfile);
		}
	}
}


int main(int argc, char *argv[])
{
	if(argc != 3) 
	{
		fprintf(stderr, "Usage : ./executable <image filename> <directory>\n");
		exit(1);
	}
	

	inputfile = open(argv[1], O_RDONLY); 
	if(inputfile == -1) 
			{
				fprintf(stderr, "Could not read input file %s\n", argv[1]);
				perror("");
				exit(1);
			}

	file = mmap(0,DISKSIZE, PROT_READ, MAP_PRIVATE | MAP_FILE, inputfile, 0);

	if (chdir(argv[2]) != 0)
	{
		printf("Directory can not be changed\n");
		return 1;
	}

	// go through root
	for ( i = 0; i < 14; i++)
	{
		searchroot(0x2600 + SEC_SIZE*i, "/");
	}
return 0;
}
