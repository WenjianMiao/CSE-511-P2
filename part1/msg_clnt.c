/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "msg.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

enum clnt_stat 
request_1(input *argp, output *clnt_res, CLIENT *clnt)
{
	return (clnt_call(clnt, REQUEST,
		(xdrproc_t) xdr_input, (caddr_t) argp,
		(xdrproc_t) xdr_output, (caddr_t) clnt_res,
		TIMEOUT));
}
