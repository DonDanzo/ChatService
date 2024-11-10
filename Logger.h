#pragma once

#include <string>
#include <fstream> 
#include <mutex>
#include "messages.pb.h"
#include "Messages.h"

class Logger
{
public:
    static Logger& Instance();// the class is singleton

    void OpenFile();// open file, where log will be written
    void CloseFile();//close current open files, where log is writting
    void Log(const std::string& msg1, const std::string& msg2 = "", const std::string& msg3 = "");// log any string message
    void Log(const std::string& caller, const ChatMessages::UserMessage& msg);//log protobuf's UserNessages
    void Log(const std::string& caller, Messages::CommunicationMessage& msg);//log communication message that contains protobuf's message
    void LogError(const std::string& msg1, const std::string& msg2 = "", const std::string& msg3 = "");//log errors

private:
    Logger() { OpenFile(); }    // open first file wen Instance is called, because it is Static and will be caleed when it is called for first time
     Logger(const Logger&) = delete;
     Logger& operator=(const Logger&) = delete;
     void CalculateFileName();//calculate the name of the file, which will be open

     std::string        m_fileName;
     std::ofstream      m_file;
     std::mutex         m_mutex;
};


