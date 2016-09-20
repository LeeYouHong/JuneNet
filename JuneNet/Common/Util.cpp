#include "Util.h"

#include <windows.h>
#include <sstream>

using JuneCommon::CUtil;
using std::stringstream;

bool JuneCommon::CUtil::StringToInt(int& result, const std::string& str)
{
	return StringToNumber(result, str);
}

bool JuneCommon::CUtil::StringToFloat(float& result, const std::string& str)
{
	return StringToNumber(result, str);
}

bool JuneCommon::CUtil::StringToBool(bool& result, const std::string& str)
{
	return StringToNumber(result, str);

}

bool JuneCommon::CUtil::StringToLong(long& result, const std::string& str)
{
	return StringToNumber(result, str);
}

bool JuneCommon::CUtil::StringToDouble(double& result, const std::string& str)
{	
	return StringToNumber(result, str);
}

std::string JuneCommon::CUtil::GBK2UTF8(const std::string& strGBK)
{
	wchar_t* str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, nullptr, 0);
	str1 = new wchar_t[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, nullptr, 0, nullptr, nullptr);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, nullptr, nullptr);
	std::string strUTF8(str2);
	delete[]str1;
	delete[]str2;

	return strUTF8;
}


std::string JuneCommon::CUtil::UTF82GBK(const std::string& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, nullptr, 0);
	unsigned short * str1 = new unsigned short[len + 1];
	memset(str1, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (const char*)strUTF8.c_str(), -1, (LPWSTR)str1, len);
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str1, -1, nullptr, 0, nullptr, nullptr);
	char *str2 = new char[len + 1];
	memset(str2, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str1, -1, str2, len, nullptr, nullptr);
	std::string strGBK(str2);
	delete[]str1;
	delete[]str2;

	return strGBK;
}

bool JuneCommon::CUtil::CheckTimeOut(int startTime, int endTime, int elapseTime)
{
	if (startTime - endTime >= elapseTime)
		return true;

	return false;
}
