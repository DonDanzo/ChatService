// ChatService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "messages.pb.h"
#include "Logger.h"
#include "Defs.h"
#include "Timer.h"

#include "SpiderClient.h"
#include "Server.h"
#include <asio/asio.hpp>
#include <ctime>

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

std::string CalculateTimeStamp()
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

#pragma endregion


#pragma region Client


//TODO: that function can be done better, for now for the tests is enought
void InitializeClientData(std::string& userName, std::string& serverAddress, uint16_t& serverPort )
{
    bool isInitializationDone = false;
    while (!isInitializationDone)
    {
        std::cout << "Please insert your username: ";
        std::cin >> userName;

        serverAddress = "127.0.0.1";
        serverPort = 23;

        std::cout << "Default adress of server is: " << serverAddress << ":" << serverPort << std::endl;
        std::cout << "Do you want to change it ? Press (Y/N) ";

        std::string choice;
        std::cin >> choice;

        if (choice == "Y")
        {
            std::cout << "Please input Server's address:";
            std::cin >> serverAddress;
            std::cout << "Please input Server's port";
            std::cin >> serverPort;
            isInitializationDone = true;
        }
        else if (choice == "N")
        {
            isInitializationDone = true;
        }
        else
        {
            std::cout << "Incorect input, please try again." << std::endl;
        }
    }
}

void PrintToConsole(asio::io_context& io_context, const std::string& msg, std::string param="")
{
    io_context.post([&msg]() {
        std::cout << msg << std::endl;
    });
}
void RunInBackground(asio::io_context& io_context)
{
    std::thread([&io_context]() {
        io_context.run();
        }).detach();
}

void StartClient()
{
    SpiderClient client;

    std::string userName;
    std::string serverAddress;
    uint16_t serverPort;
    InitializeClientData(userName, serverAddress, serverPort);

    client.Connect(serverAddress, serverPort);

    ChatMessages::UserMessage userMsg;
    userMsg.set_type(ChatMessages::MessageType::Client);
    userMsg.set_name(userName);

    userMsg.set_ipaddress("127.4.3.2");//TODO: not implemented yet, just for test
    
    userMsg.set_timestamp(CalculateTimeStamp());

    userMsg.set_data("Hello!");

    //client.Send(msg);

    bool shouldWrite = false;
    std::string msgDataStr;
    int cnt = 0;

    asio::io_context ioContext;
    RunInBackground(ioContext);

    while (true)
    {
     
        shouldWrite = GetAsyncKeyState(VK_F1) & 0x8000;
        
        if (client.IsConnected())
        {
            if (shouldWrite)
            {
                PrintToConsole(ioContext, "Please insert your text :");
                if (GetForegroundWindow() == GetConsoleWindow())
                {
                    bool res2 = GetAsyncKeyState(VK_F1) & 0x8000;
                    shouldWrite = false;
                    
                    getline(std::cin, msgDataStr);

                    PrintToConsole(ioContext, "\nInputed text: ", msgDataStr);

                    userMsg.set_timestamp(CalculateTimeStamp());
                    userMsg.set_data(msgDataStr);
                    client.Send(userMsg);
                }
            }
            
            client.ProcessIncomeMessages();
            continue;
        }
        break;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

}

#pragma endregion

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
    Logger::Instance().Log("ECTRA");

    //StartServer();
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
