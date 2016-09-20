/*
FileName: Timer.h
Description: ¶¨Ê±Æ÷
Autor: LeeYouHong
*/

#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include <vector>

#include "Singleton.h"

namespace JuneNet
{
	class CTimer
	{
	public:
		CTimer(__int64 elapse, std::function<void ()> fun);

		bool IsTimeOut();

		void Expire();

		bool IsExpire = false;

		std::function<void ()> Fun;
	private:
		__int64 TimeOutTime = 0;
	};

	class CTimerManager
	{
	public:
		void AddTimer(CTimer& timer);

		void ClearExpireTimer();

		void Run();

		void Clear();
	private:
		std::vector<CTimer> TimerList;
	};
}

using s_TimerManager = JuneNet::Singleton < JuneNet::CTimerManager >;

#endif