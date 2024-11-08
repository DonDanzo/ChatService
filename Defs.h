#pragma once
#include <string>
#include <vector>

#define DEBUG_MODE
#ifdef DEBUG_MODE
#define LOG_DEBUG(message) (std::cout <<(message) << std::endl);
#define LOG_DEBUG_PARAM(message,param) (std::cout <<(message) << param << std::endl);
#else
#define LOG_DEBUG(message) ;
#define LOG_DEBUG_PARAM(message,param) ;
#endif


namespace Defs
{
	//name of log file should be example: Log_YYYYMMDDHH.log
	const std::string LogFileHeadNameTemplate ("Log_");
	const std::string LogFileTailNameTemplate (".log");


	const std::string LogInfoHead("INFO: ");
	const std::string LogErrorHead("ERROR: ");

	
	const auto MinuteSeconds{ 60 };
	const auto HourSeconds{ 3600 };
	const auto HourMinutes{ 60 };

	const size_t NewHourTimerId{ 100 };

	const auto MaxClientsCount = 1000;
	const auto MaxBodyLength = 5000;//it can be send message with maximum 5000 symbols

};