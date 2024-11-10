// SpiderServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "SpiderServer.h"
#include "Logger.h"
#include "Helper.h"
using namespace std::chrono_literals;

bool SpiderServer::OnClientConnected(const std::string& userName, std::shared_ptr<Connection> client)
{
	LOG_DEBUG("SpiderServer::OnClientConnected()")

	//auto it = m_dequedConnections.begin();
	//
	//while (it != m_dequedConnections.end())
	//{
	//	if (it->get()->GetID() == client->GetID()){}
	//		break;
	//	it++;
	//}
	//
	//if (it == m_dequedConnections.end())
	//{
	//	//something strange behavior may be
	//	//TODO: check is it posible
	//	return false;
	//}
	//
	//it->get()->SetConnectionClientName(userName);
	//it->get()->SetConnectionState(State::Connected);

	client->SetConnectionClientName(userName);
	client->SetConnectionState(State::Connected);


	Logger::Instance().Log("User : [" + std::to_string(client->GetID()) + "]", userName, " jointed to the chat.");


	//create system message by server, to notify all users somebody has connected
	std::string systemMessage = "User " + userName + " has joined in the Chat!";
	auto msg = Helper::CreateUserMessage(ChatMessages::MessageType::System, userName, Helper::GetLocalMachineAddress(), Helper::CalculateTimeStamp(), systemMessage);
	Messages::CommunicationMessage commMsg(Messages::Types::System, msg);
	MessageAllClients(commMsg, client);

	return true;
}

void SpiderServer::OnClientDisconnect(std::shared_ptr<Connection> client)
{
	LOG_DEBUG("SpiderServer::OnClientDisconnect()")
	Logger::Instance().Log("[SERVER] Disconnected client: ", std::to_string(client->GetID()));

	//create system message by server, to notify all users somebody has diconnected
	std::string systemMessage = "User " + client->GetConnectionClientName() + " has left the Chat!";

	Logger::Instance().Log(systemMessage);
	auto msg = Helper::CreateUserMessage(ChatMessages::MessageType::System, client->GetConnectionClientName(), Helper::GetLocalMachineAddress(), Helper::CalculateTimeStamp(), systemMessage);
	Messages::CommunicationMessage commMsg(Messages::Types::System, msg);
	MessageAllClients(commMsg, client);

}

// Called when a message arrives
void SpiderServer::OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg)
{
	LOG_DEBUG("SpiderServer::OnMessage()")

		ChatMessages::UserMessage userMsg;
	userMsg.ParseFromArray(msg.GetBody().data(), msg.GetBody().size());

	if (client->GetConnectionState() == State::None)//that state is not expected, disconnect the cllient
	{
		DisconnectClient(client);
		return;
	}
	else if (client->GetConnectionState() == State::RequestToConnect)//it recevie first message for this client, expect handshake to verify the client
	{
		if (userMsg.data() != Defs::ConnectHandshakeMessageValue)//if received 1st message from client is not expected, go disconnect it
		{
			DisconnectClient(client);
			return;
		}

		OnClientConnected(userMsg.name(), client);
	}
	else if (client->GetConnectionState() == State::Connected)
	{
		MessageAllClients(msg, client);//send the message too all connected clients, ignore current client
	}
}



void SpiderServer::KickUser(std::string& userName)
{
	std::shared_ptr<Connection> foundClient;

	for (auto client : m_dequedConnections)
	{
		if (client->GetConnectionClientName() == userName)
		{
			foundClient = client;
			break;
		}
	}

	DisconnectClient(foundClient);
}


bool m_isHourTimerFinished = false;
static void HandleOnTimer(size_t timerId)
{
	LOG_DEBUG_PARAM("Handle Timer with ID: ", timerId)

		if (timerId == Defs::NewHourTimerId)
		{
			m_isHourTimerFinished = true;
			//close the file
			Logger::Instance().CloseFile();
			//open it again, it will create new files, becasue it is new hour and name will be different
			Logger::Instance().OpenFile();
		}
}

void StartServer(size_t serverPort = 11111)
{
	auto future = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);

	SpiderServer server(serverPort);
	server.Start();

	std::cout << "Server started ...." << std::endl;

	try
	{
		while (true)
		{
			server.Update(-1, true);

			if (m_isHourTimerFinished)
			{
				m_isHourTimerFinished = false;
				future = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);
			}
		}
	}
	catch (const std::exception& ex)
	{
		Logger::Instance().LogError(ex.what());
	}

	std::cout << "Server stoped ...." << std::endl;
}


int main()
{
	StartServer();

}