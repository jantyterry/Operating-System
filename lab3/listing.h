#define PAGESIZE 4096
#define hexa 0xFFF
#define LIST_LIMIT 1024
void __attribute__ ((constructor)) init(void);
void __attribute__ ((destructor)) cleanup(void);

//holds data about small payloads
struct nodeHead{
	struct nodeHead * next;
};

//hold metadata about information on page
struct pageHead{
	int total;
	int size;
	struct pageHead * next;
	struct nodeHead * usedlist;
	struct nodeHead * freelist;
};
typedef struct pageHead pageH;
typedef struct nodeHead nodeH;

int nodeH_SIZE;
int pageH_SIZE;

int fd;
pageH *pageHead;

void *memoryset(void *dst, int c, size_t n);

