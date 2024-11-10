#pragma once
#include <string>
#include <thread>

#include "ThreadSafeQueue.h"
#include "Connection.h"
#include "Messages.h"//it includes Def.h
#include "messages.pb.h"

class Client
{
public:
	Client();
	virtual ~Client();

	bool Connect(const std::string& host, const uint16_t port);	// Connect to server with ip-address and port
	void Disconnect();// Disconnect from the server	
	bool IsConnected();// Check if client is connected to server
	void Send(const ChatMessages::UserMessage& msg);// Send Brotobuf's message to server	
	ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& Incoming();// Retrieve queue of messages from server
	void Wait() { m_queuedMessagesIn.Wait(); }
protected:
	virtual void ProcessIncomeMessages() = 0;
	asio::io_context m_context;					// asio context handles the data transfer...
	std::thread m_threadContext;				// but needs a thread of its own to execute work commands
	std::unique_ptr<Connection> m_connection;	//"connection" object handles data transfer
private:
	ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>> m_queuedMessagesIn;	// thread safe queue for incoming messages from server
};

