#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <fstream>
#include <cwctype>
#include <algorithm>
#include <iterator>
#include <list>
#include "model.h"
#include <boost\format.hpp>

using namespace std;

void PreprocessString(vector<wchar_t>& buffer)
{
	size_t w = 0;
	for (size_t i = 0; i < buffer.size() && w < buffer.size(); ++i, ++w)
	{
		if (buffer[w] == '\'')
			continue;

		if (!std::iswpunct(buffer[w]))
			buffer[i] = std::towlower(buffer[w]);
		else
			buffer[i] = L' ';
	}
}

vector<wchar_t> ReadFile(const wstring& fileName)
{
	wifstream stream(fileName, std::ifstream::ate);
	if (!stream.is_open())
		throw std::runtime_error("file not open");
	
	size_t fileSize = stream.tellg();
	const uint32_t BufferSize = min(fileSize, (size_t) 1024);
	
	vector<wchar_t> result;
	result.reserve(fileSize);

	vector<wchar_t> buffer(BufferSize, 0);
	stream.seekg(0, ios::beg);

	while (true)
	{
		stream.read(&buffer[0], buffer.size());
		PreprocessString(buffer);
		copy(buffer.begin(), buffer.end(), back_inserter(result));

		if (stream.fail())
			break;
	}
	return result;
}

vector<wchar_t> DownloadUrl(const wstring& url)
{
	wstring curlPath = L"C:\\github\\markov\\curl.exe";

	FILE* fp;
	if ((fp = _wpopen((boost::wformat(L"\"%1%\" --url %2%") % curlPath % url).str().c_str(), L"rt")) == NULL)
		exit(1);
	
	fseek(fp, 0, SEEK_END); // seek to end of file
	auto size = ftell(fp); // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

	const uint32_t BufferSize = 1024;
	vector<char> buffer(BufferSize, 0);
	vector<wchar_t> result;
	result.reserve(BufferSize);
	size_t readed = 0;
	while (true)
	{
		readed = std::fread(&buffer[0], sizeof buffer[0], buffer.size(), fp);
		copy(buffer.begin(), buffer.end(), back_inserter(result));
		if (readed != BufferSize)
			break;
	}
	
	return result;
}

void ReadFiles(MarkovChainModel& completeModel)
{
	list<wstring> textFiles = { L"./test_data/medium_en2.txt", L"./test_data/medium_en3.txt"};
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

int main()
{
	uint32_t n = 2; // chain order
	//list<wstring> links = { L"ftp://geek@localhost/medium_en.txt"};
	list<wstring> links = { L"ftp://geek@localhost/medium.txt" };
	wstring pathToResultModel(L"C:\\github\\markov\\Debug\\model.txt");
	
	try
	{
		MarkovChainModel completeModel(n);
		
		while (!links.empty())
		{
			auto link = links.back();
			links.pop_back();
			MarkovChainModel tempModel(n);
			tempModel.CreateModel(DownloadUrl(link));
			completeModel.Merge(tempModel);
		}

		completeModel.Save(pathToResultModel);
		MarkovChainModel mm(n);
		mm.Load(pathToResultModel);
		bool isEqual = (mm == completeModel);
	}
	catch (std::exception& e)
	{
		cout << "Fail: " << e.what();
	}
	return 0;
}