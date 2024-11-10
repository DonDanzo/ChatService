#include "Client.h"
#include "Defs.h"

#include <iostream>

#include <asio/asio.hpp>
#include <asio/asio/buffer.hpp>

#include "Logger.h"


Client::Client()
{
}

Client::~Client()
{
	Disconnect();
}

bool Client::Connect(const std::string& host, const uint16_t port)
{
	LOG_DEBUG("Client::Connect()")
	try
	{
		asio::ip::tcp::resolver resolver(m_context);// asio logic, Resolve ip-address into tangiable physical address
		asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port)); //resolver will get ip address of soem url address

		m_connection = std::make_unique<Connection>(Owner::Client, m_context, asio::ip::tcp::socket(m_context), Defs::ClientConnectionId, m_queuedMessagesIn);// Create connection

		m_connection->ConnectToServer(endpoints);// Tell the connection object to connect to server

		m_threadContext = std::thread([this]() { m_context.run(); });// Start Context Thread
	}
	catch (std::exception& e)
	{
		Logger::Instance().LogError(e.what());
		return false;
	}
	return true;
}

void Client::Disconnect()
{
	LOG_DEBUG("Client::Disconnect()")
	if (IsConnected())
	{
		m_connection->Disconnect();
	}
		
	m_context.stop();

	if (m_threadContext.joinable())
		m_threadContext.join();

	m_connection.release();
}

bool Client::IsConnected()
{
	//LOG_DEBUG("Client::IsConnected()")
	if (m_connection)
		return m_connection->IsConnected();
	
	return false;
}

void Client::Send(const ChatMessages::UserMessage& msg)
{
	LOG_DEBUG("Client::Send()")
	if (IsConnected())
	{
		m_connection->Send(msg);
	}
}

ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& Client::Incoming()
{
	return m_queuedMessagesIn;
}