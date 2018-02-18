/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _MSG_H_RPCGEN
#define _MSG_H_RPCGEN

#include <rpc/rpc.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


struct input {
	int you;
	int me;
	int num_nodes;
	int lock_number;
	int seq;
	int release;
};
typedef struct input input;

struct output {
	int seq;
};
typedef struct output output;

#define DISLOCK 0x2ffffff
#define LOCK_V1 1

#if defined(__STDC__) || defined(__cplusplus)
#define REQUEST 2
extern  enum clnt_stat request_1(input *, output *, CLIENT *);
extern  bool_t request_1_svc(input *, output *, struct svc_req *);
extern int dislock_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define REQUEST 2
extern  enum clnt_stat request_1();
extern  bool_t request_1_svc();
extern int dislock_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_input (XDR *, input*);
extern  bool_t xdr_output (XDR *, output*);

#else /* K&R C */
extern bool_t xdr_input ();
extern bool_t xdr_output ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_MSG_H_RPCGEN */
