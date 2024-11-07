#include <iostream>
#include "Connection.h"
#include "Defs.h"
#include "Logger.h"
#include <google/protobuf/text_format.h>

// Constructor: Specify Owner, connect to context, transfer the socket
//				Provide reference to incoming message queue
Connection::Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThreadSafeQueue<std::pair<std::shared_ptr<Connection>, Messages::CommunicationMessage>>& inputQueue)
	: m_asioContext(asioContext), 
	m_socket(std::move(socket)), 
	m_queuedIncomeMessages(inputQueue)
{
	m_ownerType = parent;
}

uint32_t Connection::GetID() const
{
	return m_id;
}

void Connection::ConnectToClient(uint32_t userId)
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

	m_id = userId;
	ReadHeader();
}

void Connection::ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
{
	LOG_DEBUG("Connection::ConnectToServer()")
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
		asio::async_write(m_socket, asio::buffer(&msg.GetHeader(), Messages::MessageHeaderSizes::Header),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
			if (ec)
			{
				Logger::Instance().LogError("Failed to sending message header with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}
		});

		asio::async_write(m_socket, asio::buffer(&msg.GetBody(), msg.GetBody().size()),// send converted Communication message as raw data in the socket
			[&, this](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					Logger::Instance().LogError("Failed to sending message body with ID: ", std::to_string(m_id));
					Logger::Instance().LogError(ec.message());
					m_socket.close();
					Logger::Instance().LogError("Connection closed ... ");
					return;
				}
			});



}


void Connection::Send(const ChatMessages::UserMessage& msg)
{
	LOG_DEBUG("Connection::Send()")
	asio::post(m_asioContext,
		[this, msg]()
		{
			bool isThereMsgToBeProceed = m_queuedOutputMessages.IsEmpty();
			m_queuedOutputMessages.PushBack(msg);
			if (isThereMsgToBeProceed)
			{
				WriteData();
			}
		});
}

void Connection::WriteData()
{
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

	Messages::MessageHeader commMsgHeader(msgType, userMsg.ByteSizeLong());//create message header with message type and body size
	std::vector<uint8_t> userMsgRawData(userMsg.ByteSizeLong());
	userMsg.SerializeToArray(userMsgRawData.data(), userMsgRawData.size());//write protobuf's message data to vector uint8_t
	Messages::CommunicationMessage commMsgToSend(commMsgHeader, userMsgRawData);//that message is wrapper of header and protobuf message

	asio::async_write(m_socket, asio::buffer(&commMsgHeader, Messages::MessageHeaderSizes::Header),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
			if (ec)
			{
				Logger::Instance().LogError("Failed to sending message header with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
			}
		});

	asio::async_write(m_socket, asio::buffer(userMsgRawData.data(), userMsgRawData.size()),// send converted Communication message as raw data in the socket
		[&, this](std::error_code ec, std::size_t length)
		{
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
		});
}

void Connection::ReadHeader()
{
	LOG_DEBUG("Connection::ReadHeader()")
	// wait until recevied enough bytes to construct header, header contains size of body, so it will be parsed after header is received
	asio::async_read(m_socket, asio::buffer(&m_incomeMsg.GetHeader(), Messages::MessageHeaderSizes::Header),
		[this](std::error_code ec, std::size_t length)
		{
			if (ec)
			{
				Logger::Instance().LogError("Failed reading header of message with ID: ", std::to_string(m_id));
				Logger::Instance().LogError(ec.message());
				m_socket.close();
				Logger::Instance().LogError("Connection closed ... ");
				return;
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
{
	LOG_DEBUG("Connection::ReadBody()")
	asio::async_read(m_socket, asio::buffer(m_incomeMsg.GetBody().data(), m_incomeMsg.GetBody().size()),
		[this](std::error_code ec, std::size_t length)
		{
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