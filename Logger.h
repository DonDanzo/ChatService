#pragma once

#include <string>
#include <fstream> 
#include <mutex>
#include "messages.pb.h"


class Logger
{
public:
    static Logger& Instance();

    void OpenFile();
    void CloseFile();
    void Log(const ChatMessages::UserMessage& msg);

private:
    Logger() { OpenFile(); }
     Logger(const Logger&) = delete;
     Logger& operator=(const Logger&) = delete;
     void CalculateFileName();

     std::string        m_fileName;
     std::ofstream      m_file;
     std::mutex         m_mutex;
};


