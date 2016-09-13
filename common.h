#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <locale>
#include <codecvt>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;
const wstring CurlPath = L"curl.exe";
const uint32_t BufferSize = 1024 * 100; // 100 kb

#define Check(eval, message) if (eval) throw std::runtime_error(message)

/*
vector<wchar_t> ReadFile(const wstring& fileName)
{
	wifstream stream(fileName, std::ifstream::ate);
	if (!stream.is_open())
		throw std::runtime_error("file not open");

	size_t fileSize = stream.tellg();
	const uint32_t BufferSize = min(fileSize, (size_t)1024);

	vector<wchar_t> result;
	result.reserve(fileSize);

	vector<wchar_t> buffer(BufferSize, 0);
	stream.seekg(0, ios::beg);

	while (true)
	{
		stream.read(&buffer[0], buffer.size());
		//PreprocessString(buffer);
		copy(buffer.begin(), buffer.end(), back_inserter(result));

		if (stream.fail())
			break;
	}
	return result;
}*/

/*
void ReadFiles(MarkovChainModel& completeModel)
{
	list<wstring> textFiles = { L"./test_data/medium_en2.txt", L"./test_data/medium_en3.txt" };
	//list<wstring> textFiles = { L"./test_data/small.txt" };
	while (!textFiles.empty())
	{
		const wstring filePath = textFiles.back();
		textFiles.pop_back();

		auto buffer = ReadFile(filePath);
		MarkovChainModel tempModel(2);
		tempModel.CreateModel(buffer);
		completeModel.Merge(tempModel);
	}
}

*/

inline bool IsFileExists(const std::wstring& name) 
{
	ifstream stream(name);
	return stream.is_open();
}

#endif // COMMON_H