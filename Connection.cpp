#include <iostream>
#include "Connection.h"
#include "Defs.h"
#include "Logger.h"
#include <google/protobuf/text_format.h>

// Constructor: Specify Owner, connect to context, transfer the socket
//				Provide reference to incoming message queue
Connection::Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, size_t connectionId, ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& inputQueue)
	: m_asioContext(asioContext), 
	m_socket(std::move(socket)), 
	m_id(connectionId),
	m_queuedIncomeMessages(inputQueue)
{
	m_ownerType = parent;
}

size_t Connection::GetID() const
{
	return m_id;
}

void Connection::ConnectToClient()
{
	LOG_DEBUG("Connection::ConnectToClient()")
	if (m_ownerType != Owner::Server)
	{
		Logger::Instance().LogError("Connection to Client failed! Connection is initialized with OwnerType different than Defs::Owner::server!");
		return;
	}

	if (!m_socket.is_open())
	{
		Logger::Instance().LogError("Connection socket is not open!");
		return;
	}

	ReadHeader();
}

void Connection::ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
{
	// Only clients can connect to servers
	if (m_ownerType != Owner::Client)
	{
		Logger::Instance().LogError("Connection to Server failed! Connection is initialized with OwnerType different than Defs::Owner::client!");
		return;
	}

	// Request asio attempts to connect to an endpoint
	asio::async_connect(m_socket, endpoints,
		[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
		{
			LOG_DEBUG("Connection::ConnectToServer()")
			if (ec)
			{
				Logger::Instance().LogError(ec.message());
				return;
			}
			
			ReadHeader();
		});
}

void Connection::Disconnect()
{
	LOG_DEBUG("Connection::Disconnect()")
	if (IsConnected())
		asio::post(m_asioContext, [this]() { m_socket.close(); });
}

bool Connection::IsConnected() const
{
	return m_socket.is_open();
}

void Connection::Send(Messages::CommunicationMessage& msg)
{
	LOG_DEBUG("Connection::Send(Messages::CommunicationMessage& msg")
		asio::async_write(m_socket, asio::buffer(&msg.GetHeader(), Messages::MessageHeaderSizes::Header),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
			if (ec)
			{
				LOG_DEBUG("Connection::Send() - Header")
				Logger::Instance().LogError("Failed to sending message header with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}
			Logger::Instance().Log("Connection::Send(Messages::CommunicationMessage& msg) HEADER", msg);
		});

		asio::async_write(m_socket, asio::buffer(msg.GetBody().data(), msg.GetBody().size()),// send converted Communication message as raw data in the socket
			[&, this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					LOG_DEBUG("Connection::Send() - Body")
					Logger::Instance().LogError("Failed to sending message body with ID: ", std::to_string(m_id));
					Logger::Instance().LogError(ec.message());
					m_socket.close();
					Logger::Instance().LogError("Connection closed ... ");
					return;
				}
				Logger::Instance().Log("Connection::Send(Messages::CommunicationMessage& msg) BODY", msg);
			});
}

void Connection::Send(const ChatMessages::UserMessage& msg)
{
	LOG_DEBUG("Connection::Send(const ChatMessages::UserMessage& msg)")
	asio::post(m_asioContext,
		[this, msg]()
		{
			LOG_DEBUG("Connection::Send()")
			bool isThereMsgToBeProceed = m_queuedOutputMessages.IsEmpty();
			m_queuedOutputMessages.PushBack(msg);
			if (isThereMsgToBeProceed)
			{
				WriteData();
			}
			Logger::Instance().Log("Connection::Send(const ChatMessages::UserMessage& msg)", msg);
		});
}

void Connection::WriteData()
{
	if (m_queuedOutputMessages.IsEmpty())
			return;

	LOG_DEBUG("Connection::WriteData()")
	auto userMsg = m_queuedOutputMessages.PopFront();//pop message that will be send from the queue

	Messages::Types msgType = Messages::Types::None;
	if (userMsg.type() == ChatMessages::MessageType::Client)//message is sent by client
	{
		msgType = Messages::Types::Client;
	}
	else if (userMsg.type() == ChatMessages::MessageType::System)//it is some system message sent by server
	{
		msgType = Messages::Types::System;
	}

	Messages::CommunicationMessage commMsgToSend(msgType, userMsg);//that message is wrapper of header and protobuf message

	asio::async_write(m_socket, asio::buffer(&commMsgToSend.GetHeader(), Messages::MessageHeaderSizes::Header),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
			LOG_DEBUG("Connection::WriteData() - ASYNC HEADER")
			if (ec)
			{
				Logger::Instance().LogError("Failed to sending message header with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}
			Logger::Instance().Log("Connection::WriteData() - HEADER", commMsgToSend);
		});

	asio::async_write(m_socket, asio::buffer(commMsgToSend.GetBody().data(), commMsgToSend.GetBody().size()),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
			LOG_DEBUG("Connection::WriteData() - ASYNC BODY")
			if (ec)
			{
				Logger::Instance().LogError("Failed to sending message body with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}

			if (!m_queuedOutputMessages.IsEmpty())//if not empty, send next message
			{
				WriteData();
			}
			Logger::Instance().Log("Connection::WriteData() - BODY", commMsgToSend);
		});
}

void Connection::ReadHeader()
{
	// wait until recevied enough bytes to construct header, header contains size of body, so it will be parsed after header is received
	asio::async_read(m_socket, asio::buffer(&m_incomeMsg.GetHeader(), Messages::MessageHeaderSizes::Header),
		[this](std::error_code ec, std::size_t length)
		{
			LOG_DEBUG("Connection::ReadHeader()")

			Logger::Instance().Log("Connection::ReadHeader() - MsgSize:[", std::to_string(m_incomeMsg.GetHeader().GetSize()), "]");

			if (ec)
			{
				Logger::Instance().LogError("Failed reading header of message with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				if(m_socket.is_open())
					m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				ReadHeader();
				//return;
			}
			
			if (m_incomeMsg.GetHeader().GetType() != Messages::Types::Client
				&& m_incomeMsg.GetHeader().GetType() != Messages::Types::System)
			{
				Logger::Instance().LogError("Invalid data received Header is expected to be System or Client.");
				m_socket.close();
				return;
			}
			if (m_incomeMsg.GetHeader().GetSize() > Defs::MaxBodyLength)
			{
				Logger::Instance().LogError("Invalid data received Body length > 5000, it is: ", std::to_string(m_incomeMsg.GetHeader().GetSize()));
				m_socket.close();
				return;
			}


			m_incomeMsg.GetBody().resize(m_incomeMsg.GetHeader().GetSize());//resize body (body is vector) with expected size
			ReadBody();//go to wait all data for body and parse it	
		});
}

void Connection::ReadBody()
{	asio::async_read(m_socket, asio::buffer(m_incomeMsg.GetBody().data(), m_incomeMsg.GetBody().size()),
		[this](std::error_code ec, std::size_t length)
		{
			LOG_DEBUG("Connection::ReadBody()")
			if (ec)
			{
				Logger::Instance().LogError("Failed reading body of message with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}

			// message read successfully
			AddToIncomingMessageQueue();			
			Logger::Instance().Log("Connection::ReadBody()", m_incomeMsg);
		});
}

void Connection::AddToIncomingMessageQueue()
{
	LOG_DEBUG("Connection::AddToIncomingMessageQueue()")
	//if connection is server, we care which one connection sent the message, else no care
	if (m_ownerType == Owner::Server)
		m_queuedIncomeMessages.PushBack({ this->shared_from_this(), m_incomeMsg });
	else
		m_queuedIncomeMessages.PushBack({ nullptr, m_incomeMsg });
	
	//read next message's header
	ReadHeader();
}

void Connection::SetLastIncomeMessage(const Messages::CommunicationMessage& msg)
{
	m_incomeMsg = msg;
}
Messages::CommunicationMessage& Connection::GetLastIncomeMessage()
{
	return m_incomeMsg;
}

void Connection::SetConnectionState(const State& conState)
{
	m_connectionState = conState;
}
State& Connection::GetConnectionState()
{
	return m_connectionState;
}

void Connection::SetConnectionClientName(const std::string& clientUserName)
{
	m_clientUserName = clientUserName;
}
const std::string& Connection::GetConnectionClientName() const
{
	return m_clientUserName;
}
