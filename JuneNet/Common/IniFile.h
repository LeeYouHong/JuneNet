/*
FileName: IniFile.h
Description: Ini�ļ�����
Autor: LeeYouHong
*/

#pragma once

#ifndef __INIFILE_H__
#define __INIFILE_H__

#include <string>
#include <vector>
#include <map>

namespace JuneCommon
{
	class CIniFile
	{
	public:
		CIniFile(const std::string& fileName);
		~CIniFile();

	public:
		// �����ļ�
		bool Parse();

		// ��ȡSection��Key��ֵ
		std::string GetValue(const std::string& sectionName, const std::string& key, const std::string& defaultValue="");

	public:
		class CSection
		{
		public:
			CSection(const std::string& name, const std::vector<std::string>& raws);

		public:
			const std::string& GetName() const{ return name_; }
			std::string GetValue(const std::string& key);

		private:
			std::map<std::string, std::string> propertys_;
			std::vector<std::string> raws_;
			std::string name_;
		};

	private:
		// ȥ��line����β�հ׷�
		std::string Trim(const std::string& line);

	private:
		std::string					fileName_;
		std::vector<CSection>		sections_;
		static const int			MAX_LINE_LENGTH = 255;
	};

}

#endif