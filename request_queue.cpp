#include "request_queue.h"
#include <algorithm>

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
	vector<Document> matched_documents = search_server_.FindTopDocuments(raw_query, status);
	if (requests_.size() >= min_in_day_) {
		requests_.pop_front();
	}
	requests_.push_back({!matched_documents.empty()});
	return matched_documents;
}
vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
	vector<Document> matched_documents = search_server_.FindTopDocuments(raw_query);
	if (requests_.size() >= min_in_day_) {
		requests_.pop_front();
	}
	requests_.push_back({!matched_documents.empty()});
	return matched_documents;
}
int RequestQueue::GetNoResultRequests() const {
	return count_if(requests_.begin(), requests_.end(), [] (const RequestQueue::QueryResult& result) {
		return !result.are_results_found;
	});
}
