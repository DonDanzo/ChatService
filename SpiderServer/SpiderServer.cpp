// SpiderServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "SpiderServer.h"
#include "Logger.h"
#include "Helper.h"
using namespace std::chrono_literals;


bool SpiderServer::OnClientConnect(const std::string& clientName, std::shared_ptr<Connection> client)
{
	LOG_DEBUG("Server::OnClientConnect()")
		//TODO: Some stronger check/handshake, Send req and wait expected response
		Logger::Instance().Log("[SERVER] Client connected");

	//handshake was successfull, client connected to server
	client->ConnectToClient(m_clientsCount++);
	m_dequedConnections.push_back(std::pair<std::string, std::shared_ptr<Connection>>(clientName, std::move(client.get())));


	//create system message by server, to notify all users somebody has connected
	std::string systemMessage = "User " + clientName + " has joined in the Chat!";

	auto msg = Helper::CreateUserMessage(ChatMessages::MessageType::System, clientName, "", Helper::CalculateTimeStamp(), systemMessage);
	Messages::CommunicationMessage commMsg(Messages::Types::System, msg);
	MessageAllClients(commMsg, client);

	return true;
}

void SpiderServer::OnClientDisconnect(const std::string& clientName, std::shared_ptr<Connection> client)
{
	LOG_DEBUG("Server::OnClientDisconnect()")
		Logger::Instance().Log("[SERVER] Disconnected client: ", std::to_string(client->GetID()));

	//create system message by server, to notify all users somebody has diconnected
	std::string systemMessage = "User " + clientName + " has left the Chat!";

	auto msg = Helper::CreateUserMessage(ChatMessages::MessageType::System, clientName, "", Helper::CalculateTimeStamp(), systemMessage);
	Messages::CommunicationMessage commMsg(Messages::Types::System, msg);
	MessageAllClients(commMsg, client);

}

// Called when a message arrives
void SpiderServer::OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg)
{
	LOG_DEBUG("Server::OnMessage()")

		ChatMessages::UserMessage userMsg;
	userMsg.ParseFromArray(msg.GetBody().data(), msg.GetBody().size());

	if (client->GetConnectionState() == State::None)//that state is not expected, disconnect the cllient
	{
		DisconnectClient(userMsg.name(), client);
		return;
	}
	else if (client->GetConnectionState() == State::RequestToConnect)//it recevie first message for this client, expect handshake to verify the client
	{
		if (userMsg.data() != Defs::ConnectHandshakeMessageValue)//if received 1st message from client is not expected, go disconnect it
		{
			DisconnectClient(userMsg.name(), client);
			return;
		}

		OnClientConnect(userMsg.name(), client);
	}
	else if (client->GetConnectionState() == State::Connected)
	{
		MessageAllClients(msg, client);//send the message too all connected clients, ignore current client
	}
}



void SpiderServer::KickUser(std::string& userName)
{


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
	SpiderServer server(serverPort);
	server.Start();

	while (true)
	{
		server.Update(-1, true);
	}
}


int main()
{
	StartServer();


	auto future = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);
	int cnt = 0;
	while (true)
	{
		LOG_DEBUG_PARAM("cnnt: ", ++cnt)

			std::this_thread::sleep_for(10s);
		if (m_isHourTimerFinished)
		{
			m_isHourTimerFinished = false;
			future = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);
		}
	}
}