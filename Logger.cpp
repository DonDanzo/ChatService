#include "Logger.h"
#include "Defs.h"

Logger& Logger::Instance()
{
    static Logger s_instance;
    return s_instance;
}

void Logger::CalculateFileName()
{
    const std::time_t now = std::time(nullptr);                             // get the current time point
    std::tm calendar_time;
    auto res = localtime_s(&calendar_time, std::addressof(now));     // convert it to (local) calendar time

    if (res != 0)
    {
        //TODO: do some actions depending returned result
        LogError("Failed converting local time to calendar_time object");
        return;
    }

    m_fileName.clear();
    m_fileName.assign(Defs::LogFileHeadNameTemplate
        + std::to_string(calendar_time.tm_year)
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


void Logger::Log(const std::string& msg1, const std::string& msg2, const std::string& msg3)
{
    if (!m_file.is_open())
    {
        LogError(msg1, msg2, msg3);
    }
    std::cout << Defs::LogInfoHead << msg1 << msg2 << msg3 << std::endl;
}
void Logger::Log(const ChatMessages::UserMessage& msg)
{
    if (!m_file.is_open())
    {
        LogError(msg.DebugString());
    }
    m_file << "MsgType:[" << msg.type() << "], UserName:[" << msg.name() << "], IpAddress:[" << msg.ipaddress() << "], Time:[" << msg.timestamp() << "], Message:[" << msg.data() << "]" << std::endl;
}
void Logger::LogError(const std::string& msg1, const std::string& msg2, const std::string& msg3)
{    
    std::cerr << "ERROR: ";
    if (!m_file.is_open())
    {
        std::string errMsg = "Log: Not valid ofstream file, file is not open()";
        std::cerr << errMsg << std::endl;
        throw(errMsg);
    }
    std::cerr << msg1 << msg2 << msg3 << "\n";
}
