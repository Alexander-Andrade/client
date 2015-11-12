#include "Includes.h"
#include "Client.h"

int main(int argc,char* argv[])
{
	//IP & port from command line
	if (argc != 3)
	{
		cout << "incorrect server address" << endl;
		return -1;
	}
	try
	{
		Socket::initializeWinsock_();
		//IP, port to client
		Client client(argv[1], argv[2]);
		client.workingWithServer();
	}
	catch (exception e)
	{ 
		cout << e.what() << endl;
		getchar();
	}
	Socket::closeWinsock();
	return 0;
}