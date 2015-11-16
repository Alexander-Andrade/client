#ifndef CLIENT_H
#define CLIENT_H
#include "Connection.h"

class Client : public Connection
{
protected:
	unique_ptr<ClientSocket> _socket;
	unique_ptr<UDP_ClientSocket> _udpSocket;
public:
	Client(char* nodeName, char* serviceName,int sendBufLen = 2048, int timeOut = 30) : Connection(sendBufLen,timeOut)
	{
		_socket.reset(new ClientSocket(nodeName,serviceName));
		_udpSocket.reset(new UDP_ClientSocket(nodeName,serviceName));
		//send id to server
		_socket->send(_id);
		
		fillCommandMap();
	}

	void workingWithServer()
	{
		//command for server
		string message;
		try
		{
			while (true)
			{
				message.clear();
				std::getline(std::cin, message);
				//transfer command
				_socket->sendMessage(message);
				//execute client part of the command
				catchCommand(message);
				//receive server data
				cout << _socket->receiveMessage_();
			}
		}
		catch (exception e)
		{//disconnection or close
			cout << e.what() << endl;
			getchar();
		}
	}
protected:
	//------------------------------------files--------------------------------------//
	bool sendFile(string& message)
	{
		return Connection::sendFile((Socket*)_socket.get(), message,std::bind(&Client::tryToReconnect, this, (Socket*)_socket.get(), std::placeholders::_1));
	}
	bool receiveFile(string& message)
	{
		Connection::receiveFile((Socket*)_socket.get(), message, std::bind(&Client::tryToReconnect, this, (Socket*)_socket.get(), std::placeholders::_1));
		//send confirm to the server handshake
		return _socket->sendConfirm();
	}

	bool sendFileUdp(string& message)
	{
		//send client address to server for udp communication
		char arg = 0;
		_udpSocket->send<char>(arg);

		return Connection::sendFile((Socket*)_udpSocket.get(), message, std::bind(&Client::tryToReconnect, this, _udpSocket.get() , std::placeholders::_1));
	}
	bool receiveFileUdp(string& message)
	{
		char arg = 0;
		_udpSocket->send<char>(arg);

		//send client address to server for udp communication
		Connection::receiveFile((Socket*)_udpSocket.get(), message, std::bind(&Client::tryToReconnect, this, _udpSocket.get() ,std::placeholders::_1));
		//send confirm to the server handshake
		return _socket->sendConfirm();
	}

	Socket* tryToReconnect(Socket* socket,int timeOut)
	{
		time_t firstTimePoint = std::time(NULL);
		time_t timeDifference;
		while (true)
		{

			timeDifference = std::difftime(std::time(NULL), firstTimePoint);
			if (timeDifference > timeOut) break;
			if (socket->attachClientSocket())
			{	//it managed to reconnect
				//send client id to server
				socket->send(_id);

				return socket;
			}
		}
		return nullptr;
	}

	void fillCommandMap() override
	{
		_commandMap[string("upload")] = std::bind(&Client::sendFile, this, std::placeholders::_1);
		_commandMap[string("download")] = std::bind(&Client::receiveFile, this, std::placeholders::_1);
		_commandMap[string("upload_udp")] = std::bind(&Client::sendFileUdp, this, std::placeholders::_1);
		_commandMap[string("download_udp")] = std::bind(&Client::receiveFileUdp, this, std::placeholders::_1);
	}

};




#endif //CLIENT_H