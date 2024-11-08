#include "SpiderClient.h"
#include "Logger.h"
#include "messages.pb.h"

void SpiderClient::ProcessIncomeMessages()
{
	if (Incoming().IsEmpty())
		return;
	
	//TODO: investigate why structure binding doesnt workwhen I use that
	//const auto& [connection, msg] = Incoming().PopFront();
	
	const auto pair = Incoming().PopFront();
	auto connection = pair.first;
	auto commMmsg = pair.second;


	std::string commMsgType = "";


	ChatMessages::UserMessage userMsg;
	if (commMmsg.GetHeader().GetType() == Messages::Types::System)
	{
		commMsgType = "System";
		//do some actions if server wants something specific
	}
	else if (commMmsg.GetHeader().GetType() == Messages::Types::Client)
	{
		commMsgType = "Client";
		userMsg.ParseFromArray(commMmsg.GetBody().data(), commMmsg.GetBody().size());

		Logger::Instance().Log(userMsg);
	}

	std::cout << "Income message with Type:[" << commMsgType << ", size:[" << commMmsg.GetHeader().GetSize() << "], Data:[" << userMsg.data() <<"]."<< std::endl;



}