#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "document.h"


template <typename It>
class ItRange {
public:
	ItRange(It begin, It end)
	: it1(begin)
	, it2(end)
	{
	}

	int64_t GetDistance() const {
		return distance(it1, it2);
	}

	auto begin() const {
		return it1;
	}
	auto end() const {
		return it2;
	}

private:
	It it1;
	It it2;
};

template<typename It>
std::ostream& operator<<(std::ostream& output, const ItRange<It>& range) {
	using namespace std;
	It it = range.begin();
	for (int i = 0; i < static_cast<int>(range.GetDistance()); ++i) {
		Document doc = *it;
		output << "{ document_id = "s << doc.id <<
				 ", relevance = "s << doc.relevance <<
				 ", rating = "s << doc.rating << " }"s;
		++it;
	}
	return output;
}

template<typename It>
class Paginator {
public:

	Paginator() = default;

	Paginator(It begin, It end, std::size_t ps) {
		std::size_t p = 1;
		It start = begin;
		for (It finish = begin; finish != end;) {
			++finish;
			if (p % ps == 0 || p >= static_cast<size_t>(distance(begin, end))) {
				ItRange<It> range(start, finish);
				pages.push_back(range);
				start = finish;
			}
			++p;
		}
	}

	auto begin() const {
		return pages.begin();
	}

	auto end() const {
		return pages.end();
	}


private:
	std::vector<ItRange<It>> pages = {};
};

template<typename Container>
auto Paginate(const Container& container, size_t page_size) {
	return Paginator(begin(container), end(container), page_size);
}
