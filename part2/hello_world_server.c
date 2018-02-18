#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "service.h"
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

//some help function
int get_entry(char* name);

//hard code. You need to tell the server nodes information of the DSM system.
int num_nodes;
char* nodes[100];

int mynode = 1;
int client_pid;

Directory direct[100];
int entry_num;


bool_t
init_1_svc(InitInp* argp, void *result, struct svc_req *rqstp)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//please change the num_nodes value to be the total node number in our DSM system.
	//please change the nodes[i] to be the IP Address of node i.
	num_nodes = 1;
	for(int i=0;i<num_nodes;i++){
		nodes[i] = (char*)malloc(sizeof(char)*20);
	}
	strcpy(nodes[0],"104.39.153.3");
	//strcpy(nodes[1],"130.203.16.21");
	mynode = argp->mynode;
	client_pid = argp->mypid;
	//init directory Table
	for(int i=0;i<100;i++){
		direct[i].num_P = num_nodes;
	}
	entry_num = 0;	


	

	return true;
}

bool_t
malloc_1_svc(MallocInp* argp, int *result, struct svc_req *rqstp){
	
	//send request to other nodes to make sure if they have created the page.
	//if so, do not need to real malloc.
	*result = 0;
	for(int i=0;i<num_nodes;i++){
		if(i != mynode){
			CLIENT *c1;
			c1 = clnt_create(nodes[i], DSM, SERVICE_V1, "tcp");
			if(c1 == NULL){
    				printf("pcreate error1\n");
   				exit(1);
 			}
			RequestRet message;
			enum clnt_stat retval_1;
			retval_1 = request_1(&(argp->name),&message,c1);
			if (retval_1 != RPC_SUCCESS) { 
				clnt_perror (c1, "call failed");
			}
			if(message.size != 0){
				*result = message.size;
			}
			direct[entry_num].P[i] = message.valid;
			
		}
	}



	strcpy(direct[entry_num].name,argp->name);
	if(*result == 0){
		direct[entry_num].P[mynode] = 1;
		direct[entry_num].state = Read_Write;
		direct[entry_num].size = argp->size;
	}
	else{
		direct[entry_num].P[mynode] = 0;
		direct[entry_num].state = Invalid;
		direct[entry_num].size = *result;
	}
	


	//create shared memory with client
	direct[entry_num].shm_fd = shm_open(direct[entry_num].name,O_CREAT | O_RDWR, 0666);
		

	ftruncate(direct[entry_num].shm_fd,direct[entry_num].size);

	direct[entry_num].shm_base = mmap(0, direct[entry_num].size, PROT_READ | PROT_WRITE, MAP_SHARED, direct[entry_num].shm_fd, 0);
	if(direct[entry_num].shm_base == MAP_FAILED){
		printf("Map failed\n");
		return false;
	}

	void* ptr = direct[entry_num].shm_base;

	entry_num++;
	
	return true;
}


bool_t
free_1_svc(char** argp, int *result, struct svc_req *rqstp){

	int entry = get_entry(*argp);
	
	if(munmap(direct[entry].shm_base, direct[entry].size) == -1){
		printf("Unmap failed : %s \n", strerror(errno));
		return false;
	}

	if(close(direct[entry].shm_fd) == -1){
		printf("Close failed : %s \n", strerror(errno));
		return false;
	} 

	entry_num --;

	//swap 
	direct[entry].state = direct[entry_num].state;
	direct[entry].num_P = direct[entry_num].num_P;
	direct[entry].size = direct[entry_num].size;
	direct[entry].shm_base = direct[entry_num].shm_base;
	direct[entry].shm_fd = direct[entry_num].shm_fd;
	strcpy(direct[entry].name, direct[entry_num].name);
	for(int i=0; i<num_nodes;i++){
		direct[entry].P[i] = direct[entry_num].P[i];
	}

	*result = entry;
	
	return true;
}


bool_t
request_1_svc(char **argp, RequestRet *result, struct svc_req *rqstp){
	//to see if I have the name space.	
	result->size = 0;
	result->valid = 0;
	for(int i=0;i<entry_num;i++){
		if(strcmp(direct[i].name,*argp) == 0){
			result->size = direct[i].size;
			result->valid = direct[i].P[mynode];
			break;
		}
	}

	return true;
}

