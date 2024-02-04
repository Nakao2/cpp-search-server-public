#include "remove_duplicates.h"

#include "search_server.h"
#include <set>
#include <vector>
#include <string>
#include <map>

using namespace std;

set<string> MakeSetFromKeysOfMap(const map<string, double>& m) {
    set<string> output;
    for (const auto& [word, _] : m) {
        output.insert(word);
    }
    return output;
}

void RemoveDuplicates(SearchServer& search_server) {
    set<int> ids_to_remove;
    map<set<string>, set<int>> word_set_to_ids;
    for (int id : search_server) {
        const auto& word_freqs = search_server.GetWordFrequencies(id);
        const set<string> word_set = MakeSetFromKeysOfMap(word_freqs);
        if (word_set_to_ids[word_set].size() != 0) {
            ids_to_remove.insert(id);
        }
        else {
            word_set_to_ids.at(word_set).insert(id);
        }
    }
    for (int id : ids_to_remove) {
        cout << "Found duplicate document id "s << id << "\n";
        search_server.RemoveDocument(id);
    }
}