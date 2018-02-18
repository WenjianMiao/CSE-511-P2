all: SERVER CLIENT

clean:
	rm -f client server *.o 

.PHONY:
	all clean

# we don't need that now
# 	genmsg: msg.x
#	rpcgen  -M msg.x

#CLIENT: genmsg hello_world_client.c psu_dsm.c
CLIENT: hello_world_client.c psu_dsm.c
	gcc -g  -o client hello_world_client.c msg_clnt.c psu_dsm.c msg_xdr.c -lrt -lpthread -lnsl

#SERVER: genmsg hello_world_server.c
SERVER: hello_world_server.c
	gcc -g  -o server hello_world_server.c msg_svc.c msg_clnt.c msg_xdr.c -lrt -lpthread -lnsl
