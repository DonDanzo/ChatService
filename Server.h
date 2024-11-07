#pragma once

#include <asio/asio.hpp>
#include "Connection.h"
//#include "Defs.h"
#include "Messages.h"//includes Defs.h too




class Server
{
public:
	Server(uint16_t port);
	virtual ~Server();
	bool Start();
	void Stop();
	void WaitForClientConnection();
	void MessageClient(std::shared_ptr<Connection> client, const ChatMessages::UserMessage& msg);
	void MessageAllClients(Messages::CommunicationMessage& msg, std::shared_ptr<Connection> pIgnoreClient = nullptr);
	void Update(size_t nMaxMessages = -1, bool bWait = false);
protected:

	virtual bool OnClientConnect(std::shared_ptr<Connection> client);
	virtual void OnClientDisconnect(std::shared_ptr<Connection> client);
	virtual void OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg);
private:

	asio::io_context m_context;

	ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>> m_queuedMessagesIn;	// Thread Safe Queue for incoming message packets
	std::deque<std::shared_ptr<Connection>> m_dequedConnections;	// Container of active validated connections

	asio::io_context m_asioContext;	// Order of declaration is important - it is also the order of initialisation
	std::thread m_threadContext;

	asio::ip::tcp::acceptor m_asioAcceptor; // Handles new incoming connection attempts...
	uint32_t m_clientsCount = 0;	// Clients will be identified in the "wider system" via an ID
};



