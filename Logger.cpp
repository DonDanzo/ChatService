#include "Logger.h"
#include "Defs.h"
#include "Helper.h"

#include <mutex>

Logger& Logger::Instance()
{
    static Logger s_instance;
    return s_instance;
}

void Logger::CalculateFileName()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm calendar_time;  // Safe to use
    if (localtime_s(&calendar_time, &now_time) != 0)
    {
        LogError("Failed converting local time to calendar_time object");
        return;
    }

    m_fileName.clear();
    m_fileName.assign(Defs::LogFileHeadNameTemplate
        + std::to_string(calendar_time.tm_year - 100)
        + std::to_string(calendar_time.tm_mon)
        + std::to_string(calendar_time.tm_mday)
        + std::to_string(calendar_time.tm_hour)
        + Defs::LogFileTailNameTemplate);
}

void Logger::OpenFile()
{
    CalculateFileName();
    m_mutex.lock();
    LOG_DEBUG("OpenFile()")
    m_file = std::ofstream(m_fileName, std::ofstream::app);//open file in append mode, not ovveride old data
    m_file << std::endl;//first write new line in file
    m_mutex.unlock();
}

void Logger::CloseFile()
{
    m_mutex.lock();
    LOG_DEBUG("CloseFile()")
    m_file.close();
    m_mutex.unlock();
}


//sometimes log messages are written together and needs to check
//is it because threads do it, or it is because clients are 
void Logger::Log(const std::string& msg1, const std::string& msg2, const std::string& msg3)
{
    m_mutex.lock();
    if (!m_file.is_open())
    {
        LogError(msg1, msg2, msg3);
    }
    m_file << "| " << Helper::CalculateTimeStamp() << " |" << Defs::LogInfoHead << msg1 << msg2 << msg3 << std::endl;
    m_mutex.unlock();
}
void Logger::Log(const std::string& caller, const ChatMessages::UserMessage& msg)
{
    m_mutex.lock();
    if (!m_file.is_open())
    {
        LogError(msg.DebugString());
        return;
    }
    m_file << "| " << Helper::CalculateTimeStamp() << " |"
        << caller << "-->"
        << "MsgType:[" 
        << (msg.type() == ChatMessages::MessageType::Client ? "Client" : "System") << "], UserName:["
        << msg.name() << "], IpAddress:["
        << msg.ipaddress() << "], Time:["
        << msg.timestamp() << "], Message:[" 
        << msg.data() << "]" << std::endl;
    m_mutex.unlock();
}
void Logger::Log(const std::string& caller, Messages::CommunicationMessage& commMsg)
{
    m_mutex.lock();
    if (!m_file.is_open())
    {
        //LogError(msg.DebugString());
        return;
    }
    
    ChatMessages::UserMessage userMsg;
    std::string commMsgType = "None";


    if (commMsg.GetHeader().GetType() == Messages::Types::Client)
    {
        commMsgType == "Client";
        userMsg.ParseFromArray(commMsg.GetBody().data(), static_cast<int>(commMsg.GetBody().size()));
    }
    else if (commMsg.GetHeader().GetType() == Messages::Types::System)
    {
        commMsgType == "System";
    }
    
    m_file << "| " << Helper::CalculateTimeStamp() << " |"
        << caller << "-->"
        << "Communication message with type: [" << commMsgType << "], "
        << "UserMsgType:["
        << (userMsg.type() == ChatMessages::MessageType::Client ? "Client" : "System") << "], UserName:["
        << userMsg.name() << "], IpAddress:["
        << userMsg.ipaddress() << "], Time:["
        << userMsg.timestamp() << "], Message:["
        << userMsg.data() << "]" << std::endl;
    m_mutex.unlock();
}
void Logger::LogError(const std::string& msg1, const std::string& msg2, const std::string& msg3)
{
    m_mutex.lock();
    std::cerr << "ERROR: ";
    if (!m_file.is_open())
    {
        std::string errMsg = "Log: Not valid ofstream file, file is not open()";
        std::cerr << errMsg << std::endl;
        return;
        //throw(errMsg);
    }
    std::cerr << "| " << Helper::CalculateTimeStamp() << " |" << msg1 << msg2 << msg3 << "\n";
    m_mutex.unlock();
}
