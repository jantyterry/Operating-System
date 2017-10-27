
#define DISKSIZE 1474560
#define FIRSTCLUSTER 0x000
#define SEC_SIZE 512

#define LOGICALCLUSTER 26
#define FILESIZE 28
#define FILENAMELENGTH 8
#define FILENAMEEXT 3
#define BOOTSECTOR 0x200


int i;
int inputfile;
int outputfile;
char *file;
int directsize;
int filenum = 0;
char filename[9];   
char ext[4];
unsigned long size = 0;
unsigned short cluster;
int deleted;
unsigned short secondclust;
int cursor;
int extcursor;
unsigned long outputsize;
unsigned long segment;
void myMemCpy(void *dest, void *src, size_t n);
void *memoryset(void *dst, int c, size_t n);
