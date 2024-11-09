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

		Logger::Instance().Log(userMsg);
	}

	std::cout << "Income message with Type:[" << commMsgType << ", size:[" << commMmsg.GetHeader().GetSize() << "], Data:[" << userMsg.data() << "]." << std::endl;
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

        serverAddress = "127.0.0.1";
        serverPort = 11111;

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

void StartClient()
{
    SpiderClient client;

    std::string userName;
    std::string serverAddress;
    uint16_t serverPort;
    InitializeClientData(userName, serverAddress, serverPort);

    client.Connect(serverAddress, serverPort);

    ChatMessages::UserMessage userMsg = Helper::CreateUserMessage(
        ChatMessages::MessageType::Client,
        userName,
        "127.4.3.2",
        Helper::CalculateTimeStamp(),
        Defs::ConnectHandshakeMessageValue);

    client.Send(userMsg);

    bool shouldWrite = false;
    std::string msgDataStr;
    int cnt = 0;

    asio::io_context ioContext;
    RunInBackground(ioContext);

    while (true)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));

        shouldWrite = GetAsyncKeyState(VK_F1) & 0x8000;//press F1 key to input data message via keyboard
        if (client.IsConnected())
        {
            if (shouldWrite)
            {
                if (GetForegroundWindow() == GetConsoleWindow())
                {
                    bool res2 = GetAsyncKeyState(VK_F1) & 0x8000;
                    shouldWrite = false;

                    // std::cin.clear();
                    // ClearLastConsoleLine();
                    getline(std::cin, msgDataStr);


                    userMsg.set_timestamp(Helper::CalculateTimeStamp());
                    userMsg.set_data(msgDataStr);
                    client.Send(userMsg);
                }
            }

            client.ProcessIncomeMessages();
            continue;
        }
        Logger::Instance().Log("Client connection closed!");
        break;//if client is disconnected break the loop, and program will ends

    }

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
int main()
{
    StartClient();

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
