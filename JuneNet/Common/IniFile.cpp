#include "IniFile.h"

#include <sstream>
#include <fstream>
#include <vector>

using JuneCommon::CIniFile;
using std::string;
using std::stringstream;
using std::ifstream;
using std::vector;


CIniFile::CIniFile(const string& fileName)
	:fileName_(fileName)
{

}

CIniFile::~CIniFile()
{

}

bool CIniFile::Parse()
{
	ifstream ifs;

	ifs.open(fileName_, ifstream::in);
	if (!ifs.is_open())
	{
		return false;
	}

	char szline[MAX_LINE_LENGTH];

	vector<string> lines;
	string sectionName;

	while (ifs.getline(szline, MAX_LINE_LENGTH))
	{
		string line(szline);

		// 去除首尾空白字符
		line = Trim(line);

		// 忽略空行
		if (line.empty())
		{
			continue;
		}

		// 忽略注释
		if (line[0] == ';')
		{
			continue;
		}

		// 判断是否Section
		if (line[0] == '[' && line[line.size() - 1] == ']')
		{
			// 存储当前Section
			CSection section(sectionName, lines);
			sections_.push_back(section);

			// 获得下一个Section信息
			sectionName.assign(line.begin() + 1, line.end() - 1);
			lines.clear();

			continue;
		}
		lines.push_back(line);
	}

	// 存储最后一个Section
	CSection section(sectionName, lines);
	sections_.push_back(section);

	return true;
}

string JuneCommon::CIniFile::Trim(const string& line)
{
	stringstream ss(line);

	string result;
	string temp;

	while (ss >> temp)
	{
		result += temp;
	}


	return result;
}

string CIniFile::GetValue(const std::string& sectionName, const std::string& key, const std::string& defaultValue)
{
	string value = defaultValue;
	for (auto& section : sections_)
	{
		if (section.GetName() == sectionName)
		{
			value = section.GetValue(key);
		}
	}
	return value;
}


//CIniFile::CProperty::CProperty(const std::string& raw)
//	:raw_(raw)
//{
//	auto pos = raw_.find_first_of('=');
//	std::string key(raw_.begin(), raw_.begin() + pos);
//	std::string value(raw_.begin() + pos + 1, raw_.end());
//	kvs_.insert(make_pair(key, value));
//}

CIniFile::CSection::CSection(const string& name, const vector<string>& raws)
	:raws_(raws)
	, name_(name)
{
	for (auto& line : raws_)
	{
		auto pos = line.find_first_of('=');
		if (pos == std::string::npos)
		{
			continue;
		}
		std::string key(line.begin(), line.begin() + pos);
		std::string value(line.begin() + pos + 1, line.end());
		propertys_.insert(make_pair(key, value));
	}
}

string CIniFile::CSection::GetValue(const string& key)
{
	for (auto& pair : propertys_)
	{
		if (pair.first == key)
		{
			return pair.second;
		}
	}

	return "";
}
