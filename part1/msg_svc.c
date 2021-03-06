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
	input request_1_arg;
} A;

typedef union {
	output request_1_res;
} B;
	

void*
serv_request(void* data)
{
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
	if (!dislock_1_freeresult (transp, _xdr_result, (caddr_t) &result))
		fprintf (stderr, "%s", "unable to free results");

	return;
}

static void 
dislock_1(struct svc_req *rqstp, register SVCXPRT *transp)
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

	case REQUEST:
		_xdr_argument = (xdrproc_t) xdr_input;
		_xdr_result = (xdrproc_t) xdr_output;
		local = (bool_t (*) (char *, void *,  struct svc_req *))request_1_svc;
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

	pmap_unset (DISLOCK, LOCK_V1);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, DISLOCK, LOCK_V1, dislock_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (DISLOCK, LOCK_V1, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, DISLOCK, LOCK_V1, dislock_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (DISLOCK, LOCK_V1, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}
