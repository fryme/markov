#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cwctype>
#include <algorithm>
#include <iterator>
#include <list>
#include "model.h"
#include <boost\format.hpp>
#include <boost\program_options.hpp>

using namespace std;
const wstring CurlPath = L"C:\\github\\markov\\markov\\Release\\curl.exe";

template<class T, class T2>
void PreprocessString(vector<T>& buffer, vector<T2>& result)
{
	for (size_t i = 0; i < buffer.size() ; ++i)
	{
		if (buffer[i] == '\'' || iswspace(buffer[i])) 
		{
			result.push_back(buffer[i]);
			continue;
		}

		if (std::iswupper(buffer[i]))
			result.push_back(towlower(buffer[i]));
		else if (iswlower(buffer[i]))
			result.push_back(buffer[i]);
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
		//PreprocessString(buffer);
		copy(buffer.begin(), buffer.end(), back_inserter(result));

		if (stream.fail())
			break;
	}
	return result;
}

inline bool IsFileExists(const std::wstring& name) {
	ifstream stream(name);
	return stream.is_open();
}

vector<wchar_t> DownloadUrl(const wstring& url)
{
	FILE* fp;
	if ((fp = _wpopen((boost::wformat(L"\"%1%\" -s --url %2%") % CurlPath % url).str().c_str(), L"rt")) == NULL)
		throw runtime_error("can't download file");

	const uint32_t BufferSize = 1024 * 100; // 100 kb
	vector<char> buffer(BufferSize, 0);
	vector<wchar_t> result;
	result.reserve(BufferSize);
	size_t readed = 0;

	while (true)
	{
		readed = fread(&buffer[0], sizeof buffer[0], buffer.size(), fp);
		if (readed < BufferSize)
			buffer.resize(readed);

		PreprocessString(buffer, result);

		if (readed != BufferSize)
			break;
	}
	
	_pclose(fp);
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

void ReadLinks(const string& filePath, list<wstring>& links)
{
	wifstream stream(filePath/*, std::ifstream::ate*/);
	if (!stream.is_open())
		throw std::runtime_error("file not open");
	
	wstring line;
	while (std::getline(stream, line))
		links.push_back(line);
}

int main(int argc, char** argv)
{
	namespace po = boost::program_options;
	po::options_description description("Allowed options");
	description.add_options()("order", po::value<uint32_t>(), "order of model")
							 ("urls", po::value<string>(), "path to txt file with links to download")
							 ("out", po::value<string>(), "path to file of result model");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, description), vm);
	po::notify(vm);

	if (vm.count("help") || vm["order"].empty() || vm["urls"].empty() || vm["out"].empty()) 
	{
		cout << description << endl;
		return 1;
	}

	uint32_t order = vm["order"].as<uint32_t>();
	string pathToResultModel(vm["out"].as<string>());

	try
	{
		list<wstring> links;
		ReadLinks(vm["urls"].as<string>(), links);

		if (links.empty())
			throw runtime_error("No links");

		if (!IsFileExists(CurlPath))
			throw runtime_error("curl.exe not exists");

		MarkovChainModel completeModel(order);
		
		while (!links.empty())
		{
			auto link = links.back();
			links.pop_back();
			try
			{
				MarkovChainModel tempModel(order);
				if (tempModel.CreateModel(DownloadUrl(link)))
				{
					wcout << "Model for link: " << link << " created with size: " << tempModel.GetSize() << endl;
					completeModel.Merge(tempModel);
					wcout << "Total size after merge: " << completeModel.GetSize() << endl;
				}
			}
			catch (std::exception& e)
			{
				wcout << "Failed download link: " << link <<" error: " << e.what() << endl;
			}
		}
		
		cout << "Model saved to file: " << pathToResultModel << " file size: " << completeModel.Save(pathToResultModel);

	}
	catch (std::exception& e)
	{
		cout << "Fail: " << e.what();
		return 1;
	}

	return 0;
}