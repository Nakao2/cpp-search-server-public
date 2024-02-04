#include "search_server.h"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>
#include "log_duration.h"

using namespace std;

SearchServer::SearchServer(const string& words) {
	vector<string> words_v = SplitIntoWords(words);
	for (const string& word : words_v) {
		if (HasBannedSymbols(word)) {
			throw invalid_argument("Word '"s + word + "' contains forbidden symbols"s);
		}
		stop_words_.insert(word);
	}
}

void SearchServer::SetStopWords(const string& text) {
	for (const string& word : SplitIntoWords(text)) {
		stop_words_.insert(word);
	}
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status,
		const vector<int>& ratings) {
	if (document_id < 0) {
		throw invalid_argument("id cannot be negative"s);
	}
	if (documents_.count(document_id) != 0) {
		throw invalid_argument("This id already exists"s);
	}
	if (HasBannedSymbols(document)) {
		throw invalid_argument("Forbidden symbols in document text"s);
	}
	const vector<string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	for (const string& word : words) {
		word_to_document_ids_[word].insert(document_id);
		ids_to_word_freqs_[document_id][word] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
	document_ids_.insert(document_id);
}

int SearchServer::GetDocumentCount() const {
	return document_ids_.size();
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus input_status) const {
	return FindTopDocuments(raw_query, [input_status](int document_id, DocumentStatus status, int rating) {
		return status == input_status;
	});
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {

	const Query query = ParseQuery(raw_query);

	vector<string> matched_words;
	for (const string& word : query.plus_words) {
		if (word_to_document_ids_.count(word) == 0) {
			continue;
		}
		if (word_to_document_ids_.at(word).count(document_id)) {
			matched_words.push_back(word);
		}
	}
	for (const string& word : query.minus_words) {
		if (word_to_document_ids_.count(word) == 0) {
			continue;
		}
		if (word_to_document_ids_.at(word).count(document_id)) {
			matched_words.clear();
			break;
		}
	}
	return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const string& word) const {
	return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
	vector<string> words;
	for (const string& word : SplitIntoWords(text)) {
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
	if (ratings.empty()) {
		return 0;
	}
	int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
	return rating_sum / static_cast<int>(ratings.size());
}


SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
	bool is_minus = false;
	// Word shouldn't be empty
	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}
	if (text.empty() || text[0] == '-' || text[text.size() - 1] == '-') {
		throw invalid_argument("Wrong query words format"s);
	}
	if (HasBannedSymbols(text)) {
		throw invalid_argument("Wrong query words format"s);
	}
	return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
	Query query;
	for (const string& word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				query.minus_words.insert(query_word.data);
			} else {
				query.plus_words.insert(query_word.data);
			}
		}
	}
	return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_ids_.at(word).size());
}

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const {
	const static map<string, double> empty_map;
	if (document_ids_.count(document_id) == 0) {
		return empty_map;
	}
	return ids_to_word_freqs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
	if (document_ids_.count(document_id) == 0) {
		return;
	}
	for (const auto& [word, _] : ids_to_word_freqs_.at(document_id)) {
		word_to_document_ids_.at(word).erase(document_id);
	}
	document_ids_.erase(document_id);
	ids_to_word_freqs_.erase(document_id);
	documents_.erase(document_id);
}
