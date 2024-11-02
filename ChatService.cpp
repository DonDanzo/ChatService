// ChatService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "messages.pb.h"

void TestIsProtobufWorks()
{
    std::string ip("1.200.199.55");
    std::string name("DonDanzo");
    std::string time("2024-11-02 18:28:11");

    ChatMessages::UserMessage* msg = new ChatMessages::UserMessage();

    msg->set_m_userid(321);
    msg->set_m_username(name);
    msg->set_m_useripaddress("44.33.22.11");
    msg->set_m_usersenttime(time);

    std::cout << "Hello World!\n";

    auto resultID = msg->m_userid();
    auto resultName = msg->m_username();
    auto resultIP = msg->m_useripaddress();
    auto resultTIME = msg->m_usersenttime();

    std::cout << "resultID: " << resultID << std::endl;
    std::cout << "resultNAME: " << resultName << std::endl;
    std::cout << "resultIP: " << resultIP << std::endl;
    std::cout << "resultTIME: " << resultTIME << std::endl;
}



int main()
{
    TestIsProtobufWorks();
}
