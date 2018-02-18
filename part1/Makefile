all: SERVER CLIENT

clean:
	rm -f client server *.o 

.PHONY:
	all clean

# we don't need that now.
#genmsg: msg.x
#	rpcgen -a -M msg.x

CLIENT: hello_world_client.c psu_dist_lock_mgr.c
	gcc -g  -o client hello_world_client.c msg_clnt.c psu_dist_lock_mgr.c msg_xdr.c -lrt -lpthread -lnsl

SERVER: hello_world_server.c
	gcc -g  -o server hello_world_server.c msg_svc.c  msg_xdr.c -lrt -lpthread -lnsl