bool_t
remoterdwr_1_svc(RemoteInp* argp, int *result, struct svc_req *rqstp){
	int entry = get_entry(argp->name);
	if(argp->IsRead == 1){
		//deal with Read on PageFault. The server now must be invalid on that page.
		if(direct[entry].state != Invalid){
			printf("The state is not consistent with what we think.\n");
		}
		
		for(int i=0;i<num_nodes;i++){
			if(direct[entry].P[i] == 1 && i != mynode){
				//NetWork Read and get the copy
				CLIENT *c1;
				c1 = clnt_create(nodes[i], DSM, SERVICE_V1, "tcp");
				if(c1 == NULL){
    					printf("pcreate error1\n");
   					exit(1);
 				}
				NetworkInp inp;
				inp.name = (char*)malloc(sizeof(char)*20);
				strcpy(inp.name,direct[entry].name);
				inp.NetOp = N_Read;
				inp.mynode = mynode;
				char* buffer = (char*)malloc(sizeof(char)*direct[entry].size);
				enum clnt_stat retval_1;
				retval_1 = networkop_1(&inp,&buffer,c1);
				if (retval_1 != RPC_SUCCESS) { 
					clnt_perror (c1, "call failed");
				}

				void* start = direct[entry].shm_base;
				memcpy(start, buffer, direct[entry].size);
			}
		}

		//set my state to be read-only
		direct[entry].P[mynode] = 1;
		direct[entry].state = Read_Only;

		//set return value to read-only
		*result = direct[entry].state;

		return true;
	}

	if(argp->IsRead == 0){
		//deal with Write on PageFault. The server now can be read-only or invalid
		if(direct[entry].state == Read_Only){
			for(int i=0;i<num_nodes;i++){
				if(direct[entry].P[i] == 1 && i != mynode){
					//NetWork Invalid. Don't need to get the copy
					CLIENT *c1;
					c1 = clnt_create(nodes[i], DSM, SERVICE_V1, "tcp");
					if(c1 == NULL){
    						printf("pcreate error1\n");
   						exit(1);
 					}
					NetworkInp inp;
					inp.name = (char*)malloc(sizeof(char)*20);
					strcpy(inp.name,direct[entry].name);
					inp.NetOp = N_Inv;
					inp.mynode = mynode;
					char* buffer = (char*)malloc(sizeof(char)*direct[entry].size);
					enum clnt_stat retval_1;
					retval_1 = networkop_1(&inp,&buffer,c1);
					if (retval_1 != RPC_SUCCESS) { 
						clnt_perror (c1, "call failed");
					}

				}
			}
		}
		else{ 
			for(int i=0;i<num_nodes;i++){
				if(direct[entry].P[i] == 1 && i != mynode){
					//NetWork RdEx. Need to get the copy
					CLIENT *c1;
					c1 = clnt_create(nodes[i], DSM, SERVICE_V1, "tcp");
					if(c1 == NULL){
    						printf("pcreate error1\n");
   						exit(1);
 					}
					NetworkInp inp;
					inp.name = (char*)malloc(sizeof(char)*20);
					strcpy(inp.name,direct[entry].name);
					inp.NetOp = N_RdEx;
					inp.mynode = mynode;
					char* buffer = (char*)malloc(sizeof(char)*direct[entry].size);
					enum clnt_stat retval_1;
					retval_1 = networkop_1(&inp,&buffer,c1);
					if (retval_1 != RPC_SUCCESS) { 
						clnt_perror (c1, "call failed");
					}

					void* start = direct[entry].shm_base;
					memcpy(start, buffer, direct[entry].size);
				}
			}
		}
		
		//set my state to be read-write
		for(int i=0;i<num_nodes;i++){
			direct[entry].P[i] = 0;
		}
		direct[entry].P[mynode] = 1;
		direct[entry].state = Read_Write;
		
		//set return value to read-write;
		*result = direct[entry].state;
	}
		
	
	return true;

}

bool_t
networkop_1_svc(NetworkInp* argp, char **buffer, struct svc_req *rqstp){
	int entry = get_entry(argp->name);
	
	if(argp->NetOp == N_Read){
		//Receive Net-Read, my state can be read-only or read-write. And i need change to read-only if my state is read-write
		if(direct[entry].state == Read_Only){
			direct[entry].P[argp->mynode] = 1;
			memcpy(*buffer, direct[entry].name, direct[entry].size);
			
			return true;
		}
		else{
			direct[entry].P[argp->mynode] = 1;
			direct[entry].state = Read_Only;
			memcpy(*buffer, direct[entry].name, direct[entry].size);

			//tell the local client to change the memory state to be read-only
			sigval_t value;
			value.sival_int = 1 + 10 * entry;
			sigqueue(client_pid, SIGUSR1, value); 

		}
	}

	else if(argp->NetOp == N_Inv){
		//Receive Net-Inv, my state can be read-only or read-write. And I need to change to invalid.
		for(int i=0;i<num_nodes;i++){
			direct[entry].P[i] = 0;
		}		
		direct[entry].P[argp->mynode] = 1;
		direct[entry].state = Invalid;
	
		//tell the local client to change the memory state to be invalid;
		sigval_t value;
		value.sival_int = 4 + 10 * entry;
		sigqueue(client_pid, SIGUSR1, value); 
	}

	else{
		//Receive N-RdEx, my state can be read-only or read-write. And I need to change to invalid. Don't forget to send data copy to Network.
		for(int i=0;i<num_nodes;i++){
			direct[entry].P[i] = 0;
		}
		direct[entry].P[argp->mynode] = 1;
		direct[entry].state = Invalid;		
		
		//tell the local client to change the memory state to be invalid;
		sigval_t value;
		value.sival_int = 4 + 10 * entry;
		sigqueue(client_pid, SIGUSR1, value); 

		//send data copy to Network.
		memcpy(*buffer, direct[entry].name, direct[entry].size);
	}

	return true;
}

int
dsm_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}

//help function
int get_entry(char* name){
	int i = 0;
	for(i=0;i<entry_num;i++){
		if(strcmp(direct[i].name,name) == 0){
			break;
		}
	}
	return i;
}







