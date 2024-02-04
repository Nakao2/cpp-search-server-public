#pragma once

#include <string>
#include <vector>
#include "document.h"

std::vector<std::string> SplitIntoWords(const std::string& text);

bool HasBannedSymbols(const std::string& text);
