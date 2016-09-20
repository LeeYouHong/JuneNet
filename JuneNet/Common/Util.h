/*
FileName: Util.h
Description: 一些常用功能的封装
Autor: LeeYouHong
*/

#pragma once
#ifndef __TOOLFUN_H__
#define __TOOLFUN_H__

#include <string>
#include <sstream>

namespace JuneCommon
{
	class CUtil
	{
	public:
		/*
		* 字符串转换为数字
		*/
		static bool StringToInt(int& result, const std::string& str);

		static bool StringToFloat(float& result, const std::string& str);

		static bool StringToBool(bool& result, const std::string& str);

		static bool StringToLong(long& result, const std::string& str);

		static bool StringToDouble(double& result, const std::string& str);

		/*
		* 编码转换
		*/
		static std::string UTF82GBK(const std::string& strUTF8);

		static std::string GBK2UTF8(const std::string& strGBK);

		static bool CheckTimeOut(int startTime, int endTime, int elapseTime);
		
	private:
		template<class T>
		static bool StringToNumber(T& result, const std::string& str)
		{
			std::stringstream ss(str);

			ss >> result;

			if (ss.fail())
			{
				return false;
			}

			return true;
		}
	};
}


#endif