#include "VecodexSegment.h"
#include <algorithm>
#include <iostream>
bool SearchResult::operator==(const SearchResult& other) const {
    return id == other.id;
}

bool SearchResult::operator<(const SearchResult& other) const {
    return dist < other.dist || (dist == other.dist && id < other.id);
}

bool SearchResult::operator>(const SearchResult& other) const {
    return dist > other.dist || (dist == other.dist && id > other.id);
}
