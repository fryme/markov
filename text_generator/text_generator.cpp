#include <iostream>
#include <vector>
#include "../model.h"

using namespace std;

int main() 
{
	uint32_t k = 10; // number of words to build
	uint32_t n = 2;
	wstring beginSentence(L"when johnny");
	wstring pathToModel(L"C:\\github\\markov\\Debug\\model.txt");
	
	wstring currentSentence;
	wstring resultText;
	resultText += beginSentence;

	uint32_t counter = 0;
	MarkovChainModel model(2);
	model.Load(pathToModel);

	Sentence s(beginSentence, 2);

	while (counter < k)
	{
		wstring nextWord;
		if (model.GetNextWord(s.GetKey(), nextWord))
		{
			s.InsertWord(nextWord);
			resultText += L" ";
			resultText += nextWord;
		}
		else
		{
			break;
		}

		counter++;
	}
	
	wcout << resultText << endl;

	return 0;
}