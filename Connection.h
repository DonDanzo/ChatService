#pragma once
#include <memory>
#include <asio/asio.hpp>
#include "ThreadSafeQueue.h"
#include "Messages.h"// includes Def.h too

#include "messages.pb.h"

// A connection is Server or a Client

enum class Owner
{
	Server,
	Client
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	// Constructor: Specify Owner, connect to context, transfer the socket and provide reference to incoming message queue
	Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& inputQueue);
	virtual ~Connection() {}
		
	uint32_t GetID() const;// This ID is used system wide - its how clients will understand other clients exist across the whole system.

	void ConnectToServer(const asio::ip::tcp::resolver::results_type& address);//client side only
	void ConnectToClient(uint32_t uid = 0);//server side only

	void Disconnect();

	bool IsConnected() const;


	void StartListening();	//TODO:  not implemented yet

	void Send(const ChatMessages::UserMessage& msg);//send message to another connection, which should be owned by oposite side: eg. Server or Client
	void Send(Messages::CommunicationMessage& msg);//it will be used only on Server side, to resend message to all rest clients
private:
	void WriteData();

	void ReadHeader();//ASYNC - Prime context ready to read a message header

	void ReadBody();//ASYNC - Prime context ready to read a message body

	void AddToIncomingMessageQueue();//add full message to the incoming queue
public:
	
	asio::ip::tcp::socket m_socket;//Each connection has a unique socket to a remote 


	asio::io_context& m_asioContext;//This context is shared with the whole asio instance

	ThreadSafeQueue<ChatMessages::UserMessage> m_queuedOutputMessages;// The queue holds all messages which should be sent to the remote side

	ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& m_queuedIncomeMessages;	// This references the incoming queue of the parent object

	Messages::CommunicationMessage m_incomeMsg;	// store income messages async

	Owner m_ownerType = Owner::Server;// The "owner" decides how some of the connection behaves

	size_t m_id = 0;
	size_t m_msgId = 0;
};
