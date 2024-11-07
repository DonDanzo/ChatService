// ChatService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "messages.pb.h"
#include "Logger.h"
#include "Defs.h"
#include "Timer.h"

#include "Client.h"
#include "Server.h"

using namespace std::chrono_literals;

void TestLoggerWithProtobuf()
{
    LOG_DEBUG("TestLoggerWithProtobuf ")

    ChatMessages::UserMessage* msg = new ChatMessages::UserMessage();
    std::vector<ChatMessages::UserMessage> messages;
    messages.resize(10);

    static int id = 0;
    std::string name("a");
    char letter = 'A';

    for (auto& msg : messages)
    {
        msg.set_type(ChatMessages::MessageType::Client);
        name += (++letter);
        msg.set_name(name);
        msg.set_ipaddress("44.33.22.11");
        msg.set_timestamp("2024ta godina");
        msg.set_data(  letter % 2 ? "DA " : " AMA NE");
    }

    for (const auto& msg : messages)
    {
        Logger::Instance().Log(msg);
    }
}

#pragma region  LOGGER_TIMER
bool m_isHourTimerFinished = false;
auto CalculateTimerDuration()
{
    const std::time_t now = std::time(nullptr);                             // get the current time point
    std::tm calendar_time;
    auto res = localtime_s(&calendar_time, std::addressof(now));     // convert it to (local) calendar time

    return Defs::HourSeconds - (calendar_time.tm_min * Defs::MinuteSeconds + calendar_time.tm_sec);
}
void HandleOnTimer(size_t timerId)
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
std::future<void> StartHourTimer(const size_t& timerId)
{
    return Timer::StartTimerAsync(std::chrono::seconds(CalculateTimerDuration()), Defs::NewHourTimerId, HandleOnTimer);
}
#pragma endregion


void StartClient()
{
    ChatMessages::UserMessage msg;
    msg.set_type(ChatMessages::MessageType::Client);
    msg.set_name("0000000000");
    msg.set_ipaddress("8888888888");
    msg.set_timestamp("9999999999");
    msg.set_data("AaAaAaAaAa");

    Client client;
    client.Connect("127.0.0.1", 23);

    //client.Wait();

    client.Send(msg);


    while (true)
    {
        client.Send(msg);
        if (client.IsConnected())
        {
            continue;
        }
        break;
    }

}
void StartServer(size_t serverPort = 11111)
{
    Server server(serverPort);
    server.Start();

    while (true)
    {
        server.Update(-1, true);
    }
}



int main(int argc, char* argv [])
{
    StartServer();
    StartClient();

    auto future = StartHourTimer(Defs::NewHourTimerId);
    int cnt = 0;
    while (true)
    {
        LOG_DEBUG_PARAM("cnnt: ", ++cnt)
        TestLoggerWithProtobuf();
        std::this_thread::sleep_for(10s);
        if (m_isHourTimerFinished)
        {
            m_isHourTimerFinished = false;
            future = StartHourTimer(Defs::NewHourTimerId);
        }
    }

}
