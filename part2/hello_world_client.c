#define  _POSIX_C_SOURCE 200809L
#define  _GNU_SOURCE
#include <rpc/rpc.h>

#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#include <errno.h>
#include <ucontext.h> 

#include "global.h"

char myip[20];
int mynode;



Page_Table page_table[100];
int page_num = 0;

int main(int argc, char* argv[]){
	if(argc < 3){
		printf("Please input \"./client #node_number_of_this_client #IP_Address_of_this_client \" ");
		return 0;
	}

	mynode = strtol(argv[1],NULL,10); 
	strcpy(myip,argv[2]);
	


	initializeDSM();


	
	




	
	psu_dsm_ptr_t a = psu_dsm_malloc("a",10);
	((int*)a)[1] = 1;
	printf("%d \n",((int*)a)[1]);

	psu_dsm_free("a");



	
	

}


	
