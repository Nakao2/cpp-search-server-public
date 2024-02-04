#include "document.h"
#include <iostream>

using namespace std;

Document::Document() {
	Document::id = 0;
	Document::relevance = 0;
	Document::rating = 0;
}

Document::Document(int i, double rel, int rat) {
	Document::id = i;
	Document::relevance = rel;
	Document::rating = rat;
}

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}