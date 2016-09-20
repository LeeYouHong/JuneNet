#include "Log.h"

#include <windows.h>
#include <stdio.h>
#include <time.h>

std::string JuneCommon::CLog::TimeForNow()
{
	auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto now = std::put_time(std::localtime(&t), "%Y-%m-%d_%H.%M.%S");

	std::stringstream ss;
	ss << now;

	return ss.str();
}

std::string JuneCommon::CLog::TimeForNowMS()
{
	SYSTEMTIME lpsystime;
	GetLocalTime(&lpsystime);

	std::stringstream ss;
	ss << lpsystime.wYear << "-" << lpsystime.wMonth << "-" << lpsystime.wDay << " " << lpsystime.wHour << "." << lpsystime.wMinute << "." << lpsystime.wSecond << "." << lpsystime.wMilliseconds;

	return ss.str();
}
