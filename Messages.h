#pragma once

#include "Defs.h"
#include "messages.pb.h"

namespace Messages
{

	enum class Types : size_t
	{
		None,
		System,
		Client
	};

	namespace MessageHeaderSizes
	{
		constexpr auto Type = sizeof(Types);
		constexpr auto Size = sizeof(size_t);
		constexpr auto Header = Type + Size;
	}

	struct MessageHeader
	{
	public:
		MessageHeader() : m_type(Types::None), m_size(0)
		{
		}

		MessageHeader(Types type, size_t size) : m_type(type), m_size(size)
		{
		}

		MessageHeader(const MessageHeader& header) : m_type(header.m_type), m_size(header.m_size)
		{
		}

		void SetType(Types id) { m_type = id; }
		Types GetType() { return m_type; }

		void SetSize(size_t size) { m_size = size; }
		size_t GetSize() { return m_size; }
	private:
		Types m_type;
		size_t m_size;
	};

	struct CommunicationMessage
	{
	public:
		CommunicationMessage() : m_header(), m_body()
		{
		}
		CommunicationMessage(const Types type, ChatMessages::UserMessage& userMsg)
			: m_header(type, userMsg.ByteSizeLong())
			, m_body(userMsg.ByteSizeLong())
		{
			userMsg.SerializeToArray(m_body.data(), static_cast<int>(m_body.size()));//write protobuf's message data to m_body
		}

		void SetHeader(const MessageHeader& header) { m_header = header; }
		MessageHeader& GetHeader() { return m_header; };

		void SetBody(std::vector<uint8_t>& body) { m_body = body; }
		std::vector<uint8_t>& GetBody() 
		{ 
			return m_body; 
		};

		size_t Size() { return MessageHeaderSizes::Header + m_body.size(); }
	private:
		MessageHeader m_header;
		std::vector<uint8_t> m_body;
	};



}