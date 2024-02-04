#pragma once

#include "search_server.h"
#include <set>
#include <string>
#include <map>

std::set<std::string> MakeSetFromKeysOfMap(const std::map<std::string, double>& m);

void RemoveDuplicates(SearchServer& search_server);