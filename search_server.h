#pragma once
#include <string>
#include <set>
#include <map>
#include <stdexcept>
#include <iostream>
#include "string_processing.h"
#include "document.h"
#include "log_duration.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double VERY_SMALL_NUMBER = 1e-6;

class SearchServer {
public:

	SearchServer() = default;

	explicit SearchServer(const std::string& words);

	template<typename Container>
	explicit SearchServer(const Container& container);

    void SetStopWords(const std::string& text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);

    template <typename Function>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Function function) const;

    int GetDocumentCount() const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus input_status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

    auto begin() const {
        return document_ids_.begin();
    }
    auto end() const {
        return document_ids_.end();
    }

    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string, double>> ids_to_word_freqs_;
    std::map<std::string, std::set<int>> word_to_document_ids_;

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename Function>
    std::vector<Document> FindAllDocuments(const Query& query, Function function) const;
};

template <typename Function>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Function function) const {

	const Query query = ParseQuery(raw_query);
	std::vector<Document> matched_documents = FindAllDocuments(query, function);

	sort(matched_documents.begin(), matched_documents.end(),
			[](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < VERY_SMALL_NUMBER) {
			return lhs.rating > rhs.rating;
		} else {
			return lhs.relevance > rhs.relevance;
		}
	});
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return matched_documents;
}

template <typename Function>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Function function) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_ids_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (int document_id : word_to_document_ids_.at(word)) {
        	DocumentData current_document = documents_.at(document_id);
            const double term_freq = ids_to_word_freqs_.at(document_id).at(word);
        	if (function(document_id, current_document.status, current_document.rating)) {
        		document_to_relevance[document_id] += term_freq * inverse_document_freq;
        	}
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_ids_.count(word) == 0) {
            continue;
        }
        for (int document_id : word_to_document_ids_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

template<typename Container>
SearchServer::SearchServer(const Container& container) {
	using namespace std;
	for (const string& word : container) {
		if (word != ""s) {
			if (HasBannedSymbols(word)) {
				throw invalid_argument("Word '"s + word + "' contains forbidden symbols"s);
			}
			stop_words_.insert(word);
		}
	}
}

