#include "Timer.h"
#include <windows.h>

JuneNet::CTimer::CTimer(__int64 elapse, std::function<void ()> fun)
{
	TimeOutTime = elapse + GetTickCount();
	Fun = fun;
}

bool JuneNet::CTimer::IsTimeOut()
{
	if (TimeOutTime < GetTickCount())
	{
		return true;
	}
	return false;
}

void JuneNet::CTimer::Expire()
{
	IsExpire = true;
}

void JuneNet::CTimerManager::AddTimer(CTimer& timer)
{
	TimerList.push_back(timer);
}

void JuneNet::CTimerManager::ClearExpireTimer()
{
	for (auto iter = TimerList.begin(); iter != TimerList.end();)
	{
		if (iter->IsExpire)
		{
			iter = TimerList.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void JuneNet::CTimerManager::Run()
{
	for (auto& timer : TimerList)
	{
		if (timer.IsTimeOut())
		{
		timer.Fun();
			timer.Expire();
		}
	}

	ClearExpireTimer();
}

void JuneNet::CTimerManager::Clear()
{
	TimerList.clear();
}
