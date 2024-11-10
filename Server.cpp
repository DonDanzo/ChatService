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
							m_asioContext, std::move(socket), ++m_clientsCount, m_queuedMessagesIn);

					//OnClientTryingConnect(newconn);
					newconn->SetConnectionState(State::RequestToConnect);
					newconn->ConnectToClient();
					m_dequedConnections.push_back(std::move(newconn));

					Logger::Instance().Log("[SERVER] Connection pending authentication: ", std::to_string(m_dequedConnections.back()->GetID()));
				}
			}
			else
			{
				Logger::Instance().LogError("[SERVER] New Connection Request Error: ", ec.message());
			}
			WaitForClientConnection();
		});
}

void Server::MessageAllClients(Messages::CommunicationMessage& msg, std::shared_ptr<Connection> pIgnoreClient)
{
	LOG_DEBUG("Server::MessageAllClients()")

		Logger::Instance().Log("Server::MessageAllClients()", msg);
	bool isThereInvalidClient = false;

	std::vector<std::shared_ptr<Connection>> disconnectedClients;

	for (auto& client : m_dequedConnections)
	{
		if (client && client->IsConnected())
		{
			if (client != pIgnoreClient)
				client->Send(msg);//sends message to the client if it is not ignored
		}
		else
		{
			disconnectedClients.push_back(std::shared_ptr<Connection>(client));//store in vector all clients that are not connected
		}
	}


	for (auto& client : disconnectedClients)// disconnect all not connected clients
	{
		DisconnectClient(client);
	}
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

void Server::DisconnectClient(std::shared_ptr<Connection> client)
{
	LOG_DEBUG("Server::DisconnectClient()")
		
	auto it = m_dequedConnections.begin();
	while (it != m_dequedConnections.end())
	{
		if (it->get()->GetID() == client->GetID())
		{
			m_dequedConnections.erase(it);
			break;
		}
		it++;
	}	

	it->get()->Disconnect();
	it->get()->SetConnectionState(State::None);
	client.get()->SetConnectionState(State::None);
	OnClientDisconnect(client);
	client.reset();
	m_clientsCount--;
}