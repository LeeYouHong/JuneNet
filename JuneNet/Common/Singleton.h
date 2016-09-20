/*
FileName: Singleton.h
Description: Singleton
Autor: LeeYouHong
*/

#pragma once

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

namespace JuneNet
{
	template<class T>
	class Singleton
	{
	public:
		inline static T& Instance()
		{
			static T s_instance;
			return s_instance;
		}
	private:
		Singleton()
		{
		};
		Singleton& operator =(const Singleton&) = delete;
		Singleton(const Singleton&) = delete;
	};
}
#endif