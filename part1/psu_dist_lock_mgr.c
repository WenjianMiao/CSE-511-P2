#include <rpc/rpc.h>
#include "msg.h"
#include "global.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void* remote_request(void* args){
	input* inpp = (input *)args;
	CLIENT *c1;
	c1 = clnt_create(global_nodes[inpp->you], DISLOCK, LOCK_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}
	output outp;
	enum clnt_stat retval_1;
	retval_1 = request_1(inpp, &outp, c1);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (c1, "call failed");
	}
	
	return;
}

void psu_init_lock_mgr(char** nodes, int num_nodes){

	global_nodes = (char**)malloc(sizeof(char*) * num_nodes);
	for(int i=0; i<num_nodes; i++){
		global_nodes[i] = (char*)malloc(sizeof(char) * 20);
		strcpy(global_nodes[i], nodes[i]); 
	}
	global_num_nodes = num_nodes;
 	
	

	return ;
}


void psu_acquire_lock(int lock_number){
	CLIENT *c1;
	c1 = clnt_create(global_nodes[mynode], DISLOCK, LOCK_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}
	input inp;
	inp.you = mynode;
	inp.me = mynode;
	inp.num_nodes = global_num_nodes;
	inp.lock_number = lock_number;
	inp.release = 0;
	output outp;
	enum clnt_stat retval_1;
	retval_1 = request_1(&inp, &outp, c1);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (c1, "call failed");
	}

	int seq = outp.seq;
	
	pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*global_num_nodes);
	input *inpp = (input*)malloc(sizeof(input)*global_num_nodes);
	for(int i=0;i<global_num_nodes;i++){      
		if(i != mynode){
			inpp[i].you = i;
			inpp[i].me = mynode;
			inpp[i].num_nodes = global_num_nodes;
			inpp[i].lock_number = lock_number;
			inpp[i].seq = seq;
			/*inp.you = i;
			inp.me = mynode;
			inp.num_nodes = global_num_nodes;
			inp.lock_number = lock_number;
			inp.seq = seq;*/
			printf("True: %d\n",inpp[i].you);
			pthread_create(&tid[i],NULL,&remote_request,(void*)&inpp[i]);
		}
	}

	for(int i=0;i<global_num_nodes;i++){
		if(i != mynode){
			pthread_join(tid[i],NULL);
		}
	}

	printf("Client %d get the lock %d now!\n", mynode, lock_number);
	
}

void psu_release_lock(int lock_number){
	CLIENT *c1;
	c1 = clnt_create(global_nodes[mynode], DISLOCK, LOCK_V1, "tcp");
	if(c1 == NULL){
    		printf("pcreate error1\n");
   		exit(1);
 	}

	input inp;
	inp.you = mynode;
	inp.me = mynode;
	inp.num_nodes = global_num_nodes;
	inp.lock_number = lock_number;
	inp.release = 1;
	output outp;
	enum clnt_stat retval_1;
	retval_1 = request_1(&inp, &outp, c1);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (c1, "call failed");
	}
	printf("Client %d release the lock %d now!\n", mynode, lock_number);
}



	
	
