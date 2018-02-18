This is the 2nd part of Project 2

Use "make clean; make" to compile the code.

To run the code, you first need to open the hello_world_server.c to input the num_nodes and the IP Address of eacn nodes. Using strcpy to assign the IP Address. You also need to set the node number of the server into the variable mynode, or you use the process of initializeDSM in client. Then you need to open the hello_world_client.c to do some psu_dsm_malloc, psu_dsm_free and load, store operations. Finally, you run ./server in each node, and then run "./client #node_number_of_this_client #IP_Address_of_this_client " in each client.

Since I don't have enough time to debug, so my code may exist some problems.( I pay lots of time in Part 1 to figure out the tcp multithread  _xdr).

The Idea:
We first do some basic initialization operations and then malloc. In malloc, we need to call local server to create a shared memory and also create the directory of this page. In read/write, if we meet SIGSEGV, we handled that, calling the local server, and the local server will do things based on the MSI state. Also, when the server received Invalid message from other servers, It need send a signal to the local client to do some mprotect to change the memory access state.

Author : Wenjian Miao  &&  Yilei Lin 
