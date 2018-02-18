/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif


pthread_t p_thread;
pthread_attr_t attr;

typedef union {
	InitInp init_1_arg;
	MallocInp malloc_1_arg;
	char *request_1_arg;
	RemoteInp remoterdwr_1_arg;
	NetworkInp networkop_1_arg;
	char *free_1_arg;
} A;

typedef union {
	int malloc_1_res;
	RequestRet request_1_res;
	int remoterdwr_1_res;
	char *networkop_1_res;
	int free_1_res;
} B;

void*
serv_request(void* data){
	struct thr_data{
		struct svc_req *rqstp;
		SVCXPRT *transp;
		A argument;
		B result;
		xdrproc_t _xdr_argument;
		xdrproc_t _xdr_result;
		bool_t (*local)(char *, void *, struct svc_req *);
	} *ptr_data;	

	ptr_data = (struct thr_data  *)data;
	struct svc_req *rqstp = ptr_data-> rqstp;
	register SVCXPRT *transp = ptr_data-> transp;
	A argument = ptr_data->argument ;
	B result = ptr_data->result;
	xdrproc_t _xdr_argument = ptr_data->_xdr_argument;
	xdrproc_t _xdr_result = ptr_data->_xdr_result;
	bool_t (*local)(char *, void *, struct svc_req *) = ptr_data->local;
	
	
	bool_t retval;


	
	retval = (bool_t) (*local)((char *)&argument, (void *)&result, rqstp);
	if (retval > 0 && !svc_sendreply(transp, (xdrproc_t) _xdr_result, (char *)&result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	if (!dsm_1_freeresult (transp, _xdr_result, (caddr_t) &result))
		fprintf (stderr, "%s", "unable to free results");

	return;
}

static void 
dsm_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	struct data_str{
		struct svc_req *rqstp;
		SVCXPRT *transp;
		A argument;
		B result;
		xdrproc_t _xdr_argument;
		xdrproc_t _xdr_result;
		bool_t (*local)(char *, void *, struct svc_req *);
		
	} *data_ptr = (struct data_str*)malloc(sizeof(struct data_str));


	A argument;
	B result;
	bool_t retval;
	xdrproc_t _xdr_argument, _xdr_result;
	bool_t (*local)(char *, void *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case INIT:
		_xdr_argument = (xdrproc_t) xdr_InitInp;
		_xdr_result = (xdrproc_t) xdr_void;
		local = (bool_t (*) (char *, void *,  struct svc_req *))init_1_svc;
		break;

	case MALLOC:
		_xdr_argument = (xdrproc_t) xdr_MallocInp;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))malloc_1_svc;
		break;

	case REQUEST:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_RequestRet;
		local = (bool_t (*) (char *, void *,  struct svc_req *))request_1_svc;
		break;

	case REMOTERDWR:
		_xdr_argument = (xdrproc_t) xdr_RemoteInp;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))remoterdwr_1_svc;
		break;

	case NETWORKOP:
		_xdr_argument = (xdrproc_t) xdr_NetworkInp;
		_xdr_result = (xdrproc_t) xdr_wrapstring;
		local = (bool_t (*) (char *, void *,  struct svc_req *))networkop_1_svc;
		break;

	case FREE:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))free_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}


	data_ptr->rqstp = rqstp;
	data_ptr->transp = transp;
	data_ptr->argument = argument;
	data_ptr->result = result;
	data_ptr->_xdr_argument = _xdr_argument;
	data_ptr->_xdr_result = _xdr_result;
	data_ptr->local = local;
	
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&p_thread,&attr,serv_request,(void*)data_ptr);
}

int
main (int argc, char **argv)
{
	register SVCXPRT *transp;

	pmap_unset (DSM, SERVICE_V1);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, DSM, SERVICE_V1, dsm_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (DSM, SERVICE_V1, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, DSM, SERVICE_V1, dsm_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (DSM, SERVICE_V1, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}