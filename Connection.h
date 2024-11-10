#pragma once
#include <memory>
#include <asio/asio.hpp>
#include "ThreadSafeQueue.h"
#include "Messages.h"// includes Def.h too

#include "messages.pb.h"

// A connection is Server or a Client
enum class Owner
{
	Server,//actually not implemented yet. Server doesnt sends server commands and done receive reponse to them
	Client
};

//State of the connection, used only on server side per each client
enum class State
{
	None,
	RequestToConnect,
	Connected
};



class Connection : public std::enable_shared_from_this<Connection>
{
public:
	// Constructor: Specify Owner, connect to context, transfer the socket and provide reference to incoming message queue
	Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, size_t connectionId, ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& inputQueue);
	virtual ~Connection() {}
		
	size_t GetID() const;// This ID is used system wide - its how clients will understand other clients exist across the whole system.

	void ConnectToServer(const asio::ip::tcp::resolver::results_type& address);//client side only
	void ConnectToClient();//server side only

	void Disconnect();

	bool IsConnected() const;

	void StartListening();	//TODO:  not implemented yet

	void Send(const ChatMessages::UserMessage& msg);//send message to another connection, which should be owned by oposite side: eg. Server or Client
	void Send(Messages::CommunicationMessage& msg);//it will be used only on Server side, to resend message to all rest clients

	void SetLastIncomeMessage(const Messages::CommunicationMessage& msg);
	Messages::CommunicationMessage& GetLastIncomeMessage();

	State& GetConnectionState();
	void SetConnectionState(const State& conState);

	void SetConnectionClientName(const std::string& userName);
	const std::string& GetConnectionClientName() const;
			
private:
	void WriteData();

	void ReadHeader();//ASYNC - Prime context ready to read a message header

	void ReadBody();//ASYNC - Prime context ready to read a message body

	void AddToIncomingMessageQueue();//add full message to the incoming queue
	
	asio::ip::tcp::socket m_socket;//Each connection has a unique socket to a remote 
	asio::io_context& m_asioContext;//This context is shared with the whole asio instance

	ThreadSafeQueue<ChatMessages::UserMessage> m_queuedOutputMessages;// The queue holds all messages which should be sent to the remote side
	ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& m_queuedIncomeMessages;	// This references the incoming queue of the parent object
	Messages::CommunicationMessage m_incomeMsg;	// store income messages async
	Owner m_ownerType = Owner::Server;// The "owner" decides how some of the connection behaves
	State m_connectionState = State::None;
	size_t m_id = 0;
	size_t m_msgId = 0;
	std::string m_clientUserName;//used only on server side. Means which user is connected on that conection
};

