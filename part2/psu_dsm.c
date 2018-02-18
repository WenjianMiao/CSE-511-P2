#define  _POSIX_C_SOURCE 200809L
#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include "msg.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>

#include "global.h"


void handler_invalid(int signal, siginfo_t *info, ucontext_t* ctx);

void handler_server(int signal, siginfo_t *info, ucontext_t* ctx);

void initializeDSM(){
	struct sigaction act;
	act.sa_sigaction = handler_invalid;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaction(SIGSEGV,&act,NULL);

	struct sigaction act2;
	act2.sa_sigaction = handler_server;
	act2.sa_flags = SA_SIGINFO;
	sigemptyset(&act2.sa_mask);
	sigaction(SIGUSR1,&act2,NULL);

	CLIENT *c1;
	c1 = clnt_create(myip, DSM, SERVICE_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}
	InitInp inp;
	inp.mynode = mynode;
	inp.mypid = getpid();
	void* result;
	enum clnt_stat retval_1;
	retval_1 = init_1(&inp,result,c1);
	if (retval_1 != RPC_SUCCESS) { 
		clnt_perror (c1, "call failed");
	}
}



psu_dsm_ptr_t psu_dsm_malloc(char* name, size_t size){

	CLIENT *c1;
	c1 = clnt_create(myip, DSM, SERVICE_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}
	int result;
	MallocInp inp;
	inp.name = (char*)malloc(sizeof(char)*20);
	strcpy(inp.name,name);
	inp.size = (int)size; 
	enum clnt_stat retval_1;
	retval_1 = malloc_1(&inp,&result,c1);
	if (retval_1 != RPC_SUCCESS) { 
		clnt_perror (c1, "call failed");
	}
	
	//client share memory map
	int shm_size;
	if(result == 0){
		shm_size = (int)size;
	}
	else{
		shm_size = result;
	}


	page_table[page_num].shm_fd = shm_open(name, O_RDWR, 0666);

	if(page_table[page_num].shm_fd == -1){
		printf("Shared memory failed.\n");
		exit(1);
	}

	
	page_table[page_num].shm_base = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, page_table[page_num].shm_fd, 0);
	if(page_table[page_num].shm_base == MAP_FAILED){
		printf("Map failed: %s\n",strerror(errno));
		exit(1);
	}
	
	//if invalid, protect the memory
	if(result != 0){
		mprotect(page_table[page_num].shm_base, (size_t)shm_size , PROT_NONE);
	}


	//mprotect(page_table[page_num].shm_base, (size_t)shm_size , PROT_NONE); 



	strcpy(page_table[page_num].name,name);
	page_table[page_num].size = shm_size;


	page_num++;
	return (psu_dsm_ptr_t)page_table[page_num-1].shm_base;
	
}

void psu_dsm_free(char* name){
	CLIENT *c1;
	c1 = clnt_create(myip, DSM, SERVICE_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}

	int result;
	enum clnt_stat retval_1;
	retval_1 = free_1(&name,&result,c1);
	if (retval_1 != RPC_SUCCESS) { 
		clnt_perror (c1, "call failed");
	}

	
	if(munmap(page_table[result].shm_base, page_table[result].size) == -1){
		printf("Unmap failed : %s \n", strerror(errno));
		exit(1);
	}

	if(close(page_table[result].shm_fd) == -1){
		printf("Close failed : %s \n", strerror(errno));
		exit(1);
	} 

	if(shm_unlink(page_table[result].name) == -1){
		printf("Error removing %s : %s \n", page_table[result].name, strerror(errno));
		exit(1);
	}

	page_num--;

	//swap
	page_table[result].shm_base = page_table[page_num].shm_base;
	page_table[result].size = page_table[page_num].size;
	page_table[result].shm_fd = page_table[page_num].shm_fd;
	strcpy(page_table[result].name, page_table[page_num].name);


	


	

}



void handler_invalid(int signal, siginfo_t *info, ucontext_t* ctx){
	//first get the name of the pagefault place, and then get the reason(read/write).
	char name[20];
	int IsRead;
	int i;
	
	unsigned int addr1 = (unsigned int)info->si_addr; printf("get addr %u \n",addr1);
	for(i=0;i<page_num;i++){
		unsigned int addr2 = (unsigned int)page_table[i].shm_base;
		if(addr2 <= addr1 && addr1 < addr2 + (unsigned int)page_table[i].size){
			strcpy(name,page_table[i].name);
			break;
		}
	}

	printf("IP: %d\n", ctx->uc_mcontext.gregs[REG_RIP]);

	if (ctx->uc_mcontext.gregs[REG_ERR] & 0x2) {
		IsRead = 0;
   	} 
	else {
        	IsRead = 1;
   	}

	//printf("%s %d %u\n",name,IsRead,(unsigned int)page_table[i].shm_base);
	

	//send remote read/write request to local server
	CLIENT *c1;
	c1 = clnt_create(myip, DSM, SERVICE_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}
	int result;
	RemoteInp inp;
	inp.name = (char*)malloc(sizeof(char)*20);
	strcpy(inp.name,name);
	inp.IsRead = IsRead; 
	enum clnt_stat retval_1;
	retval_1 = remoterdwr_1(&inp,&result,c1);
	if (retval_1 != RPC_SUCCESS) { 
		clnt_perror (c1, "call failed");
	}
	

	
	//reset memory to be suitable state
	if(result == Read_Only){
		mprotect(page_table[i].shm_base, (size_t)page_table[i].size ,  PROT_READ);
	}
	else{
		mprotect(page_table[i].shm_base, (size_t)page_table[i].size , PROT_WRITE | PROT_READ);
	}

	//mprotect(page_table[i].shm_base, (size_t)page_table[i].size , PROT_WRITE | PROT_READ);
	
}





void handler_server(int signal, siginfo_t *info, ucontext_t* ctx){
	//server tell client if the client need to change the memory protect state
	//int_val value (the value server send to client. value = flag + 10 * entry).
	//flag (client adjust memory protect state based on the flag):  1 -> PROT_READ   2 -> PROT_WRITE  3 -> PROT_WRITE | PROT_READ  4 -> PROT_NONE
	//entry(the page, or the memory location that need to change the protect state);
	void *ptr_val = info->si_value.sival_ptr;
      	int int_val = info->si_value.sival_int;

	int flag, entry;
	entry = int_val / 10;
	flag = int_val % 10 ;

	if(flag == 1){
		mprotect(page_table[entry].shm_base, (size_t)page_table[entry].size , PROT_READ);
	}
	else if(flag == 2){
		mprotect(page_table[entry].shm_base, (size_t)page_table[entry].size , PROT_WRITE);
	}
	else if(flag == 3){
		mprotect(page_table[entry].shm_base, (size_t)page_table[entry].size , PROT_WRITE | PROT_READ);
	}
	else{
		mprotect(page_table[entry].shm_base, (size_t)page_table[entry].size , PROT_NONE);
	}
	
}




