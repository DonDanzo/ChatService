// SpiderClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "SpiderClient.h"
#include "Logger.h"
#include "Helper.h"
#include "messages.pb.h"

using namespace std::chrono_literals;


void SpiderClient::ProcessIncomeMessages()
{
	if (Incoming().IsEmpty())
		return;

	//TODO: investigate why structure binding doesnt work when I use that:
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
		userMsg.ParseFromArray(commMmsg.GetBody().data(), static_cast<int>(commMmsg.GetBody().size()));

		Logger::Instance().Log("SpiderClient::ProcessIncomeMessages()", userMsg);
	}
	std::cout << "<--:{" << commMsgType << "}, [" << userMsg.name() << "] : \"" << userMsg.data() << ".\"" << std::endl;
}

void ClearLastConsoleLine() {
    //// Clear the current line and return the cursor to the beginning of it
    std::cout << "\033[2K\033[0G" << std::flush;
}

//TODO: that function can be done better, for now for the tests is enought
void InitializeClientData(std::string& userName, std::string& serverAddress, uint16_t& serverPort)
{
    bool isInitializationDone = false;
    while (!isInitializationDone)
    {
        std::cout << "Please insert your username: ";
        std::cin >> userName;

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

void PrintToConsole(asio::io_context& io_context, const std::string& msg, std::string param = "")
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

bool m_isHourTimerFinished = false;
std::atomic<bool> m_isInactiveTimerFinished = false;
static void HandleOnTimer(size_t timerId)
{
        LOG_DEBUG_PARAM("Handle Timer with ID: ", timerId)
            Logger::Instance().Log("Handle Timer with ID: ", std::to_string(timerId));

       if (timerId == Defs::NewHourTimerId)
       {
           m_isHourTimerFinished = true;
           //close the file
           Logger::Instance().CloseFile();
           //open it again, it will create new files, becasue it is new hour and name will be different
           Logger::Instance().OpenFile();
       }
        
        if (timerId == Defs::InactivityTimerId)
        {
            m_isInactiveTimerFinished = true;
            Logger::Instance().Log("Client InAvailability timer occurs!");
        }        
}


static std::future<void> StartTimerAsync( std::chrono::duration<int> duration, size_t timerId, std::function<void(size_t)> callback, std::atomic<bool>& stopFlag)
{
    return std::async(std::launch::async, [duration, timerId, callback, &stopFlag]() 
     {
        while (!stopFlag) 
        {  // Keep running until stopFlag is set to true
            std::this_thread::sleep_for(duration);
            if (!stopFlag) { // Check the stop flag again after sleep
                callback(timerId);
            }
        }
    });
}



void StartClient()
{
    SpiderClient client;

    std::string userName;
    std::string serverAddress =  "127.0.0.1";
    uint16_t serverPort = 11111;


    InitializeClientData(userName, serverAddress, serverPort);

    client.Connect(serverAddress, serverPort);

    ChatMessages::UserMessage userMsg = Helper::CreateUserMessage(
        ChatMessages::MessageType::Client,
        userName,
        Helper::GetLocalMachineAddress(),
        Helper::CalculateTimeStamp(),
        Defs::ConnectHandshakeMessageValue);

    client.Send(userMsg);

    bool shouldWrite = false;
    std::string msgDataStr;
    int cnt = 0;

    asio::io_context ioContext;
    RunInBackground(ioContext);

    //start timers for logging and inactivity
    auto futureLogFileHourTimer = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);
    auto futureInactivityTimer = StartTimerAsync(std::chrono::seconds(Defs::InactivityTimeSeconds), Defs::InactivityTimerId, HandleOnTimer, m_isInactiveTimerFinished);
    Logger::Instance().Log("Log and activity timers started!");

    while (true)
    {
        if (m_isHourTimerFinished)
        {
            m_isHourTimerFinished = false;
            futureLogFileHourTimer = Helper::StartHourTimer(Defs::NewHourTimerId, HandleOnTimer);
        }

        if (m_isInactiveTimerFinished)
        {
            break;
        }


        shouldWrite = GetAsyncKeyState(VK_F1) & 0x8000;//press F1 key to input data message via keyboard
        //std::cout <<"should: "<< shouldWrite;
        if (client.IsConnected())
        {
            if (shouldWrite)
            {
                if (GetForegroundWindow() == GetConsoleWindow())
                {
                    shouldWrite = false;

                    // std::cin.clear();
                    // ClearLastConsoleLine();
                    getline(std::cin, msgDataStr);

                    if (msgDataStr.empty())//TODO: something strange behavior, need to be investigated
                        continue;

                    userMsg.set_timestamp(Helper::CalculateTimeStamp());
                    userMsg.set_data(msgDataStr);
                    client.Send(userMsg);

                    //logic for reseting timer doesnt work 
                    // TODO: investigate or find better way
                    //if (m_isInactiveTimerFinished == false)
                    //{
                    //    m_isInactiveTimerFinished = true;//stop the timer
                    //    m_isInactiveTimerFinished = false;//put it false again before start imer
                    //    //StartTimerAsync(std::chrono::seconds(Defs::InactivityTimeSeconds), Defs::InactivityTimerId, HandleOnTimer, m_isInactiveTimerFinished);//restart timer
                    //    Logger::Instance().Log("Inactive timer restarted!");
                    //}
                }
            }
            client.ProcessIncomeMessages();
            continue;
        }
        break;//if client is disconnected break the loop, and program will ends
    }

    client.Disconnect();
    ioContext.stop();
    std::cout << "Program ended \n";
    Logger::Instance().Log("Client connection closed!");
    std::_Exit(1);//TODO: check which resource keeps app to not over
}


int main()
{
    StartClient();
}
