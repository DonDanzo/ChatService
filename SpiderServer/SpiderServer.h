#pragma once
#include "Server.h"

class SpiderServer : public Server
{
public:
	SpiderServer(uint16_t port) : Server(port)
	{}

	//void OnClientTryingConnect(std::shared_ptr<Connection> client) override;
	bool OnClientConnected(const std::string& userName, std::shared_ptr<Connection> client) override;
	void OnClientDisconnect(std::shared_ptr<Connection> client) override;
	void OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg) override;

	void KickUser(std::string& userName);

};

