#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>



int Our_Sequence_Number[100]; // the sequence number chosen by a request originating at this node
int Highest_Sequence_Number[100]; //the highest sequence number seen in any REQUEST message sent or received
int Outstanding_Reply_Count[100]; // the number of REPLY messages still expected

bool Requesting_Critical_Section[100]; //TRUE when this node is requesting access to its critial section
bool* Reply_Deferred[100]; //Reply_Deferred [j] is TRUE when this node is deferring a REPLY to j's REQUEST message

pthread_mutex_t Shared_vars[100]; // Interlock access to the above shared variables when necessary

int Lock_Exist[100] = {0};



bool_t request_1_svc(input* inp, output* outp, struct svc_req *req){
	
	if(inp->you == inp->me && inp->release == 0){
		// request from local client
		int me = inp->you;
		int N = inp->num_nodes;
		int lock_number = inp->lock_number;

		if(Lock_Exist[lock_number] == 0){
			//initialize the lock
			Highest_Sequence_Number[lock_number] = 0;
			Requesting_Critical_Section[lock_number] = false;	
			Reply_Deferred[lock_number] = (bool*)malloc(sizeof(bool)*N);
			for(int j=0;j<N;j++){
				Reply_Deferred[lock_number][j] = false;
			}
			pthread_mutex_init(&Shared_vars[lock_number], NULL); 
			printf("Initialize lock %d in node %d success!\n",lock_number,me);
			Lock_Exist[lock_number] = 1;
		}
		
		//assign the seq for local client
		pthread_mutex_lock(&Shared_vars[lock_number]);
		Requesting_Critical_Section[lock_number] = true;
		Our_Sequence_Number[lock_number] = Highest_Sequence_Number[lock_number] + 1;
		pthread_mutex_unlock(&Shared_vars[lock_number]);

		outp->seq = Our_Sequence_Number[lock_number];
	
		return true;
	}
	 
	else if(inp->you == inp->me && inp->release == 1){		
		// release lock
		int me = inp->you;
		int N = inp->num_nodes;
		int lock_number = inp->lock_number;
		
		Requesting_Critical_Section[lock_number] = false;

		for(int j=0; j<N; j++){
			if(Reply_Deferred[lock_number][j]){
				Reply_Deferred[lock_number][j] = false;
			}
		}
		return true;	
	}
	else{					
		// request from remote client
		int me = inp->you;
		int N = inp->num_nodes;
		int lock_number = inp->lock_number;
		int k = inp->seq;
		int you = inp->me;

		if(Lock_Exist[lock_number] == 0){
			//initialize the lock
			Highest_Sequence_Number[lock_number] = 0;
			Requesting_Critical_Section[lock_number] = false;	
			Reply_Deferred[lock_number] = (bool*)malloc(sizeof(bool)*N);
			for(int j=0;j<N;j++){
				Reply_Deferred[lock_number][j] = false;
			}
			pthread_mutex_init(&Shared_vars[lock_number], NULL); 
			printf("Initialize lock %d in node %d success!\n",lock_number,me);
			Lock_Exist[lock_number] = 1;
		}

		if(Highest_Sequence_Number[lock_number] < k)
			Highest_Sequence_Number[lock_number] = k;
		pthread_mutex_lock(&Shared_vars[lock_number]);
		bool Defer_it = Requesting_Critical_Section[lock_number] & ( (k > Our_Sequence_Number[lock_number]) | (k == Our_Sequence_Number[lock_number] && you > me) );
		pthread_mutex_unlock(&Shared_vars[lock_number]);
		
		if(Defer_it){
			Reply_Deferred[lock_number][you] = true;
		}
		while(Reply_Deferred[lock_number][you]){
			;
		}
		
		printf("node %d reply to node %d for lock %d.\n",me, you, lock_number);
		return true;
	}
	
	return true;
}

int dislock_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free(xdr_result,result);
	return(1);
}
