#pragma once
#include <string>
#include <chrono>

#define DEBUG_MODE
#ifdef DEBUG_MODE
#define LOG_DEBUG(message) (std::cout <<(message) << std::endl);
#define LOG_DEBUG_PARAM(message,param) (std::cout <<(message) << param << std::endl);
#endif

namespace Defs
{
	//name of log file should be example: Log_YYYYMMDDHH.log
	const std::string LogFileHeadNameTemplate ("Log_");
	const std::string LogFileTailNameTemplate (".log");
	
	const auto MinuteSeconds{ 60 };
	const auto HourSeconds{ 3600 };
	const auto HourMinutes{ 60 };

	const size_t NewHourTimerId{ 100 };

};