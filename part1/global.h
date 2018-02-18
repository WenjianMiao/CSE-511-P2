extern char** global_nodes;

extern int global_num_nodes;

extern int mynode;

void psu_init_lock_mgr(char** nodes, int num_nodes); 

void psu_acquire_lock(int lock_number);

void psu_release_lock(int lock_number);
