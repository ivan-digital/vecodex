#include "Segment.h"
#include <algorithm>
#include <iostream>

bool vecodex::SearchResult::operator==(const SearchResult& other) const {
	return id == other.id;
}

bool vecodex::SearchResult::operator<(const SearchResult& other) const {
	return dist < other.dist || (dist == other.dist && id < other.id);
}

bool vecodex::SearchResult::operator>(const SearchResult& other) const {
	return dist > other.dist || (dist == other.dist && id > other.id);
}
