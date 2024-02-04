#include "string_processing.h"
#include <iostream>

using namespace std;

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


bool HasBannedSymbols(const string& text) {
	for (char symbol : text) {
		if (symbol <= 31 && symbol >= 0) {
			return true;
		}
	}
	return false;
}
