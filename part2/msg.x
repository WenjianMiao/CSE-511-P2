struct InitInp{
	int mynode;
	int mypid;
};

struct RequestRet{
	int size;
	int valid;
};

struct MallocInp{
	string name<>;
	int size;
};

struct RemoteInp{
	string name<>;
	int IsRead;
};

struct NetworkInp{
	string name<>;
	int NetOp;
	int mynode;
};

program DSM {
	version SERVICE_V1 {
	void INIT(InitInp) = 1;
	int MALLOC(MallocInp) = 2;
	RequestRet REQUEST(string) = 3;
	int REMOTERDWR(RemoteInp) = 4;
	string NETWORKOP(NetworkInp) = 5;
	int FREE(string) = 6;
	} = 1;
} = 0x2ffffff;


