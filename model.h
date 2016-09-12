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

	void CreateModel(vector<wchar_t>& text)
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

	bool Load(const wstring& filePath)
	{
		wifstream stream(filePath);
		m_chain.clear();
		
		wstring line;
		getline(stream, line);
		uint32_t newOrder = stol(line);
		if (newOrder != m_order)
			return false;
		
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
		stream.seekp(ios::end);
		return stream.tellp();
	}

	bool operator== (const MarkovChainModel& model)
	{
		return m_order == model.m_order && m_chain == model.m_chain;
	}

	bool GetNextWord(const wstring& key, wstring& word)
	{
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
	}
private:
	// Generates random between a and b
	int GenerateRandom(int a, int b)
	{
		std::random_device r;
		std::default_random_engine e1(r());
		std::uniform_int_distribution<int> uniform_dist(a, b);
		return uniform_dist(e1);
	}

	uint32_t m_order;
};


#endif // MODEL_H