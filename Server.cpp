#include "Server.h"
#include "Defs.h"

#include <asio/asio.hpp>
#include <iostream>

#include <asio/asio/buffer.hpp>
#include "Logger.h"

Server::Server(uint16_t port)
	: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
}

Server::~Server()
{
	Stop();
}

bool Server::Start()
{
	LOG_DEBUG("Server::Start()")
	try
	{
		WaitForClientConnection();

		// Launch the asio context in its own thread
		m_threadContext = std::thread([this]() { m_asioContext.run(); });
	}
	catch (std::exception& e)
	{
		// Something prohibited the server from listening
		Logger::Instance().LogError("[SERVER] Exception: ");
		Logger::Instance().LogError(e.what());
		return false;
	}
	Logger::Instance().Log("[SERVER] Started!");
	return true;
}

void Server::Stop()
{
	LOG_DEBUG("Server::Stop()")
	m_asioContext.stop();

	if (m_threadContext.joinable()) m_threadContext.join();

	Logger::Instance().Log("[SERVER] Stopped!");
}

void Server::WaitForClientConnection()
{
	LOG_DEBUG("Server::WaitForClientConnection()")
	m_asioAcceptor.async_accept(
		[this](std::error_code ec, asio::ip::tcp::socket socket)
		{
			if (!ec)
			{
				if (m_clientsCount > Defs::MaxClientsCount)
				{
					socket.close();
					Logger::Instance().Log("[SERVER] Clients count attached to Server is at maximum. Not able to accept more clients. Connection rejecetd ...");
				}
				else
				{
					LOG_DEBUG_PARAM("[SERVER] New Connection: ", socket.remote_endpoint());//TODO: check converting asio end point to string and use Logger

					std::shared_ptr<Connection> newconn =
						std::make_shared<Connection>(Owner::Server,
							m_asioContext, std::move(socket), m_queuedMessagesIn);

					if (OnClientConnect(newconn))
					{
						m_dequedConnections.push_back(std::move(newconn));
						m_dequedConnections.back()->ConnectToClient(m_clientsCount++);
						Logger::Instance().Log("[SERVER] Connection Approved: ", std::to_string(m_dequedConnections.back()->GetID()));
					}
					else
					{
						Logger::Instance().LogError("[SERVER] Connection Denied: ", std::to_string(m_dequedConnections.back()->GetID()));
					}
				}
			}
			else
			{
				Logger::Instance().LogError("[SERVER] New Connection Error: ", ec.message());
			}
			WaitForClientConnection();
		});
}

void Server::MessageClient(std::shared_ptr<Connection> client, const ChatMessages::UserMessage& msg)
{
	LOG_DEBUG("Server::MessageClient()")
	if (client && client->IsConnected())
	{
		client->Send(msg);
	}
	else
	{
		OnClientDisconnect(client);
		client.reset();

		m_dequedConnections.erase(
			std::remove(m_dequedConnections.begin(), m_dequedConnections.end(), client), m_dequedConnections.end());
	}
}

void Server::MessageAllClients(Messages::CommunicationMessage& msg, std::shared_ptr<Connection> pIgnoreClient)
{
	LOG_DEBUG("Server::MessageAllClients()")
	bool isThereInvalidClient = false;

	for (auto& client : m_dequedConnections)
	{
		if (client && client->IsConnected())
		{
			if (client != pIgnoreClient)
				client->Send(msg);//sends message to the client if it is not ignored
		}
		else
		{
			OnClientDisconnect(client);//client is disconnected, so remove it			
			isThereInvalidClient = true;
		}
	}

	if (isThereInvalidClient)
		m_dequedConnections.erase(
			std::remove(m_dequedConnections.begin(), m_dequedConnections.end(), nullptr), m_dequedConnections.end());
}

void Server::Update(size_t maxMessagesCount, bool shouldWait)
{
	LOG_DEBUG("Server::Update()")
	if (shouldWait)
	{
		m_queuedMessagesIn.Wait();
	}

	size_t messagesCount = 0;
	while (messagesCount < maxMessagesCount && !m_queuedMessagesIn.IsEmpty())
	{
		auto [connection, msg] = m_queuedMessagesIn.PopFront();
		OnMessage(connection, msg);
		messagesCount++;
	}
}

bool Server::OnClientConnect(std::shared_ptr<Connection> client) 
{
	LOG_DEBUG("Server::OnClientConnect()")
	//TODO: Send req and wait response
	Logger::Instance().Log("[SERVER] Client connected");
	return true;
}

void Server::OnClientDisconnect(std::shared_ptr<Connection> client) 
{
	LOG_DEBUG("Server::OnClientDisconnect()")
	Logger::Instance().Log("[SERVER] Disconnected client: ", std::to_string(client->GetID()));
	client.reset();
	m_clientsCount--;
}

// Called when a message arrives
void Server::OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg) 
{
	LOG_DEBUG("Server::OnMessage()")

		MessageAllClients(msg, client);//send the message too all connected clients, ignore current client
	 //Logger::Instance().Log("Server Ping: ", std::to_string(client->GetID()));
}
