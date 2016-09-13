#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <deque>
#include <iterator>
#include <fstream>
#include <random>

#include <boost/algorithm/string.hpp>

#define Check(eval, message) if (eval) throw std::runtime_error(message)

using namespace std;
// Generates random between a and b
int GenerateRandom(int a, int b)
{
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> uniform_dist(a, b);
	return uniform_dist(e1);
}

vector<wstring> SplitString(const wstring& text)
{
	vector<wstring> words;
	wstring testWs(text.begin(), text.end());
	boost::split(words, testWs, /*boost::is_any_of(L"\t ")*/boost::algorithm::is_space(), boost::token_compress_on);
	return words;
}

vector<wstring> SplitString(const vector<wchar_t>& text)
{
	wstring temp(text.begin(), text.end());
	return SplitString(temp);
}

class Sentence
{
public:
	Sentence(const wstring& str, uint32_t size)
	{
		boost::split(m_deque, str, boost::is_any_of(" "));
	}

	Sentence(vector<wstring> words, uint32_t size)
	{
		copy_n(words.begin(), size, back_inserter(m_deque));
	}

	void InsertWord(const wstring& word)
	{
		m_deque.pop_front();
		m_deque.push_back(word);
	}

	wstring GetKey()
	{
		return boost::join(m_deque, L" ");
	}

private:
	deque<wstring> m_deque;
};

class MarkovChainView
{
public:
	MarkovChainView(string filePath, uint32_t order) : m_stream(filePath), m_order(order)
	{
		Check(!m_stream.is_open(), "File not open");
	}

	bool GetNextWord(const wstring& key, wstring& word)
	{
		m_currentLine = m_size / 2;
		
		pair<wstring, forward_list<wstring>> result;
		if (SearchKey(key, 2, m_size, result))
		{
			auto& list = result.second;
			size_t size = distance(list.begin(), list.end());
			size_t pos = size != 0 ? GenerateRandom(0, size - 1) : 0;
			auto randomValue = list.begin();
			advance(randomValue, pos);
			word = *randomValue;
			return true;
		}
		else
		{
			return false;
		}


		// APPEND TO CHAIN ON SUCCESS

		/*
		auto it = m_chain.find(key);
		if (it != m_chain.end())
		{
		auto& list = it->second;
		size_t size = distance(list.begin(), list.end());
		size_t pos = size != 0 ? GenerateRandom(0, size - 1) : 0 ;
		auto randomValue = list.begin();
		advance(randomValue, pos);
		word = *randomValue;
		return true;
		}
		return false;
		*/
	}
private:

	bool SearchKey(const wstring& key, uint32_t l, uint32_t r, pair<wstring, forward_list<wstring>>& result)
	{
		if (r >= l)
		{
			int mid = l + (r - l) / 2;

			wstring line;
			line = GetLine(mid);
			auto currentKeyValue = GetKey(line);
			if (currentKeyValue.first == key)
			{
				result = currentKeyValue;
				return true;
			}

			if (currentKeyValue.first > key) 
				return SearchKey(key, l, mid - 1, result);

			return SearchKey(key, mid + 1, r, result);
		}

		return false;
	}

	bool GetKey(const wstring& line, pair<wstring, forward_list<wstring>> result)
	{
		vector<wstring> words;
		boost::split(words, line, boost::is_any_of(" "));
		auto currentWord = words.begin();

		wstring key;
		for (uint32_t i = 1; i <= m_order; ++i, ++currentWord)
		{
			if (currentWord == words.end())
				return false;

			key += *currentWord;
			if (i != m_order)
				key += L" ";
		}

		return make_pair(key, forward_list<wstring>(currentWord, words.end()));
	}

	wstring GetLine(uint32_t num)
	{
		m_stream.seekg(std::ios::beg);
		for (int i = 0; i < num - 1; ++i)
			m_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		wstring line;
		file >> line;
		return line;
	}

	wfstream m_stream;
	uint32_t m_size;
	uint32_t m_currentLine;
	uint32_t m_order;
};


class MarkovChainModel
{
	typedef map<wstring, forward_list<wstring>> Chain;
	Chain m_chain;

public:
	MarkovChainModel(uint32_t n) : m_order(n) 
	{}

	size_t GetSize()
	{
		return m_chain.size();
	}

	bool CreateModel(vector<wchar_t>& text)
	{
		vector<wstring> words(std::move(SplitString(text)));
		
		Check(m_order > words.size(), "Text is too small");

		Sentence sentence(words, m_order);
		auto currentWord = words.begin();
		std::advance(currentWord, m_order);
		wstring key;

		while (currentWord != words.end())
		{
			key = sentence.GetKey();
			
			auto currentValue = m_chain.find(key);
			if (m_chain.end() != currentValue)
				currentValue->second.push_front(*currentWord);
			else
				m_chain.emplace(key, forward_list<wstring>{ *currentWord } );

			sentence.InsertWord(*currentWord);
			currentWord++;
		}

		return true;
	}

	void Merge(const MarkovChainModel& otherModel)
	{
		if (this == &otherModel)
			return;

		for (auto keyValue : otherModel.m_chain)
		{
			auto current = m_chain.find(keyValue.first);
			if (m_chain.end() != current)
			{
				// merge lists from other model to current
				current->second.splice_after(current->second.before_begin(), keyValue.second);
			}
			else
			{
				m_chain.emplace(keyValue.first, keyValue.second);
			}
		}
	}


	bool Load(const wstring& filePath, bool fullLoad = false)
	{
		m_chain.clear();
		
		wifstream stream(filePath);
		wstring line;
		getline(stream, line);
		uint32_t newOrder = stol(line);
		if (newOrder != m_order)
			return false;
		/*
		getline(stream, line);
		m_size = stol(line);
		if (m_size == 0)
			return false;
		*/
		if (fullLoad)
		{
			while (getline(stream, line))
			{
				vector<wstring> words;
				boost::split(words, line, boost::is_any_of(" "));
				auto currentWord = words.begin();

				wstring key;
				for (uint32_t i = 1; i <= m_order; ++i, ++currentWord)
				{
					if (currentWord == words.end())
						return false;

					key += *currentWord;
					if (i != m_order)
						key += L" ";
				}
				m_chain.emplace(key, forward_list<wstring>(currentWord, words.end()));
			}
		}
		return true;
	}

	size_t Save(const string& filePath)
	{
		wofstream stream;
		stream.open(filePath);
		Check(!stream.is_open(), "File can't be opened");

		stream << m_order << endl;
		for (auto& value : m_chain)
		{
			stream << value.first << " ";
			stream << boost::join(value.second, " ");
			stream << endl;
		}
		stream.flush();
		stream.seekp(ios::beg);
		return stream.tellp();
	}

	bool operator== (const MarkovChainModel& model)
	{
		return m_order == model.m_order && m_chain == model.m_chain;
	}

	bool GetNextWord(const wstring& key, wstring& word)
	{

		/*
		auto it = m_chain.find(key);
		if (it != m_chain.end())
		{
			auto& list = it->second;
			size_t size = distance(list.begin(), list.end());
			size_t pos = size != 0 ? GenerateRandom(0, size - 1) : 0 ;
			auto randomValue = list.begin();
			advance(randomValue, pos);
			word = *randomValue;
			return true;
		}
		return false;
		*/
	}
private:

	uint32_t m_order;
};


#endif // MODEL_H