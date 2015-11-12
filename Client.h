#ifndef CLIENT_H
#define CLIENT_H
#include "Connection.h"

class Client : public Connection
{
protected:
	unique_ptr<ClientSocket> _socket;
public:
	Client(char* nodeName, char* serviceName,int sendBufLen = 3000, int timeOut = 20) : Connection(sendBufLen,timeOut)
	{
		_socket.reset(new ClientSocket(nodeName,serviceName));
		//send id to server
		_socket->send(_id);
		
		//if (!_socket->setReceiveBufferSize(receiveBufLen))
		//	throw runtime_error("fail to set receive buffer size");
		if (!_socket->setSendBufferSize(sendBufLen))
			throw runtime_error("fali to set send buffer size");

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
		return Connection::sendFile((Socket*)_socket.get(), message,std::bind(&Client::tryToReconnect, this, std::placeholders::_1));
	}
	bool receiveFile(string& message)
	{
		Connection::receiveFile((Socket*)_socket.get(), message, std::bind(&Client::tryToReconnect, this, std::placeholders::_1));
		//send confirm to the server handshake
		return sendConfirm(*_socket);
	}

	Socket* tryToReconnect(int timeOut)
	{
		time_t firstTimePoint = std::time(NULL);
		time_t timeDifference;
		while (true)
		{

			timeDifference = std::difftime(std::time(NULL), firstTimePoint);
			if (timeDifference > timeOut) break;
			if (_socket->establishConnection())
			{	//it managed to reconnect
				//send client id to server
				_socket->send(_id);

				return (Socket*)_socket.get();
			}
		}
		return nullptr;
	}

	void fillCommandMap() override
	{
		_commandMap[string("upload")] = std::bind(&Client::sendFile, this, std::placeholders::_1);
		_commandMap[string("download")] = std::bind(&Client::receiveFile, this, std::placeholders::_1);
	}

};




#endif //CLIENT_H