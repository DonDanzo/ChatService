#pragma once

#include "Messages.h"
#include "Timer.h"
#include <asio/asio.hpp>

namespace Helper
{

    //gets current time, and create it in string format for the log
    static std::string CalculateTimeStamp()
    {
        std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm calendar_time;
        localtime_s(&calendar_time, &now_time);


        return (calendar_time.tm_mday < 10 ? "0" : "") + std::to_string(calendar_time.tm_mday) + "-"// if day is in range [1:9] add "0" in front
            + (calendar_time.tm_mon < 10 ? "0" : "") + std::to_string(calendar_time.tm_mon) + "-" //if month is in range [1:9] add "0" in front
            + std::to_string(calendar_time.tm_year + 1900) + " "
            + (calendar_time.tm_hour < 10 ? "0" : "") + std::to_string(calendar_time.tm_hour) +  ":"//if hour is in range [1:9] add "0" in front
            + (calendar_time.tm_min < 10 ? "0" : "") + std::to_string(calendar_time.tm_min) + ":"//if minute is in range [1:9] add "0" in front
            + (calendar_time.tm_sec < 10 ? "0" : "") + std::to_string(calendar_time.tm_sec);//if second is in range [1:9] add "0" in front
    }


	//actually protobuf's  ChatMessages::UserMessage will be used not only for user messages
	//it will be used for System messages too
	//TODO: investigate if protobuf's messages can somehow to be templated or some more general design to be implemented
	//to create protobu'f structure for System message and use it too
	static ChatMessages::UserMessage CreateUserMessage(ChatMessages::MessageType type,
		std::string userName = "",
		std::string address = "",
		std::string time = "",
		std::string data = ""
	)
	{
		ChatMessages::UserMessage msg;
		msg.set_type(type);
		msg.set_name(userName);
		msg.set_ipaddress(address);//TODO: not implemented yet, just for test    
		msg.set_timestamp(time);
		msg.set_data(data);

		return msg;
	}

    //calculate one hour timer duration
    //if application is tarted int 10:30, it will calculate 30 minutes timer, to close log file and created new one
    static auto CalculateTimerDuration()
    {
        const std::time_t now = std::time(nullptr);                             // get the current time point
        std::tm calendar_time;
        auto res = localtime_s(&calendar_time, std::addressof(now));     // convert it to (local) calendar time

        return Defs::HourSeconds - (calendar_time.tm_min * Defs::MinuteSeconds + calendar_time.tm_sec);
    }

    //there was idea that to be more general, but for now it seems not need
    //TODO: remove it and use only StartTimerAsync if it is useless
    static std::future<void> StartHourTimer(const size_t& timerId, std::function<void(size_t)> callback)
    {
        return Timer::StartTimerAsync(std::chrono::seconds(CalculateTimerDuration()), Defs::NewHourTimerId, callback);
    }



    //get ip address of local machine and conert it in string
    static std::string GetLocalMachineAddress()
    {
        try 
        {
            asio::io_context io_context;
            std::string hostname = asio::ip::host_name();
            asio::ip::tcp::resolver resolver(io_context);
            asio::ip::tcp::resolver::query query(hostname, "");

            for (auto it = resolver.resolve(query); it != asio::ip::tcp::resolver::iterator(); ++it) 
            {
                asio::ip::address address = it->endpoint().address();
                if (address.is_v4()) // Only print IPv4 addresses
                {  
                    return address.to_string();
                }
                else if (address.is_v6()) // Only print IPv6 addresses
                {  
                    return address.to_string();
                }
            }
        }
        catch (std::exception& e) 
        {
            Logger::Instance().LogError(e.what());
        }
        return "";
    }
}