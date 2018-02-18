#define Read_Only 0
#define Read_Write 1
#define Invalid 2

extern char myip[20];

extern int mynode;




void psu_init_lock_mgr(char** nodes, int num_nodes); 

void psu_acquire_lock(int lock_number);

void psu_release_lock(int lock_number);

void initializeDSM();

typedef void* psu_dsm_ptr_t; 

psu_dsm_ptr_t psu_dsm_malloc(char* name, size_t size); 

void psu_dsm_free(char* name); 



typedef struct{
	void* shm_base;
	int size;
	char name[20];
	int shm_fd;
} Page_Table;

extern Page_Table page_table[100];
extern int page_num;



