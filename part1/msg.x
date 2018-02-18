struct input{
	int you;
	int me;
	int num_nodes;
	int lock_number;
	int seq;
	int release;
};

struct output{
	int seq;
};

program DISLOCK {
	version LOCK_V1 {
	output REQUEST(input) = 2;
	} = 1;
} = 0x2ffffff;


