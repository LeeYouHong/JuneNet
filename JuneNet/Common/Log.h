/*
FileName: Log.h
Description: »’÷æ¥Ú”°
Autor: LeeYouHong
*/

#pragma once

#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <sstream>
#include <mutex>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>


#include "Singleton.h"

namespace JuneCommon
{
	class CLog
	{
	public:
		CLog()
		{
		};

		~CLog()
		{
			LogFile.close();
		};

		bool Init()
		{
			std::stringstream fileName;
			fileName << "Log/";
			fileName << TimeForNow();
			fileName << ".txt";
			std::string tmp = fileName.str();
			LogFile.open(fileName.str(), std::fstream::out | std::fstream::app);
			if (!LogFile.is_open())
			{
				IsInit = false;
				std::cout << "Open log.txt " << std::endl;
				return false;
			}

			IsInit = true;
			return true;
		}

		template<class T>
		void _Log(std::stringstream& logger, T& t)
		{

			logger << t;
		}

		template<class T, class ... Args>
		void _Log(std::stringstream& logger, T& t, Args ... data)
		{
			logger << t;
			_Log(logger, data...);
		}

		template<class ... Args>
		void LogInfo(const Args& ... data)
		{
			std::stringstream logger;
			logger << "[Info ] [" << TimeForNowMS() << "]";
			_Log(logger, data...);

			LoggerMutex.lock();
			std::cout << logger.str() << std::endl;
			LogFile << logger.str() << std::endl;
			LoggerMutex.unlock();
		}

		template<class ... Args>
		void LogError(const Args& ... data)
		{
			std::stringstream logger;
			logger << "[Error] [" << TimeForNowMS() << "]";
			_Log(logger, data...);

			LoggerMutex.lock();
			std::cout << logger.str() << std::endl;
			LogFile << logger.str() << std::endl;
			LoggerMutex.unlock();
		}

		template<class ... Args>
		void LogWarn(const Args& ... data)
		{
			std::stringstream logger;
			logger << "[Warn ] [" << TimeForNowMS() << "]";
			_Log(logger, data...);

			LoggerMutex.lock();
			std::cout << logger.str() << std::endl;
			LogFile << logger.str() << std::endl;
			LoggerMutex.unlock();

		}

		template<class ... Args>
		void LogRelease(const Args& ... data)
		{
			std::stringstream logger;
			logger << "[Real ] [" << TimeForNowMS() << "]";
			_Log(logger, data...);

			LoggerMutex.lock();
			std::cout << logger.str() << std::endl;
			LogFile << logger.str() << std::endl;
			LoggerMutex.unlock();

		}
	private:
		std::string TimeForNow();
		std::string TimeForNowMS();

		std::mutex LoggerMutex;
		std::ofstream LogFile;

	public:
		bool IsInit = false;
	};
}

using s_LogManager = JuneNet::Singleton < JuneCommon::CLog > ;

#endif //__LOG_H__
