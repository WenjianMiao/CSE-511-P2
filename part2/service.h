#define Read_Only 0
#define Read_Write 1
#define Invalid 2

#define N_Read 3
#define N_Inv 4
#define N_RdEx 5

typedef struct{
	int state;
	int P[100];
	int num_P;
	char name[20];
	int size;
	void* shm_base;
	int shm_fd;
} Directory;
