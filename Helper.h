#pragma once

#include "Messages.h"
#include "Timer.h"

namespace Helper
{
    static std::string CalculateTimeStamp()
    {
        std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm calendar_time;
        localtime_s(&calendar_time, &now_time);


        return (calendar_time.tm_mday < 10 ? "0" : "") + std::to_string(calendar_time.tm_mday) + "-"// if day is in range [1:9] add "0" in front
            + (calendar_time.tm_mon < 10 ? "0" : "") + std::to_string(calendar_time.tm_mon) + "-" //if month is in range [1:9] add "0" in front
            + std::to_string(calendar_time.tm_year + 1900) + " "
            + std::to_string(calendar_time.tm_hour) + ":"
            + std::to_string(calendar_time.tm_min) + ":"
            + std::to_string(calendar_time.tm_sec);
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


    static auto CalculateTimerDuration()
    {
        const std::time_t now = std::time(nullptr);                             // get the current time point
        std::tm calendar_time;
        auto res = localtime_s(&calendar_time, std::addressof(now));     // convert it to (local) calendar time

        return Defs::HourSeconds - (calendar_time.tm_min * Defs::MinuteSeconds + calendar_time.tm_sec);
    }

    static std::future<void> StartHourTimer(const size_t& timerId, std::function<void(size_t)> callback)
    {
        return Timer::StartTimerAsync(std::chrono::seconds(CalculateTimerDuration()), Defs::NewHourTimerId, callback);
    }



}