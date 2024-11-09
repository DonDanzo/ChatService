#pragma once
#include "Server.h"

class SpiderServer : public Server
{
public:
	SpiderServer(uint16_t port) : Server(port)
	{}
	virtual bool OnClientConnect(const std::string& clientName, std::shared_ptr<Connection> client) override;
	virtual void OnClientDisconnect(const std::string& clientName, std::shared_ptr<Connection> client) override;
	virtual void OnMessage(std::shared_ptr<Connection> client, Messages::CommunicationMessage& msg) override;

	void KickUser(std::string& userName);

};

